#include "UnaryConstraint.hh"

namespace Prototype{

  UnaryConstraint::UnaryConstraint(const LabelStr& name,
				   const LabelStr& propagatorName,
				   const ConstraintEngineId& constraintEngine,
				   const ConstrainedVariableId& variable)
    : Constraint(name, propagatorName, constraintEngine, variable){}
}
