#ifndef _H_UnaryConstraint
#define _H_UnaryConstraint

#include "Constraint.hh"

namespace Prototype{

  class UnaryConstraint: public Constraint{
  public:
    UnaryConstraint(const LabelStr& name,
		    const LabelStr& propagatorName,
		    const ConstraintEngineId& constraintEngine,
		    const ConstrainedVariableId& variable);

    virtual const AbstractDomain& getDomain() const = 0;
  };
}

#endif
