#include "PLASMAPerformanceConstraint.hh"

namespace EUROPA {

  PLASMAPerformanceConstraint::PLASMAPerformanceConstraint(const LabelStr& name, const LabelStr& propagatorName, const ConstraintEngineId& constraintEngine, const std::vector<ConstrainedVariableId>& variables) : Constraint(name, propagatorName, constraintEngine, variables) {
    check_error (variables.size() == 2); // temporary while Factories are fixed
    check_error (getCurrentDomain(variables[X]).isEnumerated()); // enum or
								 // interval
  }

  void PLASMAPerformanceConstraint::handleExecute() {
    AbstractDomain& domx = getCurrentDomain(m_variables[X]);
    const AbstractDomain& baseDomx = m_variables[X]->baseDomain();

    if (domx.isOpen())
      return;
    check_error(!domx.isEmpty());

    if (domx.isSingleton() && (baseDomx.getUpperBound() != domx.getSingletonValue())) {
      domx.empty();
    }
  }

}
