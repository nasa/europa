#include "tn-test-module.hh"
#include "TestSupport.hh"
#include "TemporalNetwork.hh"
#include "TemporalPropagator.hh"
#include "STNTemporalAdvisor.hh"
#include "TemporalAdvisor.hh"
#include "Schema.hh"
#include "PlanDatabase.hh"
#include "PlanDatabaseDefs.hh"
#include "ObjectTokenRelation.hh"
#include "IntervalToken.hh"
#include "Timeline.hh"
#include "Utils.hh"
#include "IntervalIntDomain.hh"
#include "TokenVariable.hh"

#include "RulesEngine.hh"
#include "TestSubgoalRule.hh"

#include "Engine.hh"
#include "ModuleConstraintEngine.hh"
#include "ModulePlanDatabase.hh"
#include "ModuleRulesEngine.hh"
#include "ModuleTemporalNetwork.hh"

#include <iostream>
#include <string>
#include <list>

class TNTestEngine : public EngineBase 
{
  public:  
    TNTestEngine();
    virtual ~TNTestEngine();
          
  protected: 
    void createModules();       
};

TNTestEngine::TNTestEngine()
{
    createModules();
    doStart();
    Schema* schema = (Schema*)getComponent("Schema");  
    schema->addObjectType("Objects"); 
    schema->addPredicate("Objects.Predicate"); 
    schema->addPredicate("Objects.PredicateA"); 
    schema->addMember("Objects.PredicateA", IntervalIntDomain().getTypeName(), "IntervalParam"); 
    schema->addPredicate("Objects.PredicateB"); 
    schema->addMember("Objects.PredicateB", IntervalIntDomain().getTypeName(), "IntervalParam"); 
}

TNTestEngine::~TNTestEngine()
{
    doShutdown();
}

void TNTestEngine::createModules()
{
    addModule((new ModuleConstraintEngine())->getId());
    addModule((new ModuleConstraintLibrary())->getId());
    // TODO: TN is just an extension to CE, shouldn't have other dependencies
    addModule((new ModulePlanDatabase())->getId());
    // This is needed for the tests that use TestSubgoalRule
    // TODO: should probably remove this dependency
    addModule((new ModuleRulesEngine())->getId());
    addModule((new ModuleTemporalNetwork())->getId());
}

#define DEFAULT_SETUP_CE_ONLY(ce) \
    TNTestEngine tnte; \
    ConstraintEngine& ce = *((ConstraintEngine*)tnte.getComponent("ConstraintEngine")); ; \

#define DEFAULT_TEARDOWN_CE_ONLY()

#define CD_DEFAULT_SETUP(ce, db,  autoClose) \
    TNTestEngine tnte; \
    ConstraintEngine& ce = *((ConstraintEngine*)tnte.getComponent("ConstraintEngine")); ; \
    PlanDatabase& db = *((PlanDatabase*)tnte.getComponent("PlanDatabase")); \
    if (autoClose) \
      db.close();

#define TN_DEFAULT_TEARDOWN()


#define DEFAULT_SETUP_RULES(ce, db,  autoClose) \
    TNTestEngine tnte; \
    ConstraintEngine& ce = *((ConstraintEngine*)tnte.getComponent("ConstraintEngine")); ; \
    RulesEngine& re = *((RulesEngine*)tnte.getComponent("RulesEngine")); \
    PlanDatabase& db = *((PlanDatabase*)tnte.getComponent("PlanDatabase")); \
    if (autoClose) \
      db.close();

#define DEFAULT_TEARDOWN_RULES()

class TemporalNetworkTest {
public:
  static bool test(){
    runTest(testBasicAllocation);
    runTest(testTemporalConstraints);
    runTest(testFixForReversingEndpoints);
    runTest(testMemoryCleanups);
    return true;
  }

private:
  static bool testBasicAllocation(){
    TemporalNetwork tn;
    TimepointId origin = tn.getOrigin();
    Time delta = g_noTime();
    Time epsilon = g_noTime();
    tn.getTimepointBounds(origin, delta, epsilon);
    assertTrue(delta == 0 && epsilon == 0);

    tn.calcDistanceBounds(origin, origin, delta, epsilon);
    assertTrue(delta == 0 && epsilon == 0);
    return true;
  }

