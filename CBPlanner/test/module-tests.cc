#include "CBPlannerDefs.hh"
#include "CBPlanner.hh"
#include "DecisionManager.hh"
#include "SubgoalOnceRule.hh"
#include "CBPlannerModuleTests.hh"
#include "Condition.hh"
#include "Horizon.hh"
#include "HorizonCondition.hh"
#include "TemporalVariableCondition.hh"
#include "DynamicInfiniteRealCondition.hh"
#include "TokenDecisionPoint.hh"
#include "TestSupport.hh"
#include "IntervalIntDomain.hh"
#include "IntervalDomain.hh"
#include "DefaultPropagator.hh"
#include "EqualityConstraintPropagator.hh"
#include "Constraint.hh"
#include "Utils.hh"
#include "BinaryCustomConstraint.hh"
#include "NotFalseConstraint.hh"

#include "PlanDatabaseDefs.hh"
#include "PlanDatabase.hh"
#include "Schema.hh"
#include "Object.hh"
#include "EventToken.hh"
#include "TokenVariable.hh"
#include "ObjectTokenRelation.hh"
#include "Timeline.hh"

#include "RulesEngine.hh"
#include "Rule.hh"

#include <iostream>
#include <string>

#define DEFAULT_SETUP(ce, db, autoClose) \
    ConstraintEngine ce; \
    initCBPTestSchema(); \
    PlanDatabase db(ce.getId(), Schema::instance()); \
    new DefaultPropagator(LabelStr("Default"), ce.getId()); \
    RulesEngine re(db.getId()); \
    Horizon hor(0,200); \
    DecisionManager dm(db.getId()); \
    if (autoClose) \
      db.close();

#define DEFAULT_TEARDOWN()

#define DEFAULT_SETUP_PLAN(ce, db, autoClose) \
    ConstraintEngine ce; \
    initCBPTestSchema(); \
    PlanDatabase db(ce.getId(), Schema::instance()); \
    new DefaultPropagator(LabelStr("Default"), ce.getId()); \
    RulesEngine re(db.getId()); \
    Horizon hor(0, 200); \
    CBPlanner planner(db.getId(), hor.getId()); \
    if (autoClose) \
      db.close();

#define DEFAULT_TEARDOWN_PLAN()

#define DEFAULT_SETUP_HEURISTICS() \
    ConstraintEngine ce; \
    initCBPTestSchema(); \
    PlanDatabase db(ce.getId(), Schema::instance()); \
    HSTSHeuristics heuristics(db.getId()); 

#define DEFAULT_TEARDOWN_HEURISTICS()

#define DEFAULT_SETUP_PLAN_HEURISTICS() \
    ConstraintEngine ce; \
    initCBPTestSchema(); \
    PlanDatabase db(ce.getId(), Schema::instance()); \
    new DefaultPropagator(LabelStr("Default"), ce.getId()); \
    RulesEngine re(db.getId()); \
    Horizon hor(0, 200); \
    CBPlanner planner(db.getId(), hor.getId()); \
    HSTSHeuristics heuristics(db.getId()); 

#define DEFAULT_TEARDOWN_PLAN_HEURISTICS()

class DefaultSetupTest {
public:
  static bool test() {
    runTest(testDefaultSetup);
    return(true);
  }

private:
  static bool testDefaultSetup() {
    bool retval = false;
    DEFAULT_SETUP(ce, db, false);
    retval = testDefaultSetupImpl(ce, db, dm, hor);
    DEFAULT_TEARDOWN();
    return retval;
  }
};

class ConditionTest {
public:
  static bool test() {
    runTest(testCondition);
    runTest(testHorizon);
    runTest(testHorizonCondition);
    runTest(testTemporalVariableCondition);
    runTest(testDynamicInfiniteRealCondition);
    return(true);
  }
private:
  static bool testCondition(){
    bool retval = false;
    DEFAULT_SETUP(ce, db, false);
    retval = testConditionImpl(ce, db, dm, hor);
    DEFAULT_TEARDOWN();
    return retval;
  }

