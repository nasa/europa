#include "SolverAssembly.hh"

// Support for required major plan database components
#include "PlanDatabase.hh"
#include "PlanDatabaseWriter.hh"
#include "ConstraintEngine.hh"
#include "RulesEngine.hh"
#include "DefaultPropagator.hh"

// Transactions
#include "DbClientTransactionPlayer.hh"

#include "NddlDefs.hh"

// Misc
#include "Utils.hh"

// Solver Support
#include "ComponentFactory.hh"
#include "Solver.hh"
#include "OpenConditionDecisionPoint.hh"
#include "OpenConditionManager.hh"
#include "ThreatDecisionPoint.hh"
#include "ThreatManager.hh"
#include "UnboundVariableDecisionPoint.hh"
#include "UnboundVariableManager.hh"
#include "SolverDecisionPoint.hh"
#include "MatchingRule.hh"
#include "Filters.hh"
#include "SolverPartialPlanWriter.hh"

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

namespace EUROPA {

  const char* SolverAssembly::TX_LOG() {
    static const char* sl_txLog = "TransactionLog.xml";
    return sl_txLog;
  }

  SolverAssembly::SolverAssembly(const SchemaId& schema) : StandardAssembly(schema) {}

  SolverAssembly::~SolverAssembly() {}

  /**
   * @brief Sets up the necessary constraint factories
   */
  void SolverAssembly::initialize() {
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

    // Also add some 
    REGISTER_VARIABLE_DECISION_FACTORY(EUROPA::SOLVERS::MinValue, MinValue);
    REGISTER_COMPONENT_FACTORY(EUROPA::SOLVERS::UnboundVariableManager, UnboundVariableManager);
  
    REGISTER_OPENCONDITION_DECISION_FACTORY(EUROPA::SOLVERS::OpenConditionDecisionPoint, StandardOpenConditionHandler);
    REGISTER_COMPONENT_FACTORY(EUROPA::SOLVERS::OpenConditionManager, OpenConditionManager);
  
    REGISTER_THREAT_DECISION_FACTORY(EUROPA::SOLVERS::ThreatDecisionPoint, StandardThreatHandler);
    REGISTER_COMPONENT_FACTORY(EUROPA::SOLVERS::ThreatManager, ThreatManager);
  
    REGISTER_COMPONENT_FACTORY(EUROPA::SOLVERS::InfiniteDynamicFilter, InfiniteDynamicFilter);
    REGISTER_COMPONENT_FACTORY(EUROPA::SOLVERS::HorizonFilter, HorizonFilter);
    REGISTER_COMPONENT_FACTORY(EUROPA::SOLVERS::HorizonVariableFilter, HorizonVariableFilter);
    isInitialized() = true;
  }

  bool SolverAssembly::plan(const char* txSource, const char* config){
    check_error(config != NULL, "Must have a planner config argument.");
    TiXmlDocument doc(config);
    doc.LoadFile();
    return plan(txSource, *(doc.RootElement()));
  }

  bool SolverAssembly::plan(const char* txSource, const TiXmlElement& config, const char* averFile){
    SOLVERS::SolverId solver = (new SOLVERS::Solver(m_planDatabase, config))->getId();

#ifdef PPW_WITH_PLANNER
    SOLVERS::PlanWriter::PartialPlanWriter ppw(m_planDatabase, m_constraintEngine, m_rulesEngine, solver);
#else
    SOLVERS::PlanWriter::PartialPlanWriter ppw(m_planDatabase, m_constraintEngine, m_rulesEngine);
#endif

    if(averFile != NULL) {
      AverInterp::init(std::string(averFile), solver, 
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

  void SolverAssembly::replay(const DbClientTransactionLogId& txLog) {
    std::stringstream os1;
    m_planDatabase->getClient()->toStream(os1);
    std::ofstream out(TX_LOG());
    txLog->flush(out);
    out.close();

    std::stringstream os2;
    SolverAssembly replayed(Schema::instance());
    replayed.playTransactions(TX_LOG());
    replayed.getPlanDatabase()->getClient()->toStream(os2);

    std::string s1 = os1.str();
    std::string s2 = os2.str();

    assert(s1 == s2);
  }

  const PlanDatabaseId& SolverAssembly::getPlanDatabase() const {
    return m_planDatabase;
  }

  const unsigned int SolverAssembly::getTotalNodesSearched() const { return m_totalNodes; }

  const unsigned int SolverAssembly::getDepthReached() const { return m_finalDepth; }
}
