#ifndef _H_TemporalConstraints
#define _H_TemporalConstraints

#include "Constraints.hh"
#include "Variable.hh"
#include "IntervalDomain.hh"
#include "IntervalIntDomain.hh"
#include "BoolDomain.hh"

namespace EUROPA {

  class TemporalDistanceConstraint : public AddEqualConstraint {
  public:
    TemporalDistanceConstraint(const LabelStr& name,
		       const LabelStr& propagatorName,
		       const ConstraintEngineId& constraintEngine,
		       const std::vector<ConstrainedVariableId>& variables);

    enum Vars { SRC_VAR_INDEX = 0,
		DISTANCE_VAR_INDEX,
		DEST_VAR_INDEX};

  };

  class PrecedesConstraint : public LessThanEqualConstraint {
  public:
    PrecedesConstraint(const LabelStr& name,
		    const LabelStr& propagatorName,
		    const ConstraintEngineId& constraintEngine,
		    const std::vector<ConstrainedVariableId>& variables);

    enum Vars { SRC_VAR_INDEX = 0,
		DEST_VAR_INDEX};

  };

  class ConcurrentConstraint : public EqualConstraint {
  public:
    ConcurrentConstraint(const LabelStr& name,
		    const LabelStr& propagatorName,
		    const ConstraintEngineId& constraintEngine,
		    const std::vector<ConstrainedVariableId>& variables);

    enum Vars { SRC_VAR_INDEX = 0,
		DEST_VAR_INDEX};

  };

}

#endif
