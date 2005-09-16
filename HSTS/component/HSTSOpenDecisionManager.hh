#ifndef _H_HSTSOpenDecisionManager
#define _H_HSTSOpenDecisionManager

#include "CBPlannerDefs.hh"
#include "OpenDecisionManager.hh"
#include "HSTSDefs.hh"

namespace EUROPA {

  class HSTSOpenDecisionManager : public OpenDecisionManager {
  public:

    HSTSOpenDecisionManager(const PlanDatabaseId& db, const HeuristicsEngineId& heur, bool strictHeuristics = true);

    ~HSTSOpenDecisionManager();
    
  protected:

    bool m_strictHeuristics; /*!< if this flag is true, we ignore the implicit deicision heuristic order of
                              object, unit variable, token, non-unit variable and instead follow
                             the heuristics directly, with the exception of preferring unit variable 
			     decisions over all others.*/
  };

}

#endif
