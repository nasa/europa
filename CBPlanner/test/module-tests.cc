#include "CBPlannerDefs.hh"
#include "CBPlanner.hh"
#include "DecisionManager.hh"
#include "SubgoalOnceRule.hh"
#include "DMLogger.hh"
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
#include "CeLogger.hh"
#include "Utils.hh"

#include "PlanDatabaseDefs.hh"
#include "PlanDatabase.hh"
#include "Schema.hh"
#include "Object.hh"
#include "EventToken.hh"
#include "TokenVariable.hh"
#include "ObjectTokenRelation.hh"
#include "Timeline.hh"
#include "DbLogger.hh"

#include "RulesEngine.hh"
#include "Rule.hh"
#include "RuleInstance.hh"
#include "ObjectFilter.hh"
#include "TestRule.hh"

#include <iostream>
#include <string>

#define DEFAULT_SETUP(ce, db, schema, autoClose) \
    ConstraintEngine ce; \
    Schema schema; \
    PlanDatabase db(ce.getId(), schema.getId()); \
    new DefaultPropagator(LabelStr("Default"), ce.getId()); \
    RulesEngine re(db.getId()); \
    Horizon hor(0,200); \
    DecisionManager dm(db.getId()); \
    Id<DbLogger> dbLId; \
    if (loggingEnabled()) { \
      new CeLogger(std::cout, ce.getId()); \
      dbLId = (new DbLogger(std::cout, db.getId()))->getId(); \
      new DMLogger(std::cout, dm.getId()); \
    } \
    if (autoClose) \
      db.close();

#define DEFAULT_TEARDOWN() \
    delete (DbLogger*) dbLId;


#define DEFAULT_SETUP_PLAN(ce, db, schema, autoClose) \
    ConstraintEngine ce; \
    Schema schema; \
    PlanDatabase db(ce.getId(), schema.getId()); \
    new DefaultPropagator(LabelStr("Default"), ce.getId()); \
    RulesEngine re(db.getId()); \
    Horizon hor(0, 200); \
    CBPlanner planner(db.getId(), hor.getId()); \
    Id<DbLogger> dbLId; \
    if (loggingEnabled()) { \
      new CeLogger(std::cout, ce.getId()); \
      dbLId = (new DbLogger(std::cout, db.getId()))->getId(); \
      new DMLogger(std::cout, planner.getDecisionManager()); \
    } \
    if (autoClose) \
      db.close();

#define DEFAULT_TEARDOWN_PLAN() \
    delete (DbLogger*) dbLId;

