// Include prototypes required to integrate to the NDDL generated model
#include "Nddl.hh"

// Support for registered constraints
#include "../ConstraintEngine/ConstraintLibrary.hh"
#include "../ConstraintEngine/Constraints.hh"
#include "../ConstraintEngine/EqualityConstraintPropagator.hh"
#include "../PlanDatabase/ObjectTokenRelation.hh"
#include "../PlanDatabase/CommonAncestorConstraint.hh"

// Support fro required plan database components
#include "../PlanDatabase/PlanDatabase.hh"
#include "../PlanDatabase/Object.hh"
#include "../PlanDatabase/Schema.hh"
#include "../ConstraintEngine/ConstraintEngine.hh"
#include "../PlanDatabase/PartialPlanWriter.hh"

// Rules Engine Components
#include "../RulesEngine/RulesEngine.hh"

// Access for registered event loggers for instrumentation
#include "../ConstraintEngine/CeLogger.hh"
#include "../PlanDatabase/DbLogger.hh"
#include "../PlanDatabase/PartialPlanWriter.hh"

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
#include "HeuristicStrategy.hh"

// Support for Temporal Network
#include "TemporalNetwork.hh"
#include "TemporalPropagator.hh"
#include "TemporalAdvisor.hh"
#include "STNTemporalAdvisor.hh"

int main(){
  //REGISTER_NARY(EqualConstraint, "concurrent", "Default");
  REGISTER_NARY(EqualConstraint, "concurrent", "Temporal");
  REGISTER_NARY(EqualConstraint, "eq", "Default");
  REGISTER_NARY(NotEqualConstraint, "neq", "Default");
  REGISTER_NARY(LessThanEqualConstraint, "leq", "Default");
  //REGISTER_NARY(LessThanEqualConstraint, "before", "Default");
  REGISTER_NARY(LessThanEqualConstraint, "before", "Temporal");
  //REGISTER_NARY(AddEqualConstraint, "StartEndDurationRelation", "Default");
  REGISTER_NARY(AddEqualConstraint, "StartEndDurationRelation", "Temporal");
  REGISTER_NARY(ObjectTokenRelation, "ObjectTokenRelation", "Default");
  REGISTER_UNARY(SubsetOfConstraint, "Singleton", "Default");
  REGISTER_NARY(CommonAncestorConstraint, "commonAncestor", "Default");

  
  // Allocate the schema
  SchemaId schema = NDDL::schema();
  ConstraintEngine ce;

  // order here is important.
  new DefaultPropagator(LabelStr("Default"), ce.getId());
  new TemporalPropagator(LabelStr("Temporal"), ce.getId());

  PlanDatabase db(ce.getId(), schema);
  db.setTemporalAdvisor((new STNTemporalAdvisor(ce.getPropagatorByName(LabelStr("Temporal"))))->getId());

  RulesEngine re(db.getId());

  FlawSource source(FlawSource(db.getId()));
  Horizon hor;
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
    new PlanWriter::PartialPlanWriter(db.getId(), ce.getId());
  }
  
  NDDL::initialize(db.getId());

  // Set up the horizon  from the model now. Will cause a refresh of the query, but that is OK.
  std::list<ObjectId> objects;
  db.getObjectsByType(LabelStr("World"), objects);
  ObjectId world = objects.front();
  assert(objects.size() == 1);
  ConstrainedVariableId horizonStart = world->getVariable(LabelStr("world.m_horizonStart"));
  assert(horizonStart.isValid());
  ConstrainedVariableId horizonEnd = world->getVariable(LabelStr("world.m_horizonEnd"));
  assert(horizonEnd.isValid());
  int start = (int) horizonStart->baseDomain().getSingletonValue();
  int end = (int) horizonEnd->baseDomain().getSingletonValue();
  hor.setHorizon(start, end);

  // Create and run the planner
  ConstrainedVariableId maxPlannerSteps = world->getVariable(LabelStr("world.m_maxPlannerSteps"));
  assert(maxPlannerSteps.isValid());
  int steps = (int) maxPlannerSteps->baseDomain().getSingletonValue();
  CBPlanner planner(db.getId(),query.getId(),steps);
  HeuristicStrategyId strategy = (new HeuristicStrategy())->getId();
    
  assert(planner.run(strategy, loggingEnabled()) == 1);

  const std::list<DecisionPointId>& closed = planner.getClosedDecisions();
    
  std::cout << "Nr of Decisions = " << closed.size() << std::endl;
    
  std::cout << "Finished" << std::endl;
}
