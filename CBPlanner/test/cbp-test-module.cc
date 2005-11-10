#include "cbp-test-module.hh"
#include "CBPlannerDefs.hh"
#include "DecisionPoint.hh"
#include "CBPlanner.hh"
#include "DecisionManager.hh"
#include "OpenDecisionManager.hh"
#include "SubgoalOnceRule.hh"
#include "Condition.hh"
#include "Horizon.hh"
#include "HorizonCondition.hh"
#include "DynamicInfiniteRealCondition.hh"
#include "TemporalVariableFilter.hh"
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
#include "RuleInstance.hh"

/* Miscellaneous */
#include "TestSupport.hh"
#include "XMLUtils.hh"

/* Heuristics Engine */
#include "HeuristicsEngine.hh"
#include "Heuristic.hh"
#include "HeuristicsReader.hh"

#include "NumericDomain.hh"
#include "StringDomain.hh"
#include "ObjectDecisionPoint.hh"
#include "ConditionalRule.hh"
#include "NotFalseConstraint.hh"
#include "ConstrainedVariableDecisionPoint.hh"
#include "MasterMustBeInserted.hh"

#include <list>
#include <vector>
#include <iostream>
#include <string>
#include <fstream>

/**
 * @brief Test Constraint to only fire when all values are singletons and to then
 * require that all values are different. Deliberately want to force an inefficient search with
 * lots of backtrack.
 */
class LazyAllDiff: public Constraint {
public:
  LazyAllDiff(const LabelStr& name,
	      const LabelStr& propagatorName,
	      const ConstraintEngineId& constraintEngine,
	      const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
  }

  void handleExecute() {
    std::set<double> singletonValues;
    std::vector<ConstrainedVariableId>::const_iterator it_end = getScope().end();
    for(std::vector<ConstrainedVariableId>::const_iterator it = getScope().begin(); it != it_end; ++it){
      ConstrainedVariableId var = *it;
      if(getCurrentDomain(var).isSingleton())
	singletonValues.insert(getCurrentDomain(var).getSingletonValue());
      else
	return;
    }

    if(singletonValues.size() < getScope().size())
      getCurrentDomain(getScope().front()).empty();
  }
};


/**
 * @brief Test Constraint to only fire when all values are singletons 
 * and to then always fail. Used to force exhaustive search.
 */
class LazyAlwaysFails: public Constraint {
public:
  LazyAlwaysFails(const LabelStr& name,
	      const LabelStr& propagatorName,
	      const ConstraintEngineId& constraintEngine,
	      const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
  }

  void handleExecute() {
    std::vector<ConstrainedVariableId>::const_iterator it_end = getScope().end();
    for(std::vector<ConstrainedVariableId>::const_iterator it = getScope().begin(); it != it_end; ++it){
      ConstrainedVariableId var = *it;
      if(!getCurrentDomain(var).isSingleton())
	return;
    }

    getCurrentDomain(getScope().front()).empty();
  }
};

/**
 * @brief Test Constraint to only fire when all values are singletons 
 * and to then fail in all cases except when the values are all set to the upper bound in the
 * vase domain
 */
class FailsTillLastOne: public Constraint {
public:
  FailsTillLastOne(const LabelStr& name,
	      const LabelStr& propagatorName,
	      const ConstraintEngineId& constraintEngine,
	      const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
  }

  void handleExecute() {
    unsigned int maxValueCount = 0;
    std::vector<ConstrainedVariableId>::const_iterator it_end = getScope().end();
    for(std::vector<ConstrainedVariableId>::const_iterator it = getScope().begin(); it != it_end; ++it){
      ConstrainedVariableId var = *it;
      if(!getCurrentDomain(var).isSingleton())
	return;
      if(getCurrentDomain(var).getSingletonValue() == var->baseDomain().getUpperBound())
	maxValueCount++;
    }

    // If not alkl at the max values, then fore a backtrack
    if(maxValueCount < getScope().size())
      getCurrentDomain(getScope()[0]).empty();
  }
};

class NotSingleton : public Constraint {
public:
  NotSingleton(const LabelStr& name,
		     const LabelStr& propagatorName,
		     const ConstraintEngineId& constraintEngine,
		     const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables){}

  void handleExecute(){
    if(getScope()[0]->lastDomain().isSingleton())
      getCurrentDomain(getScope()[0]).empty();
  }
};


#define CBP_DEFAULT_SETUP(ce, db, autoClose) \
    ConstraintEngine ce; \
    initCBPTestSchema(); \
    PlanDatabase db(ce.getId(), Schema::instance()); \
    new DefaultPropagator(LabelStr("Default"), ce.getId()); \
    new DefaultPropagator(LabelStr("Temporal"), ce.getId()); \
    RulesEngine re(db.getId()); \
    Horizon hor(0,200); \
    OpenDecisionManager odm(db.getId());\
    DecisionManager dm(db.getId(), odm.getId()); \
    if (autoClose) \
      db.close();

#define CBP_DEFAULT_TEARDOWN()

#define CBP_DEFAULT_SETUP_PLAN(ce, db, autoClose) \
  SETUP_HEURISTICS("../../HeuristicsEngine/test/DefaultHeuristics.xml") \
  initCBPTestSchema();			    \
  planner.getDecisionManager()->getOpenDecisionManager()->initializeIfNeeded(); \
  if (autoClose) \
    db.close();

#define CBP_DEFAULT_TEARDOWN_PLAN()

#define SETUP_HEURISTICS(heuristicsSource) \
  READ_HEURISTICS(heuristicsSource, true)


#define READ_HEURISTICS(heuristicsSource, autoClose)		\
  ConstraintEngine ce;						\
  PlanDatabase db(ce.getId(), Schema::instance());		\
  new DefaultPropagator(LabelStr("Default"), ce.getId());	\
  new DefaultPropagator(LabelStr("Temporal"), ce.getId());	\
  RulesEngine re(db.getId());					\
  HeuristicsEngine heuristics(db.getId()); \
  HeuristicsReader hreader(heuristics.getId()); \
  hreader.read(heuristicsSource, autoClose); \
  OpenDecisionManager odm(db.getId(), heuristics.getId());  \
  DecisionManager dm(db.getId(), odm.getId()); \
  Horizon hor(0, 200);						\
  CBPlanner planner(db.getId(), hor.getId(), odm.getId());
	
#define TEARDOWN()


/***********************************************************************************************
 * Declaration of classes and definition to test rule guard behavior.
 ***********************************************************************************************/

/**
 * On activation of a token of type 'Object.P1' this will allocate a root rule instance.
 * @see GNATS_3013
 */


/**
 * Child rule will fire as a NO-OP
 */
class NestedGuardsRuleInstanceChild: public RuleInstance{
public:
  NestedGuardsRuleInstanceChild(const RuleInstanceId& parentInstance, const std::vector<ConstrainedVariableId>& guards)
    : RuleInstance(parentInstance, guards){};
  void handleExecute(){} // A NO-OP
};

/**
 * Root rule instance will be guarded by parametr LabelSetParam0. When set to a singleton it will
 * fire. Because of this, the Decision Manager will only learn that it is a guard after it has first evaluated it.
 */