class DefaultSetupTest {
public:
  static bool test() {
    runTest(testDefaultSetup);
    return(true);
  }

private:
  static bool testDefaultSetup() {
    DEFAULT_SETUP(ce, db, schema, false);
    assert(db.isClosed() == false);
    db.close();
    assert(db.isClosed() == true);
    DEFAULT_TEARDOWN();
    return(true);
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
    DEFAULT_SETUP(ce, db, schema, false);

    Condition cond(dm.getId());
    assert(!cond.hasChanged());

    assert(dm.getConditions().size() == 1);
    
    DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testHorizon() {
    DEFAULT_SETUP(ce, db, schema, true);

    Horizon hor1;
    int start, end;
    hor1.getHorizon(start,end);
    assert(start == -g_maxFiniteTime());
    assert(end == g_maxFiniteTime());

    Horizon hor2(0,200);
    hor2.getHorizon(start,end);
    assert(start == 0);
    assert(end == 200);

    hor2.setHorizon(0,400);
    hor2.getHorizon(start,end);
    assert(start == 0);
    assert(end == 400);

    return(true);
  }

  static bool testHorizonCondition() {
    DEFAULT_SETUP(ce, db, schema, false);

    HorizonCondition cond(hor.getId(), dm.getId());
    assert(cond.isPossiblyOutsideHorizon());
    assert(dm.getConditions().size() == 1);

    Timeline t(db.getId(), LabelStr("Objects"), LabelStr("t1"));
    db.close();
    IntervalToken tokenA(db.getId(), 
			 LabelStr("P1"), 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000));

    assert(ce.propagate());
    assert(cond.test(t.getId()));
    assert(cond.test(tokenA.getId()));
    assert(cond.test(tokenA.getStart()));
    assert(cond.test(tokenA.getEnd()));
    assert(cond.test(tokenA.getDuration()));

    //std::cout << " Decisions upon creation = 4 " << std::endl;

    assert(dm.getNumberOfDecisions() == 4);

    hor.setHorizon(200,400);
    assert(cond.hasChanged());
    assert(ce.propagate());
    assert(cond.test(t.getId()));
    assert(!cond.test(tokenA.getId()));
    assert(!cond.test(tokenA.getStart()));
    assert(!cond.test(tokenA.getEnd()));
    assert(!cond.test(tokenA.getDuration()));

    //std::cout << " Decisions after changing horizon = 0 " << std::endl;

    assert(dm.getNumberOfDecisions() == 0);

    cond.setNecessarilyOutsideHorizon();
    assert(cond.isNecessarilyOutsideHorizon());
    
    hor.setHorizon(0,50);
    assert(cond.hasChanged());
    assert(ce.propagate());
    assert(cond.test(t.getId()));
    assert(cond.test(tokenA.getId()));
    assert(cond.test(tokenA.getStart()));
    assert(cond.test(tokenA.getEnd()));
    assert(cond.test(tokenA.getDuration()));
    
    //std::cout << " Decisions after changing horizon = 4" << std::endl;

    assert(dm.getNumberOfDecisions() == 4);

    DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testTemporalVariableCondition() {
    DEFAULT_SETUP(ce, db, schema, false);

    TemporalVariableCondition cond(hor.getId(), dm.getId());
    assert(dm.getConditions().size() == 1);

    assert(cond.isStartIgnored());
    assert(cond.isEndIgnored());
    assert(cond.isDurationIgnored());
    assert(cond.isTemporalIgnored());
    assert(!cond.isHorizonOverlapAllowed());

    Timeline t(db.getId(), LabelStr("Objects"), LabelStr("t1"));
    db.close();
    IntervalToken tokenA(db.getId(), 
			 LabelStr("P1"), 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000));

    assert(ce.propagate());
    assert(cond.test(t.getId()));
    assert(cond.test(tokenA.getId()));
    assert(!cond.test(tokenA.getStart()));
    assert(!cond.test(tokenA.getEnd()));
    assert(!cond.test(tokenA.getDuration()));

    assert(dm.getNumberOfDecisions() == 1);
    
    assert(ce.propagate());
    cond.allowHorizonOverlap();
    assert(cond.isHorizonOverlapAllowed());

    assert(cond.test(t.getId()));
    assert(cond.test(tokenA.getId()));
    assert(!cond.test(tokenA.getStart()));
    assert(cond.test(tokenA.getEnd()));
    assert(!cond.test(tokenA.getDuration()));

    assert(dm.getNumberOfDecisions() == 2);

    assert(ce.propagate());
    cond.disallowHorizonOverlap();
    assert(!cond.isHorizonOverlapAllowed());
    
    assert(cond.test(t.getId()));
    assert(cond.test(tokenA.getId()));
    assert(!cond.test(tokenA.getStart()));
    assert(!cond.test(tokenA.getEnd()));
    assert(!cond.test(tokenA.getDuration()));

    assert(dm.getNumberOfDecisions() == 1);

    cond.setIgnoreStart(false);
    cond.setIgnoreEnd(false);
    assert(!cond.isStartIgnored());
    assert(!cond.isEndIgnored());
    assert(cond.isDurationIgnored());
    assert(!cond.isTemporalIgnored());
    assert(ce.propagate());

    assert(cond.test(t.getId()));
    assert(cond.test(tokenA.getId()));
    assert(cond.test(tokenA.getStart()));
    assert(cond.test(tokenA.getEnd()));
    assert(!cond.test(tokenA.getDuration()));

    assert(dm.getNumberOfDecisions() == 3);

    DEFAULT_TEARDOWN();
    return(true);
  }

