#include "StandardAssembly.hh"

// Support for required major plan database components
#include "PlanDatabase.hh"
#include "PlanDatabaseWriter.hh"
#include "ConstraintEngine.hh"

// Transactions
#include "DbClientTransactionLog.hh"
//#include "NddlDefs.hh"

// Misc
#include "Utils.hh"

#include <sstream>

namespace EUROPA {

  void StandardAssembly::initialize()
  {
  }

  void StandardAssembly::terminate() 
  {
  }

  StandardAssembly::StandardAssembly() 
  {
    doStart();
    getPlanDatabase()->getClient()->enableTransactionLogging();
  }

  StandardAssembly::~StandardAssembly() 
  {
    doShutdown();    
  }

  bool StandardAssembly::playTransactions(const char* txSource, bool interp)
  {
    check_error(txSource != NULL, "NULL transaction source provided.");

    static bool isFile = true;
    if(interp)
    	executeScript("nddl-xml",txSource,isFile);
    else
    	executeScript("nddl-xml-txn",txSource,isFile);

    return getConstraintEnginePtr()->constraintConsistent();
  }

  void StandardAssembly::write(std::ostream& os) const 
  {
    PlanDatabaseWriter::write(getPlanDatabasePtr()->getId(), os);
  }

  std::string StandardAssembly::toString() const 
  {
    std::ostringstream sstr;
    write(sstr);
    return sstr.str();
  }
}
