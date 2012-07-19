#ifndef _H_SolverAssembly
#define _H_SolverAssembly

/**
 * @file   SolverAssembly.hh
 * @author Michael Iatauro
 * @date   Summer,  2005
 * @brief  
 * @ingroup System
 */

#include "StandardAssembly.hh"
#include "DbClientTransactionLog.hh"

#ifndef TIXML_USE_STL
#define TIXML_USE_STL
#endif
#include "tinyxml.h"

namespace EUROPA {

#define PPW_WITH_PLANNER

  class SolverAssembly : public StandardAssembly {
  public:
    SolverAssembly(const SchemaId& schema);

    virtual ~SolverAssembly();

    /**
     * @brief Sets up the necessary constraint factories. The assembly must be explicitly
     * initialized before use.  Mostly calls StandardAssembly, but needs
     * some special constraint factories for contraints needed in test
     * cases.
     */
    static void initialize();

    /**
     * Convenient form of plan to retrieve the XML configuration from a filename.
     */
    bool plan(const char* txSource, const char* config);

    /**
     * @brief Invoke the planner. Calls playTransactions(txSource).
     * @param txSource The source from which we get the initial state
     * @param A solver configuration file
     * @param An optional aver test file
     * @return True if successfully found a plan. Otherwise false.
     */
    bool plan(const char* txSource, const TiXmlElement& config, const char* averFile = NULL);

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
