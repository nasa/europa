#include "CBPlannerDefs.hh"
#include "CBPlanner.hh"
#include "DecisionManager.hh"
#include "SubgoalOnceRule.hh"
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

#include "TestSupport.hh"

#include "NumericDomain.hh"
#include "StringDomain.hh"
#include "Choice.hh"
#include "ObjectDecisionPoint.hh"
#include "ConditionalRule.hh"
#include "NotFalseConstraint.hh"
#include "ConstrainedVariableDecisionPoint.hh"

#include <iostream>
#include <string>

#define DEFAULT_SETUP(ce, db, autoClose) \
    ConstraintEngine ce; \
    initCBPTestSchema(); \
    PlanDatabase db(ce.getId(), Schema::instance()); \
    new DefaultPropagator(LabelStr("Default"), ce.getId()); \
    new DefaultPropagator(LabelStr("Temporal"), ce.getId()); \
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
    new DefaultPropagator(LabelStr("Temporal"), ce.getId()); \
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
    new DefaultPropagator(LabelStr("Temporal"), ce.getId()); \
    RulesEngine re(db.getId()); \
    Horizon hor(0, 200); \
    CBPlanner planner(db.getId(), hor.getId()); \
    HSTSHeuristics heuristics(db.getId()); 

#define DEFAULT_TEARDOWN_PLAN_HEURISTICS()

extern bool loggingEnabled();

  /**
   * @brief Creates the type specifications required for testing
   */
  void initCBPTestSchema(){
    const SchemaId& schema = Schema::instance();
    schema->reset();
    schema->addObjectType("Objects");

    schema->addPredicate("Objects.PredicateA");
    schema->addMember("Objects.PredicateA", IntervalIntDomain().getTypeName(), "IntervalIntParam");

    schema->addPredicate("Objects.PredicateB");
    schema->addPredicate("Objects.PredicateC");
    schema->addPredicate("Objects.PredicateD");
    schema->addPredicate("Objects.PADDED");

    schema->addPredicate("Objects.P1");
    schema->addMember("Objects.P1", LabelSet().getTypeName(), "LabelSetParam0");
    schema->addMember("Objects.P1", LabelSet().getTypeName(), "LabelSetParam1");
    schema->addMember("Objects.P1", IntervalIntDomain().getTypeName(), "IntervalIntParam");

    schema->addPredicate("Objects.P1True");
    schema->addMember("Objects.P1True", BoolDomain().getTypeName(), "BoolParam");
    schema->addPredicate("Objects.P1False");
  }

  static void makeTestToken(IntervalToken& token, const std::list<double>& values){
    token.addParameter(LabelSet(values), "LabelSetParam0");
    LabelSet leaveOpen;
    leaveOpen.insert(values);
    token.addParameter(leaveOpen, "LabelSetParam1");
    token.addParameter(IntervalIntDomain(1, 20), "IntervalIntParam");
    token.close();
  }

