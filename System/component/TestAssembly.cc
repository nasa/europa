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
//#include "CBPlanner.hh"
#include "ComponentFactory.hh"
#include "Solver.hh"
#include "OpenConditionDecisionPoint.hh"
#include "OpenConditionManager.hh"
#include "ThreatDecisionPoint.hh"
#include "ThreatManager.hh"
#include "UnboundVariableDecisionPoint.hh"
#include "UnboundVariableManager.hh"
#include "DecisionPoint.hh"
#include "MatchingRule.hh"
#include "Filters.hh"
#include "PartialPlanWriter.hh"
//#include "Horizon.hh"
//#include "DecisionManager.hh"
//#include "ResourceOpenDecisionManager.hh"

// Test Support
#include "PLASMAPerformanceConstraint.hh"
#include "LoraxConstraints.hh"
#include "TestSupport.hh"

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif
#include "tinyxml.h"

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

  bool TestAssembly::plan(const char* txSource, const char* config, const char* averFile) {
    TiXmlDocument doc(config);
    doc.LoadFile();
    return plan(txSource, *(doc.RootElement()), averFile);
  }

  bool TestAssembly::plan(const char* txSource, const TiXmlElement& config, const char* averFile){
    SOLVERS::SolverId solver = (new SOLVERS::Solver(m_planDatabase, config))->getId();

#ifdef PPW_WITH_PLANNER
    PlanWriter::PartialPlanWriter ppw(m_planDatabase, m_constraintEngine, m_rulesEngine, solver);
#else
    PlanWriter::PartialPlanWriter ppw(m_planDatabase, m_constraintEngine, m_rulesEngine);
#endif

    if(averFile != NULL) {
      AverInterp::init(std::string(averFile), solver, 
                       m_planDatabase->getConstraintEngine(), m_planDatabase, m_rulesEngine);
    }

    // Set up Resource Decision Manager
    //DecisionManagerId local_dm = planner.getDecisionManager();
    //ResourceOpenDecisionManagerId local_rodm = (new ResourceOpenDecisionManager(local_dm))->getId();
    //local_dm->setOpenDecisionManager(local_rodm);

    // Now process the transactions
    if(!playTransactions(txSource))
      return false;

    // Configure the planner from data in the initial state
    std::list<ObjectId> configObjects;
    m_planDatabase->getObjectsByType("PlannerConfig", configObjects); // Standard configuration class

    check_error(configObjects.size() == 1,
		"Expect exactly one instance of the class 'PlannerConfig'");

    ObjectId configSource = configObjects.front();
    check_error(configSource.isValid());

    const std::vector<ConstrainedVariableId>& variables = configSource->getVariables();
    check_error(variables.size() == 4, "Expecting exactly 4 configuration variables");

    // Set up the horizon  from the model now. Will cause a refresh of the query, but that is OK.
    ConstrainedVariableId horizonStart = variables[0];
    ConstrainedVariableId horizonEnd = variables[1];
    ConstrainedVariableId plannerSteps = variables[2];
    ConstrainedVariableId maxDepth = variables[3];

    int start = (int) horizonStart->baseDomain().getSingletonValue();
    int end = (int) horizonEnd->baseDomain().getSingletonValue();
    SOLVERS::HorizonFilter::getHorizon() = IntervalDomain(start, end);

    // Now get planner step max
    int steps = (int) plannerSteps->baseDomain().getSingletonValue();
    int depth = (int) maxDepth->baseDomain().getSingletonValue();

    bool retval = solver->solve(steps, depth);
    
    m_totalNodes = solver->getStepCount();
    m_finalDepth = solver->getDepth();

    if(averFile != NULL)
      AverInterp::terminate();
    
    delete (SOLVERS::Solver*) solver;

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
