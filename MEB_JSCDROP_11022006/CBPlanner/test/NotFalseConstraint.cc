#include "NotFalseConstraint.hh"
#include "BoolDomain.hh"

namespace EUROPA {

  NotFalseConstraint::NotFalseConstraint(const LabelStr& name, const LabelStr& propagatorName, const ConstraintEngineId& constraintEngine, const std::vector<ConstrainedVariableId>& variables) : Constraint(name, propagatorName, constraintEngine, variables) {
    check_error (variables.size() == 1); // temporary while Factories are fixed
    check_error (getCurrentDomain(variables[X]).isBool());
  }

  void NotFalseConstraint::handleExecute() {
    AbstractDomain& domx = getCurrentDomain(m_variables[X]);
    const AbstractDomain& baseDomx = m_variables[X]->baseDomain();

    if (domx.isOpen())
      return;
    check_error(!domx.isEmpty());

    if (domx.isSingleton() && (baseDomx.getLowerBound() == domx.getSingletonValue())) {
      domx.empty();
      return;
    }
  }

}
