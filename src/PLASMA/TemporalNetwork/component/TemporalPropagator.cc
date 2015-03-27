#include "TemporalPropagator.hh"
#include "TemporalNetworkDefs.hh"
#include "TemporalNetwork.hh"
#include "TemporalNetworkListener.hh"
#include "ConstraintEngine.hh"
#include "ConstrainedVariable.hh"
#include "Domains.hh"
#include "TokenVariable.hh"
#include "Constraint.hh"
#include "Utils.hh"
#include "Debug.hh"

#include <boost/cast.hpp>
/**
 * @author Conor McGann & Sailesh Ramakrishnan
 */

// @todo: there are cases where we may be able to fail early during the
// mapping from the constraint engine to the temporal network.  In these
// cases we could propagate the temporal network as we're doing the mapping
// and detect inconsistencies at that point.  In cases where domains were
// relaxed in temporal variables we must do the mapping first or we'll run
// the risk of detecting an inconsistency where there isn't one.

// TODO: put more work into consistency and error checking with the constraint engine.
// specifically, adding/removing variables and constraints.  Propagators have no
// removeVariable methods right now.

namespace EUROPA {

#ifndef EUROPA_FAST
#define  publish(message){\
    for(std::set<TemporalNetworkListenerId>::const_iterator lit = m_listeners.begin(); lit != m_listeners.end(); ++lit)\
      (*lit)->message;\
}
#else
#define publish(message)
#endif

TemporalPropagator::TemporalPropagator(const std::string& name, 
                                       const ConstraintEngineId constraintEngine)
    : Propagator(name, constraintEngine), m_tnet((new TemporalNetwork())->getId()),
      m_activeVariables(), m_changedVariables(), m_changedConstraints(),
      m_constraintsForDeletion(), m_variablesForDeletion(),
      m_listeners(), m_mostRecentRepropagation(1){}

  TemporalPropagator::~TemporalPropagator() {
    discard(false);
  }

  void TemporalPropagator::handleDiscard(){
    check_error(Entity::isPurging());
    check_error(m_tnet);
    delete static_cast<TemporalNetwork*>(m_tnet);

    Propagator::handleDiscard();
  }

  void TemporalPropagator::notifyDeleted(const ConstrainedVariableId tempVar, const TimepointId tp) {
    check_error(!Entity::isPurging());
    debugMsg("TemporalPropagator:notifyDeleted", "Marking as deleted variable " << tempVar->getKey());
    m_changedVariables.erase(tempVar->getKey());
    m_activeVariables.erase(tempVar->getKey());

    m_variablesForDeletion.insert(tp);
    unmap(tempVar);
  }

  void TemporalPropagator::handleConstraintAdded(const ConstraintId constraint){
    debugMsg("TemporalPropagator:handleConstraintAdded", "Adding constraint " << constraint->toString());
    const std::vector<ConstrainedVariableId>& scope = constraint->getScope();
    m_changedConstraints.insert(constraint);
    if (scope.size() == 3) {//Ternary distance constraint
      buffer(scope[0]);
      buffer(scope[2]);
      incrementRefCount(scope[0]);
      incrementRefCount(scope[2]);
    }
    else { // indexes for precedes and concurrent are the same
      buffer(scope[0]);
      buffer(scope[1]);
      incrementRefCount(scope[0]);
      incrementRefCount(scope[1]);
    }
  }

  void TemporalPropagator::handleConstraintRemoved(const ConstraintId constraint){
    check_error(!Entity::isPurging());
    debugMsg("TemporalPropagator:handleConstraintRemoved", "Marking as removed constraint " << constraint->toString());
    // Delete constraint from list of pending additions and executions. It may be there if we have not propagated them yet.
    m_changedConstraints.erase(constraint);

    // Buffer temporal network constraint for deletion
    std::map<ConstraintId, TemporalConstraintId>::const_iterator it =
        m_constrToTempConstr.find(constraint);
    if(it != m_constrToTempConstr.end()) {
      const TemporalConstraintId tc = it->second; 
      m_constraintsForDeletion.insert(tc);
      unmap(constraint);
    }
    const std::vector<ConstrainedVariableId>& scope = constraint->getScope();
    if(scope.size() == 3) {
      decrementRefCount(scope[0]);
      decrementRefCount(scope[2]);
    }
    else {
      decrementRefCount(scope[0]);
      decrementRefCount(scope[1]);
    }
  }

  void TemporalPropagator::handleConstraintActivated(const ConstraintId constraint){
    debugMsg("TemporalPropagator:handleConstraintActivated", "Handling activation of constraint " << constraint->toString());
    m_changedConstraints.insert(constraint);
    ConstrainedVariableId var;
    const std::vector<ConstrainedVariableId>& scope = constraint->getScope();
    if (scope.size() == 3) {//Ternary distance constraint
      var = scope[0];
      buffer(var);
      m_activeVariables.insert(std::make_pair(var->getKey(),var));
      var = scope[2];
      buffer(var);
      m_activeVariables.insert(std::make_pair(var->getKey(),var));

    }
    else {
      var = scope[0];
      buffer(var);
      m_activeVariables.insert(std::make_pair(var->getKey(),var));
      var = scope[1];
      buffer(var);
      m_activeVariables.insert(std::make_pair(var->getKey(),var));
    }
  }

void TemporalPropagator::handleConstraintDeactivated(const ConstraintId constraint){
  // Delete constraint from list of pending additions and executions.
  // It may be there if we have not propagated them yet.
  debugMsg("TemporalPropagator:handleConstraintDeactivated",
           "Handling deactivation of constraint " << constraint->toString());
  m_changedConstraints.erase(constraint);

  // Finally, we deactivate the constraint in the temporal network if one exists
  std::map<ConstraintId, TemporalConstraintId>::const_iterator it =
      m_constrToTempConstr.find(constraint);
  if(it != m_constrToTempConstr.end()) {
    const TemporalConstraintId tnetConstraint = it->second;

    unmap(constraint);

    // if it's a violated constraint, mark deleted to force propagation in the tnet.
    bool markDeleted = (constraint->getViolation() > 0);
    m_tnet->removeTemporalConstraint(tnetConstraint, markDeleted);
    debugMsg("TemporalPropagator:handleConstraintDeactivated",
             "removed tnet constraint (markedDeleted=" << markDeleted<<
             ") for cnet constraint " << constraint->toString());
  }
  else
    debugMsg("TemporalPropagator:handleConstraintDeactivated","deactivation ignored. no tnet constraint found for cnet constraint " << constraint->toString());
}


  void TemporalPropagator::handleVariableActivated(const ConstrainedVariableId ){
    // Nothing to do - already addressed by constraint handler
  }