  static bool testTemporalConstraints(){
    TemporalNetwork tn;
    TimepointId a_end = tn.addTimepoint();
    TimepointId b_start = tn.addTimepoint();
    TimepointId b_end = tn.addTimepoint();
    TimepointId c_start = tn.addTimepoint();
    TemporalConstraintId a_before_b = tn.addTemporalConstraint(a_end, b_start, 0, g_infiniteTime());
    TemporalConstraintId start_before_end = tn.addTemporalConstraint(b_start, b_end, 1, g_infiniteTime());
    TemporalConstraintId a_meets_c = tn.addTemporalConstraint(a_end, c_start, 0, 0);
    bool res = tn.propagate();
    assertTrue(res);

    Time dist_lb, dist_ub;
    tn.calcDistanceBounds(c_start, b_end, dist_lb, dist_ub);
    assertTrue(dist_lb > 0);

    // Force failure where b meets c
    TemporalConstraintId b_meets_c = tn.addTemporalConstraint(b_end, c_start, 0, 0);
    res = tn.propagate();
    assertTrue(!res);

    // Cleanup
    tn.removeTemporalConstraint(b_meets_c);
    tn.removeTemporalConstraint(a_meets_c);
    tn.removeTemporalConstraint(start_before_end);
    tn.removeTemporalConstraint(a_before_b);
    tn.deleteTimepoint(c_start);
    tn.deleteTimepoint(b_end);
    tn.deleteTimepoint(b_start);
    tn.deleteTimepoint(a_end);
    return true;
  }

  static bool testFixForReversingEndpoints(){
    TemporalNetwork tn;

    // Allocate timepoints
    TimepointId x = tn.getOrigin();
    TimepointId y = tn.addTimepoint();
    TimepointId z = tn.addTimepoint();

    TemporalConstraintId fromage = tn.addTemporalConstraint(x, y, (Time)0, g_infiniteTime());
    TemporalConstraintId tango = tn.addTemporalConstraint(y, x, 200, 200);

    bool res = tn.propagate();
    assertTrue(!res);

    tn.removeTemporalConstraint(fromage);
    tn.removeTemporalConstraint(tango);

    res = tn.propagate();
    assertTrue(res); // Consistency restored

    TemporalConstraintId c0 = tn.addTemporalConstraint(y, x, -200, g_infiniteTime());
    TemporalConstraintId c1 = tn.addTemporalConstraint(x, z, 0, g_infiniteTime());
    TemporalConstraintId c2 = tn.addTemporalConstraint(z, y, (Time)0, g_infiniteTime());
    TemporalConstraintId c3 = tn.addTemporalConstraint(x, y, 200, g_infiniteTime());

    res = tn.propagate();
    assertTrue(res);

    // Clean up
    tn.removeTemporalConstraint(c0);
    tn.removeTemporalConstraint(c1);
    tn.removeTemporalConstraint(c2);
    tn.removeTemporalConstraint(c3);
    tn.deleteTimepoint(y);
    tn.deleteTimepoint(z);
    return true;
  }

  static bool testMemoryCleanups(){
    for(int i=0;i<10;i++){
      TemporalNetwork tn;
      TimepointId origin = tn.getOrigin();

      for(int j=0;j<100;j++){
	TimepointId x = tn.addTimepoint();
	TimepointId y = tn.addTimepoint();
	tn.addTemporalConstraint(origin, x, (Time)j, j+1);
	tn.addTemporalConstraint(x, y, (Time)j, j+1);
	Time delta = g_noTime();
	Time epsilon = g_noTime();
	tn.calcDistanceBounds(x, y, delta, epsilon);
      }
    }
    return true;
  }
};

class TemporalPropagatorTest {
public:
  static bool test(){
    runTest(testBasicAllocation);
    runTest(testTemporalPropagation);
    runTest(testCanPrecede);
    runTest(testCanFitBetween);
    runTest(testCanBeConcurrent);
    runTest(testTemporalDistance);
    runTest(testTokenStateChangeSynchronization);
    runTest(testInconsistencySynchronization);
    return true;
  }

private:

  static bool testBasicAllocation() {
    CD_DEFAULT_SETUP(ce,db,true);
    ce.propagate();
    TN_DEFAULT_TEARDOWN();
    return true;
  }
  
