#include "CBPlannerModuleTests.hh"
#include "TestSupport.hh"
#include "SubgoalOnceRule.hh"
#include "ConstrainedVariableDecisionPoint.hh"
#include "Variable.hh"
#include "BinaryCustomConstraint.hh"
#include "NotFalseConstraint.hh"
#include "ConditionalRule.hh"
#include "ObjectDecisionPoint.hh"

extern bool loggingEnabled();

namespace Prototype {

  bool testDefaultSetupImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema, DecisionManager &dm, Horizon &hor) {
    assert(db.isClosed() == false);
    db.close();
    assert(db.isClosed() == true);
    return true;
  }

  bool testConditionImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema, DecisionManager &dm, Horizon &hor) {
    Condition cond(dm.getId());
    assert(!cond.hasChanged());
  
    assert(dm.getConditions().size() == 1);
    return true;  
  }

  bool testHorizonImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema, DecisionManager &dm, Horizon &hor) {
    Horizon hor1;
    int start, end;
    hor1.getHorizon(start,end);
    assert(start == -MAX_FINITE_TIME);
    assert(end == MAX_FINITE_TIME);
  
    Horizon hor2(0,200);
    hor2.getHorizon(start,end);
    assert(start == 0);
    assert(end == 200);
  
    hor2.setHorizon(0,400);
    hor2.getHorizon(start,end);
    assert(start == 0);
    assert(end == 400);
    return true;
  }

  bool testHorizonConditionImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema, DecisionManager &dm, Horizon &hor) {
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
    return true;
  }

  bool testTemporalVariableConditionImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema, DecisionManager &dm, Horizon &hor) {
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
    return true;
  }


  bool testDynamicInfiniteRealConditionImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema, DecisionManager &dm, Horizon &hor) {
  
    DynamicInfiniteRealCondition cond(dm.getId());
    assert(dm.getConditions().size() == 1);

    std::list<LabelStr> values;
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L4"));
    values.push_back(LabelStr("L2"));
    values.push_back(LabelStr("L5"));
    values.push_back(LabelStr("L3"));

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
    return true;
  }

  bool testForwardDecisionHandlingImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema, DecisionManager &dm, Horizon &hor) {
    HorizonCondition hcond(hor.getId(), dm.getId());
    TemporalVariableCondition tcond(hor.getId(), dm.getId());
    DynamicInfiniteRealCondition dcond(dm.getId());

    assert(dm.getConditions().size() == 3);

    std::list<LabelStr> values;
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L4"));
    values.push_back(LabelStr("L2"));
    values.push_back(LabelStr("L5"));
    values.push_back(LabelStr("L3"));

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

    dm.assignDecision();

    assert(dm.getNumberOfDecisions() == 3);

    dm.assignDecision();

    assert(dm.getNumberOfDecisions() == 2);

    dm.assignDecision();

    assert(dm.getNumberOfDecisions() == 1);

    dm.assignDecision();

    assert(dm.getNumberOfDecisions() == 0);

    IntervalToken tokenB(db.getId(), 
			 LabelStr("P1"), 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000));

    assert(ce.propagate());

    dm.assignDecision();

    assert(dm.getNumberOfDecisions() == 0);
    return true;
  }

  bool testMakeMoveImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema, CBPlanner &planner) {
  
    std::list<LabelStr> values;
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L4"));
    values.push_back(LabelStr("L2"));
    values.push_back(LabelStr("L5"));
    values.push_back(LabelStr("L3"));

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
    
    assert(!planner.getDecisionManager()->assignDecision());
    return true;
  }

  bool testCurrentStateImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema, CBPlanner &planner) {
    Timeline t(db.getId(), LabelStr("Objects"), LabelStr("t1"));
    db.close();
    IntervalToken tokenA(db.getId(), 
			 LabelStr("P1"), 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000));

    assert(planner.getDecisionManager()->getCurrentDecision().isNoId());

    if (!planner.getDecisionManager()->assignDecision())
      return false;

    assert(TokenDecisionPointId::convertable(planner.getDecisionManager()->getCurrentDecision()));
    TokenDecisionPointId tokdec = planner.getDecisionManager()->getCurrentDecision();
    assert(tokdec->getToken() == tokenA.getId());
    assert(!planner.getDecisionManager()->getCurrentChoice().isNoId());
    assert(tokdec->getCurrent() == planner.getDecisionManager()->getCurrentChoice());

    planner.getDecisionManager()->synchronize();
    return true;
  }


  bool testRetractMoveImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema, CBPlanner &planner) {
    Timeline t(db.getId(), LabelStr("Objects"), LabelStr("t1"));
    db.close();
    IntervalToken tokenA(db.getId(), 
			 LabelStr("P1"), 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000));

    if (!planner.getDecisionManager()->assignDecision())
      return false;

    assert(planner.getDecisionManager()->getClosedDecisions().size() == 1);

    //std::cout << "RETRACTING" << std::endl;

    assert(planner.getDecisionManager()->retractDecision());
    return true;
  }

  bool testNoBacktrackCaseImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema, CBPlanner &planner) {
    Timeline timeline(db.getId(),LabelStr("AllObjects"), LabelStr("t1"));
    db.close();

    IntervalToken tokenA(db.getId(), 
			 LabelStr("P1"), 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);

    std::list<LabelStr> values;
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L4"));
    values.push_back(LabelStr("L2"));
    values.push_back(LabelStr("L5"));
    values.push_back(LabelStr("L3"));
    tokenA.addParameter(LabelSet(values, true));
    tokenA.addParameter(LabelSet(values, false));
    tokenA.addParameter(IntervalIntDomain(1, 20));
    tokenA.close();
    
    CBPlanner::Status res = planner.run();
    assert(res == CBPlanner::PLAN_FOUND);

    const std::list<DecisionPointId>& closed = planner.getClosedDecisions();

    assert(planner.getTime() == planner.getDepth());
    assert(closed.size() == planner.getTime());
    assert(closed.size() == 4);
    return true;
  }



  bool testSubgoalOnceRuleImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema, CBPlanner &planner) {
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
    return true;
  }

  bool testBacktrackCaseImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema, CBPlanner &planner) {
    Timeline timeline(db.getId(),LabelStr("AllObjects"), LabelStr("t1"));
    db.close();

    SubgoalOnceRule r(LabelStr("P1"), 0);

    std::list<LabelStr> values;
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L4"));
    values.push_back(LabelStr("L2"));
    values.push_back(LabelStr("L5"));
    values.push_back(LabelStr("L3"));

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

    CBPlanner::Status res = planner.run(100);

    assert(res == CBPlanner::SEARCH_EXHAUSTED);
    assert(planner.getClosedDecisions().empty());
    return true;
  }

  bool testTimeoutCaseImpl(ConstraintEngine& ce, PlanDatabase& db, Schema& schema, CBPlanner& planner)  {
    Timeline t1(db.getId(),LabelStr("AllObjects"), LabelStr("t1"));
    Timeline t2(db.getId(),LabelStr("AllObjects"), LabelStr("t2"));
    Object o1(db.getId(),LabelStr("AllObjects"),LabelStr("o1"));
    db.close();

    IntervalToken tokenA(db.getId(), 
			 LabelStr("P1"), 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);

    std::list<LabelStr> values;
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L4"));
    values.push_back(LabelStr("L2"));
    values.push_back(LabelStr("L5"));
    values.push_back(LabelStr("L3"));
    tokenA.addParameter(LabelSet(values, true));
    tokenA.addParameter(LabelSet(values, false));
    tokenA.addParameter(IntervalIntDomain(1, 20));
    tokenA.close();
    

    IntervalToken tokenB(db.getId(), 
			 LabelStr("P1"), 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);

    values.clear();
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L4"));
    values.push_back(LabelStr("L2"));
    values.push_back(LabelStr("L5"));
    values.push_back(LabelStr("L3"));
    tokenB.addParameter(LabelSet(values, true));
    tokenB.addParameter(LabelSet(values, false));
    tokenB.addParameter(IntervalIntDomain(1, 20));
    tokenB.close();
    

    IntervalToken tokenC(db.getId(), 
			 LabelStr("P1"), 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);

    values.clear();
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L4"));
    values.push_back(LabelStr("L2"));
    values.push_back(LabelStr("L5"));
    values.push_back(LabelStr("L3"));
    tokenC.addParameter(LabelSet(values, true));
    tokenC.addParameter(LabelSet(values, false));
    tokenC.addParameter(IntervalIntDomain(1, 20));
    tokenC.close();
    

    IntervalToken tokenD(db.getId(), 
			 LabelStr("P1"), 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);

    values.clear();
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L4"));
    values.push_back(LabelStr("L2"));
    values.push_back(LabelStr("L5"));
    values.push_back(LabelStr("L3"));
    tokenD.addParameter(LabelSet(values, true));
    tokenD.addParameter(LabelSet(values, false));
    tokenD.addParameter(IntervalIntDomain(1, 20));
    tokenD.close();
    

    IntervalToken tokenE(db.getId(), 
			 LabelStr("P1"), 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 LabelStr("o1"), false);

    values.clear();
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L4"));
    values.push_back(LabelStr("L2"));
    values.push_back(LabelStr("L5"));
    values.push_back(LabelStr("L3"));
    tokenE.addParameter(LabelSet(values, true));
    tokenE.addParameter(LabelSet(values, false));
    tokenE.addParameter(IntervalIntDomain(1, 20));
    tokenE.close();

    IntervalToken tokenF(db.getId(), 
			 LabelStr("P1"), 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 LabelStr("o1"), false);

    values.clear();
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L4"));
    values.push_back(LabelStr("L2"));
    values.push_back(LabelStr("L5"));
    values.push_back(LabelStr("L3"));
    tokenF.addParameter(LabelSet(values, true));
    tokenF.addParameter(LabelSet(values, false));
    tokenF.addParameter(IntervalIntDomain(1, 20));
    tokenF.close();
    

    IntervalToken tokenG(db.getId(), 
			 LabelStr("P1"), 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);

    values.clear();
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L4"));
    values.push_back(LabelStr("L2"));
    values.push_back(LabelStr("L5"));
    values.push_back(LabelStr("L3"));
    tokenG.addParameter(LabelSet(values, true));
  tokenG.addParameter(LabelSet(values, false));
  tokenG.addParameter(IntervalIntDomain(1, 20));
  tokenG.close();
    
  CBPlanner::Status res = planner.run(20);
  assert(res == CBPlanner::TIMEOUT_REACHED);

  const std::list<DecisionPointId>& closed = planner.getClosedDecisions();

  assert(closed.size() == 20);
  assert(closed.size() == planner.getTime());
  assert(planner.getTime() == planner.getDepth());

  return true;
  }

  bool testVariableDecisionCycleImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema, CBPlanner& planner) {

    std::list<LabelStr> values;
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L4"));

    Variable<LabelSet> v0(ce.getId(), LabelSet(values, true));
    Variable<LabelSet> v1(ce.getId(), LabelSet(values, false));
    Variable<IntervalDomain> v2(ce.getId(), IntervalDomain(1, 2));
    Variable<IntervalIntDomain> v3(ce.getId(), IntervalIntDomain(1, 2));
    Variable<IntervalIntDomain> v4(ce.getId(), IntervalIntDomain());
    Variable<EnumeratedDomain> v5(ce.getId(), EnumeratedDomain());
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
    assert(result == CBPlanner::PLAN_FOUND);
    assert(planner.getDepth() != planner.getTime());
    assert(planner.getDepth() == 4);
    assert(planner.getTime() == 11);

    return true;
  }

  bool testTokenDecisionCycleImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema, Horizon& hor, CBPlanner& planner) {

    hor.setHorizon(300,400);

    Timeline t1(db.getId(), LabelStr("Objects"), LabelStr("Timeline1"));
    Timeline t2(db.getId(), LabelStr("Objects"), LabelStr("Timeline2"));
    Object t3(db.getId(), LabelStr("Objects"), LabelStr("Object1"));
    db.close();

    ConditionalRule r(LabelStr("PredicateA"));

    IntervalToken tokenA(db.getId(), 
			 LabelStr("PredicateA"), 
			 true,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(200, 200),
			 LabelStr("Timeline1"), false);
    tokenA.addParameter(IntervalIntDomain(1,2));
    tokenA.close();

    IntervalToken tokenB(db.getId(), 
			 LabelStr("PredicateB"), 
			 false,
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(200, 200),
			 LabelStr("Timeline2"));

    tokenB.activate();
    t2.constrain(tokenB.getId(), TokenId::noId());

    assert(ce.propagate());

    CBPlanner::Status result = planner.run();
    assert(result == CBPlanner::PLAN_FOUND);

    assert(planner.getTime() == planner.getDepth());
    assert(planner.getTime() == 0);
    assert(planner.getClosedDecisions().empty());

    hor.setHorizon(0,200);

    result = planner.run();
    assert(result == CBPlanner::PLAN_FOUND);

    assert(planner.getTime() != planner.getDepth());
    assert(planner.getDepth() == 8);
    assert(planner.getTime() == 13);

    return true;
  }

  bool testObjectDecisionCycleImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema, Horizon& hor, CBPlanner& planner) {

    hor.setHorizon(10,500);

    Timeline t1(db.getId(), LabelStr("Objects"), LabelStr("Timeline1"));
    Object o1(db.getId(), LabelStr("Objects"), LabelStr("Object1"));
    db.close();

    IntervalToken tokenB(db.getId(), 
			 LabelStr("PredicateB"), 
			 false,
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(200, 200),
			 LabelStr("Timeline1"));

    tokenB.activate();

    assert(planner.getDecisionManager()->getNumberOfDecisions() == 1); 
    std::list<DecisionPointId> decisions;
    planner.getDecisionManager()->getOpenDecisions(decisions);
    DecisionPointId dec = decisions.front();
    assert(ObjectDecisionPointId::convertable(dec));

    tokenB.cancel();

    assert(planner.getDecisionManager()->getNumberOfDecisions() == 1); 
    decisions.clear();
    planner.getDecisionManager()->getOpenDecisions(decisions);
    dec = decisions.front();
    assert(!ObjectDecisionPointId::convertable(dec));
    assert(TokenDecisionPointId::convertable(dec));
    
    tokenB.activate();

    assert(planner.getDecisionManager()->getNumberOfDecisions() == 1); 
    decisions.clear();
    planner.getDecisionManager()->getOpenDecisions(decisions);
    dec = decisions.front();
    assert(ObjectDecisionPointId::convertable(dec));

    tokenB.getStart()->specify(0);

    assert(planner.getDecisionManager()->getNumberOfDecisions() == 1); 
    decisions.clear();
    planner.getDecisionManager()->getOpenDecisions(decisions);
    dec = decisions.front();
    assert(ObjectDecisionPointId::convertable(dec));

    tokenB.cancel();

    assert(planner.getDecisionManager()->getNumberOfDecisions() == 1); 
    decisions.clear();
    planner.getDecisionManager()->getOpenDecisions(decisions);
    dec = decisions.front();
    assert(!ObjectDecisionPointId::convertable(dec));

    assert(tokenB.getStart()->getDerivedDomain().isSingleton());
    assert(tokenB.getStart()->getDerivedDomain().getSingletonValue() == 0);

    return true;
  }

  bool testObjectHorizonImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema, Horizon& hor, CBPlanner& planner) {

    Object o1(db.getId(), LabelStr("Objects"), LabelStr("Object1"));
    db.close();

    IntervalToken tokenB(db.getId(), LabelStr("PredicateB"), false);

    hor.setHorizon(10,100);

    assert(planner.getDecisionManager()->getNumberOfDecisions() == 0); 

    tokenB.activate();

    assert(planner.getDecisionManager()->getNumberOfDecisions() == 0); 

    tokenB.getStart()->specify(50);

    assert(planner.getDecisionManager()->getNumberOfDecisions() == 1); 

    return true;
  }

  bool testMultipleDMsImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema, DecisionManager &dm, Horizon &hor) {
    return true;
  }
  
  bool testAddSubgoalAfterPlanningImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema, Horizon& hor, CBPlanner& planner) {
    hor.setHorizon(0,100);

    std::list<LabelStr> values;
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L4"));

    Variable<LabelSet> v0(ce.getId(), LabelSet(values, true));
    Variable<LabelSet> v1(ce.getId(), LabelSet(values, false));
    Variable<IntervalDomain> v2(ce.getId(), IntervalDomain(1, 2));
    Variable<IntervalIntDomain> v3(ce.getId(), IntervalIntDomain(1, 2));
    Variable<IntervalIntDomain> v4(ce.getId(), IntervalIntDomain());
    Variable<EnumeratedDomain> v5(ce.getId(), EnumeratedDomain());
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
	assert(result == CBPlanner::PLAN_FOUND);
	break;
      }
    }
    assert(planner.getDepth() ==  planner.getTime());
    assert(planner.getDepth() == 3);

    Variable<BoolDomain> v6(ce.getId(), BoolDomain());
    IntervalToken tokenA(db.getId(), 
			 LabelStr("PADDED"),
			 true); 

    tokenA.getStart()->specify(IntervalIntDomain(0, 10));
    tokenA.getEnd()->specify(IntervalIntDomain(0, 200));

    //    std::cout << "AFTER ADDING NEW GOAL TOKEN " << std::endl;

    CBPlanner::Status res = planner.run();
    assert(res == CBPlanner::PLAN_FOUND);
    assert(planner.getDepth() ==  planner.getTime());
    assert(planner.getDepth() == 6);

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
    return true;
  }


}