void TemporalPropagator::handleVariableRemoved(const ConstrainedVariableId var) {
  debugMsg("TemporalPropagator:handleVariableRemoved", var->toString());
  notifyDeleted(var, getTimepoint(var));
}

  void TemporalPropagator::handleVariableDeactivated(const ConstrainedVariableId var){
    debugMsg("TemporalPropagator:handleVariableDeactivated", var->toString());
    const TimepointId timepoint = getTimepoint(var);
    if(timepoint){
      timepoint->clearDeletionMarker();
      m_variablesForDeletion.insert(timepoint);
      unmap(var);
    }

    m_changedVariables.erase(var->getKey());
    m_activeVariables.erase(var->getKey());
  }

  void TemporalPropagator::handleNotification(const ConstrainedVariableId variable,
                                              unsigned int ,
                                              const ConstraintId constraint,
                                              const DomainListener::ChangeType& ){

    debugMsg("TemporalPropagator:handleNotification",
             variable->toLongString() << " change through " << constraint->getKey() << "-" << constraint->getName());

    checkError(constraint->isActive(),
               "Received a change notification for inactive constraint " << constraint->getName() <<
               ". Must be a bug in the ConstraintEngine.");

    m_changedConstraints.insert(constraint);

    // Only buffer variable change for registered active variables
    if(m_activeVariables.find(variable->getKey()) != m_activeVariables.end()){
      debugMsg("TemporalPropagator:handleNotification",
               variable->toLongString() << " is buffered for update.");
      m_changedVariables.insert(std::make_pair(variable->getKey(),variable));
    }
  }