  static bool testTemporalPropagation() {
    CD_DEFAULT_SETUP(ce,db,false);

    ObjectId timeline = (new Timeline(db.getId(), "Objects", "o2"))->getId();
    assertTrue(!timeline.isNoId());

    db.close();

    IntervalToken t1(db.getId(),
    		     "Objects.Predicate", 
    		     true,
    		     false,
    		     IntervalIntDomain(0, 10),
    		     IntervalIntDomain(0, 20),
    		     IntervalIntDomain(1, 1000));

    t1.duration()->restrictBaseDomain(IntervalIntDomain(5, 7));
    assertTrue(t1.end()->getDerivedDomain().getLowerBound() == 5);
    assertTrue(t1.end()->getDerivedDomain().getUpperBound() == 17);

    IntervalToken t2(db.getId(), 
    		     "Objects.Predicate", 
    		     true,
    		     false,
    		     IntervalIntDomain(0, 10),
    		     IntervalIntDomain(0, 20),
    		     IntervalIntDomain(1, 1000));

    //t2.end()->restrictBaseDomain(IntervalIntDomain(8, 10));

    std::vector<ConstrainedVariableId> temp;
    temp.push_back(t1.end());
    temp.push_back(t2.start());
    

    ConstraintId beforeConstraint = db.getConstraintEngine()->createConstraint(LabelStr("precedes"),                                                                        
                                                                        temp);
    assertTrue(!beforeConstraint.isNoId());

    assertTrue(t1.start()->getDerivedDomain().getLowerBound() == 0);
    assertTrue(t1.start()->getDerivedDomain().getUpperBound() == 5);
    assertTrue(t1.end()->getDerivedDomain().getLowerBound() == 5);
    assertTrue(t1.end()->getDerivedDomain().getUpperBound() == 10);
    assertTrue(t2.start()->getDerivedDomain().getLowerBound() == 5);
    assertTrue(t2.start()->getDerivedDomain().getUpperBound() == 10);
    assertTrue(t2.end()->getDerivedDomain().getLowerBound() == 6);
    assertTrue(t2.end()->getDerivedDomain().getUpperBound() == 20);

    delete (Constraint*) beforeConstraint;
    TN_DEFAULT_TEARDOWN();
    return true;
  }

  static bool testCanPrecede() {
    CD_DEFAULT_SETUP(ce,db,false);

    ObjectId timeline = (new Timeline(db.getId(), "Objects", LabelStr("o2")))->getId();
    assertTrue(!timeline.isNoId());

    db.close();
    
    IntervalToken first(db.getId(),
			"Objects.Predicate", 
			true,
			false,
			IntervalIntDomain(0, 100),
			IntervalIntDomain(0, 100),
			IntervalIntDomain(1, 1000));
    
    IntervalToken second(db.getId(),
			 "Objects.Predicate", 
			 true,
			 false,
			 IntervalIntDomain(0, 100),
			 IntervalIntDomain(0, 100),
			 IntervalIntDomain(1, 1000));

    ce.propagate();

    const TemporalPropagatorId& tp = (TemporalPropagatorId)ce.getPropagatorByName(LabelStr("Temporal"));

    // assert from propagator directly
    assertTrue (tp->canPrecede(first.end(), second.start()));
    assertTrue (tp->canPrecede(second.end(), first.start()));

    // compute from advisor
    assertTrue (db.getTemporalAdvisor()->canPrecede(first.getId(),second.getId()));
    
    second.start()->reset();
    second.end()->reset();

    first.start()->reset();
    first.end()->reset();

    // restrict via a constraint

    std::vector<ConstrainedVariableId> temp;
    temp.push_back(first.end());
    temp.push_back(second.start());

    ConstraintId beforeConstraint = db.getConstraintEngine()->createConstraint(LabelStr("precedes"),
									temp);
    assertTrue(beforeConstraint.isValid());

    bool res = ce.propagate();
    assertTrue(res);
    
    // compute from propagator directly
    res = tp->canPrecede(first.end(), second.start());
    assertTrue (res);
    assertTrue (!tp->canPrecede(second.end(), first.start()));

    // compute from advisor
    assertTrue (db.getTemporalAdvisor()->canPrecede(first.getId(),second.getId()));
    assertTrue (!db.getTemporalAdvisor()->canPrecede(second.getId(), first.getId()));

    delete (Constraint*) beforeConstraint;

    // restrict via specifying the domain

    IntervalIntDomain dom(21, 31);
    first.start()->restrictBaseDomain(dom);
    first.end()->restrictBaseDomain(dom);

    IntervalIntDomain dom2(1, 20);
    second.start()->restrictBaseDomain(dom2);
    second.end()->restrictBaseDomain(dom2);

    res = ce.propagate();
    assertTrue(res);
    
    // compute from propagator directly
    assertTrue (!tp->canPrecede(first.end(), second.start()));
    assertTrue (tp->canPrecede(second.end(), first.start()));
    // compute from advisor
    assertTrue (!db.getTemporalAdvisor()->canPrecede(first.getId(),second.getId()));

    TN_DEFAULT_TEARDOWN();
    return true;
  }

