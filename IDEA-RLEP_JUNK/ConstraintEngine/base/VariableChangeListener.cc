#include "VariableChangeListener.hh"
#include "Constraint.hh"
#include "ConstrainedVariable.hh"
#include "ConstraintEngine.hh"

namespace EUROPA {

  VariableChangeListener::VariableChangeListener(const ConstrainedVariableId& variable,
						 const ConstraintEngineId& constraintEngine)
    :DomainListener(), m_variable(variable), m_constraintEngine(constraintEngine){}

  void VariableChangeListener::notifyChange(const ChangeType& changeType){
    m_constraintEngine->notify(m_variable, changeType);
  }
}
