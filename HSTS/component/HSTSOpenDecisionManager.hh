#ifndef _H_HSTSOpenDecisionManager
#define _H_HSTSOpenDecisionManager

#include "CBPlannerDefs.hh"
#include "OpenDecisionManager.hh"
#include "HeuristicsEngine.hh"

namespace EUROPA {

  class HSTSOpenDecisionManager : public OpenDecisionManager {
  public:

    HSTSOpenDecisionManager(const PlanDatabaseId& db, const HeuristicsEngineId& heur, 
			    const bool strictHeuristics = false);
    ~HSTSOpenDecisionManager();

    /**
     * @brief Tests if configured to use auto allocation of a new
     * active token as a proxy when resolving an open condition.
     */
    bool isAutoAllocationEnabled() const;
  };
}

#endif