  static bool testHorizon() {
    bool retval = false;
    DEFAULT_SETUP(ce, db, true);
    retval = testHorizonImpl(ce, db, dm, hor);
    DEFAULT_TEARDOWN();
    return retval;
  }

  static bool testHorizonCondition() {
    bool retval = false;
    DEFAULT_SETUP(ce, db, false);
    retval = testHorizonConditionImpl(ce, db, dm, hor);
    DEFAULT_TEARDOWN();
    return retval;
  }

  static bool testTemporalVariableCondition() {
    bool retval = false;
    DEFAULT_SETUP(ce, db, false);
    retval = testTemporalVariableConditionImpl(ce, db, dm, hor);
    DEFAULT_TEARDOWN();
    return retval;
  }

  static bool testDynamicInfiniteRealCondition() {
    bool retval = false;
    DEFAULT_SETUP(ce, db, false);
    retval = testDynamicInfiniteRealConditionImpl(ce, db, dm, hor);
    DEFAULT_TEARDOWN();
    return retval;
  }

};

class DecisionManagerTest {
public:
  static bool test() {
    runTest(testForwardDecisionHandling);
    return(true);
  }

private:
  static bool testForwardDecisionHandling() {
    bool retval = false;
    DEFAULT_SETUP(ce, db, false);
    retval = testForwardDecisionHandlingImpl(ce, db, dm, hor);
    DEFAULT_TEARDOWN();
    return retval;
  }
};

class CBPlannerTest {
public:
  static bool test() {
    runTest(testMakeMove);
    runTest(testCurrentState);
    runTest(testRetractMove);
    runTest(testNoBacktrackCase);
    runTest(testSubgoalOnceRule);
    runTest(testBacktrackCase);
    runTest(testTimeoutCase);
    return true;
  }
private:
  static bool testMakeMove() {
    bool retval = false;
    DEFAULT_SETUP_PLAN(ce, db, false);
    retval = testMakeMoveImpl(ce, db, planner);
    DEFAULT_TEARDOWN_PLAN();
    return retval;
  }

  static bool testCurrentState() {
    bool retval = false;
    DEFAULT_SETUP_PLAN(ce, db, false);
    retval = testCurrentStateImpl(ce, db, planner);
    DEFAULT_TEARDOWN_PLAN();
    return retval;
  }

  static bool testRetractMove() {
    bool retval = false;
    DEFAULT_SETUP_PLAN(ce, db, false);
    retval = testRetractMoveImpl(ce, db, planner);
    DEFAULT_TEARDOWN_PLAN();
    return retval;
  }

  static bool testNoBacktrackCase() {
    bool retval = false;
    DEFAULT_SETUP_PLAN(ce, db, false);
    retval = testNoBacktrackCaseImpl(ce, db, planner);
    DEFAULT_TEARDOWN_PLAN();
    return retval;
  }

  static bool testSubgoalOnceRule() {
    bool retval = false;
    DEFAULT_SETUP_PLAN(ce, db, false);
    retval = testSubgoalOnceRuleImpl(ce, db, planner);
    DEFAULT_TEARDOWN_PLAN();
    return retval;
  }

  static bool testBacktrackCase() {
    bool retval = false;
    DEFAULT_SETUP_PLAN(ce, db, false);
    retval = testBacktrackCaseImpl(ce, db, planner);
    DEFAULT_TEARDOWN_PLAN();
    return retval;
  }

  static bool testTimeoutCase() {
    bool retval = false;
    DEFAULT_SETUP_PLAN(ce, db, false);
    retval = testTimeoutCaseImpl(ce, db, planner);
    DEFAULT_TEARDOWN_PLAN();
    return retval;
  }
};    

class MultipleDecisionManagerTest {
public:
  static bool test() {
    runTest(testMultipleDMs);
    return(true);
  }
private:
  static bool testMultipleDMs() {
    bool retval = false;
    DEFAULT_SETUP(ce, db, false);
    retval = testMultipleDMsImpl(ce, db, dm, hor);
    DEFAULT_TEARDOWN();
    return retval;
  }
};


