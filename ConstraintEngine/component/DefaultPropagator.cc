#include "DefaultPropagator.hh"
#include "ConstraintEngine.hh"
#include "ConstrainedVariable.hh"
#include "Constraint.hh"

namespace Prototype {

  DefaultPropagator::DefaultPropagator(const ConstraintEngineId& constraintEngine)
    : Propagator(constraintEngine){}

  void DefaultPropagator::handleConstraintAdded(const ConstraintId& constraint){
    m_agenda.push_back(constraint);
  }

  void DefaultPropagator::handleNotification(const ConstrainedVariableId& variable, 
					     int argIndex, 
					     const ConstraintId& constraint, 
					     const DomainListener::ChangeType& changeType){
    m_agenda.push_back(constraint);
  }

  bool DefaultPropagator::isAcceptable(const ConstraintId& constraint) const {
    check_error(constraint.isValid());
    return(!constraint.isNoId());
  }

  void DefaultPropagator::execute(){
    while(m_agenda.size() > 0  && !getConstraintEngine()->provenInconsistent()){
      ConstraintId constraint = m_agenda.front();
      m_agenda.pop_front();
      Propagator::execute(constraint);
    }

    check_error(m_agenda.size() == 0 || getConstraintEngine()->provenInconsistent());

    m_agenda.clear();
  }

  bool DefaultPropagator::updateRequired() const{
    return (m_agenda.size() > 0);
  }
}
