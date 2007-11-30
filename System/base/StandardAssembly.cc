#include "StandardAssembly.hh"

// Support for required major plan database components
#include "PlanDatabase.hh"
#include "PlanDatabaseWriter.hh"
#include "ConstraintEngine.hh"
#include "RulesEngine.hh"

// Transactions
#include "DbClientTransactionPlayer.hh"
#include "DbClientTransactionLog.hh"
#include "TransactionInterpreter.hh"
#include "NddlDefs.hh"

// Misc
#include "Utils.hh"

#include <fstream>
#include <sstream>

namespace EUROPA {

  /**
   * @brief Sets up the necessary constraint factories
   */
  void StandardAssembly::initialize()
  {
    check_error(!isInitialized(), 
		"Cannot initialize if already initialized. Call 'terminate' first.");
    
    EngineBase::initialize(); 
    isInitialized() = true;
  }

  void StandardAssembly::terminate() 
  {
    check_error(isInitialized(), 
		"Terminate should not be called unless the assembly has been initialized.");

    EngineBase::terminate(); 
    isInitialized() = false;
  }

  bool& StandardAssembly::isInitialized()
  {
    static bool sl_isInitialized(false);
    return sl_isInitialized;
  }

  StandardAssembly::StandardAssembly() 
  {
    check_error(ALWAYS_FAIL, "Should never get here.");
  }

  StandardAssembly::StandardAssembly(const SchemaId& schema) : m_transactionPlayer(NULL) 
  {
    check_error(isInitialized(), "Must initialize StandardAssembly before use.");
    allocateComponents();
    m_planDatabase->getClient()->enableTransactionLogging();
  }

  StandardAssembly::~StandardAssembly() 
  {
    if (m_transactionPlayer != NULL)
        delete m_transactionPlayer;    

    deallocateComponents();    
  }

  bool StandardAssembly::playTransactions(const char* txSource, bool interp)
  {
    check_error(txSource != NULL, "NULL transaction source provided.");

    // TODO: this should be done with LanguageInterpreters for nddl-xml and nddl-xml-txn
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

  void StandardAssembly::write(std::ostream& os) const 
  {
    PlanDatabaseWriter::write(m_planDatabase, os);
  }

  std::string StandardAssembly::toString() const 
  {
    std::ostringstream sstr;
    write(sstr);
    return sstr.str();
  }
}
