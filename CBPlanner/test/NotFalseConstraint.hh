#ifndef _H_NotFalseConstraint
#define _H_NotFalseConstraint

#include "ConstraintEngineDefs.hh"
#include "Constraint.hh"
#include "Variable.hh"
#include "IntervalDomain.hh"
#include "IntervalIntDomain.hh"

namespace Prototype {

  class NotFalseConstraint : public Constraint {
  public:
    NotFalseConstraint(const LabelStr& name,
		       const LabelStr& propagatorName,
		       const ConstraintEngineId& constraintEngine,
		       const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

  private:
    static const int X = 0;
  };

}
#endif
