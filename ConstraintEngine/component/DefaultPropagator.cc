#include "DefaultPropagator.hh"
#include "ConstraintEngine.hh"
#include "ConstrainedVariable.hh"
#include "Constraint.hh"

namespace Prototype {

  DefaultPropagator::DefaultPropagator(const LabelStr& name, const ConstraintEngineId& constraintEngine)
    : Propagator(name, constraintEngine), m_activeConstraint(0){}

  void DefaultPropagator::handleConstraintAdded(const ConstraintId& constraint){
    m_agenda.insert(constraint);
  }

  void DefaultPropagator::handleConstraintRemoved(const ConstraintId& constraint){
    // Remove from agenda
    m_agenda.erase(constraint);
  }

  void DefaultPropagator::handleConstraintActivated(const ConstraintId& constraint){
    m_agenda.insert(constraint);
  }

  void DefaultPropagator::handleConstraintDeactivated(const ConstraintId& constraint){
    // Remove from agenda
    m_agenda.erase(constraint);
  }

  void DefaultPropagator::handleNotification(const ConstrainedVariableId& variable, 
					     int argIndex, 
					     const ConstraintId& constraint, 
					     const DomainListener::ChangeType& changeType){
    if(constraint->getKey() != m_activeConstraint)
      m_agenda.insert(constraint);
  }

  void DefaultPropagator::execute(){
    check_error(m_agenda.size() > 0);
    check_error(!getConstraintEngine()->provenInconsistent());
    check_error(m_activeConstraint == 0);

    while(!m_agenda.empty()){
      ConstraintSet::iterator it = m_agenda.begin();
      ConstraintId constraint = *it;
      m_agenda.erase(constraint);

      if(constraint->isActive()){
	m_activeConstraint = constraint->getKey();
	Propagator::execute(constraint);
      }

      if(getConstraintEngine()->provenInconsistent())
	m_agenda.clear();
    }

    m_activeConstraint = 0;
  }

  bool DefaultPropagator::updateRequired() const{
    return (m_agenda.size() > 0);
  }
}
