// Include prototypes required to integrate to the NDDL generated model
#include "Nddl.hh"

// Support for registered constraints
#include "../ConstraintEngine/ConstraintLibrary.hh"
#include "../ConstraintEngine/Constraints.hh"
#include "../ConstraintEngine/EqualityConstraintPropagator.hh"
#include "../PlanDatabase/ObjectTokenRelation.hh"
#include "../PlanDatabase/CommonAncestorConstraint.hh"
#include "../PlanDatabase/HasAncestorConstraint.hh"

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

#include "../Resource/Resource.hh"
#include "../Resource/ResourceConstraint.hh"
#include "../Resource/ResourceDefs.hh"
#include "../Resource/ResourceTransactionConstraint.hh"
#include "../Resource/ResourcePropagator.hh"
#include "../Resource/Transaction.hh"

// Support for planner
#include "CBPlanner.hh"
#include "DecisionPoint.hh"
#include "EUROPAHeuristicStrategy.hh"

// For testing planner decisions
#include "PlannerDecisionWriter.hh"
#include "DecisionReplayer.hh"

// In case we want to print out the plan database
#include "PlanDatabaseWriter.hh"

// Transactions
#include "../PlanDatabase/DbClientTransactionLog.hh"

// Support for Temporal Network
#include "TemporalNetwork.hh"
#include "TemporalPropagator.hh"
#include "TemporalAdvisor.hh"
#include "STNTemporalAdvisor.hh"

// For cleanup purging
#include "../PlanDatabase/TokenFactory.hh"
#include "../PlanDatabase/ObjectFactory.hh"
#include "../RulesEngine/Rule.hh"

#include <fstream>

SchemaId schema;

//#define REPLAY_DECISIONS

class PlanSystem {
public:
  ConstraintEngineId constraintEngine;
  PlanDatabaseId planDatabase;
  RulesEngineId rulesEngine;
  FlawSourceId flawSource;

  HorizonId horizon;
  ConditionId horizonCondition;
  ConditionId temporalVariableCondition;
  ConditionId dynamicInfiniteRealCondition;
  std::list<ConditionId> conditions;
  FilterCriteriaId filterCriteria;
  FlawQueryId flawQuery;

  PlanSystem() {
    constraintEngine = (new ConstraintEngine())->getId();
    // order here is important.
    new DefaultPropagator(LabelStr("Default"), constraintEngine);
    new TemporalPropagator(LabelStr("Temporal"), constraintEngine);
    new ResourcePropagator(LabelStr("Resource"), constraintEngine);

    PropagatorId temporalPropagator = constraintEngine->getPropagatorByName(LabelStr("Temporal"));

    planDatabase = (new PlanDatabase(constraintEngine, schema))->getId();
    planDatabase->setTemporalAdvisor((new STNTemporalAdvisor(temporalPropagator))->getId());

    rulesEngine = (new RulesEngine(planDatabase))->getId();
    flawSource = (new FlawSource(planDatabase))->getId();
    horizon = (new Horizon())->getId();
    horizonCondition = (new HorizonCondition(horizon))->getId();
    temporalVariableCondition = (new TemporalVariableCondition(horizon))->getId();
    dynamicInfiniteRealCondition = (new DynamicInfiniteRealCondition())->getId();
    conditions.push_back(horizonCondition);
    conditions.push_back(temporalVariableCondition);
    conditions.push_back(dynamicInfiniteRealCondition);
    filterCriteria = (new FilterCriteria(conditions))->getId();
    flawQuery = (new FlawQuery(flawSource, filterCriteria))->getId();
  }
  ~PlanSystem() {
    planDatabase->purge();
    delete (FlawQuery*)flawQuery;
    delete (FilterCriteria*)filterCriteria;
    delete (Condition*)dynamicInfiniteRealCondition;
    delete (Condition*)temporalVariableCondition;
    delete (Condition*)horizonCondition;
    delete (Horizon*)horizon;
    delete (FlawSource*)flawSource;
    delete (RulesEngine*)rulesEngine;
    delete (PlanDatabase*)planDatabase;
  }
};


