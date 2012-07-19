#include "StandardAssembly.hh"

// Support fro required major plan database components
#include "PlanDatabase.hh"
#include "PlanDatabaseWriter.hh"
#include "ConstraintEngine.hh"
#include "RulesEngine.hh"
#include "DefaultPropagator.hh"

// Transactions
#include "DbClientTransactionPlayer.hh"
#include "DbClientTransactionLog.hh"
#include "TransactionInterpreter.hh"
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
    initConstraintLibrary();

    isInitialized() = true;
  }

  void StandardAssembly::terminate() {
    check_error(isInitialized(), 
		"Terminate should not be called unless the assembly has been initialized.");

    // Purge all statics
    ObjectFactory::purgeAll();
    TokenFactory::purgeAll();
    ConstraintLibrary::purgeAll();
    uninitConstraintLibrary();
    Rule::purgeAll();
    uninitNDDL();
    isInitialized() = false;
  }

  bool& StandardAssembly::isInitialized(){
    static bool sl_isInitialized(false);
    return sl_isInitialized;
  }

  StandardAssembly::StandardAssembly() {
    check_error(ALWAYS_FAIL, "Should never get here.");
  }

  StandardAssembly::StandardAssembly(const SchemaId& schema) : m_transactionPlayer(NULL) {
    check_error(isInitialized(), "Must initialize StandardAssembly before use.");

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

    m_planDatabase->getClient()->enableTransactionLogging();
  }

  StandardAssembly::~StandardAssembly() {
    // Indicate a mode swith to purging to avoid propagation of deletion and removal
    // messages. Makes for much more efficient deletion
    Entity::purgeStarted();

    delete (RulesEngine*) m_rulesEngine;
    delete m_transactionPlayer;
    delete (PlanDatabase*) m_planDatabase;
    delete (ConstraintEngine*) m_constraintEngine;

    // Return to standard behavior for deletion
    Entity::purgeEnded();
  }

  bool StandardAssembly::playTransactions(const char* txSource, bool interp){
    check_error(txSource != NULL, "NULL transaction source provided.");

    // Obtain the client to play transactions on.
    DbClientId client = m_planDatabase->getClient();

    // Construct player
    if(m_transactionPlayer == NULL) {
      if(interp)
	m_transactionPlayer = new InterpretedDbClientTransactionPlayer(client);
      else
	m_transactionPlayer = new DbClientTransactionPlayer(client);
    }

    // Open transaction source and play transactions
    std::ifstream in(txSource);

    check_error(in, "Invalid transaction source '" + std::string(txSource) + "'.");
    m_transactionPlayer->play(in);

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
