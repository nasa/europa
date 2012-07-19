#ifndef _H_CBPlannerAssembly
#define _H_CBPlannerAssembly

/**
 * @file   CBPlannerAssembly.hh
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

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif
#include "tinyxml.h"

namespace EUROPA {

#define PPW_WITH_PLANNER

  class CBPlannerAssembly : public StandardAssembly {
  public:
    CBPlannerAssembly(const SchemaId& schema);
    virtual ~CBPlannerAssembly();
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
     * @param An unused parameter for compatibility with what client expects.
     * @param The required heuristics file - can be a default
     * @param An optional aver test file input
     * @return True if successfully found a plan. Otherwise false
     */
    bool plan(const char* txSource, const TiXmlElement& config, 
	      const char* heuristics, const char* averFile = NULL);

    const PlanDatabaseId& getPlanDatabase() const;

    const unsigned int getTotalNodesSearched() const;

    const unsigned int getDepthReached() const;

    static const char* TX_LOG();

  private:
    unsigned int m_totalNodes;
    unsigned int m_finalDepth;
  };
}
#endif
