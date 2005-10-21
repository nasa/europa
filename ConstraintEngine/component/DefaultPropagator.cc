#include "DefaultPropagator.hh"
#include "ConstraintEngine.hh"
#include "ConstrainedVariable.hh"
#include "Constraint.hh"

namespace EUROPA {

  DefaultPropagator::DefaultPropagator(const LabelStr& name, const ConstraintEngineId& constraintEngine)
    : Propagator(name, constraintEngine), m_activeConstraint(0){}

  void DefaultPropagator::handleConstraintAdded(const ConstraintId& constraint){
    m_agenda.insert(constraint);
  }

  void DefaultPropagator::handleConstraintRemoved(const ConstraintId& constraint){
    // Remove from agenda
    m_agenda.erase(constraint);
    check_error(isValid());
  }

  void DefaultPropagator::handleConstraintActivated(const ConstraintId& constraint){
    m_agenda.insert(constraint);
    check_error(isValid());
  }

  void DefaultPropagator::handleConstraintDeactivated(const ConstraintId& constraint){
    // Remove from agenda
    m_agenda.erase(constraint);
    check_error(isValid());
  }

  void DefaultPropagator::handleNotification(const ConstrainedVariableId& variable, 
					     int argIndex, 
					     const ConstraintId& constraint, 
					     const DomainListener::ChangeType& changeType){
    checkError(!constraint->isDiscarded(), constraint);
    if(constraint->getKey() != m_activeConstraint)
      m_agenda.insert(constraint);
  }

  void DefaultPropagator::execute(){
    checkError(!m_agenda.empty(), "Should never be calling this with an empty agenda.");
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

  bool DefaultPropagator::isValid() const{
    for(ConstraintSet::iterator it = m_agenda.begin(); it != m_agenda.end(); ++it){
      ConstraintId constraint = *it;
      checkError(constraint.isValid(), constraint);
      checkError(!constraint->isDiscarded(), constraint->getName().toString() << "(" << constraint->getKey() << ")");
    }
    return true;
  }
}
