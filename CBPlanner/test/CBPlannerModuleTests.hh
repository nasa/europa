#ifndef _H_CBPlannerModuleTests
#define _H_CBPlannerModuleTests

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

namespace Prototype {

bool testDefaultSetupImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema, 
                          DecisionManager &dm, Horizon &hor);
bool testConditionImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema,
                       DecisionManager &dm, Horizon &hor);
bool testHorizonImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema,
                                DecisionManager &dm, Horizon &hor);
bool testHorizonConditionImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema,
                                         DecisionManager &dm, Horizon &hor);
bool testTemporalVariableConditionImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema,
                                       DecisionManager &dm, Horizon &hor);
bool testDynamicInfiniteRealConditionImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema,
                                          DecisionManager &dm, Horizon &hor);
bool testForwardDecisionHandlingImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema,
                                     DecisionManager &dm, Horizon &hor);
bool testMakeMoveImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema,
                      CBPlanner &planner);
bool testCurrentStateImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema,
                          CBPlanner &planner);
bool testRetractMoveImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema,
                         CBPlanner &planner);
bool testNoBacktrackCaseImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema,
                             CBPlanner &planner);
bool testSubgoalOnceRuleImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema, 
                             CBPlanner &planner);
bool testBacktrackCaseImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema, 
                           CBPlanner &planner);
bool testTimeoutCaseImpl(ConstraintEngine& ce, PlanDatabase& db, Schema& schema,  
		  CBPlanner& planner) ;
bool testMultipleDMsImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema, 
                         DecisionManager &dm, Horizon &hor);
}
#endif
