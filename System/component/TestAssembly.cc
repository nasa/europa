#include "TestAssembly.hh"

// Support for required major plan database components
#include "PlanDatabase.hh"
#include "PlanDatabaseWriter.hh"
#include "ConstraintEngine.hh"
#include "RulesEngine.hh"
#include "DefaultPropagator.hh"

#include "Resource.hh"
#include "ResourceConstraint.hh"
#include "ResourceDefs.hh"
#include "ResourcePropagator.hh"
#include "Transaction.hh"

// Transactions
#include "DbClientTransactionPlayer.hh"
#include "BoolTypeFactory.hh"
#include "StringTypeFactory.hh"
#include "floatType.hh"
#include "intType.hh"

// Support for Temporal Network
#include "TemporalPropagator.hh"
#include "STNTemporalAdvisor.hh"
#include "TemporalConstraints.hh"

// Support for registered constraints
#include "ConstraintLibrary.hh"
#include "Constraints.hh"
#include "EqualityConstraintPropagator.hh"
#include "ObjectTokenRelation.hh"
#include "CommonAncestorConstraint.hh"
#include "HasAncestorConstraint.hh"

#include "NddlDefs.hh"

// For cleanup purging
#include "TokenFactory.hh"
#include "ObjectFactory.hh"
#include "Rule.hh"

// Misc
#include "Utils.hh"

// Planner Support
#include "CBPlanner.hh"
#include "PartialPlanWriter.hh"
#include "Horizon.hh"
#include "DecisionManager.hh"
#include "ResourceOpenDecisionManager.hh"

// Test Support
#include "PLASMAPerformanceConstraint.hh"
#include "LoraxConstraints.hh"
#include "TestSupport.hh"

#include <string>
#include <fstream>

#include "AverInterp.hh"

#define PPW_WITH_PLANNER

const char* TX_LOG = "TransactionLog.xml";

namespace EUROPA {

  TestAssembly::TestAssembly(const SchemaId& schema) : StandardAssembly(schema) { }

  TestAssembly::~TestAssembly() {}

  /**
   * @brief Sets up the necessary constraint factories
   */
  void TestAssembly::initialize() {
    StandardAssembly::initialize();
    /*

    // Temporal Constraints
    REGISTER_CONSTRAINT(ConcurrentConstraint, "concurrent", "Temporal");
    REGISTER_CONSTRAINT(PrecedesConstraint, "precedes", "Temporal"); 
    REGISTER_CONSTRAINT(TemporalDistanceConstraint, "StartEndDurationRelation", "Temporal");
    REGISTER_CONSTRAINT(TemporalDistanceConstraint, "temporaldistance", "Temporal");

    initConstraintEngine();

    // Procedural Constraints used with Default Propagation
    REGISTER_CONSTRAINT(EqualConstraint, "eq", "Default");
    REGISTER_CONSTRAINT(NotEqualConstraint, "neq", "Default");
    REGISTER_CONSTRAINT(LessThanEqualConstraint, "leq", "Default");
    REGISTER_CONSTRAINT(AddEqualConstraint, "addEq", "Default");
    REGISTER_CONSTRAINT(NegateConstraint, "neg", "Default");
    REGISTER_CONSTRAINT(MultEqualConstraint, "mulEq", "Default");
    REGISTER_CONSTRAINT(SubsetOfConstraint, "subsetOf", "Default");
    REGISTER_CONSTRAINT(ObjectTokenRelation, "ObjectTokenRelation", "Default");
    REGISTER_CONSTRAINT(SubsetOfConstraint, "Singleton", "Default");
    REGISTER_CONSTRAINT(LockConstraint, "Lock", "Default");
    REGISTER_CONSTRAINT(CommonAncestorConstraint, "commonAncestor", "Default");
    REGISTER_CONSTRAINT(HasAncestorConstraint, "hasAncestor", "Default");

    // The only resource specific constraints
    REGISTER_CONSTRAINT(ResourceConstraint, "ResourceRelation", "Resource");
    REGISTER_CONSTRAINT(ResourceConstraint, "ResourceTransactionRelation", "Default");

    REGISTER_CONSTRAINT(AddEqualConstraint, "addEq", "Default");
    REGISTER_CONSTRAINT(ObjectTokenRelation, "ObjectTokenRelation", "Default");
    REGISTER_CONSTRAINT(SubsetOfConstraint, "Singleton", "Default");
    REGISTER_CONSTRAINT(CommonAncestorConstraint, "commonAncestor", "Default");
    REGISTER_CONSTRAINT(HasAncestorConstraint, "hasAncestor", "Default");
    REGISTER_CONSTRAINT(ResourceConstraint, "ResourceRelation", "Resource");
    REGISTER_CONSTRAINT(ResourceConstraint, "ResourceTransactionRelation", "Default");
    REGISTER_CONSTRAINT(SubsetOfConstraint, "subsetOf", "Default");
    */
    REGISTER_CONSTRAINT(PLASMAPerformanceConstraint, "perf", "Default");

    // LoraxConstraints for some of the resources tests.
    REGISTER_CONSTRAINT(SquareOfDifferenceConstraint, "diffSquare", "Default");
    REGISTER_CONSTRAINT(DistanceFromSquaresConstraint, "distanceSquares", "Default");
    REGISTER_CONSTRAINT(DriveBatteryConstraint, "driveBattery", "Default");
    REGISTER_CONSTRAINT(DriveDurationConstraint, "driveDuration", "Default");
    REGISTER_CONSTRAINT(WindPowerConstraint, "windPower", "Default");
    REGISTER_CONSTRAINT(SampleBatteryConstraint, "sampleBattery", "Default");
    REGISTER_CONSTRAINT(SampleDurationConstraint, "sampleDuration", "Default");

    isInitialized() = true;
  }

