#include "TemporalPropagator.hh"
#include "TemporalNetworkDefs.hh"
#include "TimepointWrapper.hh"
#include "TemporalNetwork.hh"
#include "TemporalNetworkListener.hh"
#include "ConstraintEngine.hh"
#include "ConstrainedVariable.hh"
#include "Constraint.hh"
#include "Utils.hh"
#include "TemporalConstraints.hh"
#include "Debug.hh"

// @todo: there are cases where we may be able to fail early during the
// mapping from the constraint engine to the temporal network.  In these
// cases we could propagate the temporal netowrk as we're doing the mapping
// and detect inconsistencies at that point.  In cases where domains were
// relaxed in temporal variables we must do the mapping first or we'll run
// the risk of detecting an inconsistency where there isn't one.

namespace EUROPA {

  //#ifndef EUROPA_FAST
#define  publish(message){\
    for(std::set<TemporalNetworkListenerId>::const_iterator lit = m_listeners.begin(); lit != m_listeners.end(); ++lit)\
      (*lit)->message;\
}
//#else
//#define publish(message)
//#endif

  typedef Id<TimepointWrapper> TimepointWrapperId;

  TemporalPropagator::TemporalPropagator(const LabelStr& name, const ConstraintEngineId& constraintEngine)
    : Propagator(name, constraintEngine), m_tnet((new TemporalNetwork())->getId()) {}

  TemporalPropagator::~TemporalPropagator() {
    check_error(Entity::isPurging() || m_wrappedTimepoints.empty());
    cleanup(m_wrappedTimepoints);
    cleanup(m_listeners);
    check_error(m_tnet.isValid());
    delete (TemporalNetwork*) m_tnet;
  }

  void TemporalPropagator::notifyDeleted(const ConstrainedVariableId& tempVar, const TimepointId& tp) {
    check_error(!Entity::isPurging());
    ////// std::cout << "deleting from temporal propagator" << tempVar->getKey() << std::endl;
/*
    for(std::map<int,ConstrainedVariableId>::iterator it = m_changedVariables.begin();
	it != m_changedVariables.end(); ++it) {
      if (!it->second.isValid()) 
	if (it->second.isNoId()) 
	  ////// std::cout << "TP::notifyDeleted m_changedVariables contains a bogus entry." << std::endl;
	else {
	  //  m_changedVariables.erase(it);
	  ////// std::cout << "TP::notifyDeleted m_changedVariables contains a non valid entry." << it->second->getKey() << "," << it->first << std::endl;
	}
      else
	if (it->second.isNoId())
	  //////std::cout << "TP::notifyDeleted m_changedVaraibles contains a valid but noId entry." << std::endl;
    }
*/
    m_changedVariables.erase(tempVar->getKey());
    m_activeVariables.erase(tempVar->getKey());
    EntityId tw = tempVar->getExternalEntity();
    tp->clearExternalEntity(); // Breaks link to original
    m_wrappedTimepoints.erase(tw);
    m_variablesForDeletion.insert(tp);
  }

  void TemporalPropagator::handleConstraintAdded(const ConstraintId& constraint){
    const std::vector<ConstrainedVariableId>& scope = constraint->getScope();
    m_changedConstraints.insert(constraint);
    if (scope.size() == 3) {//Ternary distance constraint
      if (ConstrainedVariableId::convertable(scope[TemporalDistanceConstraint::SRC_VAR_INDEX]))
	buffer(scope[TemporalDistanceConstraint::SRC_VAR_INDEX]);
      if (ConstrainedVariableId::convertable(scope[TemporalDistanceConstraint::DEST_VAR_INDEX]))
	buffer(scope[TemporalDistanceConstraint::DEST_VAR_INDEX]);
    }
    else { // indexes for precedes and concurrent are the same
      if (ConstrainedVariableId::convertable(scope[ConcurrentConstraint::SRC_VAR_INDEX]))
	buffer(scope[ConcurrentConstraint::SRC_VAR_INDEX]);
      if (ConstrainedVariableId::convertable(scope[ConcurrentConstraint::DEST_VAR_INDEX]))
	buffer(scope[ConcurrentConstraint::DEST_VAR_INDEX]);
    }
  }

  void TemporalPropagator::handleConstraintRemoved(const ConstraintId& constraint){
    check_error(!Entity::isPurging());
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
    m_changedConstraints.insert(constraint);
    ConstrainedVariableId var;
    const std::vector<ConstrainedVariableId>& scope = constraint->getScope();
    if (scope.size() == 3) {//Ternary distance constraint
      var = scope[TemporalDistanceConstraint::SRC_VAR_INDEX];
      buffer(var);
      m_activeVariables.insert(std::make_pair<int, ConstrainedVariableId>(var->getKey(),var)); 
      var = scope[TemporalDistanceConstraint::DEST_VAR_INDEX];
      buffer(var);
      m_activeVariables.insert(std::make_pair<int, ConstrainedVariableId>(var->getKey(),var)); 

    }
    else {
      var = scope[ConcurrentConstraint::SRC_VAR_INDEX];
      buffer(var);
      m_activeVariables.insert(std::make_pair<int, ConstrainedVariableId>(var->getKey(),var)); 
      var = scope[ConcurrentConstraint::DEST_VAR_INDEX];
      buffer(var);
      var = scope[ConcurrentConstraint::DEST_VAR_INDEX];
    }
  }

  void TemporalPropagator::handleConstraintDeactivated(const ConstraintId& constraint){
    // Delete constraint from list of pending additions and executions. 
    // It may be there if we have not propagated them yet.
    m_changedConstraints.erase(constraint);

    const std::vector<ConstrainedVariableId>& scope = constraint->getScope();
    for(std::vector<ConstrainedVariableId>::const_iterator it = scope.begin(); it != scope.end(); ++it){
      const ConstrainedVariableId& var = *it;
      check_error(var.isValid());

      if(!var->isActive()){
	m_changedVariables.erase(var->getKey());
	m_activeVariables.erase(var->getKey());
      }
    }
  }

  void TemporalPropagator::handleNotification(const ConstrainedVariableId& variable, 
					      int argIndex, 
					      const ConstraintId& constraint, 
					      const DomainListener::ChangeType& changeType){
    check_error(constraint->isActive());

    if(TemporalDistanceConstraintId::convertable(constraint))
      m_changedConstraints.insert(constraint);

    // Only buffer change for start or end variables
    if(variable != constraint->getScope()[TemporalDistanceConstraint::DISTANCE_VAR_INDEX]) {
      m_changedVariables.insert(std::make_pair<int,ConstrainedVariableId>(variable->getKey(),variable));
    }
  }

  void TemporalPropagator::execute(){
    check_error(!getConstraintEngine()->provenInconsistent());
    check_error(isValidForPropagation());
    //update the tnet
    updateTnet();

    //propagate the tnet
    if (!m_tnet->isConsistent()) {
      //std::cout << " Tnet is inconsistent " << std::endl;
      std::list<TimepointId> results(m_tnet->getInconsistencyReason());
      check_error(!results.empty());
      std::list<TimepointId>::iterator it = results.begin();
      if (*it == m_tnet->getOrigin()) {
	check_error (results.size()>1);
	++it;
      }
      ConstrainedVariableId var = (*it)->getExternalEntity();
      check_error (!var.isNoId());
      Propagator::getCurrentDomain(var).empty();
    }
    else // update the Temp Vars of the CNET.
      updateTempVar();
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
	     "TIMEPOINT " << timepoint->getKey() << " ADDED for variable " << var->getKey());

    m_activeVariables.insert(std::make_pair<int, ConstrainedVariableId>(var->getKey(), var));
    m_changedVariables.insert(std::make_pair<int, ConstrainedVariableId>(var->getKey(), var));

    // Key domain restriction constrain off derived domain values
    TemporalConstraintId c = m_tnet->addTemporalConstraint(m_tnet->getOrigin(), 
							   timepoint,
							   (Time) var->lastDomain().getLowerBound(),  
							   (Time) var->lastDomain().getUpperBound());
    check_error(c.isValid());

    timepoint->setBaseDomainConstraint(c);

    publish(notifyBaseDomainConstraintAdded(var, 
					    c, 
					    (Time) var->baseDomain().getLowerBound(), 
					    (Time) var->baseDomain().getUpperBound()));

    debugMsg("TemporalPropagator:addTimepoint",
	     "Constraint ADDED Base Domain for Variable " << var->getKey() <<  "(" <<  c << ") "
	     << " -[" << var->baseDomain().getLowerBound() << "," << var->baseDomain().getUpperBound() << "]-");
  }

  void TemporalPropagator::addTemporalConstraint(const ConstraintId& constraint) {
    ConstrainedVariableId start;
    ConstrainedVariableId end;
    Time lb=0;
    Time ub=0;

    if (TemporalDistanceConstraintId::convertable(constraint)) {
      start = constraint->getScope()[TemporalDistanceConstraint::SRC_VAR_INDEX];
      end = constraint->getScope()[TemporalDistanceConstraint::DEST_VAR_INDEX];
      ConstrainedVariableId distance = constraint->getScope()[TemporalDistanceConstraint::DISTANCE_VAR_INDEX];

      lb = (Time) distance->lastDomain().getLowerBound();
      ub = (Time) distance->lastDomain().getUpperBound();
    }
    else if (ConcurrentConstraintId::convertable(constraint)) {
      start = constraint->getScope()[ConcurrentConstraint::SRC_VAR_INDEX];
      end = constraint->getScope()[ConcurrentConstraint::DEST_VAR_INDEX];      
      lb = 0;
      ub = 0;
    }
    else if (constraint->getName() == LabelStr("before") || constraint->getName() == LabelStr("precedes")) {
      start = constraint->getScope()[0];
      end = constraint->getScope()[1];      
      lb = 0;
      ub = g_infiniteTime();
    }
    else {
      check_error(ALWAYS_FAILS, "Adding an unknown " + constraint->getName().toString() + " constraint to the temporal propagator.");
    }

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

      m_tnet->removeTemporalConstraint(baseDomainConstraint);

      debugMsg("TemporalPropagator:updateTnet",	"Base Domain Constraint " << baseDomainConstraint->getKey() << " DELETED");

      publish(notifyTimepointDeleted(tp));

      debugMsg("TemporalPropagator:updateTnet", "TIMEPOINT " << tp->getKey() << " DELETED");

      m_tnet->deleteTimepoint(tp);
    }
    m_variablesForDeletion.clear();

    // Process variables that have changed
    for(std::map<int,ConstrainedVariableId>::const_iterator it = m_changedVariables.begin(); it != m_changedVariables.end(); ++it){
      ConstrainedVariableId var = it->second;
      updateTimepoint(var);
    }
    m_changedVariables.clear(); 

    // Process constraints that have changed, or been added
    for(std::set<ConstraintId>::const_iterator it = m_changedConstraints.begin(); it != m_changedConstraints.end(); ++it){
      ConstraintId constraint = *it;
      updateTemporalConstraint(constraint);
    }
    m_changedConstraints.clear();
  }


  /**
   * @brief Updates the ConstrainedEngine variable for each active timepoint.
   */
  void TemporalPropagator::updateTempVar() {
    for(std::map<int,ConstrainedVariableId>::const_iterator it = m_activeVariables.begin(); it != m_activeVariables.end(); ++it){
      ConstrainedVariableId var = it->second;
      check_error(var.isValid());

      const TimepointId& tp = getTimepoint(var);
      check_error(tp.isValid());
      check_error(tp->getExternalEntity() == var, "Ensure the connection between TempVar and Timepointis correct");

      const Time& lb = tp->getLowerBound();
      const Time& ub = tp->getUpperBound();

      check_error(lb <= ub);

      IntervalIntDomain& dom = static_cast<IntervalIntDomain&>(Propagator::getCurrentDomain(var));

      check_error(!dom.isEmpty());
      check_error(dom.isMember(lb) && dom.isMember(ub), 
		  "Updated bounds must be a subset but are not for variable: " + var->getKey());

      double domlb, domub;
      dom.getBounds(domlb,domub);
      if (lb > domlb || ub < domub) {
	dom.intersect(lb, ub);
	check_error(!dom.isEmpty());
      }
    }
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
    Time lb, ub;
    m_tnet->calcDistanceBounds(_first, _second, lb, ub, true);
    return (lb <= 0 && ub >= 0);
  }

  void TemporalPropagator::updateTimepoint(const ConstrainedVariableId& var){
    check_error(var.isValid());
    check_error(!var->getExternalEntity().isNoId());

    const TimepointId& tp = getTimepoint(var);

    check_error(tp.isValid());

    // Instead of getTimepointBounds here we'd like to get cached values
    // from the last computation since the temporal network may be made
    // inconsistent in this mapping process.
    Time lbt=0, ubt=0;
    m_tnet->getLastTimepointBounds(tp, lbt, ubt);
    const TemporalConstraintId& baseDomainConstraint = tp->getBaseDomainConstraint();
    TemporalConstraintId newConstraint = updateConstraint(var, baseDomainConstraint, lbt, ubt);
    if(!newConstraint.isNoId())
      tp->setBaseDomainConstraint(baseDomainConstraint);
  }

  void TemporalPropagator::updateTemporalConstraint(const ConstraintId& constraint){
    // If the consttraint has no corresponding constraint in the tnet, then add it.
    if(constraint->getExternalEntity().isNoId()){
      addTemporalConstraint(constraint);
      return;
    }

    // We should only handle changes for a duration constraint. Note that this may change
    // when we have variable distance constraints in the system. Should generalize the scheme
    // at that time.
    if(TemporalDistanceConstraintId::convertable(constraint)) {
      const ConstrainedVariableId& distance = constraint->getScope()[TemporalDistanceConstraint::DISTANCE_VAR_INDEX];
      check_error(distance->lastDomain().getType() == AbstractDomain::INT_INTERVAL, constraint->getKey() + " is invalid")
      const TemporalConstraintId& tnetConstraint = constraint->getExternalEntity();
      Time lbt=0, ubt=0;
      tnetConstraint->getBounds(lbt, ubt);

      updateConstraint(distance, tnetConstraint, lbt, ubt);
    }
  }

  TemporalConstraintId TemporalPropagator::updateConstraint(const ConstrainedVariableId& var, 
							    const TemporalConstraintId& tnetConstraint, 
							    Time lbt,
							    Time ubt){
    TemporalConstraintId newConstraint;
    const IntervalIntDomain& dom = static_cast<const IntervalIntDomain&>(var->lastDomain());
    double lb =0, ub=0;
    dom.getBounds(lb, ub);

    check_error(lbt <= ubt);
    check_error(lb <= ub);

    if(lb < lbt || ub > ubt) { // Handle relaxation
      // think about whether we can do better here, possibly by changing
      // the condition above.  There are cases
      // where the temporal network has restricted it further so we're just
      // thrashing by removing it and adding the original constraint that
      // was previously restricted by the temporal network.
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
    else if (lb > lbt || ub < ubt) { // Handle restriction. Retain most restricted values
      m_tnet->narrowTemporalConstraint(tnetConstraint, (Time)lb, (Time)ub);
      publish(notifyBoundsRestricted(var, (Time)lb, (Time)ub));

      debugMsg("TemporalPropagator:updateTemporalConstraint",
	       "Bounds of "  << var->getKey() << " Restricted to -[" << lb << "," << ub << "]-");
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

//   const LabelStr& TemporalPropagator::durationConstraintName(){
//     static const LabelStr sl_durationConstraintName("StartEndDurationRelation");
//     return sl_durationConstraintName;
//   }

  bool TemporalPropagator::isValidForPropagation() const {
    //The set of all constraints managed by this propagator must only be one of Precedes, Concurrent or TemporalDistance
    for(std::list<ConstraintId>::const_iterator it = getConstraints().begin(); it != getConstraints().end(); ++it){
      ConstraintId constraint = *it;
      if (! TemporalDistanceConstraintId::convertable(constraint) 
	  && ! PrecedesConstraintId::convertable(constraint) 
	  && ! ConcurrentConstraintId::convertable(constraint))
	return false;
    }

    // The set of active variables should not be empty
    if(m_activeVariables.empty()) {
      debugMsg("TemporalPropagator:isValidForPropagation", "Active variables empty");
      return false;
    }

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
    // we will have already deleteed the TempVar for which this timepoint shadows it.
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
	// Confirm the shadow is linked up coorrectly
	TimepointWrapperId wrapper = var->getExternalEntity();
	TimepointId shadow = wrapper->getTimepoint();
	if(shadow->getExternalEntity() != var) {
	  debugMsg("TemporalPropagator:isValidForPropagation", "Shadow is not linked up correctly ");
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


  void TemporalPropagator::getTemporalNogood
  (const ConstrainedVariableId& useAsOrigin,
   std::vector<ConstrainedVariableId>& fromvars,
   std::vector<ConstrainedVariableId>& tovars,
   std::vector<long>& lengths)
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


} //namespace
