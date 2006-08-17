#ifndef _H_PLASMAPerformanceConstraint
#define _H_PLASMAPerformanceConstraint

#include "ConstraintEngineDefs.hh"
#include "Constraint.hh"
#include "Variable.hh"
#include "IntervalDomain.hh"
#include "IntervalIntDomain.hh"

namespace EUROPA {

  class PLASMAPerformanceConstraint : public Constraint {
  public:
    PLASMAPerformanceConstraint(const LabelStr& name,
				   const LabelStr& propagatorName,
				   const ConstraintEngineId& constraintEngine,
				   const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

  private:
    static const int X = 0;
  };

}
#endif
