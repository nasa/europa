#include "TemporalConstraints.hh"
#include "PlanDatabaseDefs.hh"
#include "TokenVariable.hh"
#include "ConstrainedVariable.hh"
#include "IntervalIntDomain.hh"

namespace EUROPA {


  TemporalDistanceConstraint::TemporalDistanceConstraint(const LabelStr& name,
					 const LabelStr& propagatorName,
					 const ConstraintEngineId& constraintEngine,
					 const std::vector<ConstrainedVariableId>& variables)
    : AddEqualConstraint(name, propagatorName, constraintEngine, variables) {
    check_error(getCurrentDomain(variables[SRC_VAR_INDEX]).getType() == AbstractDomain::INT_INTERVAL);
    check_error(getCurrentDomain(variables[DEST_VAR_INDEX]).getType() == AbstractDomain::INT_INTERVAL);
    check_error(getCurrentDomain(variables[DISTANCE_VAR_INDEX]).getType() == AbstractDomain::INT_INTERVAL);
  }


  PrecedesConstraint::PrecedesConstraint(const LabelStr& name,
				   const LabelStr& propagatorName,
				   const ConstraintEngineId& constraintEngine,
				   const std::vector<ConstrainedVariableId>& variables)
    : LessThanEqualConstraint(name, propagatorName, constraintEngine, variables) {
    check_error(getCurrentDomain(variables[SRC_VAR_INDEX]).getType() == AbstractDomain::INT_INTERVAL);
    check_error(getCurrentDomain(variables[DEST_VAR_INDEX]).getType() == AbstractDomain::INT_INTERVAL);
  }

  ConcurrentConstraint::ConcurrentConstraint(const LabelStr& name,
				   const LabelStr& propagatorName,
				   const ConstraintEngineId& constraintEngine,
				   const std::vector<ConstrainedVariableId>& variables)
    : EqualConstraint(name, propagatorName, constraintEngine, variables) {
    check_error(getCurrentDomain(variables[SRC_VAR_INDEX]).getType() == AbstractDomain::INT_INTERVAL);
    check_error(getCurrentDomain(variables[DEST_VAR_INDEX]).getType() == AbstractDomain::INT_INTERVAL);
  }

}
