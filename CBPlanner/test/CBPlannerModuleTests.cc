#include "CBPlannerModuleTests.hh"
#include "TestSupport.hh"
#include "SubgoalOnceRule.hh"
#include "ConstrainedVariableDecisionPoint.hh"
#include "Variable.hh"
#include "BinaryCustomConstraint.hh"
#include "NotFalseConstraint.hh"
#include "ConditionalRule.hh"
#include "ObjectDecisionPoint.hh"
#include "Choice.hh"
#include "StringDomain.hh"
#include "NumericDomain.hh"
#include "Generator.hh"
#include "HSTSHeuristicsReader.hh"
#include "HSTSNoBranchCondition.hh"
#include "HSTSPlanIdReader.hh"

extern bool loggingEnabled();

namespace PLASMA {

  /**
   * @brief Creates the type specifications required for testing
   */
  void initCBPTestSchema(const SchemaId& schema){
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

  void initHeuristicsSchema(const SchemaId& rover){
    rover->reset();
    rover->addObjectType(LabelStr("Object"));
    rover->addObjectType(LabelStr("Timeline"), LabelStr("Object"));
    rover->addObjectType(LabelStr("NddlResource"));
    rover->addObjectType("Resource", "NddlResource");
    rover->addMember("Resource", "float", "initialCapacity");
    rover->addMember("Resource", "float", "levelLimitMin");
    rover->addMember("Resource", "float", "levelLimitMax");
    rover->addMember("Resource", "float", "productionRateMax");
    rover->addMember("Resource", "float", "productionMax");
    rover->addMember("Resource", "float", "consumptionRateMax");
    rover->addMember("Resource", "float", "consumptionMax");
    rover->addPredicate("Resource.change");
    rover->addMember("Resource.change", "float", "quantity");
    rover->addObjectType("UnaryResource", "Timeline");
    rover->addPredicate("UnaryResource.uses");
    rover->addObjectType("Battery", "Resource");
    rover->addObjectType("Location", "Object");
    // rover->isObjectType("Location");
    rover->addMember("Location", "string", "name");
    rover->addMember("Location", "int", "x");
    rover->addMember("Location", "int", "y");
    rover->addObjectType("Path", "Object");
    rover->addMember("Path", "string", "name");
    rover->addMember("Path", "Location", "from");
    rover->addMember("Path", "Location", "to");
    rover->addMember("Path", "float", "cost");
    rover->addObjectType("Navigator", "Timeline");
    rover->addPredicate("Navigator.At");
    rover->addMember("Navigator.At", "Location", "location");
    rover->addPredicate("Navigator.Going");
    rover->addMember("Navigator.Going", "Location", "from");
    rover->addMember("Navigator.Going", "Location", "to");
    rover->addObjectType("Commands", "Timeline");
    rover->addPredicate("Commands.TakeSample");
    rover->addMember("Commands.TakeSample", "Location", "rock");
    rover->addPredicate("Commands.PhoneHome");
    rover->addPredicate("Commands.PhoneLander");
    rover->addObjectType("Instrument", "Timeline");
    rover->addPredicate("Instrument.TakeSample");
    rover->addMember("Instrument.TakeSample", "Location", "rock");
    rover->addPredicate("Instrument.Place");
    rover->addMember("Instrument.Place", "Location", "rock");
    rover->addPredicate("Instrument.Stow");
    rover->addPredicate("Instrument.Unstow");
    rover->addPredicate("Instrument.Stowed");
    rover->addObjectType("Rover", "Object");
    rover->addMember("Rover", "Commands", "commands");
    rover->addMember("Rover", "Navigator", "navigator");
    rover->addMember("Rover", "Instrument", "instrument");
    rover->addMember("Rover", "Battery", "mainBattery");
    rover->addObjectType("NddlWorld", "Timeline");
    rover->addMember("NddlWorld", "int", "m_horizonStart");
    rover->addMember("NddlWorld", "int", "m_horizonEnd");
    rover->addMember("NddlWorld", "int", "m_maxPlannerSteps");
    rover->addPredicate("NddlWorld.initialState");
    rover->addEnum("TokenStates");
    rover->addValue("TokenStates", LabelStr("INACTIVE"));
    rover->addValue("TokenStates", LabelStr("ACTIVE"));
    rover->addValue("TokenStates", LabelStr("MERGED"));
    rover->addValue("TokenStates", LabelStr("REJECTED"));
    rover->addMember("Navigator.Going", "Path", "p");
    rover->addMember("Commands.TakeSample", "Rover", "rovers");
    rover->addMember("Commands.TakeSample", "bool", "OR");
    rover->addMember("Instrument.TakeSample", "Rover", "rovers");
    rover->addMember("Instrument.Place", "Rover", "rovers");
    rover->addMember("Instrument.Unstow", "Rover", "rovers");
    rover->addMember("Instrument.Stow", "Rover", "rovers");
  }

  static void makeTestToken(IntervalToken& token, const std::list<double>& values){
    token.addParameter(LabelSet(values), "LabelSetParam0");
    LabelSet leaveOpen;
    leaveOpen.insert(values);
    token.addParameter(leaveOpen, "LabelSetParam1");
    token.addParameter(IntervalIntDomain(1, 20), "IntervalIntParam");
    token.close();
  }

  bool testDefaultSetupImpl(ConstraintEngine &ce, PlanDatabase &db, DecisionManager &dm, Horizon &hor) {
    assert(db.isClosed() == false);
    db.close();
    assert(db.isClosed() == true);
    return true;
  }

  bool testConditionImpl(ConstraintEngine &ce, PlanDatabase &db, DecisionManager &dm, Horizon &hor) {
    Condition cond(dm.getId());
    assert(!cond.hasChanged());
  
    assert(dm.getConditions().size() == 1);
    return true;  
  }

  bool testHorizonImpl(ConstraintEngine &ce, PlanDatabase &db, DecisionManager &dm, Horizon &hor) {
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

  bool testHorizonConditionImpl(ConstraintEngine &ce, PlanDatabase &db, DecisionManager &dm, Horizon &hor) {
    HorizonCondition cond(hor.getId(), dm.getId());
    assert(cond.isPossiblyOutsideHorizon());
    assert(dm.getConditions().size() == 1);

    Timeline t(db.getId(), LabelStr("Objects"), LabelStr("t1"));
    db.close();
    IntervalToken tokenA(db.getId(), 
			 "Objects.P1", 
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

  bool testTemporalVariableConditionImpl(ConstraintEngine &ce, PlanDatabase &db, DecisionManager &dm, Horizon &hor) {
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
			 "Objects.P1", 
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


  bool testDynamicInfiniteRealConditionImpl(ConstraintEngine &ce, PlanDatabase &db, DecisionManager &dm, Horizon &hor) {
  
    DynamicInfiniteRealCondition cond(dm.getId());
    assert(dm.getConditions().size() == 1);

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

  bool testHSTSNoBranchConditionImpl(ConstraintEngine &ce, PlanDatabase &db, DecisionManager &dm) {

    HSTSNoBranchCondition cond(dm.getId());
    assert(dm.getConditions().size() == 1);

    HSTSNoBranch noBranchSpec(db.getSchema());

    const LabelStr var1Name("AnObj.APred.Var1");

    noBranchSpec.addNoBranch(var1Name);

    Variable<IntervalIntDomain> var1(ce.getId(), IntervalIntDomain(), true, var1Name);
    Variable<IntervalIntDomain> var2(ce.getId(), IntervalIntDomain(), true, LabelStr("AnObj.APred.Var2"));

    std::cout << " var1 name = " << var1.getName().c_str() << std::endl;
    std::cout << " var2 name = " << var2.getName().c_str() << std::endl;

    cond.initialize(noBranchSpec);

    assert(!cond.test(var1.getId()));
    assert(cond.test(var2.getId()));

    return true;
  }

  bool testForwardDecisionHandlingImpl(ConstraintEngine &ce, PlanDatabase &db, DecisionManager &dm, Horizon &hor) {
    HorizonCondition hcond(hor.getId(), dm.getId());
    TemporalVariableCondition tcond(hor.getId(), dm.getId());
    DynamicInfiniteRealCondition dcond(dm.getId());

    assert(dm.getConditions().size() == 3);

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
			 "Objects.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000));

    assert(ce.propagate());

    dm.assignDecision();

    assert(dm.getNumberOfDecisions() == 0);
    return true;
  }

  bool testMakeMoveImpl(ConstraintEngine &ce, PlanDatabase &db, CBPlanner &planner) {
  
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
    
    assert(!planner.getDecisionManager()->assignDecision());
    return true;
  }

  bool testCurrentStateImpl(ConstraintEngine &ce, PlanDatabase &db, CBPlanner &planner) {
    Timeline t(db.getId(), LabelStr("Objects"), LabelStr("t1"));
    db.close();
    IntervalToken tokenA(db.getId(), 
			 "Objects.P1", 
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


  bool testRetractMoveImpl(ConstraintEngine &ce, PlanDatabase &db, CBPlanner &planner) {
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

    assert(planner.getDecisionManager()->getClosedDecisions().size() == 1);

    //std::cout << "RETRACTING" << std::endl;

    assert(planner.getDecisionManager()->retractDecision());
    return true;
  }

  bool testNoBacktrackCaseImpl(ConstraintEngine &ce, PlanDatabase &db, CBPlanner &planner) {
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
    assert(res == CBPlanner::PLAN_FOUND);

    const std::list<DecisionPointId>& closed = planner.getClosedDecisions();

    assert(planner.getTime() == planner.getDepth());
    assert(closed.size() == planner.getTime());
    assert(closed.size() == 4);
    return true;
  }



  bool testSubgoalOnceRuleImpl(ConstraintEngine &ce, PlanDatabase &db, CBPlanner &planner) {
    Timeline timeline(db.getId(),LabelStr("Objects"), LabelStr("t1"));
    db.close();

    SubgoalOnceRule r("Objects.P1", 0);

    IntervalToken t0(db.getId(), "Objects.P1", true, 		     
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

  bool testBacktrackCaseImpl(ConstraintEngine &ce, PlanDatabase &db, CBPlanner &planner) {
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

    assert(res == CBPlanner::SEARCH_EXHAUSTED);
    assert(planner.getClosedDecisions().empty());
    return true;
  }

  bool testTimeoutCaseImpl(ConstraintEngine& ce, PlanDatabase& db, CBPlanner& planner)  {
    Timeline t1(db.getId(),LabelStr("Objects"), LabelStr("t1"));
    Timeline t2(db.getId(),LabelStr("Objects"), LabelStr("t2"));
    Object o1(db.getId(),LabelStr("Objects"),LabelStr("o1"));
    db.close();

    IntervalToken tokenA(db.getId(), 
			 "Objects.P1", 
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

    IntervalToken tokenB(db.getId(), 
			 "Objects.P1", 
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
    makeTestToken(tokenB, values);

    IntervalToken tokenC(db.getId(), 
			 "Objects.P1", 
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
    makeTestToken(tokenC, values);
    

    IntervalToken tokenD(db.getId(), 
			 "Objects.P1", 
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
    makeTestToken(tokenD, values);

    IntervalToken tokenE(db.getId(), 
			 "Objects.P1", 
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
    makeTestToken(tokenE, values);

    IntervalToken tokenF(db.getId(), 
			 "Objects.P1", 
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
    makeTestToken(tokenF, values);

    IntervalToken tokenG(db.getId(), 
			 "Objects.P1", 
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
    makeTestToken(tokenG, values);
    
    CBPlanner::Status res = planner.run(20);
    assert(res == CBPlanner::TIMEOUT_REACHED);

    const std::list<DecisionPointId>& closed = planner.getClosedDecisions();

    assert(closed.size() == 20);
    assert(closed.size() == planner.getTime());
    assert(planner.getTime() == planner.getDepth());

    return true;
  }

  bool testVariableDecisionCycleImpl(ConstraintEngine &ce, PlanDatabase &db, CBPlanner& planner) {

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
    assert(result == CBPlanner::PLAN_FOUND);
    assert(planner.getDepth() != planner.getTime());
    assert(planner.getDepth() == 4);
    assert(planner.getTime() == 11);

    return true;
  }

  bool testTokenDecisionCycleImpl(ConstraintEngine &ce, PlanDatabase &db, Horizon& hor, CBPlanner& planner) {

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

  bool testObjectDecisionCycleImpl(ConstraintEngine &ce, PlanDatabase &db, Horizon& hor, CBPlanner& planner) {

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

  bool testObjectAndObjectVariableImpl(ConstraintEngine &ce, PlanDatabase &db, Horizon& hor, CBPlanner& planner) {
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
    
    assert(planner.getDecisionManager()->getNumberOfDecisions() == 2); 
    std::list<DecisionPointId> decisions;
    planner.getDecisionManager()->getOpenDecisions(decisions);
    DecisionPointId dec = decisions.front();
    assert(ObjectDecisionPointId::convertable(dec));
    /*
    ObjectDecisionPointId odec = dec;
    std::list<ChoiceId> choices = odec->getUpdatedChoices();
    check_error(choices.size() == 2);
    */

    dec = decisions.back();
    assert(ConstrainedVariableDecisionPointId::convertable(dec));
    assert(dec->getEntityKey() == tokenB.getObject()->getKey());

    tokenB.getObject()->specify(o1.getId());

    assert(planner.getDecisionManager()->getNumberOfDecisions() == 1); 
    decisions.clear();
    planner.getDecisionManager()->getOpenDecisions(decisions);
    dec = decisions.front();
    assert(ObjectDecisionPointId::convertable(dec));
    /* getChoices is protected; getUpdatedChoices will return previous set
       of choices; getCurrentChoices same thing.
    odec = dec;
    choices.clear();
    choices = dec->getChoices();
    check_error(choices.size() == 1);
    */

    //    o1.constrain(tokenB.getId(), TokenId::noId());
    planner.getDecisionManager()->assignDecision();

    assert(planner.getDecisionManager()->getNumberOfDecisions() == 0); 
    
    //    o1.free(tokenB.getId());
    planner.getDecisionManager()->retractDecision();

    check_error(planner.getDecisionManager()->isRetracting());
    check_error(!planner.getDecisionManager()->hasDecisionToRetract());
    //it was the only object we could constrain to and we failed, so
    //there's no more to do.

    return true;
  }

  bool testObjectHorizonImpl(ConstraintEngine &ce, PlanDatabase &db, Horizon& hor, CBPlanner& planner) {

    Object o1(db.getId(), LabelStr("Objects"), LabelStr("Object1"));
    db.close();

    IntervalToken tokenB(db.getId(), "Objects.PredicateB", false);

    hor.setHorizon(10,100);

    assert(planner.getDecisionManager()->getNumberOfDecisions() == 0); 

    tokenB.activate();

    assert(planner.getDecisionManager()->getNumberOfDecisions() == 0); 

    tokenB.getStart()->specify(50);

    assert(planner.getDecisionManager()->getNumberOfDecisions() == 1); 

    return true;
  }

  bool testMultipleDMsImpl(ConstraintEngine &ce, PlanDatabase &db, DecisionManager &dm, Horizon &hor) {
    return true;
  }
  
  bool testFindAnotherPlanImpl(ConstraintEngine &ce, PlanDatabase &db, Horizon& hor, CBPlanner& planner) {
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
    assert(result == CBPlanner::PLAN_FOUND);

    assert(planner.getTime() == planner.getDepth());
    assert(planner.getDepth() == 9);

    DecisionManagerId dm = planner.getDecisionManager();
    dm->retractDecision();
    while(dm->hasDecisionToRetract() && dm->isRetracting())
      dm->retractDecision();

    result = planner.run();
    assert(result == CBPlanner::PLAN_FOUND);

    assert(planner.getTime() == planner.getDepth());
    assert(planner.getDepth() == 10);

    return true;
  }

  bool testAddSubgoalAfterPlanningImpl(ConstraintEngine &ce, PlanDatabase &db, Horizon& hor, CBPlanner& planner) {
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
	assert(result == CBPlanner::PLAN_FOUND);
	break;
      }
    }
    assert(planner.getDepth() ==  planner.getTime());
    assert(planner.getDepth() == 3);

    Variable<BoolDomain> v6(ce.getId(), BoolDomain());
    IntervalToken tokenA(db.getId(), 
			 "Objects.PADDED",
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

  bool testDefaultInitializationImpl(HSTSHeuristics& heuristics) {
    heuristics.setDefaultPriorityPreference(HSTSHeuristics::HIGH);

    std::vector<std::pair<LabelStr,LabelStr> > domainSpecs;
    TokenType tt(LabelStr("SEP_Thrust_Timer_SV.Max_Thrust_Time"), domainSpecs);

    heuristics.setDefaultPriorityForTokenDPsWithParent(20.3, tt.getId());

    heuristics.setDefaultPriorityForTokenDPs(10000.0);
    
    heuristics.setDefaultPriorityForConstrainedVariableDPs(10000.0);

    std::vector<LabelStr> states;
    std::vector<HSTSHeuristics::CandidateOrder> orders;
    states.push_back(Token::REJECTED);
    states.push_back(Token::MERGED);
    //    states.push_back(Token::DEFER);
    states.push_back(Token::ACTIVE);
    orders.push_back(HSTSHeuristics::NONE);
    orders.push_back(HSTSHeuristics::EARLY);
    //    orders.push_back(HSTSHeuristics::NONE);
    orders.push_back(HSTSHeuristics::EARLY);
    heuristics.setDefaultPreferenceForTokenDPs(states,orders);

    heuristics.setDefaultPreferenceForConstrainedVariableDPs(HSTSHeuristics::ASCENDING);

    return true;
  }

  bool testTokenInitializationImpl(HSTSHeuristics& heuristics) {
    std::vector<std::pair<LabelStr,LabelStr> > domainSpecs;
    TokenType tta(LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Left"), domainSpecs);
    heuristics.setHeuristicsForTokenDPsWithParent(5456.2,tta.getId());

    std::vector<std::pair<LabelStr,LabelStr> > dsb;
    dsb.push_back(std::make_pair<LabelStr,LabelStr>(LabelStr("m_with"), LabelStr("SIN")));
    TokenType ttb(LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd"), dsb);
    std::vector<LabelStr> statesb;
    statesb.push_back(Token::REJECTED);
    statesb.push_back(Token::MERGED);
    //    statesb.push_back(LabelStr::DEFER);
    statesb.push_back(Token::ACTIVE);
    std::vector<HSTSHeuristics::CandidateOrder> ordersb;
    ordersb.push_back(HSTSHeuristics::NONE);
    ordersb.push_back(HSTSHeuristics::NEAR);
    //    ordersb.push_back(HSTSHeuristics::NONE);
    ordersb.push_back(HSTSHeuristics::MIN_FLEXIBLE);
    heuristics.setHeuristicsForTokenDP(334.5, ttb.getId(), HSTSHeuristics::ANY, TokenTypeId::noId(), statesb, ordersb);

    std::vector<std::pair<LabelStr,LabelStr> > dsc;
    dsc.push_back(std::make_pair<LabelStr,LabelStr>(LabelStr("m_with"),LabelStr("CON")));
    dsc.push_back(std::make_pair<LabelStr,LabelStr>(LabelStr("m_sys"),LabelStr("RES")));
    TokenType ttc(LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd"), dsc);
    TokenType mttc(LabelStr("SEP_Thrust_Timer_SV.Max_Thrust_Time"), domainSpecs);
    std::vector<LabelStr> statesc;
    std::vector<HSTSHeuristics::CandidateOrder> ordersc;
    statesc.push_back(Token::MERGED);
    statesc.push_back(Token::ACTIVE);
    ordersc.push_back(HSTSHeuristics::TGENERATOR);
    ordersc.push_back(HSTSHeuristics::LATE);
    heuristics.setHeuristicsForTokenDP(6213.7, ttc.getId(), HSTSHeuristics::AFTER, mttc.getId(), statesc, ordersc);

    std::vector<std::pair<LabelStr,LabelStr> > dsd;
    dsd.push_back(std::make_pair<LabelStr,LabelStr>(LabelStr("m_trans"),LabelStr("NO_RIGHT")));
    TokenType ttd(LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd"), dsd);
    TokenType mttd(LabelStr("SEP_Thrust_Timer_SV.Max_Thrust_Time"), domainSpecs);

    std::vector<LabelStr> statesd;
    std::vector<HSTSHeuristics::CandidateOrder> ordersd;
    statesd.push_back(Token::ACTIVE);
    ordersd.push_back(HSTSHeuristics::EARLY);
    heuristics.setHeuristicsForTokenDP(6213.7, ttd.getId(), HSTSHeuristics::ANY, TokenTypeId::noId(), statesd, ordersd);

    std::vector<LabelStr> statese;
    std::vector<HSTSHeuristics::CandidateOrder> orderse;
    heuristics.setHeuristicsForTokenDP(7652.4, ttd.getId(), HSTSHeuristics::ANY, TokenTypeId::noId(), statese, orderse);

    return true;
  }

  bool testVariableInitializationImpl(HSTSHeuristics& heuristics) {
    std::vector<LabelStr> aenums;
    heuristics.setHeuristicsForConstrainedVariableDP(443.6, LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd.with"), TokenTypeId::noId(), HSTSHeuristics::ASCENDING, NO_STRING, aenums);

    aenums.push_back(LabelStr("SIN"));
    aenums.push_back(LabelStr("CON"));
    aenums.push_back(LabelStr("SAL"));
    aenums.push_back(LabelStr("PEP"));
    heuristics.setHeuristicsForConstrainedVariableDP(443.6, LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd.with"), TokenTypeId::noId(), HSTSHeuristics::ENUMERATION, NO_STRING, aenums);

    std::vector<LabelStr> emptyList;
    heuristics.setHeuristicsForConstrainedVariableDP(2269.3, LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Left.m_with"), TokenTypeId::noId(), HSTSHeuristics::VGENERATOR, LabelStr("Generator1"),emptyList);

    std::vector<LabelStr> benums;
    heuristics.setHeuristicsForConstrainedVariableDP(234.5, LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd.m_sys"), TokenTypeId::noId(), HSTSHeuristics::ENUMERATION, NO_STRING, benums);

    std::vector<LabelStr> cenums;
    cenums.push_back(LabelStr("CON"));
    //    LabelStr parentName = stripVariableName("Made_Up_Parent_SV.Made_Up_SV.Thrust_Left.m_with");
    LabelStr parentName = LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Left");
    std::vector<std::pair<LabelStr,LabelStr> > cds;
    cds.push_back(std::make_pair<LabelStr,LabelStr>(LabelStr("m_sys"),LabelStr("ON")));
    TokenType ctt(parentName, cds);
    heuristics.setHeuristicsForConstrainedVariableDP(6234.7, LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd.m_with"), ctt.getId(), HSTSHeuristics::ENUMERATION, NO_STRING, benums);

    return true;
  }

  bool testReaderImpl(HSTSHeuristics& heuristics) {
    initHeuristicsSchema(Schema::instance());

    HSTSHeuristicsReader reader(heuristics, Schema::instance());

    reader.read("../component/Heuristics-HSTS.xml");
    
    heuristics.write();

    return true;
  }

  bool testHSTSPlanIdReaderImpl() {

    initHeuristicsSchema(Schema::instance());

    HSTSNoBranch noBranchSpec(Schema::instance());
    HSTSPlanIdReader reader(noBranchSpec);
    reader.read("../component/NoBranch.pi");

    return true;
  }

  bool testHSTSNoBranchImpl(ConstraintEngine &ce, PlanDatabase &db, DecisionManager &dm) {
    initHeuristicsSchema(Schema::instance());

    HSTSNoBranch noBranchSpec(Schema::instance());
    HSTSPlanIdReader reader(noBranchSpec);
    reader.read("../component/NoBranch.pi");

    HSTSNoBranchCondition cond(dm.getId());
    assert(dm.getConditions().size() == 1);

    Variable<IntervalIntDomain> var1(ce.getId(), IntervalIntDomain(), true, LabelStr("Navigator.Going.2"));
    Variable<IntervalIntDomain> var2(ce.getId(), IntervalIntDomain(), true, LabelStr("AnObj.APred.Var2"));

    std::cout << " var1 name = " << var1.getName().c_str() << std::endl;
    std::cout << " var2 name = " << var2.getName().c_str() << std::endl;

    cond.initialize(noBranchSpec);

    assert(!cond.test(var1.getId()));
    assert(cond.test(var2.getId()));

    return true;
  }

}
