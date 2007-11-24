#include "SolverAssembly.hh"

// Support for required major plan database components
#include "PlanDatabase.hh"
#include "Token.hh"
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
#include "Filters.hh"
#include "HSTSDecisionPoints.hh"
#include "SolverPartialPlanWriter.hh"

// Test Support
#include "TestSupport.hh"

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif
#include "tinyxml.h"

#include <string>

#define PPW_WITH_PLANNER

namespace EUROPA {

  bool SolverAssembly::s_initialized = false;
  const char* SolverAssembly::TX_LOG() {
    static const char* sl_txLog = "TransactionLog.xml";
    return sl_txLog;
  }

  SolverAssembly::SolverAssembly(const SchemaId& schema) : StandardAssembly(schema) {
    if(!s_initialized) {
    REGISTER_COMPONENT_FACTORY(EUROPA::SOLVERS::InfiniteDynamicFilter, InfiniteDynamicFilter);
    REGISTER_COMPONENT_FACTORY(EUROPA::SOLVERS::HorizonFilter, HorizonFilter);
    REGISTER_COMPONENT_FACTORY(EUROPA::SOLVERS::HorizonVariableFilter, HorizonVariableFilter);
    REGISTER_COMPONENT_FACTORY(EUROPA::SOLVERS::TokenMustBeAssignedFilter, TokenMustBeAssignedFilter);
    REGISTER_COMPONENT_FACTORY(EUROPA::SOLVERS::TokenMustBeAssignedFilter, ParentMustBeInsertedFilter);
    REGISTER_FLAW_FILTER(EUROPA::SOLVERS::SingletonFilter, Singleton);
    REGISTER_FLAW_FILTER(EUROPA::SOLVERS::MasterMustBeAssignedFilter, MasterMustBeInsertedFilter);
    
    REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::MinValue, Min);
    REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::MinValue, Max);
    REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::HSTS::ValueEnum, ValEnum);
    REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::HSTS::OpenConditionDecisionPoint, HSTSOpenConditionDecisionPoint);
    REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::HSTS::ThreatDecisionPoint, HSTSThreatDecisionPoint);

    }
  }

  SolverAssembly::~SolverAssembly() {}

  /**
   * @brief Sets up the necessary constraint factories
   */
  void SolverAssembly::initialize() {
    StandardAssembly::initialize();
    isInitialized() = true;
  }

  bool SolverAssembly::plan(const char* txSource, const char* config, bool interp){
    check_error(config != NULL, "Must have a planner config argument.");
    TiXmlDocument doc(config);
    doc.LoadFile();
    return plan(txSource, *(doc.RootElement()), interp);
  }

  bool SolverAssembly::plan(const char* txSource, const TiXmlElement& config, bool interp){
    SOLVERS::SolverId solver = (new SOLVERS::Solver(m_planDatabase, config))->getId();

#ifdef PPW_WITH_PLANNER
    SOLVERS::PlanWriter::PartialPlanWriter* ppw = 
      new SOLVERS::PlanWriter::PartialPlanWriter(m_planDatabase, m_constraintEngine, m_rulesEngine, solver);
#else
    SOLVERS::PlanWriter::PartialPlanWriter* ppw =
      new SOLVERS::PlanWriter::PartialPlanWriter(m_planDatabase, m_constraintEngine, m_rulesEngine);
#endif

    // Now process the transactions
    if(!playTransactions(txSource, interp))
      return false;

    debugMsg("SolverAssembly:plan", "Initial state: " << std::endl << PlanDatabaseWriter::toString(m_planDatabase));

    // Configure the planner from data in the initial state
    std::list<ObjectId> configObjects;
    m_planDatabase->getObjectsByType("PlannerConfig", configObjects); // Standard configuration class

    check_error(configObjects.size() == 1,
		"Expect exactly one instance of the class 'PlannerConfig'");

    ObjectId configSource = configObjects.front();
    check_error(configSource.isValid());

    const std::vector<ConstrainedVariableId>& variables = configSource->getVariables();
    checkError(variables.size() == 4,
	       "Expecting exactly 4 configuration variables.  Got " << variables.size());

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
    
    delete ppw;
    delete (SOLVERS::Solver*) solver;

    return retval;
  }

  const unsigned int SolverAssembly::getTotalNodesSearched() const { return m_totalNodes; }

  const unsigned int SolverAssembly::getDepthReached() const { return m_finalDepth; }
}
