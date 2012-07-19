#ifndef _H_LoraxConstraints
#define _H_LoraxConstraints

#include "ConstraintEngineDefs.hh"
#include "Constraint.hh"
#include "Variable.hh"
#include "IntervalDomain.hh"
#include "IntervalIntDomain.hh"
#include "BoolDomain.hh"

namespace EUROPA {

  class FinitePointConstraint : public Constraint {
  public:
    FinitePointConstraint(const LabelStr& name,
		       const LabelStr& propagatorName,
		       const ConstraintEngineId& constraintEngine,
		       const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

  private:
    static const int X = 0;
    static const int Y = 1;
    static const int ARG_COUNT = 2;
  };


  class DifferentPointConstraint : public Constraint {
  public:
    DifferentPointConstraint(const LabelStr& name,
		       const LabelStr& propagatorName,
		       const ConstraintEngineId& constraintEngine,
		       const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

  private:
    static const int X1 = 0;
    static const int Y1 = 1;
    static const int X2 = 2;
    static const int Y2 = 3;
    static const int ARG_COUNT = 4;
  };


  class DifferentConstraint : public Constraint {
  public:
    DifferentConstraint(const LabelStr& name,
		       const LabelStr& propagatorName,
		       const ConstraintEngineId& constraintEngine,
		       const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

  private:
    static const int V1 = 0;
    static const int V2 = 1;
    static const int ANS = 2;
    static const int ARG_COUNT = 3;
  };

  class AtLeastOneConstraint : public Constraint {
  public:
    AtLeastOneConstraint(const LabelStr& name,
		       const LabelStr& propagatorName,
		       const ConstraintEngineId& constraintEngine,
		       const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

  private:
    static const int V1 = 0;
    static const int V2 = 1;
    static const int ARG_COUNT = 2;
  };

  class SquareOfDifferenceConstraint : public Constraint {
  public:
    SquareOfDifferenceConstraint(const LabelStr& name,
		       const LabelStr& propagatorName,
		       const ConstraintEngineId& constraintEngine,
		       const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

  private:
    static const int V1 = 0;
    static const int V2 = 1;
    static const int RES = 2;
    static const int ARG_COUNT = 3;
  };

  class DistanceFromSquaresConstraint : public Constraint {
  public:
    DistanceFromSquaresConstraint(const LabelStr& name,
		       const LabelStr& propagatorName,
		       const ConstraintEngineId& constraintEngine,
		       const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

  private:
    static const int V1 = 0;
    static const int V2 = 1;
    static const int RES = 2;
    static const int ARG_COUNT = 3;
  };

  /**
     Constraints connecting required power and time with driving distance
   */
  class DriveBatteryConstraint : public Constraint {
  public:
    DriveBatteryConstraint(const LabelStr& name,
		       const LabelStr& propagatorName,
		       const ConstraintEngineId& constraintEngine,
		       const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

  private:
    static const int CHARGE = 0;
    static const int DISTANCE = 1;
    static const int ARG_COUNT = 2;
  };

  class DriveDurationConstraint : public Constraint {
  public:
    DriveDurationConstraint(const LabelStr& name,
		       const LabelStr& propagatorName,
		       const ConstraintEngineId& constraintEngine,
		       const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

  private:
    static const int DURATION = 0;
    static const int DISTANCE = 1;
    static const int ARG_COUNT = 2;
  };
  /**
     Constraints connecting required power and time with whatever for sampling
   */
  class SampleBatteryConstraint : public Constraint {
  public:
    SampleBatteryConstraint(const LabelStr& name,
		       const LabelStr& propagatorName,
		       const ConstraintEngineId& constraintEngine,
		       const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

  private:
    static const int CHARGE = 0;
    // there are also coordinates there, but we ignore them for now
    static const int ARG_COUNT = 3;
  };

  class SampleDurationConstraint : public Constraint {
  public:
    SampleDurationConstraint(const LabelStr& name,
		       const LabelStr& propagatorName,
		       const ConstraintEngineId& constraintEngine,
		       const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

  private:
    static const int DURATION = 0;
    // there are also coordinates there, but we ignore them for now
    static const int ARG_COUNT = 3;
  };

  /**
     Constraints connecting produced energy and wind power 
   */
  class WindPowerConstraint : public Constraint {
  public:
    WindPowerConstraint(const LabelStr& name,
		       const LabelStr& propagatorName,
		       const ConstraintEngineId& constraintEngine,
		       const std::vector<ConstrainedVariableId>& variables);

    void handleExecute();

  private:
    static const int CHARGE = 0;
    static const int VELOCITY = 1;
    static const int ARG_COUNT = 2;
  };


} // namespace EUROPA

#endif // _H_LoraxConstraints