  CBPlanner::Status TestAssembly::plan(const char* txSource, const char* averFile){
    Horizon horizon;
    CBPlanner planner(m_planDatabase, horizon.getId());

#ifdef PPW_WITH_PLANNER
    PlanWriter::PartialPlanWriter ppw(m_planDatabase, m_constraintEngine, m_rulesEngine, planner.getId());
#else
    PlanWriter::PartialPlanWriter ppw(m_planDatabase, m_constraintEngine, m_rulesEngine);
#endif

    if(averFile != NULL) {
      AverInterp::init(std::string(averFile), planner.getDecisionManager(), 
                       m_planDatabase->getConstraintEngine(), m_planDatabase, m_rulesEngine);
    }

    // Set up Resource Decision Manager
    DecisionManagerId local_dm = planner.getDecisionManager();
    ResourceOpenDecisionManagerId local_rodm = (new ResourceOpenDecisionManager(local_dm))->getId();
    local_dm->setOpenDecisionManager(local_rodm);

    // Now process the transactions
    if(!playTransactions(txSource))
      return CBPlanner::INITIALLY_INCONSISTENT;

    // Configure the planner from data in the initial state
    std::list<ObjectId> configObjects;
    m_planDatabase->getObjectsByType("PlannerConfig", configObjects); // Standard configuration class

    check_error(configObjects.size() == 1,
		"Expect exactly one instance of the class 'PlannerConfig'");

    ObjectId configSource = configObjects.front();
    check_error(configSource.isValid());

    const std::vector<ConstrainedVariableId>& variables = configSource->getVariables();
    check_error(variables.size() == 3, 
		"Expecting exactly 3 configuration variables");

    // Set up the horizon  from the model now. Will cause a refresh of the query, but that is OK.
    ConstrainedVariableId horizonStart = variables[0];
    ConstrainedVariableId horizonEnd = variables[1];
    ConstrainedVariableId plannerSteps = variables[2];

    int start = (int) horizonStart->baseDomain().getSingletonValue();
    int end = (int) horizonEnd->baseDomain().getSingletonValue();
    horizon.setHorizon(start, end);

    // Now get planner step max
    int steps = (int) plannerSteps->baseDomain().getSingletonValue();

    CBPlanner::Status retval = planner.run(steps);
    
    m_totalNodes = planner.getTime();
    m_finalDepth = planner.getDepth();

    if(averFile != NULL)
      AverInterp::terminate();
    
    return retval;
  }

  void TestAssembly::replay(const DbClientTransactionLogId& txLog) {
    std::stringstream os1;
    m_planDatabase->getClient()->toStream(os1);
    std::ofstream out(TX_LOG);
    txLog->flush(out);
    out.close();

    std::stringstream os2;
    TestAssembly replayed(Schema::instance());
    replayed.playTransactions(TX_LOG);
    replayed.getPlanDatabase()->getClient()->toStream(os2);

    std::string s1 = os1.str();
    std::string s2 = os2.str();

    assert(s1 == s2);
  }

  const PlanDatabaseId& TestAssembly::getPlanDatabase() const {
    return m_planDatabase;
  }

  const unsigned int TestAssembly::getTotalNodesSearched() const { return m_totalNodes; }

  const unsigned int TestAssembly::getDepthReached() const { return m_finalDepth; }

}
