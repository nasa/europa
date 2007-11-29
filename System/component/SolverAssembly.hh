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
     * Convenient form of plan to retrieve the XML configuration from a filename.
     */
    bool plan(const char* txSource, const char* config, bool interp = false);

    /**
     * @brief Invoke the planner. Calls playTransactions(txSource).
     * @param txSource The source from which we get the initial state
     * @param A solver configuration file
     * @return True if successfully found a plan. Otherwise false.
     */
    bool plan(const char* txSource, const TiXmlElement& config, bool interp = false);

    const unsigned int getTotalNodesSearched() const;

    const unsigned int getDepthReached() const;

    static const char* TX_LOG();

  private:
    unsigned int m_totalNodes;
    unsigned int m_finalDepth;
    static bool s_initialized;
  };
}
#endif
