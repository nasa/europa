#include "TemporalPropagator.hh"
#include "TemporalNetworkDefs.hh"
#include "TokenTemporalVariable.hh"
#include "Token.hh"
#include "TemporalNetwork.hh"
#include "ConstraintEngine.hh"
#include "ConstrainedVariable.hh"
#include "Constraint.hh"

// @todo: there are cases where we may be able to fail early during the
// mapping from the constraint engine to the temporal network.  In these
// cases we could propagate the temporal netowrk as we're doing the mapping
// and detect inconsistencies at that point.  In cases where domains were
// relaxed in temporal variables we must do the mapping first or we'll run
// the risk of detecting an inconsistency where there isn't one.

namespace Prototype {

  TemporalPropagator::TemporalPropagator(const LabelStr& name, const ConstraintEngineId& constraintEngine)
    : Propagator(name, constraintEngine), 
      m_activeConstraint(0),
      m_updateRequired(false),
      m_fullRepropRequired(true)
  {
    m_tnet = (new TemporalNetwork())->getId();
  }

  TemporalPropagator::~TemporalPropagator() {
    //std::cout << "Calling temporal prp destructor " << std::endl;
    delete (TemporalNetwork*) m_tnet;
  }

  void TemporalPropagator::handleConstraintAdded(const ConstraintId& constraint){
    m_fullRepropRequired = true;
    handleTemporalAddition(constraint);
  }

  void TemporalPropagator::handleConstraintRemoved(const ConstraintId& constraint){
    m_fullRepropRequired = true;
    handleTemporalDeletion(constraint);
  }

  void TemporalPropagator::handleConstraintActivated(const ConstraintId& constraint){
  }

  void TemporalPropagator::handleConstraintDeactivated(const ConstraintId& constraint){
  }

  void TemporalPropagator::handleNotification(const ConstrainedVariableId& variable, 
					     int argIndex, 
					     const ConstraintId& constraint, 
					     const DomainListener::ChangeType& changeType){
    if ((constraint->getKey() != m_activeConstraint) &&
	(constraint->getName() == LabelStr("StartEndDurationRelation")))
      m_agenda.insert(constraint);
    m_updateRequired = true;
    m_fullRepropRequired = true;
  }

  void TemporalPropagator::execute(){
    check_error(!getConstraintEngine()->provenInconsistent());
    check_error(m_activeConstraint == 0);

    //update the tnet
    updateTnet();

    //propagate the tnet
    if (!m_tnet->isConsistent()) {
      std::list<TimepointId> results(m_tnet->getInconsistencyReason());
      std::list<TimepointId>::iterator it = results.begin();
      Propagator::getCurrentDomain(m_tnet->getVarIdFromTimepoint(*it)).empty();
      m_agenda.clear();
    }
    else {
      //update the cnet
      updateTempVar();

      while(!m_agenda.empty()){
	std::set<ConstraintId>::iterator it = m_agenda.begin();
	ConstraintId constraint = *it;

	if(constraint->isActive()){
	  m_activeConstraint = constraint->getKey();
	  Propagator::execute(constraint);
	}

	if(getConstraintEngine()->provenInconsistent()){
	  m_agenda.clear();
	  break;
	}
	else
	  m_agenda.erase(it);
      }
      m_activeConstraint = 0;
      m_updateRequired = false;
    }
  }

  bool TemporalPropagator::updateRequired() const{
    return (m_updateRequired);
  }

  void TemporalPropagator::checkAndAddTnetVariables(const ConstraintId& constraint) {

    std::vector<TempVarId> vars;

    if (constraint->getName() == LabelStr("StartEndDurationRelation")) {    
      vars.push_back(constraint->getScope()[0]);  
      vars.push_back(constraint->getScope()[2]); //skip the duration variable
    }
    else {
      vars.push_back(constraint->getScope()[0]);  
      vars.push_back(constraint->getScope()[1]);      
    }
    //add the variables to the tnet if not there already
    for(std::vector<TempVarId>::const_iterator it = vars.begin(); it != vars.end(); ++it) {
      check_error(TempVarId::convertable((*it)));
      TempVarId var = (*it);
      if (m_tnetVariables.find(var->getKey()) == m_tnetVariables.end()) {
	TimepointId timepoint = m_tnet->addTimepoint(var);
	var->setTnetVariable(timepoint);
	m_tnetVariables.insert(std::make_pair(var->getKey(), timepoint));
	TemporalConstraintId c = m_tnet->addTemporalConstraint(m_tnet->getOrigin(), 
							       timepoint, 
							       (Time) var->getBaseDomain().getLowerBound(),  
							       (Time) var->getBaseDomain().getUpperBound());
	m_tnetVariableConstraints.insert(std::make_pair(var, c));
      }
    }
  }