  static bool testCanFitBetween() {
    CD_DEFAULT_SETUP(ce,db,false);

    ObjectId timeline = (new Timeline(db.getId(), "Objects", LabelStr("o2")))->getId();
    assertTrue(!timeline.isNoId());

    db.close();

    IntervalToken token(db.getId(),
			"Objects.Predicate", 
			true,
			false,
			IntervalIntDomain(0, 10),
			IntervalIntDomain(0, 20),
			IntervalIntDomain(1, 1000));
    IntervalToken predecessor(db.getId(),
			      "Objects.Predicate", 
			      true,
			      false,
			      IntervalIntDomain(0, 10),
			      IntervalIntDomain(0, 20),
			      IntervalIntDomain(1, 1000));
    IntervalToken successor(db.getId(),
			    "Objects.Predicate", 
			    true,
			    false,
			    IntervalIntDomain(0, 10),
			    IntervalIntDomain(0, 20),
			    IntervalIntDomain(1, 1000));
    ce.propagate();

    // compute from propagator directly
    assertTrue (((TemporalPropagatorId)ce.getPropagatorByName(LabelStr("Temporal")))->canFitBetween(token.start(), token.end(), predecessor.end(), successor.start()));

    // compute from advisor
    assertTrue (db.getTemporalAdvisor()->canFitBetween(token.getId(), predecessor.getId(), successor.getId()));

    TN_DEFAULT_TEARDOWN();
    return true;
  }

  static bool testCanBeConcurrent() {
    CD_DEFAULT_SETUP(ce,db,false);

    ObjectId timeline = (new Timeline(db.getId(), "Objects", LabelStr("o2")))->getId();
    assertTrue(!timeline.isNoId());

    db.close();

    IntervalToken t0(db.getId(),
		     "Objects.Predicate", 
		     true,
		     false,
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000));

