#include "HSTSOpenDecisionManager.hh"

namespace EUROPA {

  HSTSOpenDecisionManager::HSTSOpenDecisionManager(const PlanDatabaseId& db, 
						   const HeuristicsEngineId& heur, const bool )
    : OpenDecisionManager(db, heur){}

  HSTSOpenDecisionManager::~HSTSOpenDecisionManager() { }

  bool HSTSOpenDecisionManager::isAutoAllocationEnabled() const { return true; }
}
