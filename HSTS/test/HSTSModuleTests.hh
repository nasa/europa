#ifndef _H_HSTSModuleTests
#define _H_HSTSModuleTests

#include "ConstraintEngine.hh"
#include "ConstraintEngineDefs.hh"
#include "PlanDatabase.hh"
#include "PlanDatabaseDefs.hh"
#include "Schema.hh"
#include "Horizon.hh"
#include "DecisionManager.hh"
#include "CBPlanner.hh"
#include "Timeline.hh"
#include "IntervalToken.hh"
#include "IntervalDomain.hh"
#include "IntervalIntDomain.hh"
#include "TemporalVariableCondition.hh"
#include "DynamicInfiniteRealCondition.hh"
#include "TokenDecisionPoint.hh"
#include "HSTSHeuristics.hh"
#include "HSTSOpenDecisionManager.hh"

namespace PLASMA {

  void initCBPTestSchema();

  bool testHSTSNoBranchConditionImpl(ConstraintEngine &ce, PlanDatabase &db,
				     DecisionManager &dm);

  bool testTokenTypeImpl(HSTSHeuristics& heuristics);

  bool testDefaultInitializationImpl(HSTSHeuristics& heuristics);

  bool testTokenInitializationImpl(HSTSHeuristics& heuristics);

  bool testVariableInitializationImpl(HSTSHeuristics& heuristics);

  bool testReaderImpl(HSTSHeuristics& heuristics);

  bool testHSTSPlanIdReaderImpl();

  void initHeuristicsSchema();

  bool testHSTSNoBranchImpl(ConstraintEngine &ce, PlanDatabase& db, CBPlanner& planner);

  bool testHSTSHeuristicsAssemblyImpl(ConstraintEngine& ce, PlanDatabase& db, CBPlanner& planner, HSTSHeuristics& heuristics);
}
#endif
