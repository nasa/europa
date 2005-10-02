#ifndef _H_HSTSOpenDecisionManager
#define _H_HSTSOpenDecisionManager

#include "CBPlannerDefs.hh"
#include "OpenDecisionManager.hh"
#include "HeuristicsEngine.hh"

/**
 * @brief Basically only here for backward compatibility
 */
namespace EUROPA {

  class HSTSOpenDecisionManager : public OpenDecisionManager {
  public:

    HSTSOpenDecisionManager(const PlanDatabaseId& db, const HeuristicsEngineId& heur, 
			    const bool strictHeuristics = false);
    ~HSTSOpenDecisionManager();
  };
}

#endif
