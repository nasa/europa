#ifndef _H_BinaryCustomConstraint
#define _H_BinaryCustomConstraint

#include "ConstraintEngineDefs.hh"
#include "Constraint.hh"
#include "Variable.hh"
#include "IntervalDomain.hh"
#include "IntervalIntDomain.hh"

namespace Prototype {

  class BinaryCustomConstraint : public Constraint {
  public:
    BinaryCustomConstraint(const LabelStr& name,
			   const LabelStr& propagatorName,
			   const ConstraintEngineId& constraintEngine,
			   const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

  private:
    static const int X = 0;
    static const int Y = 1;
  };

}
#endif