class NestedGuardsRuleInstanceRoot: public RuleInstance{
public:
  NestedGuardsRuleInstanceRoot(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
    : RuleInstance(rule, token, planDb, makeScope(token->getVariable(LabelStr("LabelSetParam0")))) {}

  /** Execution will allocate a child rule which is also guarded using the object variable now. **/
  void handleExecute(){
    addChildRule(new NestedGuardsRuleInstanceChild(m_id, 
						   makeScope(getToken()->getVariable(LabelStr("LabelSetParam1")))));
  }
};

class NestedGuardsRule: public Rule {
public:
  NestedGuardsRule(): Rule(LabelStr("Object.P1")){}

  RuleInstanceId createInstance(const TokenId& token, const PlanDatabaseId& planDb,
                                const RulesEngineId &rulesEngine) const{
    RuleInstanceId rootInstance = (new NestedGuardsRuleInstanceRoot(m_id, token, planDb))->getId();
    rootInstance->setRulesEngine(rulesEngine);
    return rootInstance;
  }
};


  /**
   * @brief Creates the type specifications required for testing
   */
  void initCBPTestSchema(){
    const SchemaId& schema = Schema::instance();
    schema->reset();
    schema->addObjectType("Object");

    schema->addPredicate("Object.PredicateA");
    schema->addMember("Object.PredicateA", IntervalIntDomain().getTypeName(), "IntervalIntParam");

    schema->addPredicate("Object.PredicateB");
    schema->addPredicate("Object.PredicateC");
    schema->addPredicate("Object.PredicateD");
    schema->addPredicate("Object.PADDED");

    schema->addPredicate("Object.PredicateE");
    schema->addMember("Object.PredicateE", IntervalIntDomain().getTypeName(), "param0");
    schema->addMember("Object.PredicateE", IntervalDomain().getTypeName(), "param1");
    schema->addMember("Object.PredicateE", LabelSet().getTypeName(), "param2");

    schema->addPredicate("Object.P1");
    schema->addMember("Object.P1", LabelSet().getTypeName(), "LabelSetParam0");
    schema->addMember("Object.P1", LabelSet().getTypeName(), "LabelSetParam1");
    schema->addMember("Object.P1", IntervalIntDomain().getTypeName(), "IntervalIntParam");

    schema->addPredicate("Object.P1True");
    schema->addMember("Object.P1True", BoolDomain().getTypeName(), "BoolParam");
    schema->addPredicate("Object.P1False");
  }

  static void makeTestToken(IntervalToken& token, const std::list<double>& values){
    token.addParameter(LabelSet(values), "LabelSetParam0");
    LabelSet leaveOpen;
    leaveOpen.insert(values);
    token.addParameter(leaveOpen, "LabelSetParam1");
    token.addParameter(IntervalIntDomain(1, 20), "IntervalIntParam");
    token.close();
  }

class CBPDefaultSetupTest {
public:
  static bool test() {
    runTest(testDefaultSetup);
    return(true);
  }

private:
  static bool testDefaultSetup() {
    CBP_DEFAULT_SETUP(ce, db, false);
    assertTrue(db.isClosed() == false);
    db.close();
    assertTrue(db.isClosed() == true);
    CBP_DEFAULT_TEARDOWN();
    return true;
  }
};

class ConditionTest {
public:
  static bool test() {
    runTest(testHorizon);
    runTest(testHorizonCondition);
    runTest(testHorizonConditionNecessary);
    runTest(testSetHorizionCondition);
    runTest(testDynamicInfiniteRealCondition);
    runTest(testMasterMustBeInserted);
    runTest(testTemporalVariableFilter);
    return(true);
  }
private:

  static bool testSetHorizionCondition() {
    // create a CBPlanner
    CBP_DEFAULT_SETUP_PLAN(ce, db, true);
    // Default horizion condition should be
    assertTrue(!planner.isPossiblyOutsideHorizon());
    assertTrue(planner.isNecessarilyOutsideHorizon());

    // Set horizion condition
    planner.setNecessarilyOutsideHorizon();

    // test horizion condition setting.
    assertTrue(planner.isPossiblyOutsideHorizon());
    assertTrue(!planner.isNecessarilyOutsideHorizon());

    // set back 
    planner.setPossiblyOutsideHorizon();

    // test is back
    assertTrue(!planner.isPossiblyOutsideHorizon());
    assertTrue(planner.isNecessarilyOutsideHorizon());

    CBP_DEFAULT_TEARDOWN_PLAN();
    return true;
  }

  static bool testHorizon() {
    CBP_DEFAULT_SETUP(ce, db, true);
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
    CBP_DEFAULT_TEARDOWN();
    return true;
  }

