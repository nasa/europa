#include "TemporalPropagator.hh"
#include "TemporalNetworkDefs.hh"
#include "TokenTemporalVariable.hh"
#include "Token.hh"
#include "TemporalNetwork.hh"
#include "ConstraintEngine.hh"
#include "ConstrainedVariable.hh"
#include "Constraint.hh"

namespace Prototype {

  TemporalPropagator::TemporalPropagator(const LabelStr& name, const ConstraintEngineId& constraintEngine)
    : Propagator(name, constraintEngine), 
      m_activeConstraint(0),
      m_updateRequired(false){
    m_tnet = (new TemporalNetwork())->getId();
  }

  TemporalPropagator::~TemporalPropagator() {
    //std::cout << "Calling temporal prp destructor " << std::endl;
    delete (TemporalNetwork*) m_tnet;
  }

  void TemporalPropagator::handleConstraintAdded(const ConstraintId& constraint){
    handleTemporalAddition(constraint);
  }

  void TemporalPropagator::handleConstraintRemoved(const ConstraintId& constraint){
    handleTemporalDeletion(constraint);
  }

  void TemporalPropagator::handleConstraintActivated(const ConstraintId& constraint){
    handleTemporalAddition(constraint);
  }

  void TemporalPropagator::handleConstraintDeactivated(const ConstraintId& constraint){
    handleTemporalDeletion(constraint);
  }

  void TemporalPropagator::handleNotification(const ConstrainedVariableId& variable, 
					     int argIndex, 
					     const ConstraintId& constraint, 
					     const DomainListener::ChangeType& changeType){
    if ((constraint->getKey() != m_activeConstraint) &&
	(constraint->getName() == LabelStr("StartEndDurationRelation")))
      m_agenda.insert(constraint);
    m_updateRequired = true;
  }

  void TemporalPropagator::execute(){
    check_error(!getConstraintEngine()->provenInconsistent());
    check_error(m_activeConstraint == 0);

    //update the tnet
    updateTnet();
    //propagate the tnet
    m_tnet->isConsistent();
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
	m_tnetVariableConstraints.insert(std::make_pair(var->getKey(), c));
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
      m_tnet->removeTemporalConstraint((* m_tnetVariableConstraints.find((*it))).second);
      m_tnetVariables.erase((*it));
      m_tnetVariableConstraints.erase((*it));
    }
    m_variablesForDeletion.clear();    
  }

  void TemporalPropagator::updateTempVar() {
    for(std::map<int, TimepointId>::const_iterator it = m_tnetVariables.begin(); it != m_tnetVariables.end(); ++it) {
      Time lb, ub;
      m_tnet->getTimepointBounds((*it).second, lb, ub);
      
      if (m_tnet->getVarIdFromTimepoint((*it).second)->lastDomain().getLowerBound() != lb ||
      	  m_tnet->getVarIdFromTimepoint((*it).second)->lastDomain().getUpperBound() != ub) 
      	m_tnet->getVarIdFromTimepoint((*it).second)->specify(IntervalIntDomain(lb, ub));
    }
  }

  bool TemporalPropagator::canPrecede(const TempVarId& first, const TempVarId& second) {
    check_error(getTimepoint(first) != TimepointId::noId());
    check_error(getTimepoint(second) != TimepointId::noId());
    TimepointId fir = getTimepoint(first);
    TimepointId sec = getTimepoint(second);
    Time lb, ub;
    m_tnet->calcDistanceBounds(fir, sec, lb, ub);
    return(ub >= 0);
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
    Time slb, sub;
    Time elb, eub;
    m_tnet->calcDistanceBounds(pend, tstart, slb, sub);
    m_tnet->calcDistanceBounds(tend, succstart, elb, eub);

    return(sub >=0  && elb >=0);
  }

    
} //namespace
