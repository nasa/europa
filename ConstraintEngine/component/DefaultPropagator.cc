#include "DefaultPropagator.hh"
#include "ConstraintEngine.hh"
#include "ConstrainedVariable.hh"
#include "Constraint.hh"

namespace Prototype {

  DefaultPropagator::DefaultPropagator(const LabelStr& name, const ConstraintEngineId& constraintEngine)
    : Propagator(name, constraintEngine){}

  void DefaultPropagator::handleConstraintAdded(const ConstraintId& constraint){
    m_agenda.push_back(constraint);
  }

  void DefaultPropagator::handleConstraintRemoved(const ConstraintId& constraint){
    // This is a relaxation, so we should be able to simply clear the agenda.
    m_agenda.clear();
  }

  void DefaultPropagator::handleConstraintActivated(const ConstraintId& constraint){
    m_agenda.push_back(constraint);
  }

  void DefaultPropagator::handleConstraintDeactivated(const ConstraintId& constraint){
    // This is a no-op
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
    check_error(m_agenda.size() > 0);
    check_error(!getConstraintEngine()->provenInconsistent());

    for(int i=0;i<m_agenda.size();i++){
      ConstraintId constraint = m_agenda[i];
      if(constraint->isActive())
	Propagator::execute(constraint);

      if(getConstraintEngine()->provenInconsistent())
	break;
    }

    m_agenda.clear();
  }

  bool DefaultPropagator::updateRequired() const{
    return (m_agenda.size() > 0);
  }
}