  static bool testDynamicInfiniteRealCondition() {
    DEFAULT_SETUP(ce, db, schema, false);

    DynamicInfiniteRealCondition cond(dm.getId());
    assert(dm.getConditions().size() == 1);

    std::list<Prototype::LabelStr> values;
    values.push_back(Prototype::LabelStr("L1"));
    values.push_back(Prototype::LabelStr("L4"));
    values.push_back(Prototype::LabelStr("L2"));
    values.push_back(Prototype::LabelStr("L5"));
    values.push_back(Prototype::LabelStr("L3"));

    Variable<LabelSet> v0(ce.getId(), LabelSet(values, true));
    Variable<LabelSet> v1(ce.getId(), LabelSet(values, false));
    Variable<IntervalDomain> v2(ce.getId(), IntervalDomain(1, 20));
    Variable<IntervalIntDomain> v3(ce.getId(), IntervalIntDomain(1, 20));
    Variable<IntervalIntDomain> v4(ce.getId(), IntervalIntDomain());

    Timeline t(db.getId(), LabelStr("Objects"), LabelStr("t1"));
    db.close();
    IntervalToken tokenA(db.getId(), 
			 LabelStr("P1"), 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000));

    assert(ce.propagate());

    assert(cond.test(t.getId()));
    assert(cond.test(tokenA.getId()));
    assert(cond.test(v0.getId()));
    assert(!cond.test(v1.getId()));
    assert(!cond.test(v2.getId()));
    assert(cond.test(v3.getId()));
    assert(!cond.test(v4.getId()));
    assert(cond.test(tokenA.getDuration()));

    assert(dm.getNumberOfDecisions() == 6);

    DEFAULT_TEARDOWN();
    return(true);
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
    DEFAULT_SETUP(ce, db, schema, false);

    HorizonCondition hcond(hor.getId(), dm.getId());
    TemporalVariableCondition tcond(hor.getId(), dm.getId());
    DynamicInfiniteRealCondition dcond(dm.getId());

    assert(dm.getConditions().size() == 3);

    std::list<Prototype::LabelStr> values;
    values.push_back(Prototype::LabelStr("L1"));
    values.push_back(Prototype::LabelStr("L4"));
    values.push_back(Prototype::LabelStr("L2"));
    values.push_back(Prototype::LabelStr("L5"));
    values.push_back(Prototype::LabelStr("L3"));

    Variable<LabelSet> v0(ce.getId(), LabelSet(values, true));
    Variable<LabelSet> v1(ce.getId(), LabelSet(values, false));
    Variable<IntervalDomain> v2(ce.getId(), IntervalDomain(1, 20));
    Variable<IntervalIntDomain> v3(ce.getId(), IntervalIntDomain(1, 20));
    Variable<IntervalIntDomain> v4(ce.getId(), IntervalIntDomain());

    Timeline t(db.getId(), LabelStr("Objects"), LabelStr("t1"));
    db.close();
    IntervalToken tokenA(db.getId(), 
			 LabelStr("P1"), 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000));

    assert(ce.propagate());

    assert(dm.getNumberOfDecisions() == 3);

    dm.assignNextDecision();

    assert(dm.getNumberOfDecisions() == 3);

    dm.assignNextDecision();

    assert(dm.getNumberOfDecisions() == 2);

    dm.assignNextDecision();

    assert(dm.getNumberOfDecisions() == 1);

    dm.assignNextDecision();

    assert(dm.getNumberOfDecisions() == 0);

    IntervalToken tokenB(db.getId(), 
			 LabelStr("P1"), 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000));

    assert(ce.propagate());

    assert(dm.getNumberOfDecisions() == 1);

    dm.assignNextDecision();

    assert(dm.getNumberOfDecisions() == 0);

    DEFAULT_TEARDOWN();
    return true;
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
    //    runTest(testBacktrackCase);
    return true;
  }
