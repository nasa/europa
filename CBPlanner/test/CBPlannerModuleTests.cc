#include "CBPlannerModuleTests.hh"
#include "TestSupport.hh"
#include "SubgoalOnceRule.hh"

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
    
  CBPlanner::Status res = planner.run(loggingEnabled());
  assert(res == CBPlanner::PLAN_FOUND);

  const std::list<DecisionPointId>& closed = planner.getClosedDecisions();

  assert(closed.size() == 4);
  assert(closed.size() == planner.getTime());
  assert(planner.getTime() == planner.getDepth());
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

  CBPlanner::Status res = planner.run(loggingEnabled(), 100);
  assert(res == CBPlanner::SEARCH_EXHAUSTED);
  return true;
}

bool testMultipleDMsImpl(ConstraintEngine &ce, PlanDatabase &db, Schema &schema, DecisionManager &dm, Horizon &hor) {
  return true;
}
}