class DefaultSetupTest {
public:
  static bool test() {
    runTest(testDefaultSetup);
    return(true);
  }

private:
  static bool testDefaultSetup() {
    DEFAULT_SETUP(ce, db, false);
    assertTrue(db.isClosed() == false);
    db.close();
    assertTrue(db.isClosed() == true);
    DEFAULT_TEARDOWN();
    return true;
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
    DEFAULT_SETUP(ce, db, false);
    Condition cond(dm.getId());
    assertTrue(!cond.hasChanged());
  
    assertTrue(dm.getConditions().size() == 1);
    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testHorizon() {
    DEFAULT_SETUP(ce, db, true);
    Horizon hor1;
    int start, end;
    hor1.getHorizon(start,end);
    assertTrue(start == -MAX_FINITE_TIME);
    assertTrue(end == MAX_FINITE_TIME);
  
    Horizon hor2(0,200);
    hor2.getHorizon(start,end);
    assertTrue(start == 0);
    assertTrue(end == 200);
  
    hor2.setHorizon(0,400);
    hor2.getHorizon(start,end);
    assertTrue(start == 0);
    assertTrue(end == 400);
    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testHorizonCondition() {
    DEFAULT_SETUP(ce, db, false);
    HorizonCondition cond(hor.getId(), dm.getId());
    assertTrue(cond.isPossiblyOutsideHorizon());
    assertTrue(dm.getConditions().size() == 1);

    Timeline t(db.getId(), LabelStr("Objects"), LabelStr("t1"));
    db.close();
    IntervalToken tokenA(db.getId(), 
			 "Objects.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000));

    assertTrue(ce.propagate());
    assertTrue(cond.test(t.getId()));
    assertTrue(cond.test(tokenA.getId()));
    assertTrue(cond.test(tokenA.getStart()));
    assertTrue(cond.test(tokenA.getEnd()));
    assertTrue(cond.test(tokenA.getDuration()));

    //std::cout << " Decisions upon creation = 4 " << std::endl;

    assertTrue(dm.getNumberOfDecisions() == 4);

    hor.setHorizon(200,400);
    assertTrue(cond.hasChanged());
    assertTrue(ce.propagate());
    assertTrue(cond.test(t.getId()));
    assertTrue(!cond.test(tokenA.getId()));
    assertTrue(!cond.test(tokenA.getStart()));
    assertTrue(!cond.test(tokenA.getEnd()));
    assertTrue(!cond.test(tokenA.getDuration()));

    //std::cout << " Decisions after changing horizon = 0 " << std::endl;

    assertTrue(dm.getNumberOfDecisions() == 0);

    cond.setNecessarilyOutsideHorizon();
    assertTrue(cond.isNecessarilyOutsideHorizon());
    
    hor.setHorizon(0,50);
    assertTrue(cond.hasChanged());
    assertTrue(ce.propagate());
    assertTrue(cond.test(t.getId()));
    assertTrue(cond.test(tokenA.getId()));
    assertTrue(cond.test(tokenA.getStart()));
    assertTrue(cond.test(tokenA.getEnd()));
    assertTrue(cond.test(tokenA.getDuration()));
    
    //std::cout << " Decisions after changing horizon = 4" << std::endl;

    assertTrue(dm.getNumberOfDecisions() == 4);
    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testTemporalVariableCondition() {
    DEFAULT_SETUP(ce, db, false);
    TemporalVariableCondition cond(hor.getId(), dm.getId());
    assertTrue(dm.getConditions().size() == 1);

    assertTrue(cond.isStartIgnored());
    assertTrue(cond.isEndIgnored());
    assertTrue(cond.isDurationIgnored());
    assertTrue(cond.isTemporalIgnored());
    assertTrue(!cond.isHorizonOverlapAllowed());

    Timeline t(db.getId(), LabelStr("Objects"), LabelStr("t1"));
    db.close();
    IntervalToken tokenA(db.getId(), 
			 "Objects.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000));

    assertTrue(ce.propagate());
    assertTrue(cond.test(t.getId()));
    assertTrue(cond.test(tokenA.getId()));
    assertTrue(!cond.test(tokenA.getStart()));
    assertTrue(!cond.test(tokenA.getEnd()));
    assertTrue(!cond.test(tokenA.getDuration()));

    assertTrue(dm.getNumberOfDecisions() == 1);
    
    assertTrue(ce.propagate());
    cond.allowHorizonOverlap();
    assertTrue(cond.isHorizonOverlapAllowed());

    assertTrue(cond.test(t.getId()));
    assertTrue(cond.test(tokenA.getId()));
    assertTrue(!cond.test(tokenA.getStart()));
    assertTrue(cond.test(tokenA.getEnd()));
    assertTrue(!cond.test(tokenA.getDuration()));

    assertTrue(dm.getNumberOfDecisions() == 2);

    assertTrue(ce.propagate());
    cond.disallowHorizonOverlap();
    assertTrue(!cond.isHorizonOverlapAllowed());
    
    assertTrue(cond.test(t.getId()));
    assertTrue(cond.test(tokenA.getId()));
    assertTrue(!cond.test(tokenA.getStart()));
    assertTrue(!cond.test(tokenA.getEnd()));
    assertTrue(!cond.test(tokenA.getDuration()));

    assertTrue(dm.getNumberOfDecisions() == 1);

    cond.setIgnoreStart(false);
    cond.setIgnoreEnd(false);
    assertTrue(!cond.isStartIgnored());
    assertTrue(!cond.isEndIgnored());
    assertTrue(cond.isDurationIgnored());
    assertTrue(!cond.isTemporalIgnored());
    assertTrue(ce.propagate());

    assertTrue(cond.test(t.getId()));
    assertTrue(cond.test(tokenA.getId()));
    assertTrue(cond.test(tokenA.getStart()));
    assertTrue(cond.test(tokenA.getEnd()));
    assertTrue(!cond.test(tokenA.getDuration()));

    assertTrue(dm.getNumberOfDecisions() == 3);
    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testDynamicInfiniteRealCondition() {
    DEFAULT_SETUP(ce, db, false);
    DynamicInfiniteRealCondition cond(dm.getId());
    assertTrue(dm.getConditions().size() == 1);

    std::list<double> values;
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L4"));
    values.push_back(LabelStr("L2"));
    values.push_back(LabelStr("L5"));
    values.push_back(LabelStr("L3"));

    Variable<LabelSet> v0(ce.getId(), LabelSet(values));
    LabelSet leaveOpen;
    leaveOpen.insert(values);
    Variable<LabelSet> v1(ce.getId(), leaveOpen);
    Variable<IntervalDomain> v2(ce.getId(), IntervalDomain(1, 20));
    Variable<IntervalIntDomain> v3(ce.getId(), IntervalIntDomain(1, 20));
    Variable<IntervalIntDomain> v4(ce.getId(), IntervalIntDomain());

    Timeline t(db.getId(), LabelStr("Objects"), LabelStr("t1"));
    db.close();
    IntervalToken tokenA(db.getId(), 
			 "Objects.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000));

    assertTrue(ce.propagate());

    assertTrue(cond.test(t.getId()));
    assertTrue(cond.test(tokenA.getId()));
    assertTrue(cond.test(v0.getId()));
    assertTrue(!cond.test(v1.getId()));
    assertTrue(!cond.test(v2.getId()));
    assertTrue(cond.test(v3.getId()));
    assertTrue(!cond.test(v4.getId()));
    assertTrue(cond.test(tokenA.getDuration()));

    assertTrue(dm.getNumberOfDecisions() == 6);

    DEFAULT_TEARDOWN();
    return true;
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
    DEFAULT_SETUP(ce, db, false);
    HorizonCondition hcond(hor.getId(), dm.getId());
    TemporalVariableCondition tcond(hor.getId(), dm.getId());
    DynamicInfiniteRealCondition dcond(dm.getId());

    assertTrue(dm.getConditions().size() == 3);

    std::list<double> values;
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L4"));
    values.push_back(LabelStr("L2"));
    values.push_back(LabelStr("L5"));
    values.push_back(LabelStr("L3"));

    Variable<LabelSet> v0(ce.getId(), LabelSet(values));
    LabelSet leaveOpen;
    leaveOpen.insert(values);
    Variable<LabelSet> v1(ce.getId(), leaveOpen);
    Variable<IntervalDomain> v2(ce.getId(), IntervalDomain(1, 20));
    Variable<IntervalIntDomain> v3(ce.getId(), IntervalIntDomain(1, 20));
    Variable<IntervalIntDomain> v4(ce.getId(), IntervalIntDomain());

    Timeline t(db.getId(), LabelStr("Objects"), LabelStr("t1"));
    db.close();
    IntervalToken tokenA(db.getId(), 
			 "Objects.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000));

    assertTrue(ce.propagate());

    assertTrue(dm.getNumberOfDecisions() == 3);

    dm.assignDecision();

    assertTrue(dm.getNumberOfDecisions() == 3);

    dm.assignDecision();

    assertTrue(dm.getNumberOfDecisions() == 2);

    dm.assignDecision();

    assertTrue(dm.getNumberOfDecisions() == 1);

    dm.assignDecision();

    assertTrue(dm.getNumberOfDecisions() == 0);

    IntervalToken tokenB(db.getId(), 
			 "Objects.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000));

    assertTrue(ce.propagate());

    dm.assignDecision();

    assertTrue(dm.getNumberOfDecisions() == 0);
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
    runTest(testBacktrackCase);
    runTest(testTimeoutCase);
    return true;
  }
private:
  static bool testMakeMove() {
    DEFAULT_SETUP_PLAN(ce, db, false);
    std::list<double> values;
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L4"));
    values.push_back(LabelStr("L2"));
    values.push_back(LabelStr("L5"));
    values.push_back(LabelStr("L3"));

    Variable<LabelSet> v0(ce.getId(), LabelSet(values));
    LabelSet leaveOpen;
    leaveOpen.insert(values);
    Variable<LabelSet> v1(ce.getId(), leaveOpen);
    Variable<IntervalDomain> v2(ce.getId(), IntervalDomain(1, 20));
    Variable<IntervalIntDomain> v3(ce.getId(), IntervalIntDomain(1, 20));
    Variable<IntervalIntDomain> v4(ce.getId(), IntervalIntDomain());

    Timeline t(db.getId(), LabelStr("Objects"), LabelStr("t1"));
    db.close();
    IntervalToken tokenA(db.getId(), 
			 "Objects.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000));

    IntervalToken tokenB(db.getId(), 
			 "Objects.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000));

    // Should make all the decisions availabale
    for (int i = 0; i < 5; ++i) {
      if (!planner.getDecisionManager()->assignDecision())
	return false;
      /*
	std::cout << "\nOpen Decisions:" << std::endl;
	planner.getDecisionManager()->printOpenDecisions();
	std::cout << "ClosedDecisions:" << std::endl; 
	planner.getDecisionManager()->printClosedDecisions();
      */
      
    }    
    
    assertTrue(!planner.getDecisionManager()->assignDecision());
    DEFAULT_TEARDOWN_PLAN();
    return true;
  }

  static bool testCurrentState() {
    DEFAULT_SETUP_PLAN(ce, db, false);
    Timeline t(db.getId(), LabelStr("Objects"), LabelStr("t1"));
    db.close();
    IntervalToken tokenA(db.getId(), 
			 "Objects.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000));

    assertTrue(planner.getDecisionManager()->getCurrentDecision().isNoId());

    if (!planner.getDecisionManager()->assignDecision())
      return false;

    assertTrue(TokenDecisionPointId::convertable(planner.getDecisionManager()->getCurrentDecision()));
    TokenDecisionPointId tokdec = planner.getDecisionManager()->getCurrentDecision();
    assertTrue(tokdec->getToken() == tokenA.getId());
    assertTrue(!planner.getDecisionManager()->getCurrentChoice().isNoId());
    assertTrue(tokdec->getCurrent() == planner.getDecisionManager()->getCurrentChoice());

    planner.getDecisionManager()->synchronize();
    DEFAULT_TEARDOWN_PLAN();
    return true;
  }

  static bool testRetractMove() {
    DEFAULT_SETUP_PLAN(ce, db, false);
    Timeline t(db.getId(), LabelStr("Objects"), LabelStr("t1"));
    db.close();
    IntervalToken tokenA(db.getId(), 
			 "Objects.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000));

    if (!planner.getDecisionManager()->assignDecision())
      return false;

    assertTrue(planner.getDecisionManager()->getClosedDecisions().size() == 1);

    //std::cout << "RETRACTING" << std::endl;

    assertTrue(planner.getDecisionManager()->retractDecision());
    DEFAULT_TEARDOWN_PLAN();
    return true;
  }

  static bool testNoBacktrackCase() {
    DEFAULT_SETUP_PLAN(ce, db, false);
    Timeline timeline(db.getId(),LabelStr("Objects"), LabelStr("t1"));
    db.close();

    IntervalToken tokenA(db.getId(), 
			 LabelStr("Objects.P1"), 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);

    std::list<double> values;
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L4"));
    values.push_back(LabelStr("L2"));
    values.push_back(LabelStr("L5"));
    values.push_back(LabelStr("L3"));

    makeTestToken(tokenA, values);
    
    CBPlanner::Status res = planner.run();
    assertTrue(res == CBPlanner::PLAN_FOUND);

    const std::list<DecisionPointId>& closed = planner.getClosedDecisions();

    assertTrue(planner.getTime() == planner.getDepth());
    assertTrue(closed.size() == planner.getTime());
    assertTrue(closed.size() == 4);
    DEFAULT_TEARDOWN_PLAN();
    return true;
  }

  static bool testSubgoalOnceRule() {
    DEFAULT_SETUP_PLAN(ce, db, false);
    Timeline timeline(db.getId(),LabelStr("Objects"), LabelStr("t1"));
    db.close();

    SubgoalOnceRule r("Objects.P1", 0);

    IntervalToken t0(db.getId(), "Objects.P1", true, 		     
		     IntervalIntDomain(0, 1000),
		     IntervalIntDomain(0, 1000),
		     IntervalIntDomain(1, 1000));
    t0.activate();
    bool res(ce.propagate());
    assertTrue(res);
    TokenSet slaves = t0.getSlaves();
    assertTrue(slaves.size() == 1);

    TokenSet::iterator it = slaves.begin();
    (*it)->activate();
    res = ce.propagate();
    assertTrue(res);
    TokenSet slaves1 = (*it)->getSlaves();
    assertTrue(slaves1.size() == 1);

    TokenSet::iterator it1 = slaves1.begin();
    (*it1)->activate();
    res = ce.propagate();
    assertTrue(!res);
    DEFAULT_TEARDOWN_PLAN();
    return true;
  }

  static bool testBacktrackCase() {
    DEFAULT_SETUP_PLAN(ce, db, false);
    Timeline timeline(db.getId(),LabelStr("Objects"), LabelStr("t1"));
    db.close();

    SubgoalOnceRule r("Objects.P1", 0);

    std::list<double> values;
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L4"));
    values.push_back(LabelStr("L2"));
    values.push_back(LabelStr("L5"));
    values.push_back(LabelStr("L3"));

    IntervalToken tokenA(db.getId(), 
			 "Objects.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);
    tokenA.addParameter(LabelSet(values), "LabelSetParam0");
    // can't merge tokens with parameters that are dynamic domains
    //tokenA.addParameter(LabelSet(values, false));
    tokenA.addParameter(IntervalIntDomain(1, 20), "IntervalIntParam");
    tokenA.close();

    IntervalToken tokenB(db.getId(), 
			 "Objects.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);
    tokenB.addParameter(LabelSet(values), "LabelSetParam0");
    // can't merge tokens with parameters that are dynamic domains
    //tokenB.addParameter(LabelSet(values, false));
    tokenB.addParameter(IntervalIntDomain(1, 20), "IntervalIntParam");
    tokenB.close();
    
    IntervalToken tokenC(db.getId(), 
			 "Objects.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);
    tokenC.addParameter(LabelSet(values), "LabelSetParam0");
    // can't merge tokens with parameters that are dynamic domains
    //tokenC.addParameter(LabelSet(values, false));
    tokenC.addParameter(IntervalIntDomain(1, 20), "IntervalIntParam");
    tokenC.close();

    // an equivalence constraint between the start times will cause the
    // planner to retract the activate decision and use the merge decision
    // instead. 
    std::vector<ConstrainedVariableId> scope;
    scope.push_back(tokenA.getStart());
    scope.push_back(tokenB.getStart());

    ConstraintLibrary::createConstraint(LabelStr("eq"), ce.getId(), scope);

    CBPlanner::Status res = planner.run(100);

    assertTrue(res == CBPlanner::SEARCH_EXHAUSTED);
    assertTrue(planner.getClosedDecisions().empty());
    DEFAULT_TEARDOWN_PLAN();
    return true;
  }

  static bool testTimeoutCase() {
    DEFAULT_SETUP_PLAN(ce, db, false);
    Timeline t1(db.getId(),LabelStr("Objects"), LabelStr("t1"));
    Timeline t2(db.getId(),LabelStr("Objects"), LabelStr("t2"));
    Object o1(db.getId(),LabelStr("Objects"),LabelStr("o1"));
    db.close();

    std::list<double> values;
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L4"));
    values.push_back(LabelStr("L2"));
    values.push_back(LabelStr("L5"));
    values.push_back(LabelStr("L3"));

    IntervalToken tokenA(db.getId(), 
			 "Objects.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);
    makeTestToken(tokenA, values);

    IntervalToken tokenB(db.getId(), 
			 "Objects.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);

    makeTestToken(tokenB, values);

    IntervalToken tokenC(db.getId(), 
			 "Objects.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);

    makeTestToken(tokenC, values);
    

    IntervalToken tokenD(db.getId(), 
			 "Objects.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);

    makeTestToken(tokenD, values);

    IntervalToken tokenE(db.getId(), 
			 "Objects.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 LabelStr("o1"), false);

    makeTestToken(tokenE, values);

    IntervalToken tokenF(db.getId(), 
			 "Objects.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 LabelStr("o1"), false);

    makeTestToken(tokenF, values);

    IntervalToken tokenG(db.getId(), 
			 "Objects.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);

    makeTestToken(tokenG, values);
    
    CBPlanner::Status res = planner.run(20);
    assertTrue(res == CBPlanner::TIMEOUT_REACHED);

    const std::list<DecisionPointId>& closed = planner.getClosedDecisions();

    assertTrue(closed.size() == 20);
    assertTrue(closed.size() == planner.getTime());
    assertTrue(planner.getTime() == planner.getDepth());
    DEFAULT_TEARDOWN_PLAN();
    return true;
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
    DEFAULT_SETUP(ce, db, false);
    DEFAULT_TEARDOWN();
    return true;
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
    DEFAULT_SETUP_PLAN(ce, db, false);

    std::list<double> values;
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L4"));

    Variable<LabelSet> v0(ce.getId(), LabelSet(values));
    LabelSet leaveOpen;
    leaveOpen.insert(values);
    Variable<LabelSet> v1(ce.getId(), leaveOpen);
    Variable<IntervalDomain> v2(ce.getId(), IntervalDomain(1, 2));
    Variable<IntervalIntDomain> v3(ce.getId(), IntervalIntDomain(1, 2));
    Variable<IntervalIntDomain> v4(ce.getId(), IntervalIntDomain());
    Variable<NumericDomain> v5(ce.getId(), NumericDomain());
    v5.insert(5);
    v5.insert(23);
    v5.close();
    Variable<BoolDomain> v6(ce.getId(), BoolDomain());

    // add a constraint between v5 and v6 such that first value of v5
    // implies first value for v6, but ther's also a constraint on v6 that
    // forbids the first value.

    BinaryCustomConstraint c1(LabelStr("custom"), LabelStr("Default"), ce.getId(), makeScope(v3.getId(), v6.getId()));

    //    NotFalseConstraint c2(LabelStr("neqfalse"), LabelStr("Default"), ce.getId(), makeScope(v6.getId()));
    
    Timeline t(db.getId(), LabelStr("Objects"), LabelStr("t1"));
    db.close();

    //    int numDecs = 4;
    //    int i=0;
    CBPlanner::Status result;
    for (;;) { /* Forever: only way out is to return */
      /*
      std::cout << std::endl;
      std::cout << "Planner step = " << i++ << std::endl;
      std::cout << "Depth = " << planner.getDepth() << " nodes = " << planner.getTime();
      std::cout << " curent dec = " << planner.getDecisionManager()->getCurrentDecision();
      std::cout << " num Open Decs = " << planner.getDecisionManager()->getNumberOfDecisions() << std::endl;
      */
      //      planner.getDecisionManager()->printOpenDecisions();

      //      check_error(planner.getDecisionManager()->getNumberOfDecisions() == numDecs--);
      result = planner.step();
      if (result != CBPlanner::IN_PROGRESS) break;
    }
    assertTrue(result == CBPlanner::PLAN_FOUND);
    assertTrue(planner.getDepth() != planner.getTime());
    assertTrue(planner.getDepth() == 4);
    assertTrue(planner.getTime() == 11);

    DEFAULT_TEARDOWN_PLAN();
    return true;
  }
  static bool testTokenDecisionCycle() {
    DEFAULT_SETUP_PLAN(ce, db, false);

    hor.setHorizon(300,400);

    Timeline t1(db.getId(), LabelStr("Objects"), LabelStr("Timeline1"));
    Timeline t2(db.getId(), LabelStr("Objects"), LabelStr("Timeline2"));
    Object t3(db.getId(), LabelStr("Objects"), LabelStr("Object1"));
    db.close();

    ConditionalRule r("Objects.PredicateA");

    IntervalToken tokenA(db.getId(), 
			 "Objects.PredicateA", 
			 true,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(200, 200),
			 LabelStr("Timeline1"), false);
    tokenA.addParameter(IntervalIntDomain(1,2), "IntervalIntParam");
    tokenA.close();

    IntervalToken tokenB(db.getId(), 
			 "Objects.PredicateB", 
			 false,
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(200, 200),
			 LabelStr("Timeline2"));

    tokenB.activate();
    t2.constrain(tokenB.getId(), tokenB.getId());

    assertTrue(ce.propagate());

    CBPlanner::Status result = planner.run();
    assertTrue(result == CBPlanner::PLAN_FOUND);

    assertTrue(planner.getTime() == planner.getDepth());
    assertTrue(planner.getTime() == 0);
    assertTrue(planner.getClosedDecisions().empty());

    hor.setHorizon(0,200);

    result = planner.run();
    assertTrue(result == CBPlanner::PLAN_FOUND);

    assertTrue(planner.getTime() != planner.getDepth());

    assertTrue(planner.getDepth() == 9);
    assertTrue(planner.getTime() == 19);

    tokenA.cancel();
    tokenA.reject();
    tokenA.getParameters()[0]->reset();

    result = planner.run();
    assertTrue(result == CBPlanner::PLAN_FOUND);
    DEFAULT_TEARDOWN_PLAN();
    return true;
  }
  static bool testObjectDecisionCycle() {
    DEFAULT_SETUP_PLAN(ce, db, false);

    hor.setHorizon(10,500);

    Timeline t1(db.getId(), LabelStr("Objects"), LabelStr("Timeline1"));
    db.close();

    IntervalToken tokenB(db.getId(), 
			 "Objects.PredicateB", 
			 false,
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(200, 200));

    tokenB.activate();

    assertTrue(planner.getDecisionManager()->getNumberOfDecisions() == 1); 
    std::list<DecisionPointId> decisions;
    planner.getDecisionManager()->getOpenDecisions(decisions);
    DecisionPointId dec = decisions.front();
    assertTrue(ObjectDecisionPointId::convertable(dec));

    tokenB.cancel();

    assertTrue(planner.getDecisionManager()->getNumberOfDecisions() == 1); 
    decisions.clear();
    planner.getDecisionManager()->getOpenDecisions(decisions);
    dec = decisions.front();
    assertTrue(!ObjectDecisionPointId::convertable(dec));
    assertTrue(TokenDecisionPointId::convertable(dec));
    
    tokenB.activate();

    assertTrue(planner.getDecisionManager()->getNumberOfDecisions() == 1); 
    decisions.clear();
    planner.getDecisionManager()->getOpenDecisions(decisions);
    dec = decisions.front();
    assertTrue(ObjectDecisionPointId::convertable(dec));

    tokenB.getStart()->specify(0);

    assertTrue(planner.getDecisionManager()->getNumberOfDecisions() == 1); 
    decisions.clear();
    planner.getDecisionManager()->getOpenDecisions(decisions);
    dec = decisions.front();
    assertTrue(ObjectDecisionPointId::convertable(dec));

    tokenB.cancel();

    assertTrue(planner.getDecisionManager()->getNumberOfDecisions() == 1); 
    decisions.clear();
    planner.getDecisionManager()->getOpenDecisions(decisions);
    dec = decisions.front();
    assertTrue(!ObjectDecisionPointId::convertable(dec));

    assertTrue(tokenB.getStart()->getDerivedDomain().isSingleton());
    assertTrue(tokenB.getStart()->getDerivedDomain().getSingletonValue() == 0);

    DEFAULT_TEARDOWN_PLAN();
    return true;
  }
  static bool testObjectAndObjectVariable() {
    DEFAULT_SETUP_PLAN(ce, db, false);
    hor.setHorizon(10,500);

    Object o1(db.getId(), LabelStr("Objects"), LabelStr("Object1"));
    Timeline t1(db.getId(), LabelStr("Objects"), LabelStr("Timeline1"));
    db.close();

    IntervalToken tokenB(db.getId(), 
			 "Objects.PredicateB", 
			 false,
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(200, 200));

    tokenB.activate();
    
    assertTrue(planner.getDecisionManager()->getNumberOfDecisions() == 2); 
    std::list<DecisionPointId> decisions;
    planner.getDecisionManager()->getOpenDecisions(decisions);
    DecisionPointId dec = decisions.front();
    assertTrue(ObjectDecisionPointId::convertable(dec));
    /*
    ObjectDecisionPointId odec = dec;
    std::list<ChoiceId> choices = odec->getUpdatedChoices();
    assertTrue(choices.size() == 2);
    */

    dec = decisions.back();
    assertTrue(ConstrainedVariableDecisionPointId::convertable(dec));
    assertTrue(dec->getEntityKey() == tokenB.getObject()->getKey());

    tokenB.getObject()->specify(o1.getId());

    assertTrue(planner.getDecisionManager()->getNumberOfDecisions() == 1); 
    decisions.clear();
    planner.getDecisionManager()->getOpenDecisions(decisions);
    dec = decisions.front();
    assertTrue(ObjectDecisionPointId::convertable(dec));
    /* getChoices is protected; getUpdatedChoices will return previous set
       of choices; getCurrentChoices same thing.
    odec = dec;
    choices.clear();
    choices = dec->getChoices();
    assertTrue(choices.size() == 1);
    */

    //    o1.constrain(tokenB.getId(), TokenId::noId());
    planner.getDecisionManager()->assignDecision();

    assertTrue(planner.getDecisionManager()->getNumberOfDecisions() == 0); 
    
    //    o1.free(tokenB.getId());
    planner.getDecisionManager()->retractDecision();

    assertTrue(planner.getDecisionManager()->isRetracting());
    assertTrue(!planner.getDecisionManager()->hasDecisionToRetract());
    //it was the only object we could constrain to and we failed, so
    //there's no more to do.

    DEFAULT_TEARDOWN_PLAN();
    return true;
  }
  static bool testObjectHorizon() {
    DEFAULT_SETUP_PLAN(ce, db, false);

    Object o1(db.getId(), LabelStr("Objects"), LabelStr("Object1"));
    db.close();

    IntervalToken tokenB(db.getId(), "Objects.PredicateB", false);

    hor.setHorizon(10,100);

    assertTrue(planner.getDecisionManager()->getNumberOfDecisions() == 0); 

    tokenB.activate();

    assertTrue(planner.getDecisionManager()->getNumberOfDecisions() == 0); 

    tokenB.getStart()->specify(50);

    assertTrue(planner.getDecisionManager()->getNumberOfDecisions() == 1); 

    DEFAULT_TEARDOWN_PLAN();
    return true;
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
    DEFAULT_SETUP_PLAN(ce, db, false);
    hor.setHorizon(0,100);
    std::list<double> values;
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L4"));

    Variable<LabelSet> v0(ce.getId(), LabelSet(values));
    LabelSet leaveOpen;
    leaveOpen.insert(values);
    Variable<LabelSet> v1(ce.getId(), leaveOpen);
    Variable<IntervalDomain> v2(ce.getId(), IntervalDomain(1, 2));
    Variable<IntervalIntDomain> v3(ce.getId(), IntervalIntDomain(1, 2));
    Variable<IntervalIntDomain> v4(ce.getId(), IntervalIntDomain());
    Variable<NumericDomain> v5(ce.getId(), NumericDomain());
    v5.insert(5);
    v5.insert(23);
    v5.close();

    Object o1(db.getId(), LabelStr("Objects"), LabelStr("o1"));
    Timeline t1(db.getId(), LabelStr("Objects"), LabelStr("t1"));
    db.close();

    IntervalToken tokenA(db.getId(), 
			 "Objects.PredicateD",
			 true,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000));

    IntervalToken tokenB(db.getId(), 
			 "Objects.PredicateB",
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000));

    IntervalToken tokenC(db.getId(), 
			 "Objects.PredicateC",
			 true,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000),
			 LabelStr("o1"));

    hor.setHorizon(0,200);

    CBPlanner::Status result = planner.run();
    assertTrue(result == CBPlanner::PLAN_FOUND);

    assertTrue(planner.getTime() == planner.getDepth());
    assertTrue(planner.getDepth() == 9);

    DecisionManagerId dm = planner.getDecisionManager();
    dm->retractDecision();
    while(dm->hasDecisionToRetract() && dm->isRetracting())
      dm->retractDecision();

    result = planner.run();
    assertTrue(result == CBPlanner::PLAN_FOUND);

    assertTrue(planner.getTime() == planner.getDepth());
    assertTrue(planner.getDepth() == 10);

    DEFAULT_TEARDOWN_PLAN();
    return true;
  }
  static bool testAddSubgoalAfterPlanning() {
    DEFAULT_SETUP_PLAN(ce, db, false);
    hor.setHorizon(0,100);

    std::list<double> values;
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L4"));

    Variable<LabelSet> v0(ce.getId(), LabelSet(values));
    LabelSet leaveOpen;
    leaveOpen.insert(values);
    Variable<LabelSet> v1(ce.getId(), leaveOpen);
    Variable<IntervalDomain> v2(ce.getId(), IntervalDomain(1, 2));
    Variable<IntervalIntDomain> v3(ce.getId(), IntervalIntDomain(1, 2));
    Variable<IntervalIntDomain> v4(ce.getId(), IntervalIntDomain());
    Variable<NumericDomain> v5(ce.getId(), NumericDomain());
    v5.insert(5);
    v5.insert(23);
    v5.close();

    Timeline t(db.getId(), LabelStr("Objects"), LabelStr("t1"));
    db.close();

    //    int numDecs = 4;
    //    int i=0;
    for (;;) { /* Forever: only way out is to return */
      /*
      std::cout << std::endl;
      std::cout << "Planner step = " << i++ << std::endl;
      std::cout << "Depth = " << planner.getDepth() << " nodes = " << planner.getTime();
      std::cout << " curent dec = " << planner.getDecisionManager()->getCurrentDecision();
      std::cout << " num Open Decs = " << planner.getDecisionManager()->getNumberOfDecisions() << std::endl;
      */
      //planner.getDecisionManager()->printOpenDecisions();

      //      check_error(planner.getDecisionManager()->getNumberOfDecisions() == numDecs--);
      CBPlanner::Status result = planner.step();
      if (result != CBPlanner::IN_PROGRESS) {
	assertTrue(result == CBPlanner::PLAN_FOUND);
	break;
      }
    }
    assertTrue(planner.getDepth() ==  planner.getTime());
    assertTrue(planner.getDepth() == 3);

    Variable<BoolDomain> v6(ce.getId(), BoolDomain());
    IntervalToken tokenA(db.getId(), 
			 "Objects.PADDED",
			 true); 

    tokenA.getStart()->specify(IntervalIntDomain(0, 10));
    tokenA.getEnd()->specify(IntervalIntDomain(0, 200));

    //    std::cout << "AFTER ADDING NEW GOAL TOKEN " << std::endl;

    CBPlanner::Status res = planner.run();
    assertTrue(res == CBPlanner::PLAN_FOUND);
    assertTrue(planner.getDepth() ==  planner.getTime());
    assertTrue(planner.getDepth() == 6);

    /*
    for (;;) {
      std::cout << std::endl;
      std::cout << "Planner step = " << i++ << std::endl;
      std::cout << "Depth = " << planner.getDepth() << " nodes = " << planner.getTime();
      std::cout << " curent dec = " << planner.getDecisionManager()->getCurrentDecision();
      std::cout << " num Open Decs = " << planner.getDecisionManager()->getNumberOfDecisions() << std::endl;
      planner.getDecisionManager()->printOpenDecisions();

      //      check_error(planner.getDecisionManager()->getNumberOfDecisions() == numDecs--);
      CBPlanner::Status result = planner.step(0);
      if (result != CBPlanner::IN_PROGRESS) return result;
    }

    */
    DEFAULT_TEARDOWN_PLAN();
    return true;
  }
};

int main() {
  LockManager::instance().connect();
  LockManager::instance().lock();

  Schema::instance();
  REGISTER_CONSTRAINT(EqualConstraint, "eq", "Default");
  REGISTER_CONSTRAINT(EqualConstraint, "Equal", "Default");
  REGISTER_CONSTRAINT(LessThanConstraint, "lt", "Default");
  REGISTER_CONSTRAINT(SubsetOfConstraint, "SubsetOf", "Default");
  REGISTER_CONSTRAINT(NotFalseConstraint, "notfalse", "Default");
  REGISTER_CONSTRAINT(BinaryCustomConstraint, "custom", "Default");
  LockManager::instance().unlock();

  for (int i = 0; i < 1; i++) {
    LockManager::instance().lock();
    runTestSuite(DefaultSetupTest::test);
    runTestSuite(ConditionTest::test);
    runTestSuite(DecisionManagerTest::test);
    runTestSuite(CBPlannerTest::test);
    runTestSuite(MultipleDecisionManagerTest::test);
    runTestSuite(DecisionPointTest::test);
    runTestSuite(TwoCyclePlanningTest::test);
    LockManager::instance().unlock();
  }
  LockManager::instance().lock();

  std::cout << "Finished" << std::endl;
  ConstraintLibrary::purgeAll();
  exit(0);
}