private:
  static bool testMakeMove() {
    DEFAULT_SETUP_PLAN(ce, db, schema, false);

    std::list<Prototype::LabelStr> values;
    values.push_back(Prototype::LabelStr("L1"));
    values.push_back(Prototype::LabelStr("L4"));
    values.push_back(Prototype::LabelStr("L2"));
    values.push_back(Prototype::LabelStr("L5"));
    values.push_back(Prototype::LabelStr("L3"));

    Variable<LabelSet> v0(ce.getId(), LabelSet(values, true));
    Variable<LabelSet> v1(ce.getId(), LabelSet(values, false));
    Variable<IntervalDomain> v2(ce.getId(), IntervalDomain(1, 20));
    Variable<IntervalIntDomain> v3(ce.getId(), IntervalIntDomain(1, 20));
    Variable<IntervalIntDomain> v4(ce.getId(), IntervalIntDomain());

    Timeline t(db.getId(), LabelStr("Objects"), LabelStr("t1"));
    db.close();
    IntervalToken tokenA(db.getId(), 
			 LabelStr("P1"), 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000));

    IntervalToken tokenB(db.getId(), 
			 LabelStr("P1"), 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000));

    for (int i = 0; i < 5; ++i) {
      if (!planner.getDecisionManager()->assignNextDecision())
	return false;
      /*
      std::cout << "\nOpen Decisions:" << std::endl;
      planner.getDecisionManager()->printOpenDecisions();
      std::cout << "ClosedDecisions:" << std::endl; 
      planner.getDecisionManager()->printClosedDecisions();
      */
    }    
    assert(planner.getDecisionManager()->getNextDecision().isNoId());

    DEFAULT_TEARDOWN_PLAN();
    return(true);
  }

  static bool testCurrentState() {
    DEFAULT_SETUP_PLAN(ce, db, schema, false);

    Timeline t(db.getId(), LabelStr("Objects"), LabelStr("t1"));
    db.close();
    IntervalToken tokenA(db.getId(), 
			 LabelStr("P1"), 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000));

    assert(planner.getDecisionManager()->getCurrentDecision().isNoId());

    if (!planner.getDecisionManager()->assignNextDecision())
      return false;

    assert(TokenDecisionPointId::convertable(planner.getDecisionManager()->getCurrentDecision()));
    TokenDecisionPointId tokdec = planner.getDecisionManager()->getCurrentDecision();
    assert(tokdec->getToken() == tokenA.getId());
    assert(!planner.getDecisionManager()->getCurrentChoice().isNoId());
    assert(tokdec->getCurrent() == planner.getDecisionManager()->getCurrentChoice());

    planner.getDecisionManager()->synchronize();

    DEFAULT_TEARDOWN_PLAN();
    return(true);
  }

  static bool testRetractMove() {
    DEFAULT_SETUP_PLAN(ce, db, schema, false);

    Timeline t(db.getId(), LabelStr("Objects"), LabelStr("t1"));
    db.close();
    IntervalToken tokenA(db.getId(), 
			 LabelStr("P1"), 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000));

    if (!planner.getDecisionManager()->assignNextDecision())
      return false;

    assert(planner.getDecisionManager()->getClosedDecisions().size() == 1);

    //std::cout << "RETRACTING" << std::endl;

    assert(planner.getDecisionManager()->retractCurrentDecision());

    DEFAULT_TEARDOWN_PLAN();
    return(true);
  }

  static bool testNoBacktrackCase() {
    DEFAULT_SETUP_PLAN(ce, db, schema, false);

    Timeline timeline(db.getId(),LabelStr("AllObjects"), LabelStr("t1"));
    db.close();

    IntervalToken tokenA(db.getId(), 
			 LabelStr("P1"), 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);

    std::list<Prototype::LabelStr> values;
    values.push_back(Prototype::LabelStr("L1"));
    values.push_back(Prototype::LabelStr("L4"));
    values.push_back(Prototype::LabelStr("L2"));
    values.push_back(Prototype::LabelStr("L5"));
    values.push_back(Prototype::LabelStr("L3"));
    tokenA.addParameter(LabelSet(values, true));
    tokenA.addParameter(LabelSet(values, false));
    tokenA.addParameter(IntervalIntDomain(1, 20));
    tokenA.close();
    
    bool res = planner.run(loggingEnabled());
    assert(res == 1);

    const std::list<DecisionPointId>& closed = planner.getClosedDecisions();

    assert(closed.size() == 4);
    assert(closed.size() == planner.getTime());
    assert(planner.getTime() == planner.getDepth());

    DEFAULT_TEARDOWN_PLAN();
    return(true);
  }

  static bool testSubgoalOnceRule() {
    DEFAULT_SETUP_PLAN(ce, db, schema, false);

    Timeline timeline(db.getId(),LabelStr("AllObjects"), LabelStr("t1"));
    db.close();

    SubgoalOnceRule r(LabelStr("P1"), 0);

    IntervalToken t0(db.getId(), LabelStr("P1"), true, 		     
		     IntervalIntDomain(0, 1000),
		     IntervalIntDomain(0, 1000),
		     IntervalIntDomain(1, 1000));
    t0.activate();
    bool res(ce.propagate());
    assert(res);
    TokenSet slaves = t0.getSlaves();
    assert(slaves.size() == 1);

    TokenSet::iterator it = slaves.begin();
    (*it)->activate();
    res = ce.propagate();
    assert(res);
    TokenSet slaves1 = (*it)->getSlaves();
    assert(slaves1.size() == 1);

    TokenSet::iterator it1 = slaves1.begin();
    (*it1)->activate();
    res = ce.propagate();
    assert(!res);

    DEFAULT_TEARDOWN_PLAN();
    return true;
  }

  static bool testBacktrackCase() {
    DEFAULT_SETUP_PLAN(ce, db, schema, false);

    Timeline timeline(db.getId(),LabelStr("AllObjects"), LabelStr("t1"));
    db.close();

    SubgoalOnceRule r(LabelStr("P1"), 0);

    std::list<Prototype::LabelStr> values;
    values.push_back(Prototype::LabelStr("L1"));
    values.push_back(Prototype::LabelStr("L4"));
    values.push_back(Prototype::LabelStr("L2"));
    values.push_back(Prototype::LabelStr("L5"));
    values.push_back(Prototype::LabelStr("L3"));

    IntervalToken tokenA(db.getId(), 
			 LabelStr("P1"), 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);
    tokenA.addParameter(LabelSet(values, true));
    // can't merge tokens with parameters that are dynamic domains
    //tokenA.addParameter(LabelSet(values, false));
    tokenA.addParameter(IntervalIntDomain(1, 20));
    tokenA.close();

    IntervalToken tokenB(db.getId(), 
			 LabelStr("P2"), 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);
    tokenB.addParameter(LabelSet(values, true));
    // can't merge tokens with parameters that are dynamic domains
    //tokenB.addParameter(LabelSet(values, false));
    tokenB.addParameter(IntervalIntDomain(1, 20));
    tokenB.close();

    IntervalToken tokenC(db.getId(), 
			 LabelStr("P1"), 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);
    tokenC.addParameter(LabelSet(values, true));
    // can't merge tokens with parameters that are dynamic domains
    //tokenC.addParameter(LabelSet(values, false));
    tokenC.addParameter(IntervalIntDomain(1, 20));
    tokenC.close();

    // an equivalence constraint between the start times will cause the
    // planner to retract the activate decision and use the merge decision
    // instead. 
    std::vector<ConstrainedVariableId> scope;
    scope.push_back(tokenA.getStart());
    scope.push_back(tokenB.getStart());

    ConstraintLibrary::createConstraint(LabelStr("eq"), ce.getId(), scope);

    int res = planner.run(loggingEnabled(), 100);
    assert(res == -3);

    DEFAULT_TEARDOWN_PLAN();
    return(true);
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
    DEFAULT_SETUP(ce, db, schema, false);

    DEFAULT_TEARDOWN();
    return(true);
  }
};

int main() {
  REGISTER_NARY(EqualConstraint, "concurrent", "Default");
  REGISTER_NARY(LessThanEqualConstraint, "before", "Default");
  REGISTER_NARY(AddEqualConstraint, "StartEndDurationRelation", "Default");
  REGISTER_NARY(ObjectTokenRelation, "ObjectTokenRelation", "Default");
  REGISTER_NARY(EqualConstraint, "eq", "Default");
  REGISTER_NARY(EqualConstraint, "Equal", "Default");
  REGISTER_NARY(LessThanConstraint, "lt", "Default");
  REGISTER_UNARY(SubsetOfConstraint, "Singleton", "Default");
  REGISTER_UNARY(SubsetOfConstraint, "SubsetOf", "Default");

  runTestSuite(DefaultSetupTest::test);
  runTestSuite(ConditionTest::test);
  runTestSuite(DecisionManagerTest::test);
  runTestSuite(CBPlannerTest::test);
  runTestSuite(MultipleDecisionManagerTest::test);
  std::cout << "Finished" << std::endl;
  ConstraintLibrary::purgeAll();
}