  static bool testHorizonCondition() {
    CBP_DEFAULT_SETUP(ce, db, false);
    HorizonCondition cond(hor.getId(), dm.getId());

    assertTrue(cond.isPossiblyOutsideHorizon());
    assertTrue(!cond.isNecessarilyOutsideHorizon());
    assertTrue(dm.getConditions().size() == 1);

    Timeline t(db.getId(), LabelStr("Object"), LabelStr("t1"));

    db.close();

    odm.initializeIfNeeded();

    IntervalToken tokenA(db.getId(), 
			 "Object.P1", 
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
    tokenA.activate();
    assertTrue(ce.propagate());
    assertTrue(dm.getNumberOfDecisions() == 1, toString(dm.getNumberOfDecisions()));

    hor.setHorizon(200,400);
    assertTrue(ce.propagate());
    assertTrue(cond.test(t.getId()));
    assertTrue(!cond.test(tokenA.getId()));
    assertTrue(!cond.test(tokenA.getStart()));
    assertTrue(!cond.test(tokenA.getEnd()));
    assertTrue(!cond.test(tokenA.getDuration()));

    //std::cout << " Decisions after changing horizon = 0 " << std::endl;

    assertTrue(dm.getNumberOfDecisions() == 0, toString(dm.getNumberOfDecisions()));

    cond.setNecessarilyOutsideHorizon();
    assertTrue(cond.isNecessarilyOutsideHorizon());
    
    hor.setHorizon(0,50);
    assertTrue(ce.propagate());
    assertTrue(cond.test(t.getId()));
    assertTrue(cond.test(tokenA.getId()));
    assertTrue(cond.test(tokenA.getStart()));
    assertTrue(cond.test(tokenA.getEnd()));
    assertTrue(cond.test(tokenA.getDuration()));
    
    //std::cout << " Decisions after changing horizon = 4" << std::endl;

    assertTrue(dm.getNumberOfDecisions() == 1, toString(dm.getNumberOfDecisions()));
    CBP_DEFAULT_TEARDOWN();
    return true;
  }

static bool testHorizonConditionNecessary() {
    CBP_DEFAULT_SETUP(ce, db, false);
    HorizonCondition cond(hor.getId(), dm.getId());
    // change to using the necessary condtion for horizion containment. 
    cond.setNecessarilyOutsideHorizon();
    // check the flags are set correctly. 
    assertTrue(!cond.isPossiblyOutsideHorizon());
    assertTrue(cond.isNecessarilyOutsideHorizon());

    // build a test token. 
    Timeline t(db.getId(), LabelStr("Object"), LabelStr("t1"));
    db.close();
    odm.initializeIfNeeded();
    IntervalToken tokenA(db.getId(), 
			 "Object.P1", 
			 false,
			 IntervalIntDomain(10, 13),
			 IntervalIntDomain(20, 23),
			 IntervalIntDomain(10, 10));

    assertTrue(ce.propagate());
    assertTrue(cond.test(t.getId())); 

    // Horizon covers tokenA
    hor.setHorizon(0,30);
    assertTrue(ce.propagate());
    assertTrue(cond.test(tokenA.getId()));

    // test tokenstart.upperbound > end
    hor.setHorizon(0,10);
    assertTrue(ce.propagate());
    assertTrue(cond.test(tokenA.getId()));

    // test tokenstart.lowerbound > end
    hor.setHorizon(0,9);
    assertTrue(ce.propagate());
    assertTrue(!cond.test(tokenA.getId()));

    // test tokenend.lowerboud < start
    hor.setHorizon(21,30);
    assertTrue(ce.propagate());
    assertTrue(cond.test(tokenA.getId()));

    // test tokenend.upperbound < start
    hor.setHorizon(24,30);
    assertTrue(ce.propagate());
    assertTrue(!cond.test(tokenA.getId()));

    CBP_DEFAULT_TEARDOWN();
    return true;
  }

  static bool testDynamicInfiniteRealCondition() {
    CBP_DEFAULT_SETUP(ce, db, false);
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

    Timeline t(db.getId(), LabelStr("Object"), LabelStr("t1"));
    db.close();

    odm.initializeIfNeeded();

    IntervalToken tokenA(db.getId(), 
			 "Object.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000));
    tokenA.activate();
    assertTrue(ce.propagate());

    assertTrue(cond.test(t.getId()));
    assertTrue(cond.test(tokenA.getId()));
    assertTrue(cond.test(v0.getId()));
    assertTrue(!cond.test(v1.getId()));
    assertTrue(!cond.test(v2.getId()));
    assertTrue(cond.test(v3.getId()));
    assertTrue(!cond.test(v4.getId()));
    assertTrue(cond.test(tokenA.getDuration()));

    assertTrue(dm.getNumberOfDecisions() == 3, toString(dm.getNumberOfDecisions()));

  // make sure it appears as expected. 

    // test function to disable dynamic exclsuion
    
    // should be on by default.
     assertTrue(cond.isDynamicExclusionEnabled());
    
    // turn it off.
    cond.disableDynamicExclusion();

    // should now be off.
    assertFalse(cond.isDynamicExclusionEnabled());
 
    // V1 should now be testable as its domain is open 
    // and we have stopped excluding open domains.

    assertTrue(cond.test(t.getId()));
    assertTrue(cond.test(tokenA.getId()));
    assertTrue(cond.test(v0.getId()));
    assertTrue(cond.test(v1.getId()));  // now testable
    assertTrue(!cond.test(v2.getId()));
    assertTrue(cond.test(v3.getId()));
    assertTrue(!cond.test(v4.getId()));
    assertTrue(cond.test(tokenA.getDuration()));

    // one extra decision
    assertTrue(dm.getNumberOfDecisions() == 4, toString(dm.getNumberOfDecisions()));

    CBP_DEFAULT_TEARDOWN();
    return true;
  }

  static bool testMasterMustBeInserted(){
    CBP_DEFAULT_SETUP(ce, db, false);
    MasterMustBeInserted condition(dm.getId());

    Timeline t(db.getId(), LabelStr("Object"), LabelStr("t1"));
    db.close();

    odm.initializeIfNeeded();

    IntervalToken t0(db.getId(), 
                     "Object.P1",
                     false, 
                     IntervalIntDomain(0, 1),
                     IntervalIntDomain(0, 1),
                     IntervalIntDomain(1, 1));

    // Since it is an inactive token with no master, we will not exclude it.
    ce.propagate();
    assertTrue(MasterMustBeInserted::executeTest(t0.getId()));

    // Token State
    assertTrue(dm.getNumberOfDecisions() == 1, toString(dm.getNumberOfDecisions()));

    // Now activate it. It should still pass, since we do not care about it.
    t0.activate();
    ce.propagate();
    assertFalse(t0.isAssigned());

    // Since it must now be constrained, the decision to handle state is replaced
    assertTrue(dm.getNumberOfDecisions() == 1, toString(dm.getNumberOfDecisions()));
    assertTrue(MasterMustBeInserted::executeTest(t0.getId()));
  
    // Now allocate a slave.
    IntervalToken t1(t0.getId(), 
		     LabelStr("any"),
                     LabelStr("Object.P1"),
		     IntervalIntDomain(0, 1),
		     IntervalIntDomain(0, 1),
		     IntervalIntDomain(1, 1));

    // This should be excluded since the master has not been assigned
    ce.propagate();
    assertFalse(MasterMustBeInserted::executeTest(t1.getId()));

    // Number of decisions should not change, since it is filtered out
    assertTrue(dm.getNumberOfDecisions() == 1, toString(dm.getNumberOfDecisions()));
  
    // Constrain the master and thus enable the slave
    t.constrain(t0.getId(), t0.getId());
    ce.propagate();
    assertTrue(MasterMustBeInserted::executeTest(t1.getId()));

    // Now the number of decisions should be increased to include the State decision on the slave
    // but also reduced by constrain
    assertTrue(dm.getNumberOfDecisions() == 1, toString(dm.getNumberOfDecisions()));

    // Now free it and verify it is once again excluded
    t.free(t0.getId(), t0.getId());
    ce.propagate();

    // Now we should go back to 1
    assertTrue(dm.getNumberOfDecisions() == 1, toString(dm.getNumberOfDecisions()));
    assertFalse(MasterMustBeInserted::executeTest(t1.getId()));
    return true;
  }

  static bool testTemporalVariableFilter(){
    CBP_DEFAULT_SETUP(ce, db, false);

    TemporalVariableFilter condition(dm.getId());

    Timeline t(db.getId(), LabelStr("Object"), LabelStr("t1"));
    db.close();

    odm.initializeIfNeeded();

    IntervalToken t0(db.getId(), 
                     "Object.P1",
                     false, 
                     IntervalIntDomain(0, 10),
                     IntervalIntDomain(0, 100),
                     IntervalIntDomain(1, 100));

    // Since it is an inactive token with no master, we will not exclude it.
    t0.activate();
    ce.propagate();

    // Just a threat
    assertTrue(dm.getNumberOfDecisions() == 1, odm.printOpenDecisions());

    return true;
  }

};

class DecisionManagerTest {
public:
  static bool test() {
    runTest(testUnitHandling);
    runTest(testForwardDecisionHandling);
    runTest(testNestedGuard_GNATS_3013);
    runTest(testSynchronizationBug_GNATS_3027);
    return(true);
  }

private:
  static bool testUnitHandling() {
    CBP_DEFAULT_SETUP(ce, db, false);

    Timeline t(db.getId(), LabelStr("Object"), LabelStr("t1"));
    db.close();

    odm.initializeIfNeeded();

    std::list<double> values;
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L4"));
    values.push_back(LabelStr("L2"));
    values.push_back(LabelStr("L5"));
    values.push_back(LabelStr("L3"));
    Variable<LabelSet> v0(ce.getId(), LabelSet(values));
    Variable<IntervalIntDomain> v1(ce.getId(), IntervalIntDomain());
    Variable<IntervalIntDomain> v2(ce.getId(), IntervalIntDomain());
    ce.propagate();
    assertTrue(odm.getNumberOfDecisions() == 3);
    assertFalse(odm.isUnitDecision(v0.getId()));
    v0.specify(LabelStr("L1"));
    assertFalse(odm.isUnitDecision(v0.getId()));
    ce.propagate();
    assertFalse(odm.isVariableDecision(v0.getId()));
    v0.reset();
    assertTrue(odm.isVariableDecision(v0.getId()));

    EqualConstraint c0(LabelStr("EqualConstraint"), LabelStr("Default"), ce.getId(), makeScope(v1.getId(), v2.getId()));
    v1.specify(10);
    ce.propagate();
    assertFalse(odm.isVariableDecision(v1.getId()));

    // Propagate and expect that v2 will become a singleton. Since it is not a guard, that indicates it is
    // evaluated as a singleton
    ce.propagate();
    assertFalse(odm.isVariableDecision(v2.getId()));

    v1.reset();
    ce.propagate();
    assertTrue(odm.isVariableDecision(v2.getId()));

    DecisionPointId dec = odm.getZeroCommitmentDecision();
    assertTrue(dec.isNoId());

    // No allocate a token. Want to see that we get a zero commitment decision
    // for a token decision
    IntervalToken tokenA(db.getId(), 
			 "Object.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000));

    return true;
  }
  static bool testForwardDecisionHandling() {
    CBP_DEFAULT_SETUP(ce, db, false);
    HorizonCondition hcond(hor.getId(), dm.getId());
    DynamicInfiniteRealCondition dcond(dm.getId());

    assertTrue(dm.getConditions().size() == 2);

    std::list<double> values;
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L4"));
    values.push_back(LabelStr("L2"));
    values.push_back(LabelStr("L5"));
    values.push_back(LabelStr("L3"));

    odm.initializeIfNeeded();

    Variable<LabelSet> v0(ce.getId(), LabelSet(values));
    LabelSet leaveOpen;
    leaveOpen.insert(values);
    Variable<LabelSet> v1(ce.getId(), leaveOpen);
    Variable<IntervalDomain> v2(ce.getId(), IntervalDomain(1, 20));
    Variable<IntervalIntDomain> v3(ce.getId(), IntervalIntDomain(1, 20));
    Variable<IntervalIntDomain> v4(ce.getId(), IntervalIntDomain());

    Timeline t(db.getId(), LabelStr("Object"), LabelStr("t1"));
    db.close();
    IntervalToken tokenA(db.getId(), 
			 "Object.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000));

    assertTrue(ce.propagate());

    assertTrue(dm.isVariableDecision(v0.getId()), "Finite and closed.");
    assertFalse(dm.isVariableDecision(v1.getId()), "Is still open.");
    assertFalse(dm.isVariableDecision(v2.getId()), "Has infinite values.");
    assertTrue(dm.isVariableDecision(v3.getId()), "Has finite bounds and values.");
    assertFalse(dm.isVariableDecision(v4.getId()), "Has infinite bounds.");
    assertTrue(dm.isTokenDecision(tokenA.getId()), "Is inactive.");
    assertFalse(dm.isObjectDecision(tokenA.getId()), "Is inactive so can't be ordered.");
    assertTrue(dm.getNumberOfDecisions() == 3);

    tokenA.activate();
    assertTrue(ce.propagate());
    assertFalse(dm.isTokenDecision(tokenA.getId()), "Should now be active.");
    assertTrue(dm.isObjectDecision(tokenA.getId()), "Should be required for ordering.");
    assertTrue(dm.assignDecision());
    assertFalse(dm.isObjectDecision(tokenA.getId()), "Should have been decided next.");

    assertTrue(dm.getNumberOfDecisions() == 2, toString(dm.getNumberOfDecisions()));

    assertTrue(dm.assignDecision());

    assertTrue(dm.getNumberOfDecisions() == 1, toString(dm.getNumberOfDecisions()));

    assertTrue(dm.assignDecision());

    assertTrue(dm.getNumberOfDecisions() == 0, toString(dm.getNumberOfDecisions()));
    assertFalse(dm.assignDecision(), "No more decisions.");

    IntervalToken tokenB(db.getId(), 
			 "Object.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000));

    assertTrue(ce.propagate());
    assertTrue(dm.assignDecision());
    assertTrue(dm.getNumberOfDecisions() == 0);
    assertFalse(dm.assignDecision(), "No more decisions.");
    CBP_DEFAULT_TEARDOWN();
    return true;
  }


  /**
   * Tests the case of a variable being initially found undecidable
   * but later becoming decidable. This can occur, even with restrictions in the
   * plan database, since a variable may have a unit valued derived domain but not be a
   * guard. It may subsequently be found to be a guard.
   */
  static bool testNestedGuard_GNATS_3013(){
    CBP_DEFAULT_SETUP(ce, db, false);

    odm.initializeIfNeeded();
    assertTrue(ce.propagate());
    assertTrue(dm.getOpenDecisionManager().isValid()); // Force allocate of default
    assertTrue(dm.getNumberOfDecisions() == 0);

    // Allocate a timeline and close the database.
    Timeline t(db.getId(), LabelStr("Object"), LabelStr("t1"));

    db.close();

    // Register a test rule to be fired.
    NestedGuardsRule r;

    std::list<double> values;
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L4"));
    values.push_back(LabelStr("L2"));
    values.push_back(LabelStr("L5"));
    values.push_back(LabelStr("L3"));

    // Spec domain will be a singleton so should not be a flaw
    Variable<LabelSet> v0(ce.getId(), LabelSet(LabelStr("L3")));

    IntervalToken tokenA(db.getId(), 
			 "Object.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);
    tokenA.addParameter(LabelSet(values), "LabelSetParam0");
    tokenA.addParameter(LabelSet(values), "LabelSetParam1");
    tokenA.close();

    // Set up constraints os they are all equal
    ConstrainedVariableId v1 = tokenA.getVariable(LabelStr("LabelSetParam0"));
    ConstrainedVariableId v2 = tokenA.getVariable(LabelStr("LabelSetParam1"));
    EqualConstraint c0(LabelStr("EqualConstraint"), LabelStr("Default"), ce.getId(), makeScope(v0.getId(), v1));
    EqualConstraint c1(LabelStr("EqualConstraint"), LabelStr("Default"), ce.getId(), makeScope(v1, v2));

    // Note that the 2 additional parameters will not initially become decisions in this scenario
    // since they are singletons and we do not as yet have a rule fired since the token is not active.
    // Furthermore, since the token is inactive, they will be filtered out anyway.
    assertTrue(ce.propagate());

    // Now we expect that v1 and v2 are singletons
    assertTrue(v1->lastDomain().isSingleton());
    assertTrue(v2->lastDomain().isSingleton());
    assertTrue(dm.getNumberOfDecisions() == 1, toString(dm.getNumberOfDecisions()));

    // Now activate the token and expect that we will get an additional decision arising to
    // insert the token, and another to bind what will now be a compat guard variable. We will also
    // get decisions for start, end and duration.
    tokenA.activate();
    assertTrue(ce.propagate());
    assertTrue(dm.getNumberOfDecisions() == 2, dm.getOpenDecisionManager()->printOpenDecisions());
    assertTrue(dm.getOpenDecisionManager()->isUnitDecision(v1));
    assertFalse(dm.getOpenDecisionManager()->isUnitDecision(v2));

    // Now bind the value of the first additional parameter which should be an active guard.
    // We expect it to be removed from the decision manager. We also expect an additional decision
    // point to be allocated since the new child rule is in place.
    v1->specify(LabelStr("L3"));
    assertTrue(ce.propagate());
    assertFalse(dm.getOpenDecisionManager()->isUnitDecision(v1));
    assertTrue(dm.getOpenDecisionManager()->isUnitDecision(v2));
    assertTrue(dm.getNumberOfDecisions() == 2, toString(dm.getNumberOfDecisions()));

    // Now bind the second parameter and the net open decision count should go down.
    v2->specify(LabelStr("L3"));
    assertTrue(ce.propagate());
    assertTrue(dm.getNumberOfDecisions() == 1, toString(dm.getNumberOfDecisions()));

    // Now unwind incrementally and test that the correct refresh of flaws has occurred
    v2->reset();
    v1->reset();
    assertTrue(ce.propagate());
    dm.getOpenDecisionManager()->printOpenDecisions();
    assertTrue(dm.getNumberOfDecisions() == 2, dm.getOpenDecisionManager()->printOpenDecisions());
    assertTrue(dm.getOpenDecisionManager()->isUnitDecision(v1));
    assertFalse(dm.getOpenDecisionManager()->isUnitDecision(v2));
    tokenA.cancel();
    assertTrue(ce.propagate());
    assertTrue(dm.getNumberOfDecisions() == 1, dm.getOpenDecisionManager()->printOpenDecisions());
    return(true);
  }

  static bool testSynchronizationBug_GNATS_3027(){
    CBP_DEFAULT_SETUP(ce, db, false);
    HorizonCondition hcond(hor.getId(), dm.getId());
    DynamicInfiniteRealCondition dcond(dm.getId());

    assertTrue(dm.getConditions().size() == 2);

    Timeline t(db.getId(), LabelStr("Object"), LabelStr("t1"));
    db.close();

    odm.initializeIfNeeded();

    IntervalToken tokenA(db.getId(), 
			 "Object.P1", 
			 true,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000));

    NotSingleton constraint("NOT_SINGLETON", "Default", ce.getId(), makeScope(tokenA.getState(), tokenA.getObject()));

    assertTrue(ce.propagate());

    assertTrue(dm.getNumberOfDecisions() == 1);

    DecisionPointId decision = dm.getCurrentDecision();
    assertTrue(decision.isNoId());

    // Assign a decision. Should find the decision to make and that will become the 
    // current decision
    dm.assignDecision();
    decision = dm.getCurrentDecision();
    assertTrue(decision.isId());

    // Expect a failure
    assertTrue(ce.provenInconsistent());

    // Now retract the decision. It should still be current
    unsigned int retractCounter = 0;
    dm.retractDecision(retractCounter);
    assertTrue(ce.propagate());
    assertTrue(decision == dm.getCurrentDecision());
    assertTrue(retractCounter == 0);

    // Now synchronize the decision manager and confirm we still have the same current decision
    assertTrue(ce.propagate());
    assertTrue(decision == dm.getCurrentDecision());

    // Finish out this decision
    dm.assignDecision();
    dm.retractDecision(retractCounter);
    CBP_DEFAULT_TEARDOWN();
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
    runTest(testStraightCSPSolution);
    runTest(testExhaustiveCSPSearch);
    runTest(testExhaustiveTokenSearch);
    runTest(testSubgoalOnceRule);
    runTest(testBacktrackCase);
    runTest(testCompleteCSPSearch);
    runTest(testResetPlannerCase);
    runTest(testTimeoutCase);
    return true;
  }
private:
  static bool testMakeMove() {
    CBP_DEFAULT_SETUP_PLAN(ce, db, false);
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

    Timeline t(db.getId(), LabelStr("Object"), LabelStr("t1"));
    db.close();
    IntervalToken tokenA(db.getId(), 
			 "Object.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000));

    IntervalToken tokenB(db.getId(), 
			 "Object.P1", 
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
    CBP_DEFAULT_TEARDOWN_PLAN();
    return true;
  }

  static bool testCurrentState() {
    CBP_DEFAULT_SETUP_PLAN(ce, db, false);
    Timeline t(db.getId(), LabelStr("Object"), LabelStr("t1"));
    db.close();
    IntervalToken tokenA(db.getId(), 
			 "Object.P1", 
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

    CBP_DEFAULT_TEARDOWN_PLAN();
    return true;
  }

  static bool testRetractMove() {
    CBP_DEFAULT_SETUP_PLAN(ce, db, false);
    Timeline t(db.getId(), LabelStr("Object"), LabelStr("t1"));
    db.close();
    IntervalToken tokenA(db.getId(), 
			 "Object.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000));

    if (!planner.getDecisionManager()->assignDecision())
      return false;

    assertTrue(planner.getDecisionManager()->getClosedDecisions().size() == 1);

    //std::cout << "RETRACTING" << std::endl;

    unsigned int count;
    assertTrue(planner.getDecisionManager()->retractDecision(count));
    CBP_DEFAULT_TEARDOWN_PLAN();
    return true;
  }

  static bool testNoBacktrackCase() {
    CBP_DEFAULT_SETUP_PLAN(ce, db, false);
    Timeline timeline(db.getId(),LabelStr("Object"), LabelStr("t1"));
    db.close();

    IntervalToken tokenA(db.getId(), 
			 LabelStr("Object.P1"), 
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

    const DecisionStack& closed = planner.getClosedDecisions();

    assertTrue(planner.getTime() == planner.getDepth());
    assertTrue(closed.size() == planner.getTime());
    assertTrue(closed.size() == 4);
    CBP_DEFAULT_TEARDOWN_PLAN();
    return true;
  }

  static bool testSubgoalOnceRule() {
    CBP_DEFAULT_SETUP_PLAN(ce, db, false);
    Timeline timeline(db.getId(),LabelStr("Object"), LabelStr("t1"));
    db.close();

    SubgoalOnceRule r("Object.P1", 0);

    IntervalToken t0(db.getId(), "Object.P1", true, 		     
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
    CBP_DEFAULT_TEARDOWN_PLAN();
    return true;
  }

  static bool testStraightCSPSolution() {
    CBP_DEFAULT_SETUP_PLAN(ce, db, false);
    Timeline timeline(db.getId(),LabelStr("Object"), LabelStr("t1"));
    db.close();

    std::list<double> values;
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L4"));
    values.push_back(LabelStr("L2"));
    values.push_back(LabelStr("L5"));
    values.push_back(LabelStr("L3"));

    Variable<LabelSet> v0(ce.getId(), LabelSet(values));
    Variable<IntervalIntDomain> v1(ce.getId(), IntervalIntDomain(1, 10));
    Variable<IntervalIntDomain> v2(ce.getId(), IntervalIntDomain(1, 10));
    Variable<IntervalIntDomain> v3(ce.getId(), IntervalIntDomain(1, 10));
    Variable<IntervalIntDomain> v4(ce.getId(), IntervalIntDomain(1, 10));

    CBPlanner::Status status = planner.run();

    assertTrue(status == CBPlanner::PLAN_FOUND);
    assertTrue(planner.getDepth() == 5);
    assertTrue(planner.getTime() == 5);
    CBP_DEFAULT_TEARDOWN_PLAN();
    return true;
  }

  /**
   * Force an exhaustive search over a simple problem structure to force the basic
   * CBP algorithm through its paces.
   */
  static bool testExhaustiveCSPSearch() {
    CBP_DEFAULT_SETUP_PLAN(ce, db, false);
    Timeline timeline(db.getId(),LabelStr("Object"), LabelStr("t1"));
    db.close();

    std::list<double> values;
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L2"));
    
    Variable<LabelSet> v0(ce.getId(), LabelSet(values));
    Variable<IntervalIntDomain> v1(ce.getId(), IntervalIntDomain(1, 2));

    LazyAlwaysFails constraint("ALWAYS_FAILS", "Default", ce.getId(), 
			       makeScope(v0.getId(),v1.getId()));

    CBPlanner::Status status = planner.run();
  
    assertTrue(status == CBPlanner::SEARCH_EXHAUSTED);
    assertTrue(planner.getDepth() == 0);
    assertTrue(planner.getTime() == 6, toString(planner.getTime()));

    CBP_DEFAULT_TEARDOWN_PLAN();
    return true;
  }

  /**
   * Force an exhaustive search over a simple problem structure to force the basic
   * CBP algorithm through its paces.
   */
  static bool testExhaustiveTokenSearch() {
    CBP_DEFAULT_SETUP_PLAN(ce, db, false);
    Timeline timeline(db.getId(),LabelStr("Object"), LabelStr("t1"));
    db.close();

    std::list<double> values;
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L4"));
    values.push_back(LabelStr("L2"));
    values.push_back(LabelStr("L5"));
    values.push_back(LabelStr("L3"));

    IntervalToken tokenA(db.getId(), 
			 "Object.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);
    tokenA.addParameter(LabelSet(values), "LabelSetParam0");
    tokenA.close();

    IntervalToken tokenB(db.getId(), 
			 "Object.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);
    tokenB.addParameter(LabelSet(values), "LabelSetParam0");
    tokenB.close();

    LazyAlwaysFails constraint("ALWAYS_FAILS", "Default", ce.getId(), 
			       makeScope(tokenA.getState(), tokenB.getState()));

    CBPlanner::Status status = planner.run(100);
  
    assertTrue(status == CBPlanner::SEARCH_EXHAUSTED);
    assertTrue(planner.getDepth() == 0);

    CBP_DEFAULT_TEARDOWN_PLAN();
    return true;
  }

  /**
   * The goal of this test is to force the planner through an exhaustive search
   * and make sure it does not find a solution but backtracks all the way out. This is accomplished by:
   * 0. Set up a planning problem with 1 inactive tokens need to be planned out on the same object. The tokens
   *    include parameters which force decisions on variables.
   * 1. Introducing a subgoal rule that will  prohibit the subgoal from merging back on to the active master eventually.
   *    It will circumvent the lookahead to make the behavior more interesting.
   * 3. In the interim, we interleave variable decisions to make the search work hard.
   */
  static bool testBacktrackCase() {
    CBP_DEFAULT_SETUP_PLAN(ce, db, false);
    Timeline timeline(db.getId(),LabelStr("Object"), LabelStr("t1"));
    db.close();

    SubgoalOnceRule r("Object.P1", 0);

    std::list<double> values;
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L2"));

    IntervalToken tokenA(db.getId(), 
			 "Object.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);
    tokenA.addParameter(LabelSet(values), "LabelSetParam0");
    tokenA.addParameter(IntervalIntDomain(1, 2), "IntervalIntParam");
    tokenA.close();

    IntervalToken tokenB(db.getId(), 
			 "Object.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);
    tokenB.addParameter(LabelSet(values), "LabelSetParam0");
    tokenB.addParameter(IntervalIntDomain(1, 2), "IntervalIntParam");
    tokenB.close();

    // an equivalence constraint between the start times prevent A and B from being sequenced no the
    // same timeline
    std::vector<ConstrainedVariableId> scope;
    scope.push_back(tokenA.getStart());
    scope.push_back(tokenB.getStart());

    ConstraintLibrary::createConstraint(LabelStr("eq"), ce.getId(), scope);

    CBPlanner::Status res = planner.run(100);

    assertTrue(res == CBPlanner::SEARCH_EXHAUSTED, toString(res));
    assertTrue(planner.getClosedDecisions().empty());
    CBP_DEFAULT_TEARDOWN_PLAN();
    return true;
  }

  // Test that all options will be tried before coming to a conclusion 
  static bool testCompleteCSPSearch(){
    CBP_DEFAULT_SETUP_PLAN(ce, db, false);
    Timeline timeline(db.getId(),LabelStr("Object"), LabelStr("t1"));
    db.close();

    std::list<double> values;
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L2"));
    values.push_back(LabelStr("L3"));
    
    Variable<LabelSet> v0(ce.getId(), LabelSet(values));
    Variable<IntervalIntDomain> v1(ce.getId(), IntervalIntDomain(1, 4));

    FailsTillLastOne constraint("failsTillLastOne", "Default", ce.getId(), 
			       makeScope(v0.getId(),v1.getId()));

    CBPlanner::Status status = planner.run();
  
    assertTrue(status == CBPlanner::PLAN_FOUND);
    assertTrue(planner.getDepth() == 2);
    assertTrue(planner.getTime() == 3 + 3*4, toString(planner.getTime()));

    planner.retract();
    status = planner.run();
    assertTrue(status == CBPlanner::PLAN_FOUND);
    assertTrue(planner.getDepth() == 2);
    assertTrue(planner.getTime() == 3 + 3*4, toString(planner.getTime()));
    CBP_DEFAULT_TEARDOWN_PLAN();
    return true; 
  }

   // test CBPlanner::reset. Sets up aproblem identical to 
   // backtrack testcase then does a reset when no plan is found. 
   static bool testResetPlannerCase() {

    CBP_DEFAULT_SETUP_PLAN(ce, db, false);
    Timeline timeline(db.getId(),LabelStr("Object"), LabelStr("t1"));
    db.close();

    SubgoalOnceRule r("Object.P1", 0);

    std::list<double> values;
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L2"));
    values.push_back(LabelStr("L3"));

    IntervalToken tokenA(db.getId(), 
			 "Object.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);
    tokenA.addParameter(LabelSet(values), "LabelSetParam0");
    tokenA.addParameter(IntervalIntDomain(1, 2), "IntervalIntParam");
    tokenA.close();

    IntervalToken tokenB(db.getId(), 
			 "Object.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);
    tokenB.addParameter(LabelSet(values), "LabelSetParam0");
    tokenB.addParameter(IntervalIntDomain(1, 2), "IntervalIntParam");
    tokenB.close();

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

    // reset planner - the purpose of this test..
    planner.reset(); 

    res = planner.run(100);

    assertTrue(res == CBPlanner::SEARCH_EXHAUSTED);
    assertTrue(planner.getClosedDecisions().empty());

    CBP_DEFAULT_TEARDOWN_PLAN();
    return true;
  }


  static bool testTimeoutCase() {
    CBP_DEFAULT_SETUP_PLAN(ce, db, false);
    Timeline t1(db.getId(),LabelStr("Object"), LabelStr("t1"));
    Timeline t2(db.getId(),LabelStr("Object"), LabelStr("t2"));
    Object o1(db.getId(),LabelStr("Object"),LabelStr("o1"));
    db.close();

    std::list<double> values;
    values.push_back(LabelStr("L1"));
    values.push_back(LabelStr("L4"));
    values.push_back(LabelStr("L2"));
    values.push_back(LabelStr("L5"));
    values.push_back(LabelStr("L3"));

    IntervalToken tokenA(db.getId(), 
			 "Object.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);
    makeTestToken(tokenA, values);

    IntervalToken tokenB(db.getId(), 
			 "Object.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);

    makeTestToken(tokenB, values);

    IntervalToken tokenC(db.getId(), 
			 "Object.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);

    makeTestToken(tokenC, values);
    

    IntervalToken tokenD(db.getId(), 
			 "Object.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);

    makeTestToken(tokenD, values);

    IntervalToken tokenE(db.getId(), 
			 "Object.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 LabelStr("o1"), false);

    makeTestToken(tokenE, values);

    IntervalToken tokenF(db.getId(), 
			 "Object.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 LabelStr("o1"), false);

    makeTestToken(tokenF, values);

    IntervalToken tokenG(db.getId(), 
			 "Object.P1", 
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 20),
			 IntervalIntDomain(1, 1000),
			 Token::noObject(), false);

    makeTestToken(tokenG, values);
    
    CBPlanner::Status res = planner.run(9);
    assertTrue(res == CBPlanner::TIMEOUT_REACHED);

    const DecisionStack& closed = planner.getClosedDecisions();

    assertTrue(closed.size() == 9);
    assertTrue(closed.size() == planner.getTime());
    assertTrue(planner.getTime() == planner.getDepth());
    CBP_DEFAULT_TEARDOWN_PLAN();
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
    CBP_DEFAULT_SETUP(ce, db, false);
    CBP_DEFAULT_TEARDOWN();
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
    CBP_DEFAULT_SETUP_PLAN(ce, db, false);

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
    
    Timeline t(db.getId(), LabelStr("Object"), LabelStr("t1"));
    db.close();

    //    int numDecs = 4;
    //    int i=0;
    CBPlanner::Status result;
    for (;;) { /* Forever: only way out is to return */
      result = planner.step();
      if (result != CBPlanner::IN_PROGRESS) break;
    }
    assertTrue(result == CBPlanner::PLAN_FOUND);
    assertTrue(planner.getDepth() != planner.getTime());
    assertTrue(planner.getDepth() == 4, toString(planner.getDepth()));
    assertTrue(planner.getTime() == 11, toString(planner.getTime()));

    CBP_DEFAULT_TEARDOWN_PLAN();
    return true;
  }
  static bool testTokenDecisionCycle() {
    CBP_DEFAULT_SETUP_PLAN(ce, db, false);

    hor.setHorizon(300,400);

    Timeline t1(db.getId(), LabelStr("Object"), LabelStr("Timeline1"));
    Timeline t2(db.getId(), LabelStr("Object"), LabelStr("Timeline2"));
    Object t3(db.getId(), LabelStr("Object"), LabelStr("Object1"));
    db.close();

    ConditionalRule r("Object.PredicateA");

    IntervalToken tokenA(db.getId(), 
			 "Object.PredicateA", 
			 true,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(200, 200),
			 LabelStr("Timeline1"), false);
    tokenA.addParameter(IntervalIntDomain(1,2), "IntervalIntParam");
    tokenA.close();

    IntervalToken tokenB(db.getId(), 
			 "Object.PredicateB", 
			 false,
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(200, 200),
			 LabelStr("Timeline2"));

    tokenB.activate();
    t2.constrain(tokenB.getId(), tokenB.getId());

    assertTrue(ce.propagate());

    CBPlanner::Status result = planner.run(18);
    assertTrue(result == CBPlanner::PLAN_FOUND);

    assertTrue(planner.getTime() == planner.getDepth());
    assertTrue(planner.getTime() == 0);
    assertTrue(planner.getClosedDecisions().empty());

    hor.setHorizon(0,200);

    result = planner.run();
    assertTrue(result == CBPlanner::PLAN_FOUND);

    assertTrue(planner.getTime() != planner.getDepth());

    //assertTrue(planner.getDepth() == 7, toString(planner.getDepth()));
    //assertTrue(planner.getTime() == 17, toString(planner.getTime()));

    tokenA.cancel();
    tokenA.reject();
    tokenA.getParameters()[0]->reset();

    result = planner.run();
    assertTrue(result == CBPlanner::PLAN_FOUND);
    CBP_DEFAULT_TEARDOWN_PLAN();
    return true;
  }
  static bool testObjectDecisionCycle() {
    CBP_DEFAULT_SETUP_PLAN(ce, db, false);

    hor.setHorizon(10,500);

    Timeline t1(db.getId(), LabelStr("Object"), LabelStr("Timeline1"));
    db.close();

    IntervalToken tokenB(db.getId(), 
			 "Object.PredicateB", 
			 false,
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(200, 200));

    tokenB.activate();
    assertTrue(ce.propagate());
    assertTrue(planner.getDecisionManager()->getNumberOfDecisions() == 1);
    assertTrue(planner.getDecisionManager()->isObjectDecision(tokenB.getId()));
    tokenB.cancel();

    assertTrue(ce.propagate());
    assertTrue(planner.getDecisionManager()->getNumberOfDecisions() == 1); 
    assertFalse(planner.getDecisionManager()->isObjectDecision(tokenB.getId()));
    assertTrue(planner.getDecisionManager()->isTokenDecision(tokenB.getId()));
    
    tokenB.activate();

    assertTrue(ce.propagate());
    assertTrue(planner.getDecisionManager()->getNumberOfDecisions() == 1); 
    assertTrue(planner.getDecisionManager()->isObjectDecision(tokenB.getId()));

    tokenB.getStart()->specify(0);
    assertTrue(ce.propagate());
    assertTrue(planner.getDecisionManager()->getNumberOfDecisions() == 1);
    assertTrue(planner.getDecisionManager()->isObjectDecision(tokenB.getId()));

    tokenB.cancel();
    assertTrue(ce.propagate());
    assertTrue(planner.getDecisionManager()->getNumberOfDecisions() == 1); 
    assertFalse(planner.getDecisionManager()->isObjectDecision(tokenB.getId()));

    assertTrue(tokenB.getStart()->getDerivedDomain().isSingleton());
    assertTrue(tokenB.getStart()->getDerivedDomain().getSingletonValue() == 0);

    CBP_DEFAULT_TEARDOWN_PLAN();
    return true;
  }
  static bool testObjectAndObjectVariable() {
    CBP_DEFAULT_SETUP_PLAN(ce, db, false);
    hor.setHorizon(10,500);

    Object o1(db.getId(), LabelStr("Object"), LabelStr("Object1"));
    Timeline t1(db.getId(), LabelStr("Object"), LabelStr("Timeline1"));
    db.close();

    IntervalToken tokenB(db.getId(), 
			 "Object.PredicateB", 
			 false,
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(200, 200));

    tokenB.activate();
    
    assertTrue(ce.propagate());
    assertTrue(planner.getDecisionManager()->getNumberOfDecisions() == 2); 
    assertTrue(planner.getDecisionManager()->isObjectDecision(tokenB.getId()));
    assertTrue(planner.getDecisionManager()->isVariableDecision(tokenB.getObject()));

    tokenB.getObject()->specify(o1.getId());

    assertTrue(ce.propagate());
    assertTrue(!planner.getDecisionManager()->isVariableDecision(tokenB.getObject()));
    assertTrue(!planner.getDecisionManager()->isObjectDecision(tokenB.getId()));
    assertTrue(tokenB.isAssigned());
    assertTrue(planner.getDecisionManager()->getNumberOfDecisions() == 0);

    planner.getDecisionManager()->assignDecision();

    assertTrue(ce.propagate());
    assertTrue(planner.getDecisionManager()->getNumberOfDecisions() == 0); 
    
    unsigned int count;
    planner.getDecisionManager()->retractDecision(count);

    assertTrue(planner.getDecisionManager()->isRetracting());
    assertTrue(!planner.getDecisionManager()->hasDecisionToRetract());
    //it was the only object we could constrain to and we failed, so
    //there's no more to do.

    CBP_DEFAULT_TEARDOWN_PLAN();
    return true;
  }
  static bool testObjectHorizon() {
    CBP_DEFAULT_SETUP_PLAN(ce, db, false);

    Object o1(db.getId(), LabelStr("Object"), LabelStr("Object1"));
    Object o2(db.getId(), LabelStr("Object"), LabelStr("Object2"));
    db.close();

    IntervalToken tokenB(db.getId(), "Object.PredicateB", false);

    hor.setHorizon(10,100);

    assertTrue(ce.propagate());
    assertTrue(planner.getDecisionManager()->getNumberOfDecisions() == 0); 

    tokenB.activate();

    assertTrue(ce.propagate());
    assertTrue(planner.getDecisionManager()->getNumberOfDecisions() == 0); 

    tokenB.getStart()->specify(50);

    assertTrue(ce.propagate());
    assertTrue(planner.getDecisionManager()->isVariableDecision(tokenB.getObject()));
    assertTrue(planner.getDecisionManager()->getNumberOfDecisions() == 1); 


    // Bind the object variable which will remove both flaws.
    tokenB.getObject()->specify(o2.getId());
    assertTrue(ce.propagate());
    assertTrue(!planner.getDecisionManager()->isVariableDecision(tokenB.getObject()));
    assertTrue(planner.getDecisionManager()->getNumberOfDecisions() == 0);

    CBP_DEFAULT_TEARDOWN_PLAN();
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
    CBP_DEFAULT_SETUP_PLAN(ce, db, false);
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

    Object o1(db.getId(), LabelStr("Object"), LabelStr("o1"));
    Timeline t1(db.getId(), LabelStr("Object"), LabelStr("t1"));
    db.close();

    IntervalToken tokenA(db.getId(), 
			 "Object.PredicateD",
			 true,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000));

    IntervalToken tokenB(db.getId(), 
			 "Object.PredicateB",
			 false,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000));

    IntervalToken tokenC(db.getId(), 
			 "Object.PredicateC",
			 true,
			 IntervalIntDomain(0, 10),
			 IntervalIntDomain(0, 200),
			 IntervalIntDomain(1, 1000),
			 LabelStr("o1"));

    hor.setHorizon(0,200);

    CBPlanner::Status result = planner.run();
    assertTrue(result == CBPlanner::PLAN_FOUND);

    assertTrue(planner.getTime() == planner.getDepth());

    DecisionManagerId ldm = planner.getDecisionManager();
    unsigned int count;
    ldm->retractDecision(count);
    while(ldm->hasDecisionToRetract() && ldm->isRetracting())
      ldm->retractDecision(count);

    result = planner.run();

    assertTrue(result == CBPlanner::PLAN_FOUND);

    assertTrue(planner.getTime() == planner.getDepth());

    CBP_DEFAULT_TEARDOWN_PLAN();
    return true;
  }
  static bool testAddSubgoalAfterPlanning() {
    CBP_DEFAULT_SETUP_PLAN(ce, db, false);
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

    Timeline t(db.getId(), LabelStr("Object"), LabelStr("t1"));
    db.close();

    //    int numDecs = 4;
    //    int i=0;
    for (;;) { /* Forever: only way out is to return */
      CBPlanner::Status result = planner.step();
      if (result != CBPlanner::IN_PROGRESS) {
	assertTrue(result == CBPlanner::PLAN_FOUND);
	break;
      }
    }
    assertTrue(planner.getDepth() ==  planner.getTime());

    Variable<BoolDomain> v6(ce.getId(), BoolDomain());
    IntervalToken tokenA(db.getId(), 
			 "Object.PADDED",
			 true); 

    tokenA.getStart()->restrictBaseDomain(IntervalIntDomain(0, 10));
    tokenA.getEnd()->restrictBaseDomain(IntervalIntDomain(0, 200));

    CBPlanner::Status res = planner.run();
    assertTrue(res == CBPlanner::PLAN_FOUND);
    CBP_DEFAULT_TEARDOWN_PLAN();
    return true;
  }
};



void CBPlannerModuleTests::runTests() {
    LockManager::instance().connect();
    LockManager::instance().lock();

    Schema::instance();
    REGISTER_CONSTRAINT(EqualConstraint, "eq", "Default");
    REGISTER_CONSTRAINT(EqualConstraint, "Equal", "Default");
    REGISTER_CONSTRAINT(LessThanConstraint, "lt", "Default");
    REGISTER_CONSTRAINT(SubsetOfConstraint, "SubsetOf", "Default");
    REGISTER_CONSTRAINT(NotFalseConstraint, "notfalse", "Default");
    REGISTER_CONSTRAINT(BinaryCustomConstraint, "custom", "Default");

    // Constraints used for testing
    REGISTER_CONSTRAINT(LazyAllDiff, "lazyAllDiff",  "Default");
    REGISTER_CONSTRAINT(LazyAlwaysFails, "lazyAlwaysFails",  "Default");
    REGISTER_CONSTRAINT(FailsTillLastOne, "failsTillLastOne",  "Default");

    LockManager::instance().unlock();

    for (int i = 0; i < 1; i++) {
      LockManager::instance().lock();
      runTestSuite(CBPDefaultSetupTest::test);
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
    uninitConstraintLibrary();
  }