  void TemporalPropagator::addTnetConstraint(const ConstraintId& constraint) {
    TempVarId start;
    TempVarId end;
    Time lb, ub;

    if (constraint->getName() == LabelStr("StartEndDurationRelation")) {    
      start = constraint->getScope()[0];
      TempVarId duration = constraint->getScope()[1];
      end = constraint->getScope()[2];

      check_error(getTimepoint(start) != TimepointId::noId());
      check_error(getTimepoint(end) != TimepointId::noId());

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
    
    TemporalConstraintId c = m_tnet->addTemporalConstraint(getTimepoint(start), 
							   getTimepoint(end), 
							   lb,  
							   ub);
    m_tnetConstraints.insert(std::make_pair(constraint->getKey(), c));

  }

  TimepointId TemporalPropagator::getTimepoint(const TempVarId& var) {
    if ( m_tnetVariables.find(var->getKey())  == m_tnetVariables.end())
      return TimepointId::noId();
    else
      return (* m_tnetVariables.find(var->getKey())).second;
  }

  void TemporalPropagator::handleTemporalAddition(const ConstraintId& constraint) {
    checkAndAddTnetVariables(constraint);
    addTnetConstraint(constraint);
    m_updateRequired = true;
    if (constraint->getName() == LabelStr("StartEndDurationRelation")) 
      m_agenda.insert(constraint);    
  }

  void TemporalPropagator::handleTemporalDeletion(const ConstraintId& constraint) {
    //buffer for deletion
    m_constraintsForDeletion.insert(constraint->getKey());
    // Remove from agenda
    m_agenda.erase(constraint);
  }

  void TemporalPropagator::updateTnet() {

    for(std::set<int>::const_iterator it = m_constraintsForDeletion.begin(); it != m_constraintsForDeletion.end(); ++it) {
      check_error(m_tnetConstraints.find((*it)) != m_tnetConstraints.end());
      m_tnet->removeTemporalConstraint((* m_tnetConstraints.find((*it))).second);
      m_tnetConstraints.erase((*it));
    }
    m_constraintsForDeletion.clear();
    
    for(std::set<int>::const_iterator it = m_variablesForDeletion.begin(); it != m_variablesForDeletion.end(); ++it) {
      check_error(m_tnetVariables.find((*it)) != m_tnetVariables.end());
      m_tnet->deleteTimepoint((* m_tnetVariables.find((*it))).second);
      for (std::map<TempVarId, TemporalConstraintId>::iterator varIt = m_tnetVariableConstraints.begin(); varIt != m_tnetVariableConstraints.end(); ++varIt) {
	if (varIt->first->getKey() == *it) {
	  m_tnet->removeTemporalConstraint(varIt->second);
	  m_tnetVariables.erase(*it);
	  m_tnetVariableConstraints.erase(varIt->first);
	  break;
	}
      }
    }
    m_variablesForDeletion.clear();    

    // mirror bounds into the temporal network variables.

    for (std::map<TempVarId, TemporalConstraintId>::iterator varIt = m_tnetVariableConstraints.begin(); varIt != m_tnetVariableConstraints.end(); ++varIt) {
      TempVarId var = varIt->first;
      int lb = (int)Propagator::getCurrentDomain(var).getLowerBound();
      int ub = (int)Propagator::getCurrentDomain(var).getUpperBound();
      TimepointId timepoint = m_tnetVariables.find(var->getKey())->second;
      Time lbt, ubt;

      // Instead of getTimepointBounds here we'd like to get cached values
      // from the last computation since the temporal network may be made
      // inconsistent in this mapping process.
      m_tnet->getLastTimepointBounds(timepoint, lbt, ubt);
      if (lb != lbt || ub != ubt) {
	if (lb >= lbt && ub <= ubt)
	  m_tnet->narrowTemporalConstraint(varIt->second, lb, ub);
	else {
	  // think about whether we can do better here, possibly by changing
	  // the condition above.  There are cases
	  // where the temporal network has restricted it further so we're just
	  // thrashing by removing it and adding the original constraint that
	  // was previously restricted by the temporal network.
	  m_tnet->removeTemporalConstraint(varIt->second);
	  TemporalConstraintId c(m_tnet->addTemporalConstraint(m_tnet->getOrigin(), timepoint, (Time)lb, (Time)ub));
	  varIt->second = c;
	}
      }
    }
  }

  void TemporalPropagator::updateTempVar() {
    for(std::map<int, TimepointId>::const_iterator it = m_tnetVariables.begin(); it != m_tnetVariables.end(); ++it) {
      Time lb, ub;
      m_tnet->getTimepointBounds((*it).second, lb, ub);
      
      if (Propagator::getCurrentDomain(m_tnet->getVarIdFromTimepoint((*it).second)).getLowerBound() > lb ||
      	  Propagator::getCurrentDomain(m_tnet->getVarIdFromTimepoint((*it).second)).getUpperBound() < ub) {
	std::cout << "Warning: bounds retrieved are  not a subset of the domain." << std::endl;
	std::cout << " Domain = " << Propagator::getCurrentDomain(m_tnet->getVarIdFromTimepoint((*it).second)) << std::endl;
	std::cout << " Bounds = [" << lb << "," << ub << "]" << std::endl;
	check_error(false);
      }
      else {
	TempVarId var = m_tnet->getVarIdFromTimepoint((*it).second);
	if (TokenId::convertable(var->getParent())) {
	  TokenId tok = var->getParent();
	  if (!tok->isMerged()) // no need to update merged token variables
	    var->specify(IntervalIntDomain(lb, ub));
	}
	else
	  var->specify(IntervalIntDomain(lb, ub));
      }
    }
  }

  bool TemporalPropagator::canPrecede(const TempVarId& first, const TempVarId& second) {
    check_error(getTimepoint(first) != TimepointId::noId());
    check_error(getTimepoint(second) != TimepointId::noId());

    TimepointId fir = getTimepoint(first);
    TimepointId sec = getTimepoint(second);

    // further propagation in temporal network will only restrict values
    // further, so if we already are in violation, we will continue to be
    // in violation.
    // quick check to see if last time we computed bounds we were in violation
    Time flb, fub;
    m_tnet->getLastTimepointBounds(fir, flb, fub);

    Time slb, sub;
    m_tnet->getLastTimepointBounds(sec, slb, sub);

    //std::cout << "TemporalPropagator::canPrecede testing " << sub << " < " << flb << std::endl;

    if (sub < flb)
      return false;

    //std::cout << "TemporalPropagator::canPrecede computing distance < 0 {" << sec << "," << fir << "]";

    bool result=m_tnet->isDistanceLessThan(fir,sec,0);
//     if (result)
//       std::cout << " false" << std::endl;
//     else
//       std::cout << " true" << std::endl;

    return !result;
    //    return (!m_tnet->isDistanceLessThan(sec,fir,0));

  }

  bool TemporalPropagator::canFitBetween(const TempVarId& start, const TempVarId& end,
					 const TempVarId& predend, const TempVarId& succstart) {
    check_error(getTimepoint(start) != TimepointId::noId());
    check_error(getTimepoint(end) != TimepointId::noId());
    check_error(getTimepoint(predend) != TimepointId::noId());
    check_error(getTimepoint(succstart) != TimepointId::noId());
    TimepointId tstart = getTimepoint(start);
    TimepointId tend = getTimepoint(end);
    TimepointId pend= getTimepoint(predend);
    TimepointId sstart = getTimepoint(succstart);

    //    std::cout << "TemporalPropagator:: canFitBetween computing if slot duration zero {" << pend << "," << sstart << "}";

    bool result = m_tnet->isDistanceLessThan(pend,sstart,1);
    if (result) {
      //      std::cout << " true" << std::endl;
      return false;
    }
    //    else 
    //      std::cout << " false" << std::endl;
      

    // is slot duration zero?
    /*
    if (m_tnet->isDistanceLessThan(pend,sstart,1))
      return false;
    */

    Time slb, sub;
    m_tnet->getLastTimepointBounds(tstart, slb, sub);

    Time elb, eub;
    m_tnet->getLastTimepointBounds(tend, elb, eub);

    Time minDurationOfToken = elb-sub;
    
    //    std::cout << "TemporalPropagator:: canFitBetween computing last computed minDurationOfToken = " << minDurationOfToken << std::endl;

    if (m_tnet->isDistanceLessThan(pend,sstart,minDurationOfToken))
      return false;

    m_tnet->getTimepointBounds(tstart, slb, sub);
    m_tnet->getTimepointBounds(tend, elb, eub);
    minDurationOfToken = elb-sub;
    
    //    std::cout << "TemporalPropagator:: canFitBetween computing minDurationOfToken = " << minDurationOfToken << std::endl;

    return (!m_tnet->isDistanceLessThan(pend,sstart,minDurationOfToken));

  }
    
} //namespace
