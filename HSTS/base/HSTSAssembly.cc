#include "HSTSAssembly.hh"

// Support fro required major plan database components
#include "PlanDatabase.hh"
#include "PlanDatabaseWriter.hh"
#include "ConstraintEngine.hh"
#include "RulesEngine.hh"
#include "DefaultPropagator.hh"

// Transactions
#include "DbClientTransactionPlayer.hh"
#include "DbClientTransactionLog.hh"
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
#include "HSTSOpenDecisionManager.hh"
#include "HeuristicsReader.hh"
#include "HeuristicsEngine.hh"
#include "HSTSNoBranchCondition.hh"
#include "HSTSPlanIdReader.hh"
#include "MasterMustBeInserted.hh"

#include <fstream>

namespace EUROPA {

  /**
   * @brief Sets up the necessary constraint factories
   */
  void HSTSAssembly::initialize() {
    check_error(!isInitialized(), 
		"Cannot initialize if already initialized. Call 'terminate' first.");
    initNDDL();

    initConstraintEngine();
    initConstraintLibrary();

    // Procedural Constraints used with Default Propagation
    REGISTER_CONSTRAINT(AddEqualConstraint, "addEq", "Default");
    REGISTER_CONSTRAINT(SubsetOfConstraint, "subsetOf", "Default");
    REGISTER_CONSTRAINT(SubsetOfConstraint, "Singleton", "Default");
    REGISTER_CONSTRAINT(CommonAncestorConstraint, "commonAncestor", "Default");
    REGISTER_CONSTRAINT(HasAncestorConstraint, "hasAncestor", "Default");

   
    //REGISTER_CONSTRAINT(CondAllSameConstraint, "condasame", "Default"); // P4-C3-init.nddl, at least.

    isInitialized() = true;
  }

  void HSTSAssembly::terminate() {
    check_error(isInitialized(), 
		"Terminate should not be called unless the assembly has been initialized.");

    // Purge all statics
    ObjectFactory::purgeAll();
    TokenFactory::purgeAll();
    ConstraintLibrary::purgeAll();
    Rule::purgeAll();
    uninitNDDL();
  }

  bool& HSTSAssembly::isInitialized() {
    static bool sl_isInitialized(false);
    return sl_isInitialized;
  }

  HSTSAssembly::HSTSAssembly(const SchemaId& schema) {
    check_error(isInitialized(), "Must initialize HSTSAssembly before use.");

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

  HSTSAssembly::~HSTSAssembly() {
    // Indicate a mode swith to purging to avoid propagation of deletion and removal
    // messages. Makes for much more efficient deletion
    Entity::purgeStarted();

    delete (RulesEngine*) m_rulesEngine;
    delete (PlanDatabase*) m_planDatabase;
    delete (ConstraintEngine*) m_constraintEngine;

    // Return to standard behavior for deletion
    Entity::purgeEnded();
  }

  bool HSTSAssembly::playTransactions(const char* txSource) {
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

  CBPlanner::Status HSTSAssembly::plan(const char* txSource, const char* heurSource, const char* pidSource) {
    Horizon horizon; // will be initialized after we read the transactions
    // Set up Heuristics Decision Manager
    HeuristicsEngineId heuristics((new HeuristicsEngine(m_planDatabase))->getId());
    HSTSOpenDecisionManager odm(m_planDatabase, heuristics);
    CBPlanner planner(m_planDatabase, horizon.getId(), odm.getId());
    PlanWriter::PartialPlanWriter ppw(m_planDatabase, m_constraintEngine, m_rulesEngine, planner.getId());
    MasterMustBeInserted masterMustBeInserted(planner.getDecisionManager());

    // Initialize heuristics
    if (heurSource != 0 && heurSource != NULL && heurSource[0] != '\0') {
      HeuristicsReader hreader(heuristics);
      hreader.read(heurSource);
    }
    else
      heuristics->initialize();

    ConditionId cond;

    if (pidSource != 0 && pidSource != NULL && pidSource[0] != '\0') {
      // Set up the no Branch Spec
      HSTSNoBranchId noBranchSpec(new HSTSNoBranch());
      HSTSPlanIdReader pireader(noBranchSpec);
      pireader.read(pidSource);
      cond = (new HSTSNoBranchCondition(planner.getDecisionManager()))->getId();
      ((HSTSNoBranchCondition*)cond)->initialize(noBranchSpec);
    }

    // Now process the transactions
    if (!playTransactions(txSource)) {
      if (!cond.isNoId()) {
	delete (HSTSNoBranchCondition*) cond;
      }
      return(CBPlanner::INITIALLY_INCONSISTENT);
    }

    // Configure the planner from data in the initial state
    std::list<ObjectId> configObjects;
    m_planDatabase->getObjectsByType("PlannerConfig", configObjects); // HSTS configuration class

    check_error(configObjects.size() == 1,
		"Expect exactly one instance of the class 'PlannerConfig'");

    ObjectId configSource = configObjects.front();
    check_error(configSource.isValid());

    const std::vector<ConstrainedVariableId>& variables = configSource->getVariables();
    check_error(variables.size() == 4, 
		"Expecting exactly 4 configuration variables.");

    // Set up the horizon from the model now.  Will cause a refresh of the query, but that is OK.
    ConstrainedVariableId horizonStart = variables[0];
    check_error(horizonStart->baseDomain().isSingleton());
    ConstrainedVariableId horizonEnd = variables[1];
    check_error(horizonEnd->baseDomain().isSingleton());
    ConstrainedVariableId plannerSteps = variables[2];
    check_error(plannerSteps->baseDomain().isSingleton());

    int start = (int) horizonStart->baseDomain().getSingletonValue();
    int end = (int) horizonEnd->baseDomain().getSingletonValue();
    horizon.setHorizon(start, end);

    // Now get planner step max
    int steps = (int) plannerSteps->baseDomain().getSingletonValue();

    CBPlanner::Status retval = planner.run(steps);

    m_totalNodes = planner.getTime();
    m_finalDepth = planner.getDepth();

    if (!cond.isNoId()) {
      delete (HSTSNoBranchCondition*) cond;
    }

    // Now run it
    return(retval);
  }

  void HSTSAssembly::write(std::ostream& os) const {
    PlanDatabaseWriter::write(m_planDatabase, os);
  }

  const unsigned int HSTSAssembly::getTotalNodesSearched() const { return m_totalNodes; }

  const unsigned int HSTSAssembly::getDepthReached() const { return m_finalDepth; }


}
