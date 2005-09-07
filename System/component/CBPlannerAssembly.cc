#include "CBPlannerAssembly.hh"

// Support for required major plan database components
#include "PlanDatabase.hh"
#include "PlanDatabaseWriter.hh"
#include "ConstraintEngine.hh"
#include "RulesEngine.hh"

// Transactions
#include "DbClientTransactionPlayer.hh"

#include "NddlDefs.hh"

// Misc
#include "Utils.hh"

// Planner Support
#include "CBPlanner.hh"
#include "PartialPlanWriter.hh"
#include "Horizon.hh"
#include "DecisionManager.hh"
#include "MasterMustBeInserted.hh"
#include "TemporalVariableFilter.hh"

// Test Support
#include "PLASMAPerformanceConstraint.hh"
#include "LoraxConstraints.hh"
#include "TestSupport.hh"

#include <string>

#include "AverInterp.hh"

#define PPW_WITH_PLANNER

namespace EUROPA {

  const char* CBPlannerAssembly::TX_LOG() {
    static const char* sl_txLog = "TransactionLog.xml";
    return sl_txLog;
  }

  CBPlannerAssembly::CBPlannerAssembly(const SchemaId& schema) : StandardAssembly(schema) { }

  CBPlannerAssembly::~CBPlannerAssembly() {}

  /**
   * @brief Sets up the necessary constraint factories
   */
  void CBPlannerAssembly::initialize() {
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

  bool CBPlannerAssembly::plan(const char* txSource, const TiXmlElement&, const char* averFile){
    Horizon horizon;
    CBPlanner planner(m_planDatabase, horizon.getId());
    /*
#ifdef PPW_WITH_PLANNER
    PlanWriter::PartialPlanWriter ppw(m_planDatabase, m_constraintEngine, m_rulesEngine, planner.getId());
#else
    PlanWriter::PartialPlanWriter ppw(m_planDatabase, m_constraintEngine, m_rulesEngine);
#endif
    */
    if(averFile != NULL) {
      AverInterp::init(std::string(averFile), planner.getDecisionManager(), 
                       m_planDatabase->getConstraintEngine(), m_planDatabase, m_rulesEngine);
    }

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
    check_error(variables.size() == 4, 
		"Expecting exactly 4 configuration variables");

    // Set up the horizon  from the model now. Will cause a refresh of the query, but that is OK.
    ConstrainedVariableId horizonStart = variables[0];
    ConstrainedVariableId horizonEnd = variables[1];
    ConstrainedVariableId plannerSteps = variables[2];

    int start = (int) horizonStart->baseDomain().getSingletonValue();
    int end = (int) horizonEnd->baseDomain().getSingletonValue();
    horizon.setHorizon(start, end);

    // Now get planner step max
    int steps = (int) plannerSteps->baseDomain().getSingletonValue();

    // Add the MasterMustBeInserted condition
    MasterMustBeInserted condition1(planner.getDecisionManager());

    // Filter all temporal variables using a static filter
    TemporalVariableFilter condition2(planner.getDecisionManager());

    CBPlanner::Status retval = planner.run(steps);
    
    m_totalNodes = planner.getTime();
    m_finalDepth = planner.getDepth();

    if(averFile != NULL)
      AverInterp::terminate();

    if(retval == CBPlanner::PLAN_FOUND)
      return true;
    else
      return false;
  }

  const PlanDatabaseId& CBPlannerAssembly::getPlanDatabase() const {
    return m_planDatabase;
  }

  const unsigned int CBPlannerAssembly::getTotalNodesSearched() const { return m_totalNodes; }

  const unsigned int CBPlannerAssembly::getDepthReached() const { return m_finalDepth; }

}
