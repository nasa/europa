#ifndef _H_TestAssembly
#define _H_TestAssembly

/**
 * @file   TestAssembly.hh
 * @author Tania Bedrax-Weiss
 * @date   Mon Jan 10 16:46:40 2005
 * @brief  
 * @ingroup System
 */

#include "PlanDatabaseDefs.hh"
#include "RulesEngineDefs.hh"
#include "CBPlanner.hh"
#include "StandardAssembly.hh"
#include "DbClientTransactionLog.hh"

namespace EUROPA {

#define PPW_WITH_PLANNER

  class TestAssembly : public StandardAssembly {
  public:
    TestAssembly(const SchemaId& schema);
    virtual ~TestAssembly();
    /**
     * @brief Sets up the necessary constraint factories. The assembly must be explicitly
     * initialized before use.  Mostly calls StandardAssembly, but needs
     * some special constraint factories for contraints needed in test
     * cases.
     */
    static void initialize();

    /**
     * @brief Invoke the planner. Calls playTransactions(txSource).
     * @param txSource The source from which we get the initial state
     * @param enableTransactionLogging When true, we log transactions so we
     * can replay them for test purposes only.
     * @return The result of planning
     * @see CBPlanner::Status
     */
    CBPlanner::Status plan(const char* txSource);

    /** 
     * @brief Replays the transaction log and verifies that the outputs are
     * the same. Useful for testing.
     * 
     * @param txLog The transaction log.
     */    
    void replay(const DbClientTransactionLogId& txLog);

    const PlanDatabaseId& getPlanDatabase() const;

  };
}
#endif
