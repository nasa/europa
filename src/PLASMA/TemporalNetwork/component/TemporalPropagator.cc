#include "TemporalPropagator.hh"
#include "TemporalNetworkDefs.hh"
#include "TimepointWrapper.hh"
#include "TemporalNetwork.hh"
#include "TemporalNetworkListener.hh"
#include "ConstraintEngine.hh"
#include "ConstrainedVariable.hh"
#include "IntervalIntDomain.hh"
#include "TokenVariable.hh"
#include "Constraint.hh"
#include "Utils.hh"
#include "Debug.hh"

/**
 * @author Conor McGann & Sailesh Ramakrishnan
 */

// @todo: there are cases where we may be able to fail early during the
// mapping from the constraint engine to the temporal network.  In these
// cases we could propagate the temporal network as we're doing the mapping
// and detect inconsistencies at that point.  In cases where domains were
// relaxed in temporal variables we must do the mapping first or we'll run
// the risk of detecting an inconsistency where there isn't one.

namespace EUROPA {

#ifndef EUROPA_FAST
#define  publish(message){\
    for(std::set<TemporalNetworkListenerId>::const_iterator lit = m_listeners.begin(); lit != m_listeners.end(); ++lit)\
      (*lit)->message;\
}
#else
#define publish(message)
#endif

  typedef Id<TimepointWrapper> TimepointWrapperId;

  TemporalPropagator::TemporalPropagator(const LabelStr& name, const ConstraintEngineId& constraintEngine)
    : Propagator(name, constraintEngine), m_tnet((new TemporalNetwork())->getId()), 
      m_mostRecentRepropagation(1){}

  TemporalPropagator::~TemporalPropagator() {
    discard(false);
  }

  void TemporalPropagator::handleDiscard(){
    check_error(Entity::isPurging() || m_wrappedTimepoints.empty());
    Entity::discardAll(m_wrappedTimepoints);
    check_error(m_tnet.isValid());
    delete (TemporalNetwork*) m_tnet;

    Propagator::handleDiscard();
  }

  void TemporalPropagator::notifyDeleted(const ConstrainedVariableId& tempVar, const TimepointId& tp) {
    check_error(!Entity::isPurging());
    debugMsg("TemporalPropagator:notifyDeleted", "Marking as deleted variable " << tempVar->getKey());
    m_changedVariables.erase(tempVar->getKey());
    m_activeVariables.erase(tempVar->getKey());

    EntityId tw = tempVar->getExternalEntity();
    tp->clearExternalEntity(); // Breaks link to original
    tempVar->clearExternalEntity(); // Break link here too
    m_wrappedTimepoints.erase(tw);
    m_variablesForDeletion.insert(tp);
  }

  void TemporalPropagator::handleConstraintAdded(const ConstraintId& constraint){
    debugMsg("TemporalPropagator:handleConstraintAdded", "Adding constraint " << constraint->toString());
    const std::vector<ConstrainedVariableId>& scope = constraint->getScope();
    m_changedConstraints.insert(constraint);
    if (scope.size() == 3) {//Ternary distance constraint
      buffer(scope[0]);
      buffer(scope[2]);
    }
    else { // indexes for precedes and concurrent are the same
      buffer(scope[0]);
      buffer(scope[1]);
    }
  }

  void TemporalPropagator::handleConstraintRemoved(const ConstraintId& constraint){
    check_error(!Entity::isPurging());
    debugMsg("TemporalPropagator:handleConstraintRemoved", "Marking as removed constraint " << constraint->toString());
    // Delete constraint from list of pending additions and executions. It may be there if we have not propagated them yet.
    m_changedConstraints.erase(constraint);

    // Buffer temporal network constraint for deletion
    if(!constraint->getExternalEntity().isNoId()){
      const TemporalConstraintId& tc = constraint->getExternalEntity();
      tc->clearExternalEntity();
      m_constraintsForDeletion.insert(tc);
      constraint->clearExternalEntity();
    }
  }

  void TemporalPropagator::handleConstraintActivated(const ConstraintId& constraint){
    debugMsg("TemporalPropagator:handleConstraintActivated", "Handling activation of constraint " << constraint->toString());
    m_changedConstraints.insert(constraint);
    ConstrainedVariableId var;
    const std::vector<ConstrainedVariableId>& scope = constraint->getScope();
    if (scope.size() == 3) {//Ternary distance constraint
      var = scope[0];
      buffer(var);
      m_activeVariables.insert(std::make_pair<int, ConstrainedVariableId>(var->getKey(),var)); 
      var = scope[2];
      buffer(var);
      m_activeVariables.insert(std::make_pair<int, ConstrainedVariableId>(var->getKey(),var)); 

    }
    else {
      var = scope[0];
      buffer(var);
      m_activeVariables.insert(std::make_pair<int, ConstrainedVariableId>(var->getKey(),var)); 
      var = scope[1];
      buffer(var);
      m_activeVariables.insert(std::make_pair<int, ConstrainedVariableId>(var->getKey(),var)); 
    }
  }

  void TemporalPropagator::handleConstraintDeactivated(const ConstraintId& constraint){
    // Delete constraint from list of pending additions and executions. 
    // It may be there if we have not propagated them yet.
    debugMsg("TemporalPropagator:handleConstraintDeactivated", 
	     "Handling deactivation of constraint " << constraint->toString());
    m_changedConstraints.erase(constraint);

    // Finally, we deactivate the constraint in the temporal network if one exists
    const TemporalConstraintId& tnetConstraint = constraint->getExternalEntity();
    if (tnetConstraint.isId()){
      tnetConstraint->clearExternalEntity();
      constraint->clearExternalEntity();
      bool markDeleted = (constraint->getViolation() > 0);
      m_tnet->removeTemporalConstraint(tnetConstraint, markDeleted);
    }
  }


  void TemporalPropagator::handleVariableActivated(const ConstrainedVariableId& var){
    // Nothing to do - already addressed by constraint handler
  }

  void TemporalPropagator::handleVariableDeactivated(const ConstrainedVariableId& var){
    debugMsg("TemporalPropagator:handleVariableDeactivated", var->toString());
    const TimepointId& timepoint = getTimepoint(var);
    if(timepoint.isId()){
      timepoint->clearDeletionMarker();
      m_variablesForDeletion.insert(timepoint);
      EntityId tw = var->getExternalEntity();
      tw->discard();
    }

    m_changedVariables.erase(var->getKey());
    m_activeVariables.erase(var->getKey());
  }

  void TemporalPropagator::handleNotification(const ConstrainedVariableId& variable, 
                                              int argIndex, 
                                              const ConstraintId& constraint, 
                                              const DomainListener::ChangeType& changeType){

    debugMsg("TemporalPropagator:handleNotification",
             variable->toString() << " change through " << constraint->getName().toString());

    checkError(constraint->isActive(),
               "Received a change notification for inactive constraint " << constraint->getName().toString() <<
               ". Must be a bug in the ConstraintEngine.");

    m_changedConstraints.insert(constraint);

    // Only buffer variable change for registered active variables
    if(m_activeVariables.find(variable->getKey()) != m_activeVariables.end()){
      debugMsg("TemporalPropagator:handleNotification",
               variable->toString() << " is buffered for update.");
      m_changedVariables.insert(std::make_pair<int,ConstrainedVariableId>(variable->getKey(),variable));
    }
  }

  bool TemporalPropagator::isConsistentWithConstraintNetwork() {
    bool consistent = true;
    for(std::set<EntityId>::const_iterator vit = m_wrappedTimepoints.begin(); 
        vit != m_wrappedTimepoints.end(); ++vit) {
      Id<TimepointWrapper> wrap = static_cast<Id<TimepointWrapper> >(*vit);
      ConstrainedVariableId var = wrap->getTempVar();
      TimepointId tp = wrap->getTimepoint();
      if(!var->lastDomain().isMember((double) tp->getUpperBound()) ||
         !var->lastDomain().isMember((double) tp->getLowerBound())) {
        debugMsg("TemporalPropagator:isConsistentWithConstraintNetwork",
                 "Timepoint " << tp << "[" << tp->getLowerBound() << " " << tp->getUpperBound() 
                 << "] and variable " << var->toString() << " are out of synch.");
        consistent = false;
      }
    }
    return consistent;
  }

  bool TemporalPropagator::isEqualToConstraintNetwork() {
    bool consistent = true;
    for(std::set<EntityId>::const_iterator vit = m_wrappedTimepoints.begin(); 
        vit != m_wrappedTimepoints.end(); ++vit) {
      Id<TimepointWrapper> wrap = static_cast<Id<TimepointWrapper> >(*vit);
      ConstrainedVariableId var = wrap->getTempVar();
      TimepointId tp = wrap->getTimepoint();
      if(var->lastDomain().getUpperBound() != (double) tp->getUpperBound() ||
         var->lastDomain().getLowerBound() != (double) tp->getLowerBound()) {
        debugMsg("TemporalPropagator:isEqualToConstraintNetwork",
                 "Timepoint " << tp << "[" << tp->getLowerBound() << " " << tp->getUpperBound() 
                 << "] and variable " << var->toString() << " are out of synch.");
        consistent = false;
      }
    }
    return consistent;
  }

  ConstraintId getConstraint(ConstrainedVariableId& fromvar,
		                     ConstrainedVariableId& tovar,
                             Time& length)
  {
	ConstraintSet constraints;
	fromvar->constraints(constraints);
	
	ConstraintSet::const_iterator it = constraints.begin();
	for (;it != constraints.end();++it) {
	    ConstraintId c = (*it);
	    if (c->isVariableOf(tovar)) {
	    	// TODO: fromvar and tovar could be together in more than one constraint
	    	// need to make sure it is a temporal constraint and use length to make
	    	// sure it is the right instance
	    	return c;
	    }
	}
	
	return ConstraintId::noId();
  }
  
  void TemporalPropagator::execute(){
    static unsigned int sl_counter(0);
    sl_counter++;

    check_error(!getConstraintEngine()->provenInconsistent());
    check_error(isValidForPropagation());
    //update the tnet
    debugMsg("TemporalPropagator:execute", "Calling updateTnet()");
    updateTnet();

    // If already inconsistent by applying constraints directly, skip the rest
    if(getConstraintEngine()->provenInconsistent() && !getConstraintEngine()->getAllowViolations())
      return;

    //propagate the tnet
    if (!m_tnet->propagate()) {
      debugMsg("TemporalPropagator:execute", "Tnet is inconsistent.");
      
      const std::set<TimepointId>& updatedTimepoints = m_tnet->getUpdatedTimepoints();
      checkError(!updatedTimepoints.empty(), sl_counter);
      TimepointId tp = *(updatedTimepoints.begin());
      ConstrainedVariableId var = tp->getExternalEntity();

      if (getConstraintEngine()->getAllowViolations()) {
          // Map tnet violation to violated constraints
          std::vector<ConstrainedVariableId> fromvars,tovars;
          std::vector<Time> lengths;
          ConstrainedVariableId origin = var;//m_tnet->getOrigin()->getExternalEntity();
          
          getTemporalNogood(origin,fromvars,tovars,lengths);
      
          for (unsigned int i=0;i<fromvars.size();i++) {
        	  std::ostringstream os,os1;
    		  os << (i+1) << " of " << fromvars.size();
    	      debugMsg("TemporalPropagator:violations", "Violated edge:"+os.str());
    	      
    	      if (fromvars[i].isNoId() || tovars[i].isNoId()){
    	    	  debugMsg("TemporalPropagator:violations", "from or to var is noId, skipping");
    	    	  continue;
    	      }
    	       
    		  os1 << fromvars[i]->toString() << " "
    		     << tovars[i]->toString() << " "
    		     << lengths[i];              	  
    	      debugMsg("TemporalPropagator:violations", "Violated edge:"+os1.str());
    	
    	      ConstraintId c = getConstraint(fromvars[i],tovars[i],lengths[i]);
    	      if (!c.isNoId()) {
    	          notifyConstraintViolated(c);
    	          notifyVariableEmptied(fromvars[i]);
    	          notifyVariableEmptied(tovars[i]);
    	          debugMsg("TemporalPropagator:violations", "Violated constraint:"+c->toString());
              }
    	      else {
    	    	  // TODO: why is this possible?
    	          debugMsg("TemporalPropagator:violations", "No constraint found for edge, skipping");    	    	  
    	      }
          }      
      }          
      else {
          check_error (!var.isNoId());
          Propagator::getCurrentDomain(var).empty();
      }
    }
    else {// update the Temp Vars of the CNET.
      debugMsg("TemporalPropagator:execute", "Calling updateTempVar");
      updateTempVar();
    }
  }

  bool TemporalPropagator::updateRequired() const{
    bool fullyPropagated = (m_constraintsForDeletion.empty() &&
                            m_variablesForDeletion.empty() &&
                            m_changedConstraints.empty() &&
                            m_changedVariables.empty());

    return (!fullyPropagated);
  }

  void TemporalPropagator::addTimepoint(const ConstrainedVariableId& var) {
    check_error(var->getExternalEntity().isNoId());
    TimepointId timepoint = m_tnet->addTimepoint();
    EntityId tw = (new TimepointWrapper(getId(), var, timepoint))->getId();

    var->setExternalEntity(tw);
    timepoint->setExternalEntity(var);
    m_wrappedTimepoints.insert(tw);

    publish(notifyTimepointAdded(var, timepoint));

    debugMsg("TemporalPropagator:addTimepoint",
             "TIMEPOINT " << timepoint << " ADDED for variable " << var->getKey());

    m_activeVariables.insert(std::make_pair<int, ConstrainedVariableId>(var->getKey(), var));
    m_changedVariables.insert(std::make_pair<int, ConstrainedVariableId>(var->getKey(), var));

    // Key domain restriction constrain off derived domain values
    TemporalConstraintId c = m_tnet->addTemporalConstraint(m_tnet->getOrigin(), 
                                                           timepoint,
                                                           (Time) var->lastDomain().getLowerBound(),  
                                                           (Time) var->lastDomain().getUpperBound());
    check_error(c.isValid());

    timepoint->setBaseDomainConstraint(c);

    // Note, this is misleading. It is actually a constraint on the derived domain.
    publish(notifyBaseDomainConstraintAdded(var,
                                            c, 
                                            (Time) var->lastDomain().getLowerBound(), 
                                            (Time) var->lastDomain().getUpperBound()));
    
    debugMsg("TemporalPropagator:addTimepoint",
             "Constraint ADDED for Variable " << var->getKey() <<  "(" <<  c << ") "
             << " -[" << var->lastDomain().getLowerBound() << "," << var->lastDomain().getUpperBound() << "]-");

    checkError(!var->lastDomain().areBoundsFinite() || m_tnet->hasEdgeToOrigin(timepoint), 
	       "It should have an edge to the origin, but it doesn't!" << 
	       var->toString() << " and timepoint " << timepoint);
  }

  void TemporalPropagator::addTemporalConstraint(const ConstraintId& constraint) {
    static const LabelStr sl_temporaldistance("temporaldistance");
    static const LabelStr sl_temporalDistance("temporalDistance");
    static const LabelStr sl_concurrent("concurrent");
    static const LabelStr sl_precedes("precedes");
    static const LabelStr sl_before("before");

    checkError(constraint->isActive(), constraint->toString());

    checkError(constraint->getScope().size() == 2 || constraint->getScope().size() == 3,
               "Invalid argument count of " << constraint->getScope().size() <<
               " for constraint " << constraint->getName().toString());

    checkError(constraint->getName() == sl_temporaldistance ||
               constraint->getName() == sl_temporalDistance ||
               constraint->getName() == sl_concurrent ||
               constraint->getName() == sl_precedes ||
               constraint->getName() == sl_before,
               "Invalid constraint name " << constraint->getName().toString() << " for temporal propagation.");

    ConstrainedVariableId start = constraint->getScope()[0];
    ConstrainedVariableId end = constraint->getScope()[1];
    Time lb=0;
    Time ub=0;

    if(constraint->getScope().size() == 3){
      ConstrainedVariableId distance = end;
      end = constraint->getScope()[2];
      lb = (Time) distance->lastDomain().getLowerBound();
      ub = (Time) distance->lastDomain().getUpperBound();
    }
    else if (constraint->getName() != sl_concurrent)
      ub = g_infiniteTime();

    checkError(start->isActive(), start->toString());
    checkError(end->isActive(), end->toString());
    const TimepointId& startTp = getTimepoint(start);
    const TimepointId& endTp = getTimepoint(end);
    check_error(startTp.isValid());
    check_error(endTp.isValid());

    
    TemporalConstraintId c = m_tnet->addTemporalConstraint(startTp, 
                                                           endTp, 
                                                           lb,  
                                                           ub);
    constraint->setExternalEntity(c);
    c->setExternalEntity(constraint);
    publish(notifyConstraintAdded(constraint, c, lb,ub));

    debugMsg("TemporalPropagator:addTemporalConstraint",
             "Constraint ADDED " << constraint->getName().toString() << "(" <<  constraint->getKey() << ") - [" << c << "] " 
             << " --[" << lb << "," << ub << "]--> ");
  }

  void TemporalPropagator::updateTnet() {
    // When updating, if we have deletions
    if(!m_constraintsForDeletion.empty() || !m_variablesForDeletion.empty())
      m_mostRecentRepropagation = getConstraintEngine()->mostRecentRepropagation();

    // Process constraints for deletion
    for(std::set<TemporalConstraintId>::const_iterator it = m_constraintsForDeletion.begin(); it != m_constraintsForDeletion.end(); ++it) {
      TemporalConstraintId constraint = *it;

      publish(notifyConstraintDeleted(constraint->getKey(), constraint));

      debugMsg("TemporalPropagator:updateTnet",	"Constraint " << constraint->getKey() << " DELETED");

      m_tnet->removeTemporalConstraint(constraint);
    }
    m_constraintsForDeletion.clear();
    
    // Process variables for deletion
    for(std::set<TimepointId>::const_iterator it = m_variablesForDeletion.begin(); it != m_variablesForDeletion.end(); ++it) {
      TimepointId tp = *it;
      TemporalConstraintId baseDomainConstraint = tp->getBaseDomainConstraint();
      check_error(baseDomainConstraint.isValid());
      check_error(tp->getExternalEntity().isNoId()); // Should have cleared its connection to the TempVar
      publish(notifyConstraintDeleted(baseDomainConstraint->getKey(), baseDomainConstraint));

      m_tnet->removeTemporalConstraint(baseDomainConstraint, tp->getDeletionMarker());

      debugMsg("TemporalPropagator:updateTnet",	"Base Domain Constraint " << baseDomainConstraint->getKey() << " DELETED");

      publish(notifyTimepointDeleted(tp));

      debugMsg("TemporalPropagator:updateTnet", "TIMEPOINT " << tp->getKey() << " DELETED");

      m_tnet->deleteTimepoint(tp);
    }
    m_variablesForDeletion.clear();

    // Process variables that have changed
    for(std::map<int,ConstrainedVariableId>::const_iterator it = m_changedVariables.begin(); it != m_changedVariables.end(); ++it){
      ConstrainedVariableId var = it->second;

      if(!var->isActive())
	continue;

      debugMsg("TemporalPropagator:updateTnet", "Calling updateTimepoint");
      updateTimepoint(var);
    }
    m_changedVariables.clear(); 

    // Process constraints that have changed, or been added
    for(std::set<ConstraintId>::const_iterator it = m_changedConstraints.begin(); it != m_changedConstraints.end(); ++it){
      ConstraintId constraint = *it;
      if(!constraint->isActive())
	continue;

      debugMsg("TemporalPropagator:updateTnet", "Calling updateTemporalConstraint");
      updateTemporalConstraint(constraint);

      // If the cnet has become inconsistent, which can happen since distance bounds may be restricted as we apply
      // a constraint, then we can skip out.
      if(getConstraintEngine()->provenInconsistent())
	break;
    }

    m_changedConstraints.clear();
  }


  /**
   * @brief Updates the ConstrainedEngine variable for each active timepoint.
   */
  void TemporalPropagator::updateTempVar() {
    debugMsg("TemporalPropagator:updateTempVar", "In updateTempVar");

    std::vector<TokenId> updatedTokens; // Used to push update to duration
    const std::set<TimepointId> updatedTimepoints = m_tnet->getUpdatedTimepoints();
    for(std::set<TimepointId>::const_iterator it = updatedTimepoints.begin(); it != updatedTimepoints.end(); ++it){
      const TimepointId& tp = *it;
      check_error(tp.isValid());
      const Time& lb = tp->getLowerBound();
      const Time& ub = tp->getUpperBound();

      check_error(lb <= ub);

      check_error(tp->getExternalEntity().isValid(), "Ensure the connection between TempVar and Timepoint is correct");
      ConstrainedVariableId var = tp->getExternalEntity();
      if(!var->isActive()){
	handleVariableDeactivated(var);
	continue;
      }

      IntervalIntDomain& dom = static_cast<IntervalIntDomain&>(Propagator::getCurrentDomain(var));

      checkError(!dom.isEmpty(), var->toString());

      checkError(dom.isMember(lb) && dom.isMember(ub), 
                 "Updated bounds [" << lb << " " << ub << "] from timepoint " << tp << " are outside of " 
                 << dom.toString() << " for " << var->toString());

      dom.intersect(lb, ub);

      if(TokenId::convertable(var->parent())){
	TokenId token = var->parent();
	// If we get a hit, then buffer the token for later update to duration (so we only do it once with updated bounds
	if (var == token->start() || var == token->end())
	  updatedTokens.push_back(token);
      }
    }

    // TODO JRB: looks like this call should be skipped since updateTemporalConstraint will always be called before 
    // this and already seems to be doing it 
    // CMG: updateTemporalConstraint is only called for local changes. Here we are applying the propagated effect of the constraint network back to
    // the token duration. The network should be consistent.
    for(std::vector<TokenId>::const_iterator it = updatedTokens.begin(); it != updatedTokens.end(); ++it)
      updateDuration(*it);
    
    m_tnet->resetUpdatedTimepoints();
  }
      
  void TemporalPropagator::updateDuration(const TokenId& token) const{
    // Because we know the context is an update from a temporal network, which has already factored in all of the
    // restrictions from the duration to get here, we can make the following assumptions:
    // 1. The domains are not empty
    // 2. The duration will necessarily be restricted.
    // 3. The start and end bounds are current
    const AbstractDomain& start = token->start()->lastDomain();
    if(!start.areBoundsFinite())
      return;

    const AbstractDomain& end = token->end()->lastDomain();
    if(!end.areBoundsFinite())
      return;

    IntervalIntDomain& duration = static_cast<IntervalIntDomain&>(Propagator::getCurrentDomain(token->duration()));

    // TODO JRB : this check may may fail if we support violations
    check_error(!start.isEmpty() && !end.isEmpty() && !duration.isEmpty(), "Can't update token duration with empty variable. Token:" 
    		+ token->toString() 
    		+ " start:"	+ start.toString()
    		+ " end:" + end.toString()
    		+ " duration:" + duration.toString());

    double maxDuration = end.getUpperBound() - start.getLowerBound();
    double minDuration = end.getLowerBound() - start.getUpperBound();
    
    // TODO JRB: if this causes a violation we need to assign a constraint as the culprit
    duration.intersect(minDuration, maxDuration);
  }

  bool TemporalPropagator::canPrecede(const ConstrainedVariableId& first, const ConstrainedVariableId& second) {
    check_error(!updateRequired());
    const TimepointId& fir = getTimepoint(first);
    const TimepointId& sec = getTimepoint(second);
    check_error(fir.isValid());
    check_error(sec.isValid());


    debugMsg("TemporalPropagator:canPrecede", "determining if  " << first->lastDomain() << " precedes " << second->lastDomain());

    // further propagation in temporal network will only restrict values
    // further, so if we already are in violation, we will continue to be
    // in violation.
    // quick check to see if last time we computed bounds we were in violation
    Time flb, fub;
    m_tnet->getLastTimepointBounds(fir, flb, fub);

    Time slb, sub;
    m_tnet->getLastTimepointBounds(sec, slb, sub);

    if (sub < flb) {
      debugMsg("TemporalPropagator:canPrecede", "second upper bound = " << sub << " < first lower bound " << flb << " returning before calculating distance");
      return false;
    }

    bool result=m_tnet->isDistanceLessThan(fir,sec,0);
    condDebugMsg(result, "TemporalPropagator:canPrecede", " calculated distance between first and second < 0");
    condDebugMsg(!result, "TemporalPropagator:canPrecede", " calculated distance between first and second >= 0");
    return !result;
  }

  bool TemporalPropagator::canFitBetween(const ConstrainedVariableId& start, const ConstrainedVariableId& end,
                                         const ConstrainedVariableId& predend, const ConstrainedVariableId& succstart) {
    check_error(!updateRequired());
    const TimepointId& tstart = getTimepoint(start);
    const TimepointId& tend = getTimepoint(end);
    const TimepointId& pend= getTimepoint(predend);
    const TimepointId& sstart = getTimepoint(succstart);

    check_error(tstart.isValid());
    check_error(tend.isValid());
    check_error(pend.isValid());
    check_error(sstart.isValid());

    bool result = m_tnet->isDistanceLessThan(pend,sstart,1);
    if (result) 
      return false;

    Time slb, sub;
    m_tnet->getLastTimepointBounds(tstart, slb, sub);

    Time elb, eub;
    m_tnet->getLastTimepointBounds(tend, elb, eub);

    Time minDuration = elb-sub;

    if (m_tnet->isDistanceLessThan(pend,sstart,minDuration))
      return false;

    m_tnet->getTimepointBounds(tstart, slb, sub);
    m_tnet->getTimepointBounds(tend, elb, eub);
    minDuration = elb-sub;

    return (!m_tnet->isDistanceLessThan(pend,sstart,minDuration));
  }

  bool TemporalPropagator::canBeConcurrent(const ConstrainedVariableId& first, const ConstrainedVariableId& second) {
    const TimepointId& _first = getTimepoint(first);
    const TimepointId& _second = getTimepoint(second);

    debugMsg("TemporalPropagator:canBeConcurrent", 
	     "determining if  " << first->lastDomain() << " can be concurrent with " << second->lastDomain());

    // If either node does not have a timepoint, then it means that there is no temporal constraint for it. That being the case,
    // the only question is whether the domains have an intersection
    if(_first.isNoId() || _second.isNoId())
      return first->lastDomain().intersects(second->lastDomain());

    checkError( !first->lastDomain().areBoundsFinite() || m_tnet->hasEdgeToOrigin(_first), 
	       "It should have an edge to the origin, but it doesn't!" << 
	       first->toString() << " and timepoint " <<  _first);

    checkError(!second->lastDomain().areBoundsFinite() || m_tnet->hasEdgeToOrigin(_second), 
	       "It should have an edge to the origin, but it doesn't!" << 
	       second->toString() << " and timepoint " <<  _second);

    Time lb, ub;
    m_tnet->calcDistanceBounds(_first, _second, lb, ub, true);

    debugMsg("TemporalPropagator:canBeConcurrent", "calculated bounds " << lb << " ... " << ub);

    bool result = lb <= 0 && ub >= 0;

    // Conditionally output the edge distances in the tnet if the test is negative.
    condDebugMsg(!result, "TemporalPropagator:canBeConcurrent:verbose",
		 "FROM " << _first << " TO " << _second << " WITH ORIGIN " << m_tnet->getOrigin()
		 << std::endl << m_tnet->toString());
 
    return result;
  }

  void TemporalPropagator::updateTimepoint(const ConstrainedVariableId& var){
    static unsigned int sl_counter(0);
    sl_counter++;

    check_error(var.isValid());
    checkError(var->isActive(), var->toString());
    check_error(!var->getExternalEntity().isNoId());

    const TimepointId& tp = getTimepoint(var);

    debugMsg("TemporalPropagator:updateTimepoint", "Updating timepoint " << tp << " to variable " << var->toString());

    check_error(tp.isValid());

    const IntervalIntDomain& timeBounds = static_cast<const IntervalIntDomain&>(var->lastDomain());
    Time lb = (Time) timeBounds.getLowerBound();
    Time ub = (Time) timeBounds.getUpperBound();
    const TemporalConstraintId& baseDomainConstraint = tp->getBaseDomainConstraint();
    TemporalConstraintId newConstraint = updateConstraint(var, baseDomainConstraint, lb, ub);

    if(!newConstraint.isNoId())
      tp->setBaseDomainConstraint(baseDomainConstraint);

    checkError(!var->lastDomain().areBoundsFinite() || m_tnet->hasEdgeToOrigin(tp), 
	       "Counter:" << sl_counter << ". It should have an edge to the origin, but it doesn't!" << 
	       var->toString() << " and timepoint " <<  tp);
  }

  void TemporalPropagator::updateTemporalConstraint(const ConstraintId& constraint){
    debugMsg("TemporalPropagator:updateTemporalConstraint", "In updateTemporalConstraint");
    // If the consttraint has no corresponding constraint in the tnet, then add it.
    if(constraint->getExternalEntity().isNoId()){
      addTemporalConstraint(constraint);
      return;
    }

    // Update for the distance variable
    if(constraint->getScope().size() == 3) { // TODO JRB: this seems brittle, what if we get other temporal constraints with 3 parameters?
      const ConstrainedVariableId& distance = constraint->getScope()[1];

      // In order to avoid the unhappy situation where temporalDistance does not maintain the semantics of addEq
      // we now apply the distance bounds to the distance variable.
      // TODO JRB: this seems to be done already by the updateDuration() method, what's the difference?
      // CMG: The temporal distance constraint can apply to relationships between timepoints in general. The later use of updateDuration
      // applies to update the duration of a token specifically.
      const IntervalIntDomain& sourceDom = constraint->getScope()[0]->lastDomain();
      const IntervalIntDomain& targetDom = constraint->getScope()[2]->lastDomain();

      // Checks for finiteness are to av oid overflow or underflow.
      if(sourceDom.isFinite() && targetDom.isFinite()){
	IntervalIntDomain& distanceDom = static_cast<IntervalIntDomain&>(Propagator::getCurrentDomain(distance));
	Time minDistance = (Time) (targetDom.getLowerBound() - sourceDom.getUpperBound());
	Time maxDistance = (Time) (targetDom.getUpperBound() - sourceDom.getLowerBound());
	// JRB: if this intersect() call causes a violation we need to have the constraint network know the culprit (constraint)
	distance->setCurrentPropagatingConstraint(constraint);
	if(distanceDom.intersect(minDistance, maxDistance) && distanceDom.isEmpty())
	  return;
      }

      checkError(distance->getExternalEntity().isNoId(), 
		 "No support for timepoints being distances. " << distance->toString());

      check_error(distance->lastDomain().isInterval(), constraint->getKey() + " is invalid");
      const TemporalConstraintId& tnetConstraint = constraint->getExternalEntity();
      const IntervalIntDomain& dom = static_cast<const IntervalIntDomain&>(distance->lastDomain());
      Time lb= (Time) dom.getLowerBound();
      Time ub= (Time) dom.getUpperBound();
      debugMsg("TemporalPropagator:updateTemporalConstraint", "Calling updateConstraint");
      updateConstraint(distance, tnetConstraint, lb, ub);
    }
  }

  TemporalConstraintId TemporalPropagator::updateConstraint(const ConstrainedVariableId& var, 
                                                            const TemporalConstraintId& tnetConstraint, 
                                                            Time lb,
                                                            Time ub){
    static unsigned int sl_counter(0);
    sl_counter++;

    if(tnetConstraint.isNoId())
      return m_tnet->addTemporalConstraint(m_tnet->getOrigin(), getTimepoint(var), lb, ub);

    TemporalConstraintId newConstraint;

    // Initialize the bounds from the current constraint
    Time lbt, ubt;
    tnetConstraint->getBounds(lbt, ubt);
    checkError(lbt <= ubt, lbt << ">" << ubt);

    // Note that at this point we may in fact have a case where lbt > ubt in the event of a relaxation.

    checkError(lb <= ub, lb << ">" << ub);

    debugMsg("TemporalPropagator:updateTemporalConstraint", "Update base constraint for Variable " << var->getKey() 
             << " [" << lbt << "," << ubt << "]"
             << " dom-[" << lb << "," << ub << "]-");

    if(lb < lbt || ub > ubt) { // Handle relaxation
      m_mostRecentRepropagation = getConstraintEngine()->mostRecentRepropagation();

      // think about whether we can do better here, possibly by changing
      // the condition above.  There are cases
      // where the temporal network has restricted it further so we're just
      // thrashing by removing it and adding the original constraint that
      // was previously restricted by the temporal network.
      checkError(tnetConstraint.isValid(), tnetConstraint);
      publish(notifyConstraintDeleted(tnetConstraint->getKey(), tnetConstraint));
      debugMsg("TemporalPropagator:updateTemporalConstraint", "Constraint " << tnetConstraint->getKey() << " DELETED");

      // Now switch it out
      TimepointId source, target;
      m_tnet->getConstraintScope(tnetConstraint, source, target); // Pull old timepoints.
      EntityId cnetConstraint = tnetConstraint->getExternalEntity();
      tnetConstraint->clearExternalEntity();
      m_tnet->removeTemporalConstraint(tnetConstraint);
      newConstraint = m_tnet->addTemporalConstraint(source, target, (Time)lb, (Time)ub);
      if(!cnetConstraint.isNoId()){
        newConstraint->setExternalEntity(cnetConstraint);
        cnetConstraint->clearExternalEntity();
        cnetConstraint->setExternalEntity(newConstraint);
        publish(notifyConstraintAdded(cnetConstraint, newConstraint,  (Time)lb, (Time)ub));

        debugMsg("TemporalPropagator:updateTemporalConstraint", 
                 "Constraint " << newConstraint->getKey() << " added for " << cnetConstraint->getKey());
      }
      else {
        check_error(target == getTimepoint(var));
        target->setBaseDomainConstraint(newConstraint);
        publish(notifyBaseDomainConstraintAdded(var, newConstraint,  (Time)lb, (Time)ub));

        debugMsg("TemporalPropagator:updateTemporalConstraint", 
                 "Added Base Domain Constraint for Variable " << var->getKey() <<  "(" <<  newConstraint << ") "
                 << " -[" << lb << "," << ub << "]-");
      }
    } 
    else if (!tnetConstraint->isComplete() || lb > lbt || ub < ubt) { // Handle restriction. Retain most restricted values
      Time newLb = std::max(lb, lbt);
      Time newUb = std::min(ub, ubt);
      Time currentLb = 0; 
      Time currentUb = 0;
      tnetConstraint->getBounds(currentLb, currentUb);
      if(!tnetConstraint->isComplete() || currentUb > newUb || currentLb < newLb ){ 
	newLb = std::max(newLb, currentLb);
	newUb = std::min(newUb, currentUb);
	m_tnet->narrowTemporalConstraint(tnetConstraint, newLb, newUb);
	publish(notifyBoundsRestricted(var, newLb, newUb));
	debugMsg("TemporalPropagator:updateTemporalConstraint",
		 "Bounds of "  << var->getKey() << " Restricted to -[" << newLb << "," << newUb << "]-");
      }
    }

    return newConstraint;
  }


  void TemporalPropagator::buffer(const ConstrainedVariableId& var){
    if(var->getExternalEntity().isNoId())
      addTimepoint(var);
    else
      m_changedVariables.insert(std::make_pair<int,ConstrainedVariableId>(var->getKey(),var));
  }

  void TemporalPropagator::addListener(const TemporalNetworkListenerId& listener) {
    m_listeners.insert(listener);
  }

  bool TemporalPropagator::isValidForPropagation() const {

    // All buffers should only contain valid id's
    if(!allValid(m_activeVariables) ||
       !allValid(m_changedVariables) ||
       !allValid(m_changedConstraints) ||
       !allValid(m_constraintsForDeletion) ||
       !allValid(m_variablesForDeletion) ||
       !allValid(m_listeners)) {
      debugMsg("TemporalPropagator:isValidForPropagation", "buffers have something invalid"); 
      return false;
    }

    // For all buffered timepoints for deletion, none should have any dangling external entities. This is because
    // we will have already deleteed the Constraint for which this Constraint shadows it.
    for(std::set<TemporalConstraintId>::const_iterator it = m_constraintsForDeletion.begin(); it != m_constraintsForDeletion.end(); ++it){
      TemporalConstraintId shadow = *it;
      if(!shadow->getExternalEntity().isNoId()) {
        debugMsg("TemporalPropagator:isValidForPropagation", "Shadow is noid for deleted constraints");
        return false;
      }
    }

    // For all buffered constraints for deletion, none should have any dangling external entities. This is because
    // we will have already deleted the TempVar for which this timepoint shadows it.
    for(std::set<TimepointId>::const_iterator it = m_variablesForDeletion.begin(); it != m_variablesForDeletion.end(); ++it){
      TimepointId timepoint = *it;
      if(!timepoint->getExternalEntity().isNoId()) {
        debugMsg("TemporalPropagator:isValidForPropagation", "Shadow is noid for deleted variables");
        return false;
      }
    }

    // For all buffered vars's, it either has an external entity or it doesn't. No invalid one.
    // Should also ensure that ONLY start and end variables have external entities.
    for(std::map<int,ConstrainedVariableId>::const_iterator it = m_changedVariables.begin(); it != m_changedVariables.end(); ++it){
      const ConstrainedVariableId& var = it->second;
      if(!var->getExternalEntity().isNoId()){ // It must be a start or end variable
        // Confirm the shadow is linked up correctly
        TimepointWrapperId wrapper = var->getExternalEntity();
        TimepointId shadow = wrapper->getTimepoint();
        if(shadow->getExternalEntity() != var) {
          debugMsg("TemporalPropagator:isValidForPropagation", 
		   "Shadow is not linked up correctly for " << var->toString());
          return false;
        }
      }
    }

    // For all bufferec constraints for change, it should have no shadow, or a good shadow. Also, if it has a shadow,
    // we should ensure that it is linked correctly 
    for(std::set<ConstraintId>::const_iterator it = m_changedConstraints.begin(); it != m_changedConstraints.end(); ++it){
      ConstraintId constraint = *it;
      if(!constraint->getExternalEntity().isNoId()){
        EntityId shadow = constraint->getExternalEntity();
        if(shadow->getExternalEntity() != constraint) {
          debugMsg("TemporalPropagator:isValidForPropagation", "Shadow of constraints is not linked up");
          return false;
        }
      }
    }

    // For all buffered constraints, 
    return true;
  }

  const IntervalIntDomain TemporalPropagator::getTemporalDistanceDomain(const ConstrainedVariableId& first, 
                                                                        const ConstrainedVariableId& second, 
                                                                        const bool exact) {
    TimepointId tstart = getTimepoint(first);
    TimepointId tend = getTimepoint(second);
    Time lb, ub;
    m_tnet->calcDistanceBounds(tstart, tend, lb, ub, exact);
    return(IntervalIntDomain(lb,ub));
  }

  void TemporalPropagator::getTemporalDistanceDomains(const ConstrainedVariableId& first, 
                                                      const std::vector<ConstrainedVariableId>& seconds, 
                                                      std::vector<IntervalIntDomain>& domains)
  {
    // More efficient to get several at once. Exact calculation.
    TimepointId tstart = getTimepoint(first);
    std::vector<TimepointId> tends;
    std::vector<Time> lbs;
    std::vector<Time> ubs;
    for (unsigned i=0; i<seconds.size(); i++) {
      TimepointId tend = getTimepoint(seconds[i]);
      tends.push_back(tend);
    }
    m_tnet->calcDistanceBounds(tstart, tends, lbs, ubs);
    checkError((lbs.size() == seconds.size()) && (lbs.size() == ubs.size()),
               "size mismatch in TemporalPropagator getTemporalDistanceDomains");
    for (unsigned i=0; i<seconds.size(); i++) {
      domains.push_back(IntervalIntDomain(lbs[i],ubs[i]));
    }
    return;
  }

  void TemporalPropagator::getMinPerturbTimes(const std::vector<ConstrainedVariableId>& timevars,
                                              const std::vector<Time>& oldreftimes,
                                              std::vector<Time>& newreftimes)
  {
    checkError(oldreftimes.size() == timevars.size(),
               "Size mismatch in TemporalPropagator::getMinPerturbTimes");

    // METHOD: Simulate the propagations that would occur with naive
    // minperturb.  Minimize the need for distance calculations by (1)
    // pulling the lower-bound propagations from prior nodes, (2)
    // calculating the new reftime, and (3) pushing the upper-bound
    // propagations to subsequent nodes (using uppers as a cache).

    newreftimes.clear();
    newreftimes.reserve(timevars.size());
    std::vector<Time> uppers;
    uppers.reserve(timevars.size());
    for (unsigned i=0; i<timevars.size(); i++) {
      TimepointId tp = getTimepoint(timevars[i]);
      uppers.push_back(tp->getUpperBound());  // will be refined by props
      newreftimes.push_back(0);   // Temporary value, will be overwritten
    }

    for (unsigned i=0; i<timevars.size(); i++) {
      TimepointId tp = getTimepoint(timevars[i]);
      m_tnet->dijkstra(tp);  // One distance calc for each node

      // Pull the lb propagations to refine the lb.
      Time lb = tp->getLowerBound();
      for (unsigned j=0; j<i; j++) {
        TimepointId tp1 = getTimepoint(timevars[j]);
        Time distance = m_tnet->getDistance(tp1);
        Time lb1 = newreftimes[j] - distance;
        if (lb1 > lb) lb = lb1;
      }
      // The ub props for j<i have already been pushed to uppers[i].
      Time ub = uppers[i];
      checkError(lb <= ub, "lb>ub in TemporalPropagator::getMinPerturbTimes");

      // Calculate the new reftime
      if (oldreftimes[i] < lb)
        newreftimes[i] = lb;
      else if (oldreftimes[i] > ub)
        newreftimes[i] = ub;
      else
        newreftimes[i] = oldreftimes[i];
      //std::cerr << timevars[i]->parent()->toString() << " "
                //<< timevars[i]->lastDomain().getLowerBound() << " "
                //<< timevars[i]->lastDomain().getUpperBound() << " "
                //<< newreftimes[i] << " " << oldreftimes[i] << "\n";

      // Push the ub props for j>i to uppers.
      for (unsigned j=i+1; j<timevars.size(); j++) {
        TimepointId tp1 = getTimepoint(timevars[j]);
        Time distance = m_tnet->getDistance(tp1);
        Time ub1 = newreftimes[i] + distance;
        if (ub1 < uppers[j]) uppers[j] = ub1;
      }
    }
  }

  unsigned int TemporalPropagator::mostRecentRepropagation() const{
    checkError(getConstraintEngine()->mostRecentRepropagation() >= m_mostRecentRepropagation,
               "Cannot imagine how it could be more recent since we should always have a " <<
	       " stricter criteria for capturing the need for a reprop. " <<  
               getConstraintEngine()->mostRecentRepropagation() << " < " <<  m_mostRecentRepropagation);

    return m_mostRecentRepropagation;
  }

  void TemporalPropagator::getTemporalNogood
  (const ConstrainedVariableId& useAsOrigin,
   std::vector<ConstrainedVariableId>& fromvars,
   std::vector<ConstrainedVariableId>& tovars,
   std::vector<Time>& lengths)//std::vector<int>& lengths)//std::vector<long>& lengths)
  {
    std::list<DedgeId> edgeNogoodList = m_tnet->getEdgeNogoodList();
    TimepointId origin = m_tnet->getOrigin();
    ConstrainedVariableId originvar(useAsOrigin);
    for (std::list<DedgeId>::const_iterator it = edgeNogoodList.begin();
         it != edgeNogoodList.end(); it++) {
      DedgeId edge = *it;
      TimepointId from = (TimepointId) edge->from;
      TimepointId to = (TimepointId) edge->to;
      Time length = edge->length;
      ConstrainedVariableId fromvar,tovar;
      if (from == origin)
        fromvar = originvar;
      else
        fromvar = from->getExternalEntity();
      if (to == origin)
        tovar = originvar;
      else
        tovar = to->getExternalEntity();
      fromvars.push_back(fromvar);
      tovars.push_back(tovar);
      lengths.push_back(length);
    }
  }

  TemporalConstraintId TemporalPropagator::addSpecificationConstraint(const TemporalConstraintId& tc, const TimepointId& tp, 
                                                                      const Time lb, const Time ub) {
    if(tc.isNoId()) 
      return m_tnet->addTemporalConstraint(m_tnet->getOrigin(), tp, lb, ub, false);
    m_tnet->narrowTemporalConstraint(tc, lb, ub);
    return tc;
  }

} //namespace