class DecisionPointTest {
public:
  static bool test() {
    runTest(testVariableDecisionCycle);
    runTest(testTokenDecisionCycle);
    runTest(testObjectDecisionCycle);
    runTest(testObjectAndObjectVariable);
    runTest(testObjectHorizon);
    return(true);
  }
private:
  static bool testVariableDecisionCycle() {
    bool retval = false;
    DEFAULT_SETUP_PLAN(ce, db, false);
    retval = testVariableDecisionCycleImpl(ce, db, planner);
    DEFAULT_TEARDOWN_PLAN();
    return retval;
  }
  static bool testTokenDecisionCycle() {
    bool retval = false;
    DEFAULT_SETUP_PLAN(ce, db, false);
    retval = testTokenDecisionCycleImpl(ce, db, hor, planner);
    DEFAULT_TEARDOWN_PLAN();
    return retval;
  }
  static bool testObjectDecisionCycle() {
    bool retval = false;
    DEFAULT_SETUP_PLAN(ce, db, false);
    retval = testObjectDecisionCycleImpl(ce, db, hor, planner);
    DEFAULT_TEARDOWN_PLAN();
    return retval;
  }
  static bool testObjectAndObjectVariable() {
    bool retval = false;
    DEFAULT_SETUP_PLAN(ce, db, false);
    retval = testObjectAndObjectVariableImpl(ce, db, hor, planner);
    DEFAULT_TEARDOWN_PLAN();
    return retval;
  }
  static bool testObjectHorizon() {
    bool retval = false;
    DEFAULT_SETUP_PLAN(ce, db, false);
    retval = testObjectHorizonImpl(ce, db, hor, planner);
    DEFAULT_TEARDOWN_PLAN();
    return retval;
  }
};

class TwoCyclePlanningTest {
public:
  static bool test() {
    runTest(testFindAnotherPlan);
    runTest(testAddSubgoalAfterPlanning);
    return(true);
  }
private:
  static bool testFindAnotherPlan() {
    bool retval = false;
    DEFAULT_SETUP_PLAN(ce, db, false);
    retval = testFindAnotherPlanImpl(ce, db, hor, planner);
    DEFAULT_TEARDOWN_PLAN();
    return retval;
  }
  static bool testAddSubgoalAfterPlanning() {
    bool retval = false;
    DEFAULT_SETUP_PLAN(ce, db, false);
    retval = testAddSubgoalAfterPlanningImpl(ce, db, hor, planner);
    DEFAULT_TEARDOWN_PLAN();
    return retval;
  }
};

int main() {
  Schema::instance();
  REGISTER_CONSTRAINT(EqualConstraint, "concurrent", "Default");
  REGISTER_CONSTRAINT(LessThanEqualConstraint, "precedes", "Default");
  REGISTER_CONSTRAINT(AddEqualConstraint, "StartEndDurationRelation", "Default");
  REGISTER_CONSTRAINT(ObjectTokenRelation, "ObjectTokenRelation", "Default");
  REGISTER_CONSTRAINT(EqualConstraint, "eq", "Default");
  REGISTER_CONSTRAINT(EqualConstraint, "Equal", "Default");
  REGISTER_CONSTRAINT(LessThanConstraint, "lt", "Default");
  REGISTER_CONSTRAINT(SubsetOfConstraint, "SubsetOf", "Default");
  REGISTER_CONSTRAINT(NotFalseConstraint, "notfalse", "Default");
  REGISTER_CONSTRAINT(BinaryCustomConstraint, "custom", "Default");
  for (int i = 0; i < 1; i++) {
    runTestSuite(DefaultSetupTest::test);
    runTestSuite(ConditionTest::test);
    runTestSuite(DecisionManagerTest::test);
    runTestSuite(CBPlannerTest::test);
    runTestSuite(MultipleDecisionManagerTest::test);
    runTestSuite(DecisionPointTest::test);
    runTestSuite(TwoCyclePlanningTest::test);
  }
  std::cout << "Finished" << std::endl;
  ConstraintLibrary::purgeAll();
  exit(0);
}
