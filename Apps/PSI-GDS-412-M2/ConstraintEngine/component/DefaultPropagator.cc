#include "DefaultPropagator.hh"
#include "ConstraintEngine.hh"
#include "ConstrainedVariable.hh"
#include "Constraint.hh"
#include "Debug.hh"

namespace EUROPA {

  DefaultPropagator::DefaultPropagator(const LabelStr& name, const ConstraintEngineId& constraintEngine)
    : Propagator(name, constraintEngine), m_activeConstraint(0){}

  void DefaultPropagator::handleConstraintAdded(const ConstraintId& constraint){
    debugMsg("DefaultPropagator:handleConstraintAdded", "Adding to the agenda: " << constraint->getName().toString() << "(" << constraint->getKey() << ")");
    m_agenda.insert(constraint);
  }

  void DefaultPropagator::handleConstraintRemoved(const ConstraintId& constraint){
    // Remove from agenda
    debugMsg("DefaultPropagator:handleConstraintRemoved", "Removing from the agenda: " << constraint->getName().toString() << "(" << constraint->getKey() << ")");
    m_agenda.erase(constraint);
    check_error(isValid());
  }

  void DefaultPropagator::handleConstraintActivated(const ConstraintId& constraint){
    debugMsg("DefaultPropagator:handleConstraintActivated", "Adding to the agenda: " << constraint->getName().toString() << "(" << constraint->getKey() << ")");
    m_agenda.insert(constraint);
    check_error(isValid());
  }

  void DefaultPropagator::handleConstraintDeactivated(const ConstraintId& constraint){
    // Remove from agenda
    debugMsg("DefaultPropagator:handleConstraintDeactivated", "Removing from the agenda: " << constraint->getName().toString() << "(" << constraint->getKey() << ")");
    m_agenda.erase(constraint);
    check_error(isValid());
  }

  void DefaultPropagator::handleNotification(const ConstrainedVariableId& variable, 
					     int argIndex, 
					     const ConstraintId& constraint, 
					     const DomainListener::ChangeType& changeType){
    checkError(!constraint->isDiscarded(), constraint);
    if(constraint->getKey() != m_activeConstraint) {
      debugMsg("DefaultPropagator:handleNotification", 
          "Adding to the agenda: " << constraint->getName().toString() << "(" << constraint->getKey() << ")"
          << " because of " << DomainListener::toString(changeType) << " change to " << variable->toString()
      );
      m_agenda.insert(constraint);
    }        
  }

  void DefaultPropagator::execute(){
    checkError(!m_agenda.empty(), "Should never be calling this with an empty agenda.");
    check_error(!getConstraintEngine()->provenInconsistent());
    check_error(m_activeConstraint == 0);

    while(!m_agenda.empty() && 
          !getConstraintEngine()->provenInconsistent()){
      ConstraintSet::iterator it = m_agenda.begin();
      ConstraintId constraint = *it;
      m_agenda.erase(constraint);

      if(constraint->isActive()){
          m_activeConstraint = constraint->getKey();
          Propagator::execute(constraint);
      }
    }

    // If we can continue propagation despite the discovered inconsistency, 
    // keep agenda for when the ConstraintEngine recovers and decides to resume propagation
    if(getConstraintEngine()->provenInconsistent()) {
        if (getConstraintEngine()->canContinuePropagation()) {
	        debugMsg("DefaultPropagator:agenda","CE was proven inconsistent, keeping agenda because propagation can continue later");
	        // TODO: should remove from the agenda any constraints associated with the empty variable, since it'll be relaxed and they'll ba added again
        }
        else {
            m_agenda.clear();
	        debugMsg("DefaultPropagator:agenda","Cleared agenda because CE was proven inconsistent");
        }
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
      checkError(!constraint->isDiscarded(), 
		 constraint->getName().toString() << "(" << constraint->getKey() << ") Id=" << constraint);
    }
    return true;
  }
}
