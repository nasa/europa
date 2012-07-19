#include "AverTestAssembly.hh"

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

#include "NddlDefs.hh"

// Misc
#include "Utils.hh"

// Planner Support
#include "Solver.hh"
#include "SolverPartialPlanWriter.hh"


// Test Support
#include "TestSupport.hh"

#include "AverInterp.hh"

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif
#include "tinyxml.h"

#include <string>
#include <fstream>

#define PPW_WITH_PLANNER

const char* TX_LOG = "TransactionLog.xml";
const char* s_plannerConfig = "DefaultPlannerConfig.xml";

namespace EUROPA {

  AverTestAssembly::AverTestAssembly(const SchemaId& schema, const char* averFile) : StandardAssembly(schema) { 
    TiXmlDocument doc(s_plannerConfig);
    doc.LoadFile();
    m_planner = (new SOLVERS::Solver(m_planDatabase, *(doc.RootElement())))->getId();
    AverInterp::init(averFile, m_planner, m_constraintEngine, m_planDatabase, m_rulesEngine);
  }

  AverTestAssembly::~AverTestAssembly() {
    AverInterp::terminate();
    delete (SOLVERS::Solver*) m_planner;
  }

  /**
   * @brief Sets up the necessary constraint factories
   */
  void AverTestAssembly::initialize() {
    StandardAssembly::initialize();
    isInitialized() = true;
  }

  int AverTestAssembly::plan(const char* txSource, const char* averFile){
    return 2;
  }

  void AverTestAssembly::replay(const DbClientTransactionLogId& txLog) {
  }

  const PlanDatabaseId& AverTestAssembly::getPlanDatabase() const {
    return m_planDatabase;
  }
  
  const RulesEngineId& AverTestAssembly::getRulesEngine() const {
    return m_rulesEngine;
  }
}
