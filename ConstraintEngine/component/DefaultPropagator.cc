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
    // This is a relaxation, so we should be able to simply clear the agenda.
    //std::cout << "Constraint removed, clearing agenda" << std::endl;
    //m_agenda.clear();
  }

  void DefaultPropagator::handleConstraintActivated(const ConstraintId& constraint){
    m_agenda.insert(constraint);
  }

  void DefaultPropagator::handleConstraintDeactivated(const ConstraintId& constraint){
    // This is a no-op
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
      std::set<ConstraintId>::iterator it = m_agenda.begin();
      ConstraintId constraint = *it;

      if(constraint.isValid() && constraint->isActive()){
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
      /*
    for(int i=0;i<m_agenda.size();i++){
      ConstraintId constraint = m_agenda[i];
      if(constraint->isActive()){
	m_activeConstraint = constraint->getKey();
	Propagator::execute(constraint);
      }

      if(getConstraintEngine()->provenInconsistent())
	break;
    }
      */
    m_activeConstraint = 0;
  }

  bool DefaultPropagator::updateRequired() const{
    return (m_agenda.size() > 0);
  }
}
