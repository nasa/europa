// Include prototypes required to integrate to the NDDL generated model
#include "Nddl.hh"

// Support for registered constraints
#include "../ConstraintEngine/ConstraintLibrary.hh"
#include "../ConstraintEngine/Constraints.hh"
#include "../ConstraintEngine/EqualityConstraintPropagator.hh"
#include "../PlanDatabase/ObjectTokenRelation.hh"

// Support fro required plan database components
#include "../PlanDatabase/PlanDatabase.hh"
#include "../PlanDatabase/RulesEngine.hh"
#include "../PlanDatabase/Schema.hh"
#include "../ConstraintEngine/ConstraintEngine.hh"

// Access for registered event loggers for instrumentation
#include "../ConstraintEngine/CeLogger.hh"
#include "../PlanDatabase/DbLogger.hh"

// Utility for obtaining defualt constraint library registration
#include "../ConstraintEngine/TestSupport.hh"

// Support for flaws and queries
#include "../PlanIdentification/FlawSource.hh"
#include "../PlanIdentification/FlawSourceLogger.hh"
#include "../PlanIdentification/Condition.hh"
#include "../PlanIdentification/Horizon.hh"
#include "../PlanIdentification/HorizonCondition.hh"
#include "../PlanIdentification/TemporalVariableCondition.hh"
#include "../PlanIdentification/DynamicInfiniteRealCondition.hh"
#include "../PlanIdentification/FilterCriteria.hh"
#include "../PlanIdentification/FlawQuery.hh"
#include "../PlanIdentification/FlawQueryLogger.hh"

// Support for planner
#include "CBPlanner.hh"
#include "DecisionPoint.hh"

int main(){
  // Initialize constraints
  initConstraintLibrary();
  
  REGISTER_NARY(EqualConstraint, "concurrent", "Default");
  REGISTER_NARY(EqualConstraint, "eq", "Default");
  REGISTER_NARY(LessThanEqualConstraint, "leq", "Default");
  REGISTER_NARY(LessThanEqualConstraint, "before", "Default");
  REGISTER_NARY(AddEqualConstraint, "StartEndDurationRelation", "Default");
  REGISTER_NARY(ObjectTokenRelation, "ObjectTokenRelation", "Default");
  REGISTER_UNARY(SubsetOfConstraint, "Singleton", "Default");
  REGISTER_NARY(LessThanEqualConstraint, "precede", "Default");

  
  // Allocate the schema
  SchemaId schema = NDDL::schema();
  ConstraintEngine ce;
  PlanDatabase db(ce.getId(), schema);

  new DefaultPropagator(LabelStr("Default"), ce.getId());
  RulesEngine re(db.getId());

  FlawSource source(FlawSource(db.getId()));
  Horizon hor(0,200);
  HorizonCondition hcond(hor.getId());
  TemporalVariableCondition tcond(hor.getId());
  DynamicInfiniteRealCondition dcond;
  std::list<ConditionId> conditions;
  conditions.push_back(hcond.getId());
  conditions.push_back(tcond.getId());
  conditions.push_back(dcond.getId());
  FilterCriteria filter(conditions);
  FlawQuery query(source.getId(),filter.getId());
  if (loggingEnabled()) {
    new CeLogger(std::cout, ce.getId());
    new DbLogger(std::cout, db.getId());
    new FlawSourceLogger(std::cout, source.getId());
    new FlawQueryLogger(std::cout, query.getId());
  }
  
  NDDL::initialize(db.getId());

  CBPlanner planner(db.getId(),query.getId(),100);
    
  assert(planner.run(loggingEnabled()) == 1);

  const std::list<DecisionPointId>& closed = planner.getClosedDecisions();
    
  std::cout << "Nr of Decisions = " << closed.size() << std::endl;
    
  std::cout << "Finished" << std::endl;
}
