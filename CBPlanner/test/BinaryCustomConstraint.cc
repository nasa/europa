#include "BinaryCustomConstraint.hh"
#include "BoolDomain.hh"

namespace EUROPA {

  BinaryCustomConstraint::BinaryCustomConstraint(const LabelStr& name, const LabelStr& propagatorName, const ConstraintEngineId& constraintEngine, const std::vector<ConstrainedVariableId>& variables) : Constraint(name, propagatorName, constraintEngine, variables) {
    check_error (variables.size() == 2); // temporary while Factories are fixed
    check_error (getCurrentDomain(variables[X]).getType() == AbstractDomain::INT_INTERVAL);
    check_error (getCurrentDomain(variables[Y]).getType() == AbstractDomain::BOOL);
  }

  void BinaryCustomConstraint::handleExecute() {
    AbstractDomain& domx = getCurrentDomain(m_variables[X]);
    AbstractDomain& domy = getCurrentDomain(m_variables[Y]);
    const AbstractDomain& baseDomx = m_variables[X]->baseDomain();

    if (domx.isOpen())
      return;
    if (domy.isOpen())
      return;
    check_error(!domx.isEmpty());
    check_error(!domy.isEmpty());

    if (domy.isSingleton() && (domx.isSingleton() && (baseDomx.getLowerBound() == domx.getSingletonValue()))) 
      domy.empty();
  }

}
