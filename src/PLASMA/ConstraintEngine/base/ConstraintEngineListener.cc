#include "ConstraintEngineListener.hh"
#include "ConstraintEngine.hh"
#include "Entity.hh"

namespace EUROPA {

  ConstraintEngineListener::ConstraintEngineListener(const ConstraintEngineId constraintEngine)
    :m_id(this), m_constraintEngine(constraintEngine){
    check_error(m_constraintEngine.isValid());
    m_constraintEngine->add(m_id);
  }

ConstraintEngineListener::ConstraintEngineListener()
    : m_id(this), m_constraintEngine() {
}

  ConstraintEngineListener::~ConstraintEngineListener(){
    check_error(m_id.isValid());
    check_error(m_constraintEngine.isValid());
    m_constraintEngine->remove(m_id);
    m_id.remove();
  }

  const ConstraintEngineListenerId ConstraintEngineListener::getId() const {
    return(m_id);
 }

  void ConstraintEngineListener::setConstraintEngine(const ConstraintEngineId constraintEngine) {
	  m_constraintEngine = constraintEngine;
	  check_error(m_constraintEngine.isValid());
	  m_constraintEngine->add(m_id);
  }

void ConstraintEngineListener::notifyPropagationCommenced(){
}

void ConstraintEngineListener::notifyPropagationCompleted(){
}

void ConstraintEngineListener::notifyPropagationPreempted(){
}

void ConstraintEngineListener::notifyAdded(const ConstraintId){
}

void ConstraintEngineListener::notifyActivated(const ConstraintId){
}

void ConstraintEngineListener::notifyDeactivated(const ConstrainedVariableId){
}

void ConstraintEngineListener::notifyActivated(const ConstrainedVariableId){
}

void ConstraintEngineListener::notifyDeactivated(const ConstraintId){
}

void ConstraintEngineListener::notifyRemoved(const ConstraintId){
}

void ConstraintEngineListener::notifyExecuted(const ConstraintId){
}

void ConstraintEngineListener::notifyAdded(const ConstrainedVariableId){
}

void ConstraintEngineListener::notifyRemoved(const ConstrainedVariableId){
}

void ConstraintEngineListener::notifyChanged(const ConstrainedVariableId,
                                             const DomainListener::ChangeType&){
}

void ConstraintEngineListener::notifyViolationAdded(const ConstraintId){
}

void ConstraintEngineListener::notifyViolationRemoved(const ConstraintId){
}

}