int main(int argc, const char ** argv){
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
  REGISTER_NARY(HasAncestorConstraint, "hasAncestor", "Default");
  REGISTER_NARY(ResourceConstraint, "ResourceRelation", "Resource");
  REGISTER_NARY(ResourceTransactionConstraint, "HorizonRelation", "Default");

  // Allocate the schema
  schema = NDDL::schema();

  std::string closedDecisions;

  // Compute or load the closed decisions
  if (argc == 1) {
    PlanSystem planSystem;

    DbClientTransactionLog * transactions = new DbClientTransactionLog(planSystem.planDatabase->getClient());

    if (loggingEnabled()) {
      new CeLogger(std::cout, planSystem.constraintEngine);
      new DbLogger(std::cout, planSystem.planDatabase);
      new FlawSourceLogger(std::cout, planSystem.flawSource);
      new FlawQueryLogger(std::cout, planSystem.flawQuery);
      new PlanWriter::PartialPlanWriter(planSystem.planDatabase, planSystem.constraintEngine);
    }
  
    NDDL::initialize(planSystem.planDatabase);

    // Set up the horizon  from the model now. Will cause a refresh of the query, but that is OK.
    std::list<ObjectId> objects;
    planSystem.planDatabase->getObjectsByType(LabelStr("World"), objects);
    ObjectId world = objects.front();
    check_error(objects.size() == 1);
    ConstrainedVariableId horizonStart = world->getVariable(LabelStr("world.m_horizonStart"));
    check_error(horizonStart.isValid());
    ConstrainedVariableId horizonEnd = world->getVariable(LabelStr("world.m_horizonEnd"));
    check_error(horizonEnd.isValid());
    int start = (int) horizonStart->baseDomain().getSingletonValue();
    int end = (int) horizonEnd->baseDomain().getSingletonValue();
    planSystem.horizon->setHorizon(start, end);

    // Create and run the planner
    ConstrainedVariableId maxPlannerSteps = world->getVariable(LabelStr("world.m_maxPlannerSteps"));
    check_error(maxPlannerSteps.isValid());
    int steps = (int) maxPlannerSteps->baseDomain().getSingletonValue();
    CBPlanner planner(planSystem.planDatabase, planSystem.flawQuery, steps);
    EUROPAHeuristicStrategy strategy;
      
    int res = planner.run(strategy.getId(), loggingEnabled());
    //int res = planner.run(strategy.getId(), true);
    check_error(res == 1);
	
    const std::list<DecisionPointId>& closed = planner.getClosedDecisions();
      
    std::cout << "Nr of Decisions = " << closed.size() << std::endl;
      
    // planner decisions
    std::cout << "Saving Planner Decisions..." << std::endl;
    PlannerDecisionWriter decisionWriter(planner.getId(), planSystem.planDatabase);
    closedDecisions = decisionWriter.closedDecisionsString();

    // save decisions
    std::string filename(argv[0]);
    filename += "-decisions.xml";
    std::ofstream out(filename.c_str());
    out << closedDecisions.c_str() << std::endl;
    out.close();

    // save transactions
    std::string filename2(argv[0]);
    filename2 += "-transactions.xml";
    std::ofstream out2(filename2.c_str());
    transactions->flush(out2);
    out2.close();

    std::cout << "Plan Database:" << std::endl;
    PlanDatabaseWriter::write(planSystem.planDatabase, std::cout);
  } else {
    // load the planner decisions from the file
    std::string filename(argv[0]);
    filename += "-decisions.xml";
    std::ifstream in(filename.c_str());
    std::string readbuf;
    while (in.good()) {
      getline(in, readbuf);
      closedDecisions += readbuf;
    }
  }

#ifdef REPLAY_DECISIONS
  {
    // replay the planner decisions
    PlanSystem planSystem;

    if (loggingEnabled()) {
      new CeLogger(std::cout, planSystem.constraintEngine);
      new DbLogger(std::cout, planSystem.planDatabase);
      new FlawSourceLogger(std::cout, planSystem.flawSource);
      new FlawQueryLogger(std::cout, planSystem.flawQuery);
      new PlanWriter::PartialPlanWriter(planSystem.planDatabase, planSystem.constraintEngine);
    }
  
    int initial_key = NDDL::initialize(planSystem.planDatabase);

    // Set up the horizon  from the model now. Will cause a refresh of the query, but that is OK.
    std::list<ObjectId> objects;
    planSystem.planDatabase->getObjectsByType(LabelStr("World"), objects);
    ObjectId world = objects.front();
    check_error(objects.size() == 1);
    ConstrainedVariableId horizonStart = world->getVariable(LabelStr("world.m_horizonStart"));
    check_error(horizonStart.isValid());
    ConstrainedVariableId horizonEnd = world->getVariable(LabelStr("world.m_horizonEnd"));
    check_error(horizonEnd.isValid());
    int start = (int) horizonStart->baseDomain().getSingletonValue();
    int end = (int) horizonEnd->baseDomain().getSingletonValue();
    planSystem.horizon->setHorizon(start, end);

    // planner decisions
    std::cout << "Replaying Planner Decisions..." << std::endl;
    DecisionReplayer replayer(planSystem.planDatabase, initial_key);
    replayer.replay(closedDecisions);

    std::cout << "Replay Database:" << std::endl;
    PlanDatabaseWriter::write(planSystem.planDatabase, std::cout);
  }
#endif // REPLAY_DECISIONS

  ObjectFactory::purgeAll();
  TokenFactory::purgeAll();
  ConstraintLibrary::purgeAll();
	
  std::cout << "Finished" << std::endl;
}