    IntervalToken t1(db.getId(),
		     "Objects.Predicate", 
		     true,
		     false,
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000));

    IntervalToken t2(db.getId(),
		     "Objects.Predicate", 
		     true,
		     false,
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000));

    ce.propagate();

    // Check that they can coincide, trivially.
    assertTrue(db.getTemporalAdvisor()->canBeConcurrent(t0.getId(), t1.getId()));

    // May 1 very tight, but still ok
    t0.start()->specify(1);
    t0.end()->specify(2);
    assertTrue(ce.propagate());
    assertTrue(db.getTemporalAdvisor()->canBeConcurrent(t0.getId(), t1.getId()));

    // Make it too tight.
    t1.end()->specify(10);
    ce.propagate();
    assertTrue(!db.getTemporalAdvisor()->canBeConcurrent(t0.getId(), t1.getId()));

    // Reset, but impose constraints
    t0.start()->reset();
    t0.end()->reset();
    t1.end()->reset();


    ConstraintId c0 = ce.getId()->createConstraint(LabelStr("precedes"),
							  makeScope(t0.end(), t1.start()));


    ConstraintId c1 = ce.getId()->createConstraint(LabelStr("precedes"),
							  makeScope(t1.end(), t2.start()));

    assertTrue(ce.propagate());

    assertTrue(!db.getTemporalAdvisor()->canBeConcurrent(t0.getId(), t1.getId()));
    assertTrue(!db.getTemporalAdvisor()->canBeConcurrent(t1.getId(), t2.getId()));
    assertTrue(!db.getTemporalAdvisor()->canBeConcurrent(t0.getId(), t2.getId()));

    delete (Constraint*) c0;
    delete (Constraint*) c1;

    TN_DEFAULT_TEARDOWN();
    return true;
  }

  static bool testTemporalDistance() {
    TNTestEngine tnte; 
    ConstraintEngine& ce = *((ConstraintEngine*)tnte.getComponent("ConstraintEngine")); ; 
    PlanDatabase& db = *((PlanDatabase*)tnte.getComponent("PlanDatabase")); 

    IntervalIntDomain d1 = IntervalIntDomain(-10, 10);
    IntervalIntDomain d2 = IntervalIntDomain( 20, 30);


    ConstrainedVariableId v1 = (new Variable<IntervalIntDomain> (ce.getId(), d1, false, true, "v1"))->getId();
    ConstrainedVariableId v2 = (new Variable<IntervalIntDomain> (ce.getId(), d2, false, true, "v2"))->getId();

    // <-10>----------<10>
    //                          <20>----------<30>
    {
      const IntervalIntDomain distance = db.getTemporalAdvisor()->getTemporalDistanceDomain( v1, v2, true );      

      assertTrue( 10 == distance.getLowerBound() );
      assertTrue( 40 == distance.getUpperBound() );
    }
    
    {
      const IntervalIntDomain distance = db.getTemporalAdvisor()->getTemporalDistanceDomain( v2, v1, true );
      
      assertTrue( -40 == distance.getLowerBound() );
      assertTrue( -10 == distance.getUpperBound() );
    }
    
    IntervalIntDomain d3 = IntervalIntDomain( 5, 15 );
    ConstrainedVariableId v3 = (new Variable<IntervalIntDomain> (ce.getId(), d3, false, true, "v3"))->getId();

    // <-10>----------<10>
    //        <5>----------<15>
    {
      const IntervalIntDomain distance = db.getTemporalAdvisor()->getTemporalDistanceDomain( v1, v3, true );
      
      assertTrue( -5 == distance.getLowerBound() );
      assertTrue( 25 == distance.getUpperBound() );
    }
    
    {
      const IntervalIntDomain distance = db.getTemporalAdvisor()->getTemporalDistanceDomain( v3, v1, true );
      
      assertTrue( -25 == distance.getLowerBound() );
      assertTrue(   5 == distance.getUpperBound() );
    }


    IntervalIntDomain d4 = IntervalIntDomain( 0, 5 );
    ConstrainedVariableId v4 = (new Variable<IntervalIntDomain> (ce.getId(), d4, false, true, "v4"))->getId();

    // <-10>--------------<10>
    //           <0>-<5>
    {
      const IntervalIntDomain distance = db.getTemporalAdvisor()->getTemporalDistanceDomain( v1, v4, true );
      
      assertTrue( -10 == distance.getLowerBound() );
      assertTrue(  15 == distance.getUpperBound() );
    }
    
    {
      const IntervalIntDomain distance = db.getTemporalAdvisor()->getTemporalDistanceDomain( v4, v1, true );
      
      assertTrue( -15 == distance.getLowerBound() );
      assertTrue(  10 == distance.getUpperBound() );
    }

    IntervalIntDomain d5 = IntervalIntDomain( -20, 20 );
    ConstrainedVariableId v5 = (new Variable<IntervalIntDomain> (ce.getId(), d5, false, true, "v5"))->getId();

    //           <-10>--------------<10>
    //  <-20>---------------------------------<20>
    {
      const IntervalIntDomain distance = db.getTemporalAdvisor()->getTemporalDistanceDomain( v1, v5, true );
      
      assertTrue( -30 == distance.getLowerBound() );
      assertTrue(  30 == distance.getUpperBound() );
    }

    {
      const IntervalIntDomain distance = db.getTemporalAdvisor()->getTemporalDistanceDomain( v5, v1, true );
      
      assertTrue( -30 == distance.getLowerBound() );
      assertTrue(  30 == distance.getUpperBound() );
    }

    IntervalIntDomain d6 = IntervalIntDomain( -g_infiniteTime(), g_infiniteTime() );
    ConstrainedVariableId v6 = (new Variable<IntervalIntDomain> (ce.getId(), d6, false, true, "v6"))->getId();

    //            <-10>------------<10>
    //  <-inf>--------------------------------<inf>

    ConstrainedVariableId v7 = (new Variable<IntervalIntDomain> (ce.getId(), d6, false, true, "v7"))->getId();

    {
      const IntervalIntDomain distance = db.getTemporalAdvisor()->getTemporalDistanceDomain( v1, v7, true );
      
      assertTrue( -g_infiniteTime() == distance.getLowerBound() );
      assertTrue(  g_infiniteTime() == distance.getUpperBound() );
    }

    {
      const IntervalIntDomain distance = db.getTemporalAdvisor()->getTemporalDistanceDomain( v7, v1, true );
      
      assertTrue( -g_infiniteTime() == distance.getLowerBound() );
      assertTrue(  g_infiniteTime() == distance.getUpperBound() );
    }

    //  <-inf>--------------------------------<inf>
    //  <-inf>--------------------------------<inf>

    {
      const IntervalIntDomain distance = db.getTemporalAdvisor()->getTemporalDistanceDomain( v6, v7, true );
      
      assertTrue( -g_infiniteTime() == distance.getLowerBound() );
      assertTrue(  g_infiniteTime() == distance.getUpperBound() );
    }

    delete (ConstrainedVariable*) v1;
    delete (ConstrainedVariable*) v2;
    delete (ConstrainedVariable*) v3;
    delete (ConstrainedVariable*) v4;
    delete (ConstrainedVariable*) v5;
    delete (ConstrainedVariable*) v6;
    delete (ConstrainedVariable*) v7;

    return true;
  }

  static bool testTokenStateChangeSynchronization() {
    CD_DEFAULT_SETUP(ce,db,false);

    ObjectId timeline = (new Timeline(db.getId(), "Objects", LabelStr("o2")))->getId();
    assertTrue(!timeline.isNoId());

    db.close();

    // Allocate a token
    IntervalToken t1(db.getId(),
    		     "Objects.Predicate", 
    		     true,
    		     false,
    		     IntervalIntDomain(0, 10),
    		     IntervalIntDomain(0, 20),
    		     IntervalIntDomain(1, 1000));

    // Activate immediately. We will merge against it.
    t1.activate();

    // Allocate another
    IntervalToken t2(db.getId(), 
    		     "Objects.Predicate", 
    		     true,
    		     false,
    		     IntervalIntDomain(0, 10),
    		     IntervalIntDomain(0, 20),
    		     IntervalIntDomain(1, 1000));

    // Allocate a constraint on the inactive token, to constrain a timepoint
    Variable<IntervalIntDomain> v0(ce.getId(), IntervalIntDomain());
    EqualConstraint c0(LabelStr("eq"), LabelStr("Default"), ce.getId() , makeScope(t2.end(), v0.getId()));

    // Conduct the merge.
    t2.doMerge(t1.getId());

    // Now changes on v0 should propagate to the end variable of t1.
    v0.restrictBaseDomain(IntervalIntDomain(8, 10));
    assertTrue(t1.end()->getDerivedDomain() == IntervalIntDomain(8, 10));

    // If we split again, expect that the restriction now applies to the end-point
    // of the inactive token
    t2.cancel();

    assertTrue(t2.end()->getDerivedDomain() == IntervalIntDomain(8, 10));

    TN_DEFAULT_TEARDOWN();
    return true;
  }

  /*
    1. Token is activated. It subgoals tokens and places temporal
    relations between master and slaves.
    2. Constraints are present which trigger inconsistency prior
    to execution of the Temporal propagator.
    3. Retract activation of Token (should delete slaves and related
    entities).
  */

  static bool testInconsistencySynchronization() {
    DEFAULT_SETUP_RULES(ce,db,false);

    ObjectId timeline = (new Timeline(db.getId(), "Objects", LabelStr("o2")))->getId();
    assertTrue(!timeline.isNoId());

    db.close();

    // create the rule
    TestSubgoalRule r("Objects.PredicateA");

    // Allocate a token
    IntervalToken t1(db.getId(),
    		     "Objects.PredicateA", 
    		     true,
    		     false,
    		     IntervalIntDomain(0, 10),
    		     IntervalIntDomain(0, 20),
    		     IntervalIntDomain(1, 5),
		     "o2", false);
    t1.addParameter(IntervalIntDomain(0,1),"IntervalParam");
    t1.close();

    assertTrue(ce.propagate());

    // Activate immediately to trigger the rule.
    t1.activate();
    assertTrue(t1.slaves().size() == 1);

    assertTrue (ce.propagate());

    TokenId slave = *t1.slaves().begin();

    assertTrue(t1.start()->derivedDomain().getLowerBound() == 0);
    assertTrue(t1.start()->derivedDomain().getUpperBound() == 10);
    assertTrue(t1.end()->derivedDomain().getLowerBound() == 1);
    assertTrue(t1.end()->derivedDomain().getUpperBound() == 15);
    assertTrue(slave->start()->derivedDomain().getLowerBound() == 1);
    assertTrue(slave->start()->derivedDomain().getUpperBound() == 15);
    assertTrue(slave->end()->derivedDomain().getLowerBound() == 2);
    assertTrue(slave->end()->derivedDomain().getUpperBound() == 100);

    std::vector<ConstrainedVariableId> scope;
    scope.push_back(slave->end());
    scope.push_back(t1.parameters()[0]);
    ce.getId()->createConstraint(LabelStr("leq"), scope);

    assertTrue (!ce.propagate());

    assertTrue(t1.start()->derivedDomain().getLowerBound() == 3);
    assertTrue(t1.start()->derivedDomain().getUpperBound() == -2);
    assertTrue(t1.end()->derivedDomain().getLowerBound() == 3);
    assertTrue(t1.end()->derivedDomain().getUpperBound() == -2);
    assertTrue(slave->start()->derivedDomain().getLowerBound() == 3);
    assertTrue(slave->start()->derivedDomain().getUpperBound() == -2);
    assertTrue(slave->end()->derivedDomain().getLowerBound() == 3);
    assertTrue(slave->end()->derivedDomain().getUpperBound() == -2);

    t1.cancel();

    assertTrue(ce.propagate());

    assertTrue(t1.start()->derivedDomain().getLowerBound() == 0);
    assertTrue(t1.start()->derivedDomain().getUpperBound() == 10);
    assertTrue(t1.end()->derivedDomain().getLowerBound() == 1);
    assertTrue(t1.end()->derivedDomain().getUpperBound() == 15);

    DEFAULT_TEARDOWN_RULES();
    return true;
  }

};