bool TemporalPropagator::isConsistentWithConstraintNetwork() {
  bool consistent = true;
  for(std::map<ConstrainedVariableId, TimepointId>::const_iterator it = m_varToTimepoint.begin();
      it != m_varToTimepoint.end(); ++it) {
    ConstrainedVariableId var = it->first;
    TimepointId tp = it->second;
    if(!var->lastDomain().isMember(tp->getUpperBound()) ||
       !var->lastDomain().isMember(tp->getLowerBound())) {
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
  for(std::map<ConstrainedVariableId, TimepointId>::const_iterator it = m_varToTimepoint.begin();
      it != m_varToTimepoint.end(); ++it) {
    ConstrainedVariableId var = it->first;
    TimepointId tp = it->second;
    if(var->lastDomain().getUpperBound() != tp->getUpperBound() ||
       var->lastDomain().getLowerBound() != tp->getLowerBound()) {
      debugMsg("TemporalPropagator:isEqualToConstraintNetwork",
               "Timepoint " << tp << "[" << tp->getLowerBound() << " " << tp->getUpperBound()
               << "] and variable " << var->toString() << " are out of synch.");
      consistent = false;
    }
  }
  return consistent;
}

namespace {
ConstraintId getConstraint(ConstrainedVariableId fromvar,
                           ConstrainedVariableId tovar,
                           Time& ) {
  ConstraintSet fromConstraints;
  fromvar->constraints(fromConstraints);

  ConstraintSet::const_iterator it = fromConstraints.begin();
  for (;it != fromConstraints.end();++it) {
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
}

  /*
   * The Temporal Propagator has 4 phases :
   * 1- updateTnet() moves all the pending changes (CRUD operations on constraints and vars) 
   *    from the cnet to the tnet
   *    the tnet performs incremental propagation on whatever it can while this happens
   * 2- m_tnet->propagate() performs any inference that couldn't be done incrementally while updateTnet() happened
   * 3- If there are constraint violations, update the cnet with the new constraint violation info
   * 4- updateCnet() moves all the newly inferred bounds from the tnet to the cnet
   */
  void TemporalPropagator::execute()
  {
    check_error(!getConstraintEngine()->provenInconsistent());
    check_error(isValidForPropagation());
    debugMsg("TemporalPropagator:execute", "Temporal propagator started");

    debugMsg("TemporalPropagator:execute", "Updating tnet");
    updateTnet();

    // If already inconsistent by applying constraints directly, skip the rest
    if (getConstraintEngine()->provenInconsistent() &&
           !getConstraintEngine()->getAllowViolations())
      return;

    debugMsg("TemporalPropagator:execute", "Propagating tnet");
    bool tnetConsistent=m_tnet->propagate();

    if (!tnetConsistent) {
      debugMsg("TemporalPropagator:execute", "Handling violations");
      handleViolations();
    }
    else {
      debugMsg("TemporalPropagator:execute", "Updating cnet");
      updateCnet();
    }

    debugMsg("TemporalPropagator:execute", "Temporal propagator done");
  }

  void TemporalPropagator::handleViolations()
  {
      const std::set<TimepointId>& updatedTimepoints = m_tnet->getUpdatedTimepoints();
      checkError(!updatedTimepoints.empty(), "updated timepoints are expected if tnet is not consistent");
      std::map<TimepointId, ConstrainedVariableId>::const_iterator it =
          m_timepointToVar.find(*(updatedTimepoints.begin()));
      check_error(it != m_timepointToVar.end());
      ConstrainedVariableId var = it->second;
      
      if (getConstraintEngine()->getAllowViolations())
          collectViolations(var);
      else
          Propagator::getCurrentDomain(var).empty();
  }

  void TemporalPropagator::collectViolations(ConstrainedVariableId var)
  {
      // Map tnet violation to violated constraints
      std::vector<ConstrainedVariableId> fromvars,tovars;
      std::vector<Time> lengths;
      ConstrainedVariableId origin = var;

      getTemporalNogood(origin,fromvars,tovars,lengths);

      for (unsigned int i=0;i<fromvars.size();i++) {
          std::ostringstream os;
          os << (i+1) << " of " << fromvars.size();
          debugMsg("TemporalPropagator:violations", "Violated edge:"+os.str());

          if (fromvars[i].isNoId() || tovars[i].isNoId()) {
              // TODO: why is this possible?
              debugMsg("TemporalPropagator:violations", "fromVar or toVar is noId, skipping");
              continue;
          }

          std::ostringstream os1;
          os1 << "{" << fromvars[i]->toLongString() << "} {"
          << tovars[i]->toLongString() << "} {"
          << lengths[i] << "}";
          debugMsg("TemporalPropagator:violations", "Violated edge:"+os1.str());

          ConstraintId c = getConstraint(fromvars[i],tovars[i],lengths[i]);
          if (!c.isNoId()) {
              debugMsg("TemporalPropagator:violations", "Dealing with violated constraint:"+c->toString());
              notifyConstraintViolated(c);
              notifyVariableEmptied(fromvars[i]);
              notifyVariableEmptied(tovars[i]);
          }
          else {
              // TODO: why is this possible? what edges are introduced in the temporal graph that don't correspond to cnet constraints?
            //The "base domain constraint" edges, representing any unconstrained restrictions
              debugMsg("TemporalPropagator:violations", "No constraint found for edge, skipping");
          }
      }
  }

  bool TemporalPropagator::updateRequired() const{
    bool fullyPropagated = (m_constraintsForDeletion.empty() &&
                            m_variablesForDeletion.empty() &&
                            m_changedConstraints.empty() &&
                            m_changedVariables.empty()
                            && !m_tnet->updateRequired());

    return (!fullyPropagated);
  }

  void TemporalPropagator::addTimepoint(const ConstrainedVariableId var) {
    check_error(m_varToTimepoint.find(var) == m_varToTimepoint.end());
    
    TimepointId timepoint = m_tnet->addTimepoint();
    mapVariable(var, timepoint);
    
    debugMsg("TemporalPropagator:addTimepoint",
             "External: " << var->getKey() << " -> " << timepoint);

    publish(notifyTimepointAdded(var, timepoint));

    debugMsg("TemporalPropagator:addTimepoint",
             "TIMEPOINT " << timepoint << " ADDED for variable " << var->getKey());

    m_activeVariables.insert(std::make_pair(var->getKey(), var));
    m_changedVariables.insert(std::make_pair(var->getKey(), var));

    // Key domain restriction constrain off derived domain values
    TemporalConstraintId c = m_tnet->addTemporalConstraint(m_tnet->getOrigin(),
                                                           timepoint,
                                                           cast_int(var->lastDomain().getLowerBound()),
                                                           cast_int(var->lastDomain().getUpperBound()));
    check_error(c);

    debugMsg("TemporalPropagator:addTimepoint",
             "Setting base domain constraint for " << var->getKey() << " to " << c <<
             " [" << c->getLowerBound() << " " << c->getUpperBound() << "]");
    timepoint->setBaseDomainConstraint(c);

    // Note, this is misleading. It is actually a constraint on the derived domain.
    publish(notifyBaseDomainConstraintAdded(var,
                                            c,
                                            cast_int(var->lastDomain().getLowerBound()),
                                            cast_int(var->lastDomain().getUpperBound())));

    debugMsg("TemporalPropagator:addTimepoint",
             "Constraint ADDED for Variable " << var->getKey() <<  "(" <<  c << ") "
             << " -[" << var->lastDomain().getLowerBound() << "," << var->lastDomain().getUpperBound() << "]-");

    checkError(!var->lastDomain().areBoundsFinite() || m_tnet->hasEdgeToOrigin(timepoint),
	       "It should have an edge to the origin, but it doesn't!" <<
	       var->toString() << " and timepoint " << timepoint);
  }

  void TemporalPropagator::addTemporalConstraint(const ConstraintId constraint) {
    static const std::string sl_temporaldistance("temporaldistance");
    static const std::string sl_temporalDistance("temporalDistance");
    static const std::string sl_concurrent("concurrent");
    static const std::string sl_precedes("precedes");
    static const std::string sl_strictlyPrecedes("strictlyPrecedes");
    static const std::string sl_before("before");

    checkError(constraint->isActive(), constraint->toString());

    checkError(constraint->getScope().size() == 2 || constraint->getScope().size() == 3,
               "Invalid argument count of " << constraint->getScope().size() <<
               " for constraint " << constraint->getName());

    checkError(constraint->getName() == sl_temporaldistance ||
               constraint->getName() == sl_temporalDistance ||
               constraint->getName() == sl_concurrent ||
               constraint->getName() == sl_precedes ||
               constraint->getName() == sl_strictlyPrecedes ||
               constraint->getName() == sl_before,
               "Invalid constraint name " << constraint->getName() << " for temporal propagation.");

    ConstrainedVariableId start = constraint->getScope()[0];
    ConstrainedVariableId end = constraint->getScope()[1];
    Time lb=0;
    Time ub=0;

    if(constraint->getScope().size() == 3){
      ConstrainedVariableId distance = end;
      end = constraint->getScope()[2];
      lb = cast_int(distance->lastDomain().getLowerBound());
      ub = cast_int(distance->lastDomain().getUpperBound());
    }
    else if (constraint->getName() != sl_concurrent) {
      ub = cast_int(PLUS_INFINITY);
      if(constraint->getName() == sl_strictlyPrecedes)
        lb = 1;
    }

    checkError(start->isActive(), start->toString());
    checkError(end->isActive(), end->toString());
    const TimepointId startTp = getTimepoint(start);
    const TimepointId endTp = getTimepoint(end);
    check_error(startTp);
    check_error(endTp);


    TemporalConstraintId c = m_tnet->addTemporalConstraint(startTp,
                                                           endTp,
                                                           lb,
                                                           ub);

    mapConstraint(constraint, c);
    debugMsg("TemporalPropagator:addTemporalConstraint",
             "External: " << constraint->getKey() << " -> " << c);

    publish(notifyConstraintAdded(constraint, c, lb,ub));

    debugMsg("TemporalPropagator:addTemporalConstraint",
             "ADDED-Constraint " << constraint->getName() << "(" <<  constraint->getKey() << ") - [" << c << "] "
             << " --[" << lb << "," << ub << "]--> ");
  }

void TemporalPropagator::processConstraintDeletions() {
  debugMsg("TemporalPropagator:updateTnet", "Processing constraints for deletion... ");
  for( TemporalConstraintsSet::const_iterator it = m_constraintsForDeletion.begin(); 
       it != m_constraintsForDeletion.end(); 
       ++it) {
    TemporalConstraintId constraint = *it;
    publish(notifyConstraintDeleted(constraint->getKey(), constraint));
    debugMsg("TemporalPropagator:updateTnet",	"DELETED-Constraint " << constraint->getKey());
    m_tnet->removeTemporalConstraint(constraint);
  }
  m_constraintsForDeletion.clear();
  debugMsg("TemporalPropagator:updateTnet", "DONE Processing constraints for deletion... ");
}

void TemporalPropagator::processVariableDeletions() {
  debugMsg("TemporalPropagator:updateTnet", "Processing variables for deletion... ");
  for(std::set<TimepointId>::const_iterator it = m_variablesForDeletion.begin();
      it != m_variablesForDeletion.end(); ++it) {
    TimepointId tp = *it;
    TemporalConstraintId baseDomainConstraint = tp->getBaseDomainConstraint();
    check_error(baseDomainConstraint);
    check_error(m_timepointToVar.find(tp) == m_timepointToVar.end()); // Should have cleared its connection to the TempVar
    publish(notifyConstraintDeleted(baseDomainConstraint->getKey(), baseDomainConstraint));

    m_tnet->removeTemporalConstraint(baseDomainConstraint, tp->getDeletionMarker());
    debugMsg("TemporalPropagator:updateTnet",
             "DELETED-Constraint (Base Domain) " << baseDomainConstraint->getKey());
    publish(notifyTimepointDeleted(tp));
    debugMsg("TemporalPropagator:updateTnet", "DELETED-TIMEPOINT " << tp->getKey());

    m_tnet->deleteTimepoint(tp);
  }
  m_variablesForDeletion.clear();
  debugMsg("TemporalPropagator:updateTnet", "DONE Processing variables for deletion... ");
}

void TemporalPropagator::processVariableChanges() {
  // Process all relaxations first, so that we don't stump on tnet propagation later
  debugMsg("TemporalPropagator:updateTnet", "Processing changed variables... ");
  for(std::map<eint,ConstrainedVariableId>::const_iterator it = m_changedVariables.begin(); it != m_changedVariables.end(); ++it){
    ConstrainedVariableId var = it->second;
    if(!var->isActive())
      continue;
    if (wasRelaxed(var))
      updateTimepoint(var);
  }
  debugMsg("TemporalPropagator:updateTnet", "DONE Processing relaxed variables... ");

  // Now take care of other changes
  for(std::map<eint,ConstrainedVariableId>::const_iterator it = m_changedVariables.begin(); it != m_changedVariables.end(); ++it){
    ConstrainedVariableId var = it->second;
    if(!var->isActive())
      continue;
    if (!wasRelaxed(var))
      updateTimepoint(var);
  }
  m_changedVariables.clear();
  debugMsg("TemporalPropagator:updateTnet", "DONE Processing changed variables... ");
}

void TemporalPropagator::processConstraintChanges() {
  debugMsg("TemporalPropagator:updateTnet", "Processing changed constraints... ");
  for( ConstraintsSet::const_iterator it = m_changedConstraints.begin(); it != m_changedConstraints.end(); ++it){
    ConstraintId constraint = *it;
    if(!constraint->isActive())
      continue;

    updateTemporalConstraint(constraint);

    // If the cnet has become inconsistent, which can happen since distance bounds may be restricted as we apply
    // a constraint, then we can skip out.
    if(getConstraintEngine()->provenInconsistent() && !getConstraintEngine()->getAllowViolations())
      break;
  }
  m_changedConstraints.clear();
  debugMsg("TemporalPropagator:updateTnet", "DONE Processing changed constraints... ");

}

  void TemporalPropagator::updateTnet()
  {
      // When updating, if we have deletions
      if(!m_constraintsForDeletion.empty() || !m_variablesForDeletion.empty())
          m_mostRecentRepropagation = getConstraintEngine()->mostRecentRepropagation();

      // Process constraints for deletion
      processConstraintDeletions();

      // Process variables for deletion
      processVariableDeletions();

      // Process variables that have changed
      processVariableChanges();

      // Process constraints that have changed, or been added
      processConstraintChanges();
  }


  /**
   * @brief Updates the cnet with the latest values in the tnet
   */
void TemporalPropagator::updateCnet() {
  debugMsg("TemporalPropagator:updateCnet", "In updateCnet");

  std::vector<TokenId> updatedTokens; // Used to push update to duration
  const std::set<TimepointId> updatedTimepoints = m_tnet->getUpdatedTimepoints();
  for(std::set<TimepointId>::const_iterator it = updatedTimepoints.begin();
      it != updatedTimepoints.end(); ++it){
    const TimepointId tp = *it;
    check_error(tp);
    const Time& lb = tp->getLowerBound();
    const Time& ub = tp->getUpperBound();

    check_error(lb <= ub);

    std::map<TimepointId, ConstrainedVariableId>::const_iterator mIt = m_timepointToVar.find(tp);
    check_error(mIt != m_timepointToVar.end(),
                "Ensure the connection between TempVar and Timepoint is correct");
    ConstrainedVariableId var = mIt->second;
    if(!var->isActive()){
      handleVariableDeactivated(var);
      continue;
    }

    IntervalIntDomain& dom = static_cast<IntervalIntDomain&>(Propagator::getCurrentDomain(var));

    checkError(!dom.isEmpty(), var->toString());

    checkError(dom.isMember(lb) && dom.isMember(ub),
               "Updated bounds [" << lb << " " << ub << "] from timepoint " << tp << " are outside of "
               << dom.toString() << " for " << var->toString());

    // PHM Support for reftime calculations
    if (m_tnet->getReferenceTimepoint()) {
      bool changed = tp->updatePrevReftime();
      if (changed) {
        DomainListenerId listener = dom.getListener();
        listener->notifyChange(DomainListener::REFTIME_CHANGED);
      }
    }

    if (lb == dom.getLowerBound() && ub == dom.getUpperBound())
      continue;  // PHM 06/06/11 This might often be the case

    dom.intersect(mapToExternalInfinity(lb), mapToExternalInfinity(ub));

    if(TokenId::convertable(var->parent())){
      TokenId token = var->parent();
      // If we get a hit, then buffer the token for later update to duration (so we only do it once with updated bounds
      if (var == token->start() || var == token->end())
        updatedTokens.push_back(token);
    }
  }

  for(std::vector<TokenId>::const_iterator it = updatedTokens.begin(); it != updatedTokens.end(); ++it)
    updateCnetDuration(*it);

  m_tnet->resetUpdatedTimepoints();
}

  /*
   * This is necessary because the tnet keeps durations on the edges of the temporal
   * graph and therefore doesn't update them.
   */
  void TemporalPropagator::updateCnetDuration(const TokenId token) const{
    // Because we know the context is an update from a temporal network, which has already factored in all of the
    // restrictions from the duration to get here, we can make the following assumptions:
    // 1. The domains are not empty
    // 2. The duration will necessarily be restricted.
    // 3. The start and end bounds are current
    const Domain& start = token->start()->lastDomain();
    if(!start.areBoundsFinite())
      return;

    const Domain& end = token->end()->lastDomain();
    if(!end.areBoundsFinite())
      return;

    IntervalIntDomain& duration = static_cast<IntervalIntDomain&>(Propagator::getCurrentDomain(token->duration()));

    // TODO JRB : this check may may fail if we support violations
    check_error(!start.isEmpty() && !end.isEmpty() && !duration.isEmpty(), "Can't update token duration with empty variable. Token:"
    		+ token->toString()
    		+ " start:"	+ start.toString()
    		+ " end:" + end.toString()
    		+ " duration:" + duration.toString());

    edouble maxDuration = end.getUpperBound() - start.getLowerBound();
    edouble minDuration = end.getLowerBound() - start.getUpperBound();

    if (minDuration <= duration.getLowerBound() && maxDuration >= duration.getUpperBound())
      return;  // PHM 06/06/11 This will almost always be the case.for DE.

    // TODO JRB: this should never cause a new violation
    duration.intersect(minDuration, maxDuration);
  }

  bool TemporalPropagator::canPrecede(const ConstrainedVariableId first, const ConstrainedVariableId second) {
    check_error(!updateRequired());
    const TimepointId fir = getTimepoint(first);
    const TimepointId sec = getTimepoint(second);
    check_error(fir);
    check_error(sec);


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

  bool TemporalPropagator::canFitBetween(const ConstrainedVariableId start, const ConstrainedVariableId end,
                                         const ConstrainedVariableId predend, const ConstrainedVariableId succstart) {
    check_error(!updateRequired());
    const TimepointId tstart = getTimepoint(start);
    const TimepointId tend = getTimepoint(end);
    const TimepointId pend= getTimepoint(predend);
    const TimepointId sstart = getTimepoint(succstart);

    check_error(tstart);
    check_error(tend);
    check_error(pend);
    check_error(sstart);

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

  bool TemporalPropagator::canBeConcurrent(const ConstrainedVariableId first, const ConstrainedVariableId second) {
    const TimepointId _first = getTimepoint(first);
    const TimepointId _second = getTimepoint(second);

    debugMsg("TemporalPropagator:canBeConcurrent",
	     "determining if  " << first->lastDomain() << " can be concurrent with " << second->lastDomain());

    // If either node does not have a timepoint, then it means that there is no temporal constraint for it. That being the case,
    // the only question is whether the domains have an intersection
    if(_first == NULL || _second == NULL)
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

bool TemporalPropagator::wasRelaxed(const ConstrainedVariableId var) {
  const TimepointId tp = getTimepoint(var);
  check_error(tp);
  const TemporalConstraintId tnetConstraint = tp->getBaseDomainConstraint();
  Time lbt, ubt;
  tnetConstraint->getBounds(lbt, ubt);

  const IntervalIntDomain& timeBounds = static_cast<const IntervalIntDomain&>(var->lastDomain());
  Time lb = cast_long(timeBounds.getLowerBound());
  Time ub = cast_long(timeBounds.getUpperBound());

  //if(lb < lbt || ub > ubt) {
  if(lb ==MINUS_INFINITY && ub==PLUS_INFINITY) {
    debugMsg("TemporalPropagator:wasRelaxed", "Variable " << var->toLongString() << " was relaxed. "
             << "tnet bounds: [" << lbt << "," << ubt << "] "
             << "cnet bounds: [" << lb << "," << ub << "] "
             );
    return true;
  }
  else
    return false;
}

void TemporalPropagator::updateTimepoint(const ConstrainedVariableId var) {
  debugMsg("TemporalPropagator:updateTimepoint",
           "In updateTimepoint for var " << var->getKey());
  static unsigned int sl_counter(0);
  sl_counter++;

  check_error(var.isValid());
  checkError(var->isActive(), var->toString());
  check_error(m_varToTimepoint.find(var) != m_varToTimepoint.end());

  const TimepointId tp = getTimepoint(var);

  debugMsg("TemporalPropagator:updateTimepoint",
           "Updating timepoint " << tp << " to variable " << var->toLongString());

  check_error(tp);

  const IntervalIntDomain& timeBounds = static_cast<const IntervalIntDomain&>(var->lastDomain());
  Time lb = cast_int(timeBounds.getLowerBound());
  Time ub = cast_int(timeBounds.getUpperBound());
  const TemporalConstraintId baseDomainConstraint = tp->getBaseDomainConstraint();
  TemporalConstraintId newConstraint = updateConstraint(var, baseDomainConstraint, lb, ub);

  if(newConstraint != NULL) {
    //tp->setBaseDomainConstraint(baseDomainConstraint);
    debugMsg("TemporalPropagator:updateTimepoint",
             "Setting base domain constraint for " << var->getKey() << " to " <<
             newConstraint << " [" << newConstraint->getLowerBound() << " " <<
             newConstraint->getUpperBound() << "]");
    tp->setBaseDomainConstraint(newConstraint);
  }

  checkError(!var->lastDomain().areBoundsFinite() || m_tnet->hasEdgeToOrigin(tp),
             "Counter:" << sl_counter << ". It should have an edge to the origin, but it doesn't!" <<
             var->toString() << " and timepoint " <<  tp);
}

  void TemporalPropagator::updateTemporalConstraint(const ConstraintId constraint)
  {
    debugMsg("TemporalPropagator:updateTemporalConstraint", "In updateTemporalConstraint for constraint " << constraint->getKey());
    // If the constraint has no corresponding constraint in the tnet, then add it.
    if(m_constrToTempConstr.find(constraint) == m_constrToTempConstr.end()) {
      addTemporalConstraint(constraint);
      return;
    }
    updateTnetDuration(constraint);
  }

  /*
   * This is necessary because the tnet keeps durations on the edges of the temporal
   * graph and therefore doesn't update them.
   */
void TemporalPropagator::updateTnetDuration(const ConstraintId constraint) {
  // Update for the distance variable
  if(constraint->getScope().size() == 3) { // TODO JRB: this seems brittle, what if we get other temporal constraints with 3 parameters?
    const ConstrainedVariableId distance = constraint->getScope()[1];

    // In order to avoid the unhappy situation where temporalDistance does not maintain the semantics of addEq
    // we now apply the distance bounds to the distance variable.
    const IntervalIntDomain& sourceDom = constraint->getScope()[0]->lastDomain();
    const IntervalIntDomain& targetDom = constraint->getScope()[2]->lastDomain();

    // Checks for finiteness are to avoid overflow or underflow.
    if(sourceDom.isFinite() && targetDom.isFinite()){
      IntervalIntDomain& distanceDom = static_cast<IntervalIntDomain&>(Propagator::getCurrentDomain(distance));
      Time minDistance = cast_int(targetDom.getLowerBound() - sourceDom.getUpperBound());
      Time maxDistance = cast_int(targetDom.getUpperBound() - sourceDom.getLowerBound());

      // if this intersect() call causes a violation
      // we need to have the constraint network know the culprit (constraint)
      distance->setCurrentPropagatingConstraint(constraint);
      if(distanceDom.intersect(minDistance, maxDistance) && distanceDom.isEmpty())
        return;
    }

    checkError(m_varToTimepoint.find(distance) == m_varToTimepoint.end(),
               "No support for timepoints being distances. " << distance->toLongString());

    checkError(distance->lastDomain().isInterval(), constraint->getKey() << " is invalid");
    
    const TemporalConstraintId tnetConstraint = m_constrToTempConstr.find(constraint)->second;
    const IntervalIntDomain& dom = static_cast<const IntervalIntDomain&>(distance->lastDomain());
    Time lb= cast_int(dom.getLowerBound());
    Time ub= cast_int(dom.getUpperBound());
    debugMsg("TemporalPropagator:updateTemporalConstraint", "Calling updateConstraint");
    updateConstraint(distance, tnetConstraint, lb, ub);
  }
}

TemporalConstraintId TemporalPropagator::updateConstraint(const ConstrainedVariableId var,
                                                          const TemporalConstraintId tnetConstraint,
                                                          Time lbc,
                                                          Time ubc) {
  debugMsg("TemporalPropagator:updateConstraint",
           "In updateConstraint for var " << var->getKey());

  static unsigned int sl_counter(0);
  sl_counter++;

  Time lb = mapToInternalInfinity(lbc);
  Time ub = mapToInternalInfinity(ubc);

  if(tnetConstraint == NULL)
    return m_tnet->addTemporalConstraint(m_tnet->getOrigin(), getTimepoint(var), lb, ub);

  TemporalConstraintId newConstraint = tnetConstraint;

  // Initialize the bounds from the current constraint
  Time lbt, ubt;
  tnetConstraint->getBounds(lbt, ubt);
  checkError(lbt <= ubt, lbt << ">" << ubt << " in " << tnetConstraint);

  // Note that at this point we may in fact have a case where lbt > ubt in the event of a relaxation.

  checkError(lb <= ub, lb << ">" << ub);

  debugMsg("TemporalPropagator:updateConstraint", "Updating bounds for Variable " << var->getKey()
           << " tnet bounds : [" << lbt << "," << ubt << "]"
           << " cnet bounds : [" << lbc << "," << ubc << "]");

  if(lb < lbt || ub > ubt) { // Handle relaxation
    m_mostRecentRepropagation = getConstraintEngine()->mostRecentRepropagation();

    checkError(tnetConstraint, tnetConstraint);
    publish(notifyConstraintDeleted(tnetConstraint->getKey(), tnetConstraint));
    debugMsg("TemporalPropagator:updateConstraint",
             "DELETED-Constraint " << tnetConstraint->getKey());

    // Now switch it out
    TimepointId source, target;
    m_tnet->getConstraintScope(tnetConstraint, source, target); // Pull old timepoints.
    
    std::map<TemporalConstraintId, ConstraintId>::const_iterator  cnetConstraintIt =
        m_tempConstrToConstr.find(tnetConstraint);
    
    ConstraintId cnetConstraint;
    if(cnetConstraintIt != m_tempConstrToConstr.end())
      cnetConstraint = cnetConstraintIt->second;
    unmap(tnetConstraint);

    m_tnet->removeTemporalConstraint(tnetConstraint);
    newConstraint = m_tnet->addTemporalConstraint(source, target, lb, ub);
    if(!cnetConstraint.isNoId()){
      mapConstraint(cnetConstraint, newConstraint);
      
      publish(notifyConstraintAdded(cnetConstraint, newConstraint,  lb, ub));

      debugMsg("TemporalPropagator:updateConstraint",
               "ADDED-Constraint (tnet) " << newConstraint->getKey() << " for external constraint " << cnetConstraint->getKey());
    }
    else {
      check_error(target == getTimepoint(var));

      debugMsg("TemporalPropagator:updateConstraint",
               "Setting base domain constraint for " << target->getKey() << " to " <<
               newConstraint << " [" << newConstraint->getLowerBound() << " " <<
               newConstraint->getUpperBound() << "]");

      target->setBaseDomainConstraint(newConstraint);
      publish(notifyBaseDomainConstraintAdded(var, newConstraint,  lb, ub));

      debugMsg("TemporalPropagator:updateConstraint",
               "ADDED-Constraint (Base Domain) " <<  newConstraint << " for Variable " << var->getKey()
               << " [" << lb << "," << ub << "]");
    }
  }
  //else if (!tnetConstraint->isComplete() || lb > lbt || ub < ubt) { // Handle restriction. Retain most restricted values
  else if (lb > lbt || ub < ubt) { // Handle restriction. Retain most restricted values
    Time newLb = std::max(lb, lbt);
    Time newUb = std::min(ub, ubt);
    Time currentLb = 0;
    Time currentUb = 0;
    tnetConstraint->getBounds(currentLb, currentUb);
    //if(!tnetConstraint->isComplete() || currentUb > newUb || currentLb < newLb ){
    if(currentUb > newUb || currentLb < newLb ){
      newLb = std::max(newLb, currentLb);
      newUb = std::min(newUb, currentUb);
      m_tnet->narrowTemporalConstraint(tnetConstraint, newLb, newUb);
      publish(notifyBoundsRestricted(var, newLb, newUb));
      debugMsg("TemporalPropagator:updateConstraint",
               "UPDATED-VariableBounds of "  << var->getKey() << " Restricted to [" << newLb << "," << newUb << "]");
    }
  }

  return newConstraint;
}


  void TemporalPropagator::buffer(const ConstrainedVariableId var){
    if(m_varToTimepoint.find(var) == m_varToTimepoint.end())
      addTimepoint(var);
    else
      m_changedVariables.insert(std::make_pair(var->getKey(),var));
  }

  void TemporalPropagator::addListener(const TemporalNetworkListenerId listener) {
    m_listeners.insert(listener);
  }

bool TemporalPropagator::isValidForPropagation() const {

  // All buffers should only contain valid id's
  if(!allValid(m_activeVariables) ||
     !allValid(m_changedVariables) ||
     !allValid(m_changedConstraints) ||
     std::find(m_constraintsForDeletion.begin(), m_constraintsForDeletion.end(), TemporalConstraintId()) != m_constraintsForDeletion.end() ||
     std::find(m_variablesForDeletion.begin(), m_variablesForDeletion.end(), TimepointId()) != m_variablesForDeletion.end() ||
     std::find(m_listeners.begin(), m_listeners.end(), static_cast<TemporalNetworkListenerId>(NULL)) != m_listeners.end()) {
    debugMsg("TemporalPropagator:isValidForPropagation", "buffers have something invalid");
    condDebugMsg(!allValid(m_activeVariables), "TemporalPropagator:isValidForPropagation",
                 "active variable");
    for(std::map<eint, ConstrainedVariableId>::const_iterator it = m_activeVariables.begin();
        it != m_activeVariables.end(); ++it) {
      if(it->second.isNoId() || it->second.isInvalid())
        std::cout << it->first << " " << it->second << std::endl;
    }
    condDebugMsg(!allValid(m_changedVariables), "TemporalPropagator:isValidForPropagation",
                 "changed variable");
    condDebugMsg(!allValid(m_changedConstraints), "TemporalPropagator:isValidForPropagation",
                 "changed constraint");
    condDebugMsg(std::find(m_constraintsForDeletion.begin(), m_constraintsForDeletion.end(),
                           TemporalConstraintId()) != m_constraintsForDeletion.end(),
                 "TemporalPropagator:isValidForPropagation",
                 "constraint for deletion");
    condDebugMsg(std::find(m_variablesForDeletion.begin(), m_variablesForDeletion.end(), TimepointId()) != m_variablesForDeletion.end(),
                 "TemporalPropagator:isValidForPropagation",
                 "variable for deletion");
    condDebugMsg(std::find(m_listeners.begin(), m_listeners.end(), static_cast<TemporalNetworkListenerId>(NULL)) != m_listeners.end(),
                 "TemporalPropagator:isValidForPropagation",
                 "listener");
    
    return false;
  }

  // For all buffered timepoints for deletion, none should have any dangling external entities. This is because
  // we will have already deleteed the Constraint for which this Constraint shadows it.
  for(TemporalConstraintsSet::const_iterator it = m_constraintsForDeletion.begin();
      it != m_constraintsForDeletion.end(); ++it){
    TemporalConstraintId shadow = *it;
    if(m_tempConstrToConstr.find(shadow) != m_tempConstrToConstr.end()) {
      debugMsg("TemporalPropagator:isValidForPropagation",
               "Shadow is noid for deleted constraints ");
      return false;
    }
  }

  // For all buffered constraints for deletion, none should have any dangling external entities. This is because
  // we will have already deleted the TempVar for which this timepoint shadows it.
  for(std::set<TimepointId>::const_iterator it = m_variablesForDeletion.begin();
      it != m_variablesForDeletion.end(); ++it){
    TimepointId timepoint = *it;
    if(m_timepointToVar.find(timepoint) != m_timepointToVar.end()) {
      debugMsg("TemporalPropagator:isValidForPropagation",
               "Shadow is noid for deleted variables");
      return false;
    }
  }

  // For all buffered vars's, it either has an external entity or it doesn't. No invalid one.
  // Should also ensure that ONLY start and end variables have external entities.
  for(std::map<eint,ConstrainedVariableId>::const_iterator it = m_changedVariables.begin();
      it != m_changedVariables.end(); ++it){
    const ConstrainedVariableId var = it->second;
    std::map<ConstrainedVariableId, TimepointId>::const_iterator varIt = m_varToTimepoint.find(var);
    if(varIt == m_varToTimepoint.end()){ // It must be a start or end variable
      debugMsg("TemporalPropagator:isValidForPropagation",
               "Shadow is not linked up correctly for " << var->toString());
      return false;
    }
  }

  // For all bufferec constraints for change, it should have no shadow, or a good shadow. Also, if it has a shadow,
  // we should ensure that it is linked correctly
  // for( ConstraintsSet::const_iterator it = m_changedConstraints.begin(); it != m_changedConstraints.end(); ++it){
  //   ConstraintId constraint = *it;
  //   typedef ConstrToTempConstrMap::left_map::const_iterator left_map_iterator;
  //   left_map_iterator left_map_it = m_constrToTempConstr.left.find(constraint);
  //   if(left_map_it == m_constrToTempConstr.left.end()) {
  //     debugMsg("TemporalPropagator:isValidForPropagation",
  //              "Shadow of constraints is not linked up " << constraint->toString());
  //     return false;
  //   }
  // }

  // For all buffered constraints,
  return true;
}

  const IntervalIntDomain TemporalPropagator::getTemporalDistanceDomain(const ConstrainedVariableId first,
                                                                        const ConstrainedVariableId second,
                                                                        const bool exact) {
    TimepointId tstart = getTimepoint(first);
    TimepointId tend = getTimepoint(second);
    if(!(tstart && tend)) {
      eint minDistance = std::max(MINUS_INFINITY,
                                  second->lastDomain().getLowerBound() -
                                  first->lastDomain().getUpperBound());
      // if(second->lastDomain().getLowerBound() > MINUS_INFINITY &&
      //    first->lastDomain().getUpperBound() < PLUS_INFINITY)
      //   minDistance = std::max(min_distance,  < PLUS_INFINITY);
      eint maxDistance = std::min(PLUS_INFINITY,
                                  second->lastDomain().getUpperBound() -
                                  first->lastDomain().getLowerBound());
      return IntervalIntDomain(minDistance, maxDistance);
      
    }
    Time lb, ub;
    m_tnet->calcDistanceBounds(tstart, tend, lb, ub, exact);
    return(IntervalIntDomain(lb,ub));
  }

  void TemporalPropagator::getTemporalDistanceDomains(const ConstrainedVariableId first,
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

  void TemporalPropagator::getTemporalDistanceSigns(const ConstrainedVariableId first,
                                                    const std::vector<ConstrainedVariableId>& seconds,
                                                    std::vector<Time>& lbs,
                                                    std::vector<Time>& ubs)
  {
    // More efficient to get several at once. Exact calculation.  The
    // lbs, ubs "bounds" are accurate only to their signs (neg/0/pos).
    // Fast function for determining which of the seconds precede or
    // follow first.
    TimepointId tstart = getTimepoint(first);
    std::vector<TimepointId> tends;
    for (unsigned i=0; i<seconds.size(); i++) {
      TimepointId tend = getTimepoint(seconds[i]);
      tends.push_back(tend);
    }
    m_tnet->calcDistanceSigns(tstart, tends, lbs, ubs);
    checkError((lbs.size() == seconds.size()) && (lbs.size() == ubs.size()),
               "size mismatch in TemporalPropagator getTemporalDistanceDomains");
    return;
  }

  /* PHM Support for reftime calculations.  Reftimes provide a tnet
   * solution that is close to a given preferred set of times for the
   * timevars. They are like a third "bound" in addition to the
   * lower/upper ones.  The .preferred times are given as bounds on
   * constraints from a designated "origin" var, called the refpoint,
   * to the other timevars.  The bounds may be all lower or upper (but
   * not mixed); this influences which timevar is moved to satisfy a
   * constraint.  The tnet then calculates and returns the reftimes.
   */

  void TemporalPropagator::setRefpointVar(const ConstrainedVariableId var)
  {
    // PHM Support for reftime calculations.  Designates the refpoint.
    if (var.isId())
      m_tnet->setReferenceTimepoint(getTimepoint(var));
    else
      m_tnet->setReferenceTimepoint();
  }

  Time TemporalPropagator::getReferenceTime(const ConstrainedVariableId var) {
    // PHM Support for reftime calculations.  Returns the reftime
    // value after the refpoint and Preferred time constraints are set
    // up. Returns lb before that.
    check_error(var.isId());
    TimepointId tp = getTimepoint(var);
    if (tp && m_tnet->getReferenceTimepoint()) {
      return tp->getReftime();
    }
    // Otherwise var is not (yet) ref timepoint, just return lb for now
    return cast_int(var->lastDomain().getLowerBound());
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
        if(distance == POS_INFINITY)
          continue;
        Time lb1 = newreftimes[j] - distance;
        if (lb1 > lb) lb = lb1;
      }
      // The ub props for j<i have already been pushed to uppers[i].
      Time ub = uppers[i];
      checkError(lb <= ub, "lb>ub in TemporalPropagator::getMinPerturbTimes: " << lb << " " << ub);

      // Calculate the new reftime
      if (oldreftimes[i] < lb)
        newreftimes[i] = lb;
      else if (oldreftimes[i] > ub)
        newreftimes[i] = ub;
      else
        newreftimes[i] = oldreftimes[i];
      //std::cerr << timevars[i]->parent()->toString() << " "
      //          << timevars[i]->lastDomain().getLowerBound() << " "
      //          << timevars[i]->lastDomain().getUpperBound() << " "
      //          << newreftimes[i] << " " << oldreftimes[i] << "\n";

      // Push the ub props for j>i to uppers.
      for (unsigned j=i+1; j<timevars.size(); j++) {
        TimepointId tp1 = getTimepoint(timevars[j]);
        Time distance = m_tnet->getDistance(tp1);
        if(distance == POS_INFINITY)
          continue;
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
  (const ConstrainedVariableId useAsOrigin,
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
      TimepointId from = boost::dynamic_pointer_cast<Timepoint>(edge->from);
      TimepointId to = boost::dynamic_pointer_cast<Timepoint>(edge->to);
      Time length = edge->length;
      ConstrainedVariableId fromvar,tovar;
      if (from == origin)
        fromvar = originvar;
      else
        fromvar = m_timepointToVar.find(from)->second;
      if (to == origin)
        tovar = originvar;
      else
        tovar = m_timepointToVar.find(to)->second;
      fromvars.push_back(fromvar);
      tovars.push_back(tovar);
      lengths.push_back(length);
    }
  }

  TemporalConstraintId TemporalPropagator::addSpecificationConstraint(const TemporalConstraintId tc, const TimepointId tp,
                                                                      const Time lb, const Time ub) {
    if(tc == NULL)
      return m_tnet->addTemporalConstraint(m_tnet->getOrigin(), tp, lb, ub, false);
    m_tnet->narrowTemporalConstraint(tc, lb, ub);
    return tc;
  }

void TemporalPropagator::mapVariable(const ConstrainedVariableId var,
                                     const TimepointId tp) {
  m_varToTimepoint.insert(std::make_pair(var, tp));
  m_timepointToVar.insert(std::make_pair(tp, var));
}

void TemporalPropagator::unmap(const ConstrainedVariableId var) {
  std::map<ConstrainedVariableId, TimepointId>::iterator it = m_varToTimepoint.find(var);
  if(it != m_varToTimepoint.end()) {
    m_timepointToVar.erase(it->second);
    m_varToTimepoint.erase(it);
  }
}

void TemporalPropagator::unmap(const TimepointId tp) {
  std::map<TimepointId, ConstrainedVariableId>::iterator it = m_timepointToVar.find(tp);
  if(it != m_timepointToVar.end()) {
    m_varToTimepoint.erase(it->second);
    m_timepointToVar.erase(it);
  }
}

void TemporalPropagator::mapConstraint(const ConstraintId constr,
                                       const TemporalConstraintId temp) {
  m_constrToTempConstr.insert(std::make_pair(constr, temp));
  m_tempConstrToConstr.insert(std::make_pair(temp, constr));
}
void TemporalPropagator::unmap(const ConstraintId constr) {
  std::map<ConstraintId, TemporalConstraintId>::iterator it = m_constrToTempConstr.find(constr);
  if(it != m_constrToTempConstr.end()) {
    m_tempConstrToConstr.erase(it->second);
    m_constrToTempConstr.erase(it);
  }
}
void TemporalPropagator::unmap(const TemporalConstraintId temp) {
  std::map<TemporalConstraintId, ConstraintId>::iterator it = m_tempConstrToConstr.find(temp);
  if(it != m_tempConstrToConstr.end()) {
    m_constrToTempConstr.erase(it->second);
    m_tempConstrToConstr.erase(it);
  }
}

void TemporalPropagator::incrementRefCount(const ConstrainedVariableId var) {
  std::map<ConstrainedVariableId, unsigned int>::iterator it = m_refCount.find(var);
  if(it == m_refCount.end())
    it = m_refCount.insert(std::make_pair(var, 0)).first;
  ++(it->second);
}

void TemporalPropagator::decrementRefCount(const ConstrainedVariableId var) {
  std::map<ConstrainedVariableId, unsigned int>::iterator it = m_refCount.find(var);
  checkError(it != m_refCount.end(),
             "Failed to find reference count for " << var->toString());
  --(it->second);
  if(it->second == 0) {
    m_refCount.erase(var);
    notifyDeleted(var, getTimepoint(var));
  }
}

} //namespace
