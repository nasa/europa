#include "StandardAssembly.hh"

// Support fro required major plan database components
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

#include "SAVH_Profile.hh"
#include "SAVH_FVDetector.hh"
#include "SAVH_TimetableProfile.hh"
#include "SAVH_TimetableFVDetector.hh"
#include "SAVH_ProfilePropagator.hh"
#include "SAVH_FlowProfile.hh"
#include "SAVH_IncrementalFlowProfile.hh"

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

#include <fstream>
#include <sstream>

namespace EUROPA {

  /**
   * @brief Sets up the necessary constraint factories
   */
  void StandardAssembly::initialize(){
    check_error(!isInitialized(), 
		"Cannot initialize if already initialized. Call 'terminate' first.");
    initNDDL();

    // Procedural Constraints used with Default Propagation
    REGISTER_CONSTRAINT(EqualConstraint, "eq", "Default");
    REGISTER_CONSTRAINT(NotEqualConstraint, "neq", "Default");
    REGISTER_CONSTRAINT(LessThanEqualConstraint, "leq", "Default");
    REGISTER_CONSTRAINT(LessThanConstraint, "lessThan", "Default");
    REGISTER_CONSTRAINT(AddEqualConstraint, "addEq", "Default");
    REGISTER_CONSTRAINT(NegateConstraint, "neg", "Default");
    REGISTER_CONSTRAINT(MultEqualConstraint, "mulEq", "Default");
    REGISTER_CONSTRAINT(AddMultEqualConstraint, "addMulEq", "Default");
    REGISTER_CONSTRAINT(AddMultEqualConstraint, "addmuleq", "Default");
    REGISTER_CONSTRAINT(SubsetOfConstraint, "subsetOf", "Default");
    REGISTER_CONSTRAINT(SubsetOfConstraint, "Singleton", "Default");
    REGISTER_CONSTRAINT(LockConstraint, "Lock", "Default");
    REGISTER_CONSTRAINT(CommonAncestorConstraint, "commonAncestor", "Default");
    REGISTER_CONSTRAINT(HasAncestorConstraint, "hasAncestor", "Default");
    REGISTER_CONSTRAINT(TestEQ, "testEQ", "Default");
    REGISTER_CONSTRAINT(TestLessThan, "testLEQ", "Default");
    REGISTER_CONSTRAINT(EqualSumConstraint, "sum", "Default");

    isInitialized() = true;
  }

  void StandardAssembly::terminate() {
    check_error(isInitialized(), 
		"Terminate should not be called unless the assembly has been initialized.");

    // Purge all statics
    ObjectFactory::purgeAll();
    TokenFactory::purgeAll();
    ConstraintLibrary::purgeAll();
    Rule::purgeAll();
    uninitNDDL();
    isInitialized() = false;
  }

  bool& StandardAssembly::isInitialized(){
    static bool sl_isInitialized(false);
    return sl_isInitialized;
  }

  StandardAssembly::StandardAssembly(const SchemaId& schema){
    check_error(isInitialized(), "Must initialize StandardAssembly before use.");

    // Allocate the Constraint Engine
    m_constraintEngine = (new ConstraintEngine())->getId();

    // Allocate the plan database
    m_planDatabase = (new PlanDatabase(m_constraintEngine, schema))->getId();

    // Construct propagators - order of introduction determines order of propagation.
    // Note that propagators will subsequently be managed by the constraint engine
    new DefaultPropagator(LabelStr("Default"), m_constraintEngine);
    new TemporalPropagator(LabelStr("Temporal"), m_constraintEngine);
    new ResourcePropagator(LabelStr("Resource"), m_constraintEngine, m_planDatabase);
    new SAVH::ProfilePropagator(LabelStr("SAVH_Resource"), m_constraintEngine);

    // Link up the Temporal Advisor in the PlanDatabase so that it can use the temporal
    // network for determining temporal distances between time points.
    PropagatorId temporalPropagator = m_constraintEngine->getPropagatorByName(LabelStr("Temporal"));
    m_planDatabase->setTemporalAdvisor((new STNTemporalAdvisor(temporalPropagator))->getId());

    // Allocate the rules engine to process rules
    m_rulesEngine = (new RulesEngine(m_planDatabase))->getId();

    m_planDatabase->getClient()->enableTransactionLogging();
  }

  StandardAssembly::~StandardAssembly() {
    // Indicate a mode swith to purging to avoid propagation of deletion and removal
    // messages. Makes for much more efficient deletion
    Entity::purgeStarted();

    delete (RulesEngine*) m_rulesEngine;
    delete (PlanDatabase*) m_planDatabase;
    delete (ConstraintEngine*) m_constraintEngine;

    // Return to standard behavior for deletion
    Entity::purgeEnded();
  }

  bool StandardAssembly::playTransactions(const char* txSource){
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

  void StandardAssembly::write(std::ostream& os) const {
    PlanDatabaseWriter::write(m_planDatabase, os);
  }

  std::string StandardAssembly::toString() const {
    std::ostringstream sstr;
    write(sstr);
    return sstr.str();
  }

}