class TemporalNetworkConstraintEngineOnlyTest {
public:
  static bool test() {
    runTest(testBasicAllocation);
    runTest(testTemporalPropagation);
    runTest(testTemporalNogood);
    return true;
  }
private:

  static bool testBasicAllocation() {
    DEFAULT_SETUP_CE_ONLY(ce);
    ce.propagate();
    DEFAULT_TEARDOWN_CE_ONLY();
    return true;
  }
  

  /**  
   *  duplicates behavior of testTemporalPropagation in the TemporalPropagatorTest.
   */

  static bool testTemporalPropagation() {
    DEFAULT_SETUP_CE_ONLY(ce);

    IntervalIntDomain domStart = IntervalIntDomain(0,10);
    IntervalIntDomain domEnd = IntervalIntDomain(0,20);
    IntervalIntDomain domDur = IntervalIntDomain(1,1000);

    ConstrainedVariableId v1 = (new Variable<IntervalIntDomain> (ce.getId(), domStart, false, true, "v1"))->getId();
    ConstrainedVariableId v2 = (new Variable<IntervalIntDomain> (ce.getId(), domDur, false, true, "v2"))->getId();
    ConstrainedVariableId v3 = (new Variable<IntervalIntDomain> (ce.getId(), domEnd, false, true, "v3"))->getId();
    ConstrainedVariableId v4 = (new Variable<IntervalIntDomain> (ce.getId(), domStart, false, true, "v4"))->getId();
    ConstrainedVariableId v5 = (new Variable<IntervalIntDomain> (ce.getId(), domDur, false, true, "v5"))->getId();
    ConstrainedVariableId v6 = (new Variable<IntervalIntDomain> (ce.getId(), domEnd, false, true, "v6"))->getId();

    v2->restrictBaseDomain(IntervalIntDomain(5, 7));

    std::vector<ConstrainedVariableId> temp;
    temp.push_back(v1);
    temp.push_back(v2);
    temp.push_back(v3);
    ConstraintId duration1 = 
        ce.getId()->createConstraint(LabelStr("temporalDistance"), temp);

    assertTrue(!duration1.isNoId());

    temp.clear();
    temp.push_back(v4);
    temp.push_back(v5);
    temp.push_back(v6);
    ConstraintId duration2 = 
        ce.getId()->createConstraint(LabelStr("temporalDistance"), temp);

    assertTrue(!duration2.isNoId());

    temp.clear();
    temp.push_back(v3);
    temp.push_back(v4);
    ConstraintId beforeConstraint = 
        ce.getId()->createConstraint(LabelStr("precedes"), temp);

    assertTrue(!beforeConstraint.isNoId());

    assertTrue(v1->derivedDomain().getLowerBound() == 0);
    assertTrue(v1->derivedDomain().getUpperBound() == 5);
    assertTrue(v3->derivedDomain().getLowerBound() == 5);
    assertTrue(v3->derivedDomain().getUpperBound() == 10);
    assertTrue(v4->derivedDomain().getLowerBound() == 5);
    assertTrue(v4->derivedDomain().getUpperBound() == 10);
    assertTrue(v6->derivedDomain().getLowerBound() == 6);
    assertTrue(v6->derivedDomain().getUpperBound() == 20);
    
    delete (Constraint*) beforeConstraint;
    delete (Constraint*) duration1;
    delete (Constraint*) duration2;
    delete (ConstrainedVariable*) v1;
    delete (ConstrainedVariable*) v2;
    delete (ConstrainedVariable*) v3;
    delete (ConstrainedVariable*) v4;
    delete (ConstrainedVariable*) v5;
    delete (ConstrainedVariable*) v6;

    DEFAULT_TEARDOWN_CE_ONLY();
    return true;
  }

