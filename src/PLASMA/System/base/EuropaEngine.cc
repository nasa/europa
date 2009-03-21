
#include "EuropaEngine.hh"
#include "ModuleConstraintEngine.hh"
#include "ModulePlanDatabase.hh"
#include "ModuleRulesEngine.hh"
#include "ModuleTemporalNetwork.hh"
#include "ModuleSolvers.hh"
#include "ModuleNddl.hh"
#ifndef NO_RESOURCES
#include "ModuleResource.hh"
#include "ModuleAnml.hh"
#endif

#include "ConstraintEngine.hh"
#include "PlanDatabase.hh"
#include "RulesEngine.hh"

// Solver Support
#include "Solver.hh"
#include "SolverPartialPlanWriter.hh"
#include "Filters.hh"
#include "PlanDatabaseWriter.hh"

namespace EUROPA {
//namespace System { //TODO: mcr

  //Logger  &EuropaEngine::LOGGER = Logger::getInstance( "EUROPA::System::EuropaEngine", Logger::DEBUG );
  LOGGER_CLASS_INSTANCE_IMPL( EuropaEngine, "EUROPA::System::EuropaEngine", DEBUG );

    EuropaEngine::EuropaEngine()
    {
    }

    EuropaEngine::~EuropaEngine()
    {
    }

    void EuropaEngine::initializeModules()
    {
        // Only do this once
        if (m_modules.size() == 0)
            createModules();
        EngineBase::initializeModules();
    }

    void EuropaEngine::createModules()
    {
        // TODO: make this data-driven
        addModule((new ModuleConstraintEngine())->getId());
        addModule((new ModuleConstraintLibrary())->getId());
        addModule((new ModulePlanDatabase())->getId());
        addModule((new ModuleRulesEngine())->getId());
        addModule((new ModuleTemporalNetwork())->getId());
        addModule((new ModuleSolvers())->getId());
        addModule((new ModuleNddl())->getId());
#ifndef NO_RESOURCES
        addModule((new ModuleResource())->getId());
        addModule((new ModuleAnml())->getId());
#endif
    }

    ConstraintEngineId& EuropaEngine::getConstraintEngine() { return (ConstraintEngineId&)((ConstraintEngine*)getComponent("ConstraintEngine"))->getId(); }
    PlanDatabaseId&     EuropaEngine::getPlanDatabase()     { return (PlanDatabaseId&)((PlanDatabase*)getComponent("PlanDatabase"))->getId(); }
    RulesEngineId&      EuropaEngine::getRulesEngine()      { return (RulesEngineId&)((RulesEngine*)getComponent("RulesEngine"))->getId(); }

    const ConstraintEngine* EuropaEngine::getConstraintEnginePtr() const { return (const ConstraintEngine*)getComponent("ConstraintEngine"); }
    const PlanDatabase*     EuropaEngine::getPlanDatabasePtr()     const { return (const PlanDatabase*)getComponent("PlanDatabase"); }
    const RulesEngine*      EuropaEngine::getRulesEnginePtr()      const { return (const RulesEngine*)getComponent("RulesEngine"); }

    // TODO: remains of the old Assemblies, these are only used by test code, should be dropped, eventually.
    bool EuropaEngine::playTransactions(const char* txSource, const char* language)
    {
      check_error(txSource != NULL, "NULL transaction source provided.");

      static bool isFile = true;
      executeScript(language,txSource,isFile);

      return getConstraintEnginePtr()->constraintConsistent();
    }

    void EuropaEngine::write(std::ostream& os) const
    {
      PlanDatabaseWriter::write(getPlanDatabasePtr()->getId(), os);
    }

    const char* EuropaEngine::TX_LOG() {
      static const char* sl_txLog = "TransactionLog.xml";
      return sl_txLog;
    }

    bool EuropaEngine::plan(const char* txSource, const char* plannerConfig, const char* language){
      TiXmlDocument doc(plannerConfig);
      doc.LoadFile();
      const TiXmlElement& config = *(doc.RootElement());

      SOLVERS::SolverId solver = (new SOLVERS::Solver(getPlanDatabase(), config))->getId();

      SOLVERS::PlanWriter::PartialPlanWriter* ppw =
        new SOLVERS::PlanWriter::PartialPlanWriter(getPlanDatabase(), getConstraintEngine(), getRulesEngine(), solver);

      // Now process the transactions
      if(!playTransactions(txSource, language))
        return false;

      //debugMsg("EuropaEngine:plan", "Initial state: " << std::endl << PlanDatabaseWriter::toString(getPlanDatabase()))
      //LOGGER << Logger::DEBUG << "plan: Initial state: " << Logger::eol << PlanDatabaseWriter::toString(getPlanDatabase());
      LOGGER_DEBUG_MSG( DEBUG, "Initial state: " << LOGGER_ENDL << PlanDatabaseWriter::toString(getPlanDatabase()) )

      // Configure the planner from data in the initial state
      std::list<ObjectId> configObjects;
      getPlanDatabase()->getObjectsByType("PlannerConfig", configObjects); // Standard configuration class

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

    unsigned int EuropaEngine::getTotalNodesSearched() const { return m_totalNodes; }

    unsigned int EuropaEngine::getDepthReached() const { return m_finalDepth; }
}

