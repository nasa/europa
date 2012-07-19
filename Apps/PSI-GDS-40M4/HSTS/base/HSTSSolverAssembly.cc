#include "HSTSSolverAssembly.hh"
#include "FlawFilter.hh"
#include "FlawHandler.hh"
#include "Filters.hh"
#include "HSTSDecisionPoints.hh"
#include "ConstraintLibrary.hh"
#include "Constraints.hh"
#include "NddlDefs.hh"
#include "CommonAncestorConstraint.hh"
#include "HasAncestorConstraint.hh"
#include "Object.hh"
#include "ConstrainedVariable.hh"
#include "DbClient.hh"
#include "DbClientTransactionPlayer.hh"
#include "STNTemporalAdvisor.hh"
#include "ConstrainedVariable.hh"
#include "AbstractDomain.hh"
#include "SolverPartialPlanWriter.hh"

#include <fstream>

namespace EUROPA {
  namespace HSTS {
    void SolverAssembly::initialize() {
      static bool isInitialized = false;
      check_error(!isInitialized, "Cannot initialize if already initialized.");
      initNDDL();
      initConstraintEngine();
      initConstraintLibrary();


      REGISTER_CONSTRAINT(LessThanConstraint, "lessThan", "Default");
      REGISTER_CONSTRAINT(AddEqualConstraint, "addEq", "Default");
      REGISTER_CONSTRAINT(SubsetOfConstraint, "subsetOf", "Default");
      REGISTER_CONSTRAINT(SubsetOfConstraint, "Singleton", "Default");
      REGISTER_CONSTRAINT(CommonAncestorConstraint, "commonAncestor", "Default");
      REGISTER_CONSTRAINT(HasAncestorConstraint, "hasAncestor", "Default");

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

      isInitialized = true;
    }

    bool SolverAssembly::playTransactions(const char* txSource) {
      check_error(txSource != NULL, "NULL transaction source provided.");

      // Obtain the client to play transactions on.
      DbClientId client = m_planDatabase->getClient();

      // Construct player
      DbClientTransactionPlayer player(client);

      // Open transaction source and play transactions
      std::ifstream in(txSource);

      check_error(in, "Invalid transaction source '" + std::string(txSource) + "'.");
      player.play(in);

      return m_constraintEngine->constraintConsistent();
    }

    SolverAssembly::SolverAssembly(SchemaId schema) {
      // Allocate the Constraint Engine
      m_constraintEngine = (new ConstraintEngine())->getId();
      
      // Allocate the plan database
      m_planDatabase = (new PlanDatabase(m_constraintEngine, schema))->getId();
      // Construct propagators - order of introduction determines order of propagation.
      // Note that propagators will subsequently be managed by the constraint engine
      new DefaultPropagator(LabelStr("Default"), m_constraintEngine);
      new TemporalPropagator(LabelStr("Temporal"), m_constraintEngine);
      
      // Link up the Temporal Advisor in the PlanDatabase so that it can use the temporal
      // network for determining temporal distances between time points.
      PropagatorId temporalPropagator = m_constraintEngine->getPropagatorByName(LabelStr("Temporal"));
      m_planDatabase->setTemporalAdvisor((new STNTemporalAdvisor(temporalPropagator))->getId());
      
      // Allocate the rules engine to process rules
      m_rulesEngine = (new RulesEngine(m_planDatabase))->getId();
    }

    bool SolverAssembly::plan(const char* txSource, const char* sconfigSource) {
      TiXmlDocument doc(sconfigSource);
      doc.LoadFile();
      SOLVERS::SolverId solver = (new SOLVERS::Solver(m_planDatabase, *(doc.RootElement())))->getId();
      
#ifdef PPW_WITH_PLANNER
      SOLVERS::PlanWriter::PartialPlanWriter ppw(m_planDatabase, m_constraintEngine, m_rulesEngine, solver);
#else
      SOLVERS::PlanWriter::PartialPlanWriter ppw(m_planDatabase, m_constraintEngine, m_rulesEngine);
#endif
      
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
                
      delete (SOLVERS::Solver*) solver;
      
      return retval;
    }
  }
}
