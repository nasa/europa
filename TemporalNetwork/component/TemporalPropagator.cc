#include "TemporalPropagator.hh"
#include "TemporalNetworkDefs.hh"
#include "TimepointWrapper.hh"
#include "Token.hh"
#include "TemporalNetwork.hh"
#include "TemporalNetworkListener.hh"
#include "ConstraintEngine.hh"
#include "ConstrainedVariable.hh"
#include "Constraint.hh"
#include "Utils.hh"

// @todo: there are cases where we may be able to fail early during the
// mapping from the constraint engine to the temporal network.  In these
// cases we could propagate the temporal netowrk as we're doing the mapping
// and detect inconsistencies at that point.  In cases where domains were
// relaxed in temporal variables we must do the mapping first or we'll run
// the risk of detecting an inconsistency where there isn't one.

namespace Prototype {

#define  publish(message){\
    for(std::set<TemporalNetworkListenerId>::const_iterator lit = m_listeners.begin(); lit != m_listeners.end(); ++lit)\
      (*lit)->message;\
  }

  typedef Id<TimepointWrapper> TimepointWrapperId;

  const TimepointId& TemporalPropagator::getTimepoint(const TempVarId& var){
    check_error(var->getIndex() != DURATION_VAR_INDEX);
    check_error(var->getExternalEntity().isValid());
    TimepointWrapperId wrapper(var->getExternalEntity());
    return wrapper->getTimepoint();
  }

  TemporalPropagator::TemporalPropagator(const LabelStr& name, const ConstraintEngineId& constraintEngine)
    : Propagator(name, constraintEngine), m_tnet((new TemporalNetwork())->getId()) {}

  TemporalPropagator::~TemporalPropagator() {
    cleanup(m_listeners);
    check_error(m_tnet.isValid());
    delete (TemporalNetwork*) m_tnet;
  }

  void TemporalPropagator::notifyDeleted(const TempVarId& tempVar, const TimepointId& tp) {
    m_changedVariables.erase(tempVar);
    m_activeVariables.erase(tempVar);
    tp->clearExternalEntity(); // Breaks link to original
    m_variablesForDeletion.insert(tp);
  }

  void TemporalPropagator::handleConstraintAdded(const ConstraintId& constraint){
    m_constraintsForAddition.insert(constraint);
  }

  void TemporalPropagator::handleConstraintRemoved(const ConstraintId& constraint){
    // Delete constraint from list of pending additions and executions. It may be there if we have not propagated them yet.
    m_constraintsForAddition.erase(constraint);
    m_constraintsForExecution.erase(constraint);
    m_durationChanges.erase(constraint);

    // Buffer temporal network constraint for deletion
    if(!constraint->getExternalEntity().isNoId()){
      const TemporalConstraintId& tc = constraint->getExternalEntity();
      tc->clearExternalEntity();
      m_constraintsForDeletion.insert(tc);
      constraint->clearExternalEntity();
    }
  }

  void TemporalPropagator::handleConstraintActivated(const ConstraintId& constraint){
    if(constraint->getName() == LabelStr("StartEndDurationRelation"))
      m_constraintsForExecution.insert(constraint);

    const std::vector<ConstrainedVariableId>& scope = constraint->getScope();
    for(std::vector<ConstrainedVariableId>::const_iterator it = scope.begin(); it != scope.end(); ++it){
      const ConstrainedVariableId& var = *it;
      check_error(var.isValid());

      if(var->getIndex() == DURATION_VAR_INDEX){ // Buffer the duration constraint instead of the variable
	m_durationChanges.insert(constraint);
	return;
      }
      else {
	m_changedVariables.insert(var);
	m_activeVariables.insert(var);
      }
    }
  }

  void TemporalPropagator::handleConstraintDeactivated(const ConstraintId& constraint){
    // Delete constraint from list of pending additions and executions. It may be there if we have not propagated them yet.
    m_constraintsForAddition.erase(constraint);
    m_constraintsForExecution.erase(constraint);
    m_durationChanges.erase(constraint);

    const std::vector<ConstrainedVariableId>& scope = constraint->getScope();
    for(std::vector<ConstrainedVariableId>::const_iterator it = scope.begin(); it != scope.end(); ++it){
      const ConstrainedVariableId& var = *it;
      check_error(var.isValid());
      TokenId parentToken = var->getParent();
      if(parentToken->isMerged()){
	m_changedVariables.erase(var);
	m_activeVariables.erase(var);
      }
    }
  }

  void TemporalPropagator::handleNotification(const ConstrainedVariableId& variable, 
					      int argIndex, 
					      const ConstraintId& constraint, 
					      const DomainListener::ChangeType& changeType){
    check_error(constraint->isActive());

    // Ignore if this is the active constraint
    if(constraint == m_activeConstraint)
      return;

    if(constraint->getName() == LabelStr("StartEndDurationRelation")){
      m_constraintsForExecution.insert(constraint);
      if(variable->getIndex() == DURATION_VAR_INDEX){ // Buffer the duration constraint instead of the variable
	m_durationChanges.insert(constraint);
	return;
      }
    }

    // Only insert for start or end variables
    check_error(variable->getIndex() != DURATION_VAR_INDEX);
    m_changedVariables.insert(variable);
  }


  void TemporalPropagator::execute(){
    check_error(!getConstraintEngine()->provenInconsistent());

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
      TempVarId var = (*it)->getExternalEntity();
      check_error (!var.isNoId());
      Propagator::getCurrentDomain(var).empty();
    }
    else {  //update the cnet
      bool validUpdate = updateTempVar();
      check_error(validUpdate);

      while(!m_constraintsForExecution.empty()){
	std::set<ConstraintId>::iterator it = m_constraintsForExecution.begin();
	ConstraintId constraint = *it;

	if(constraint->isActive()){
	  m_activeConstraint = constraint;
	  Propagator::execute(constraint);
	}

	if (getConstraintEngine()->provenInconsistent())
	  m_constraintsForExecution.clear();
	else
	  m_constraintsForExecution.erase(it);
      }
    }
    m_activeConstraint = ConstraintId::noId();
  }

  bool TemporalPropagator::updateRequired() const{
    bool fullyPropagated = (m_constraintsForExecution.empty() && 
			    m_constraintsForDeletion.empty() &&
			    m_constraintsForAddition.empty() &&
			    m_changedVariables.empty() &&
			    m_durationChanges.empty());

    return (!fullyPropagated);
  }

  void TemporalPropagator::checkAndAddTnetVariables(const ConstraintId& constraint) {
    std::vector<TempVarId> vars;

    if (constraint->getName() == LabelStr("StartEndDurationRelation")) {    
      vars.push_back(constraint->getScope()[0]);  
      vars.push_back(constraint->getScope()[2]);
    }
    else {
      vars.push_back(constraint->getScope()[0]);  
      vars.push_back(constraint->getScope()[1]);      
    }

    // Allocate Timepoints if necessary
    for(std::vector<TempVarId>::const_iterator it = vars.begin(); it != vars.end(); ++it) {
      check_error(TempVarId::convertable((*it)));
      TempVarId var = (*it);
      if(var->getExternalEntity().isNoId()){
	TimepointId timepoint = m_tnet->addTimepoint();
	EntityId tw = (new TimepointWrapper(getId(), var, timepoint))->getId();
	var->setExternalEntity(tw);
	timepoint->setExternalEntity(var);
	publish(notifyTimepointAdded(var, timepoint));

	m_activeVariables.insert(var);
	m_changedVariables.insert(var);

	TemporalConstraintId c = m_tnet->addTemporalConstraint(m_tnet->getOrigin(), 
							       timepoint, 
							       (Time) var->getBaseDomain().getLowerBound(),  
							       (Time) var->getBaseDomain().getUpperBound());
	check_error(c.isValid());

	timepoint->setBaseDomainConstraint(c);

	publish(notifyBaseDomainConstraintAdded(var, c, (Time) var->getBaseDomain().getLowerBound(), (Time) var->getBaseDomain().getUpperBound()));
      }
    }
  }

  void TemporalPropagator::addTnetConstraint(const ConstraintId& constraint) {
    TempVarId start;
    TempVarId end;
    Time lb=0;
    Time ub=0;

    if (constraint->getName() == LabelStr("StartEndDurationRelation")) {    
      start = constraint->getScope()[0];
      TempVarId duration = constraint->getScope()[1];
      end = constraint->getScope()[2];

      lb = (Time) duration->getBaseDomain().getLowerBound();
      ub = (Time) duration->getBaseDomain().getUpperBound();
    }
    else if (constraint->getName() == LabelStr("concurrent")) {
      start = constraint->getScope()[0];
      end = constraint->getScope()[1];      
      lb = 0;
      ub = 0;
    }
    else if (constraint->getName() == LabelStr("before")) {
      start = constraint->getScope()[0];
      end = constraint->getScope()[1];      
      lb = 0;
      ub = g_infiniteTime();
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
  }

  void TemporalPropagator::handleTemporalAddition(const ConstraintId& constraint) {
    checkAndAddTnetVariables(constraint);
    addTnetConstraint(constraint);
    if (constraint->getName() == LabelStr("StartEndDurationRelation")) 
      m_constraintsForExecution.insert(constraint);    
  }

  void TemporalPropagator::updateTnet() {
    // Process constraints for deletion
    for(std::set<TemporalConstraintId>::const_iterator it = m_constraintsForDeletion.begin(); it != m_constraintsForDeletion.end(); ++it) {
      TemporalConstraintId constraint = *it;

      publish(notifyConstraintDeleted(constraint->getKey(), constraint));

      m_tnet->removeTemporalConstraint(constraint);
    }
    m_constraintsForDeletion.clear();
    
    // Process variables for deletion
    for(std::set<TimepointId>::const_iterator it = m_variablesForDeletion.begin(); it != m_variablesForDeletion.end(); ++it) {
      TimepointId tp = *it;
      TemporalConstraintId baseDomainConstraint = tp->getBaseDomainConstraint();
      check_error(baseDomainConstraint.isValid());

      publish(notifyConstraintDeleted(baseDomainConstraint->getKey(), baseDomainConstraint));

      m_tnet->removeTemporalConstraint(baseDomainConstraint);

      publish(notifyTimepointDeleted(tp));

      m_tnet->deleteTimepoint(tp);
    }
    m_variablesForDeletion.clear();    

    // Process constraints for addition (may also add variables)
    for(std::set<ConstraintId>::const_iterator it = m_constraintsForAddition.begin(); it != m_constraintsForAddition.end(); ++it){
      ConstraintId constraint = *it;
      handleTemporalAddition(constraint);
    }
    m_constraintsForAddition.clear();

    // Process variables that have changed
    for(std::set<TempVarId>::const_iterator it = m_changedVariables.begin(); it != m_changedVariables.end(); ++it){
      TempVarId var = *it;
      updateTimepoint(var);
    }
    m_changedVariables.clear();

    // Finally, process any changed StartEndDuration constraints to update (narrow) the temporal constraint associated
    for(std::set<ConstraintId>::const_iterator it = m_durationChanges.begin(); it != m_durationChanges.end(); ++it){
      ConstraintId constraint = *it;
      updateDurationConstraint(constraint);
    }
    m_durationChanges.clear();
  }


  /**
   * @brief Updates the ConstrainedEngine variable for each active timepoint.
   */
  bool TemporalPropagator::updateTempVar() {
    for(std::set<TempVarId>::const_iterator it = m_activeVariables.begin(); it != m_activeVariables.end(); ++it){
      TempVarId var = *it;
      check_error(var.isValid());
      check_error(var->getIndex() != DURATION_VAR_INDEX);

      const TimepointId& tp = getTimepoint(var);
      check_error(tp.isValid());
      check_error(tp->getExternalEntity() == var);

      Time lb, ub;
      m_tnet->getTimepointBounds(tp, lb, ub);
      check_error(lb <= ub);

      IntervalIntDomain& dom = static_cast<IntervalIntDomain&>(Propagator::getCurrentDomain(var));

      check_error(!dom.isEmpty());
      if (lb > dom.getLowerBound() || ub < dom.getUpperBound()) {
	dom.intersect(lb, ub);
	check_error(!dom.isEmpty());
      }
    }
    return true;
  }
      
  bool TemporalPropagator::canPrecede(const TempVarId& first, const TempVarId& second) {
    check_error(!updateRequired());
    const TimepointId& fir = getTimepoint(first);
    const TimepointId& sec = getTimepoint(second);
    check_error(fir.isValid());
    check_error(sec.isValid());

    // further propagation in temporal network will only restrict values
    // further, so if we already are in violation, we will continue to be
    // in violation.
    // quick check to see if last time we computed bounds we were in violation
    Time flb, fub;
    m_tnet->getLastTimepointBounds(fir, flb, fub);

    Time slb, sub;
    m_tnet->getLastTimepointBounds(sec, slb, sub);

    if (sub < flb)
      return false;

    bool result=m_tnet->isDistanceLessThan(fir,sec,0);
    return !result;
  }

  bool TemporalPropagator::canFitBetween(const TempVarId& start, const TempVarId& end,
					 const TempVarId& predend, const TempVarId& succstart) {
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

    Time minDurationOfToken = elb-sub;

    if (m_tnet->isDistanceLessThan(pend,sstart,minDurationOfToken))
      return false;

    m_tnet->getTimepointBounds(tstart, slb, sub);
    m_tnet->getTimepointBounds(tend, elb, eub);
    minDurationOfToken = elb-sub;

    return (!m_tnet->isDistanceLessThan(pend,sstart,minDurationOfToken));
  }

  void TemporalPropagator::updateTimepoint(const TempVarId& var){
    check_error(var.isValid());
    check_error(var->getIndex() != DURATION_VAR_INDEX);

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

  void TemporalPropagator::updateDurationConstraint(const ConstraintId& constraint){
    check_error(constraint->getName() == LabelStr("StartEndDurationRelation"));
    const TempVarId& duration = constraint->getScope()[DURATION_VAR_INDEX];
    Time lb = (Time) duration->lastDomain().getLowerBound();
    Time ub = (Time) duration->lastDomain().getUpperBound();
    const TemporalConstraintId& tnetConstraint = constraint->getExternalEntity();
    updateConstraint(duration, tnetConstraint, lb, ub);
  }

  TemporalConstraintId TemporalPropagator::updateConstraint(const TempVarId& var, 
							    const TemporalConstraintId& tnetConstraint, 
							    Time lbt,
							    Time ubt){
    TemporalConstraintId newConstraint;
    int lb = (int)Propagator::getCurrentDomain(var).getLowerBound();
    int ub = (int)Propagator::getCurrentDomain(var).getUpperBound();

    check_error(lbt <= ubt);
    check_error(lb <= ub);

    if (lb > lbt || ub < ubt) { // Handle restriction
      m_tnet->narrowTemporalConstraint(tnetConstraint, lb, ub);
      publish(notifyBoundsRestricted(var, lb, ub));
    }
    else if(lb < lbt || ub > ubt) { // Handle relaxation
      // think about whether we can do better here, possibly by changing
      // the condition above.  There are cases
      // where the temporal network has restricted it further so we're just
      // thrashing by removing it and adding the original constraint that
      // was previously restricted by the temporal network.
      publish(notifyConstraintDeleted(tnetConstraint->getKey(), tnetConstraint));
      // Now switch it out
      TimepointId source, target;
      m_tnet->getConstraintScope(tnetConstraint, source, target); // Pull old timepoints.
      EntityId cnetConstraint = tnetConstraint->getExternalEntity();
      tnetConstraint->clearExternalEntity();
      m_tnet->removeTemporalConstraint(tnetConstraint);
      newConstraint = m_tnet->addTemporalConstraint(source, target, (Time)lb, (Time)ub);
      if(!cnetConstraint.isNoId()){
	cnetConstraint->clearExternalEntity();
	cnetConstraint->setExternalEntity(newConstraint);
      }
      else {
	check_error(target == getTimepoint(var));
	target->setBaseDomainConstraint(newConstraint);
      }
      publish(notifyConstraintAdded(cnetConstraint, newConstraint,  (Time)lb, (Time)ub));
    }

    return newConstraint;
  }

  void TemporalPropagator::addListener(const TemporalNetworkListenerId& listener) {
    m_listeners.insert(listener);
  }
} //namespace