  static bool testTemporalNogood() {
    TNTestEngine tnte; 
    ConstraintEngine& ce = *((ConstraintEngine*)tnte.getComponent("ConstraintEngine"));  
    TemporalPropagator* tp = (TemporalPropagator*) 
        ((Propagator*)ce.getPropagatorByName(LabelStr("Temporal")));

    IntervalIntDomain domStart = IntervalIntDomain(1,10);
    IntervalIntDomain domEnd = IntervalIntDomain(0,1);
    IntervalIntDomain domDur = IntervalIntDomain(1,1);

    ConstrainedVariableId v1 = (new Variable<IntervalIntDomain> (ce.getId(), domStart, false, true, "v1"))->getId();
    ConstrainedVariableId v2 = (new Variable<IntervalIntDomain> (ce.getId(), domDur, false, true, "v2"))->getId();
    ConstrainedVariableId v3 = (new Variable<IntervalIntDomain> (ce.getId(), domEnd, false, true, "v3"))->getId();

    std::vector<ConstrainedVariableId> temp;
    temp.push_back(v1);
    temp.push_back(v2);
    temp.push_back(v3);
    ConstraintId constraint = 
        ce.getId()->createConstraint(LabelStr("temporalDistance"),
                                          temp);
    bool consistent = ce.propagate();
    std::vector<ConstrainedVariableId> fromvars;
    std::vector<ConstrainedVariableId> tovars;
    std::vector<int> lengths;//std::vector<long> lengths;
    ConstrainedVariableId origin;
    tp->getTemporalNogood(origin,fromvars,tovars,lengths);

    assertTrue(!consistent);

    assertTrue(fromvars.size()==3);
    assertTrue(tovars.size()==3);
    assertTrue(lengths.size()==3);

    assertTrue(fromvars.at(0)==origin);
    assertTrue(tovars.at(0)==v3);
    assertTrue(lengths.at(0)==1);

    assertTrue(fromvars.at(1)==v1);
    assertTrue(tovars.at(1)==origin);
    assertTrue(lengths.at(1)==-1);

    assertTrue(fromvars.at(2)==v3);
    assertTrue(tovars.at(2)==v1);
    assertTrue(lengths.at(2)==-1);

    delete (Constraint*) constraint;
    delete (ConstrainedVariable*) v1;
    delete (ConstrainedVariable*) v2;
    delete (ConstrainedVariable*) v3;
    return true;
  }
};


void TemporalNetworkModuleTests::runTests(std::string path) {
  setTestLoadLibraryPath(path);
  runTestSuite(TemporalNetworkTest::test);
  runTestSuite(TemporalNetworkConstraintEngineOnlyTest::test);
  runTestSuite(TemporalPropagatorTest::test);
  std::cout << "Finished" << std::endl;
}
