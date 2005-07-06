#include "TestSupport.hh"
#include "TemporalNetwork.hh"
#include "TemporalPropagator.hh"
#include "STNTemporalAdvisor.hh"
#include "TemporalAdvisor.hh"
#include "Schema.hh"
#include "PlanDatabase.hh"
#include "ObjectTokenRelation.hh"
#include "IntervalToken.hh"
#include "Timeline.hh"
#include "Utils.hh"
#include "IntervalIntDomain.hh"
#include "LockManager.hh"
#include "TokenVariable.hh"

#include "RulesEngine.hh"
#include "TestSubgoalRule.hh"

#include <iostream>
#include <string>
#include <list>

#define DEFAULT_SETUP_CE_ONLY(ce) \
  ConstraintEngine ce; \
  new DefaultPropagator(LabelStr("Default"), ce.getId()); \
  new TemporalPropagator(LabelStr("Temporal"), ce.getId());

#define DEFAULT_TEARDOWN_CE_ONLY()

#define DEFAULT_SETUP(ce, db,  autoClose) \
    ConstraintEngine ce; \
    Schema::instance()->reset();\
    Schema::instance()->addObjectType("Objects"); \
    Schema::instance()->addPredicate("Objects.Predicate"); \
    Schema::instance()->addPredicate("Objects.PredicateA"); \
    Schema::instance()->addMember("Objects.PredicateA", IntervalIntDomain().getTypeName(), "IntervalParam"); \
    Schema::instance()->addPredicate("Objects.PredicateB"); \
    Schema::instance()->addMember("Objects.PredicateB", IntervalIntDomain().getTypeName(), "IntervalParam"); \
    PlanDatabase db(ce.getId(), Schema::instance());\
    new DefaultPropagator(LabelStr("Default"), ce.getId()); \
    new TemporalPropagator(LabelStr("Temporal"), ce.getId()); \
    db.setTemporalAdvisor((new STNTemporalAdvisor(ce.getPropagatorByName(LabelStr("Temporal"))))->getId()); \
    if (autoClose) \
      db.close();

#define DEFAULT_TEARDOWN()


#define DEFAULT_SETUP_RULES(ce, db,  autoClose) \
    ConstraintEngine ce; \
    Schema::instance()->reset();\
    Schema::instance()->addObjectType("Objects"); \
    Schema::instance()->addPredicate("Objects.Predicate"); \
    Schema::instance()->addPredicate("Objects.PredicateA"); \
    Schema::instance()->addMember("Objects.PredicateA", IntervalIntDomain().getTypeName(), "IntervalParam"); \
    Schema::instance()->addPredicate("Objects.PredicateB"); \
    Schema::instance()->addMember("Objects.PredicateB", IntervalIntDomain().getTypeName(), "IntervalParam"); \
    PlanDatabase db(ce.getId(), Schema::instance());\
    new DefaultPropagator(LabelStr("Default"), ce.getId()); \
    new TemporalPropagator(LabelStr("Temporal"), ce.getId()); \
    db.setTemporalAdvisor((new STNTemporalAdvisor(ce.getPropagatorByName(LabelStr("Temporal"))))->getId()); \
    RulesEngine re(db.getId()); \
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
    bool res = tn.isConsistent();
    assertTrue(res);

    Time dist_lb, dist_ub;
    tn.calcDistanceBounds(c_start, b_end, dist_lb, dist_ub);
    assertTrue(dist_lb > 0);

    // Force failure where b meets c
    TemporalConstraintId b_meets_c = tn.addTemporalConstraint(b_end, c_start, 0, 0);
    res = tn.isConsistent();
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

    bool res = tn.isConsistent();
    assertTrue(!res);

    tn.removeTemporalConstraint(fromage);
    tn.removeTemporalConstraint(tango);

    res = tn.isConsistent();
    assertTrue(res); // Consistency restored

    TemporalConstraintId c0 = tn.addTemporalConstraint(y, x, -200, g_infiniteTime());
    TemporalConstraintId c1 = tn.addTemporalConstraint(x, z, 0, g_infiniteTime());
    TemporalConstraintId c2 = tn.addTemporalConstraint(z, y, (Time)0, g_infiniteTime());
    TemporalConstraintId c3 = tn.addTemporalConstraint(x, y, 200, g_infiniteTime());

    res = tn.isConsistent();
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
    runTest(testTokenStateChangeSynchronization);
    runTest(testInconsistencySynchronization);
    return true;
  }

private:

  static bool testBasicAllocation() {
    DEFAULT_SETUP(ce,db,true);
    ce.propagate();
    DEFAULT_TEARDOWN();
    return true;
  }
  
  static bool testTemporalPropagation() {
    DEFAULT_SETUP(ce,db,false);

    ObjectId timeline = (new Timeline(db.getId(), "Objects", "o2"))->getId();
    assertTrue(!timeline.isNoId());

    db.close();

    IntervalToken t1(db.getId(),
    		     "Objects.Predicate", 
    		     true,
    		     IntervalIntDomain(0, 10),
    		     IntervalIntDomain(0, 20),
    		     IntervalIntDomain(1, 1000));

    t1.getDuration()->specify(IntervalIntDomain(5, 7));
    assertTrue(t1.getEnd()->getDerivedDomain().getLowerBound() == 5);
    assertTrue(t1.getEnd()->getDerivedDomain().getUpperBound() == 17);

    IntervalToken t2(db.getId(), 
    		     "Objects.Predicate", 
    		     true,
    		     IntervalIntDomain(0, 10),
    		     IntervalIntDomain(0, 20),
    		     IntervalIntDomain(1, 1000));

    //t2.getEnd()->specify(IntervalIntDomain(8, 10));

    std::vector<ConstrainedVariableId> temp;
    temp.push_back(t1.getEnd());
    temp.push_back(t2.getStart());
    

    ConstraintId beforeConstraint = ConstraintLibrary::createConstraint(LabelStr("precedes"),
                                                                        db.getConstraintEngine(),
                                                                        temp);
    assertTrue(!beforeConstraint.isNoId());

    assertTrue(t1.getStart()->getDerivedDomain().getLowerBound() == 0);
    assertTrue(t1.getStart()->getDerivedDomain().getUpperBound() == 5);
    assertTrue(t1.getEnd()->getDerivedDomain().getLowerBound() == 5);
    assertTrue(t1.getEnd()->getDerivedDomain().getUpperBound() == 10);
    assertTrue(t2.getStart()->getDerivedDomain().getLowerBound() == 5);
    assertTrue(t2.getStart()->getDerivedDomain().getUpperBound() == 10);
    assertTrue(t2.getEnd()->getDerivedDomain().getLowerBound() == 6);
    assertTrue(t2.getEnd()->getDerivedDomain().getUpperBound() == 20);

    delete (Constraint*) beforeConstraint;
    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testCanPrecede() {
    DEFAULT_SETUP(ce,db,false);

    ObjectId timeline = (new Timeline(db.getId(), "Objects", LabelStr("o2")))->getId();
    assertTrue(!timeline.isNoId());

    db.close();
    
    IntervalToken first(db.getId(),
			"Objects.Predicate", 
			true,
			IntervalIntDomain(0, 100),
			IntervalIntDomain(0, 100),
			IntervalIntDomain(1, 1000));
    
    IntervalToken second(db.getId(),
			 "Objects.Predicate", 
			 true,
			 IntervalIntDomain(0, 100),
			 IntervalIntDomain(0, 100),
			 IntervalIntDomain(1, 1000));

    ce.propagate();

    const TemporalPropagatorId& tp = (TemporalPropagatorId)ce.getPropagatorByName(LabelStr("Temporal"));

    // assert from propagator direcly
    assertTrue (tp->canPrecede(first.getEnd(), second.getStart()));
    assertTrue (tp->canPrecede(second.getEnd(), first.getStart()));

    // compute from advisor
    assertTrue (db.getTemporalAdvisor()->canPrecede(first.getId(),second.getId()));

    // restrict via specifying the domain

    IntervalIntDomain dom(21, 31);
    first.getStart()->specify(dom);
    first.getEnd()->specify(dom);

    IntervalIntDomain dom2(1, 20);
    second.getStart()->specify(dom2);
    second.getEnd()->specify(dom2);

    bool res = ce.propagate();
    assertTrue(res);
    
    // compute from propagator directly
    assertTrue (!tp->canPrecede(first.getEnd(), second.getStart()));
    assertTrue (tp->canPrecede(second.getEnd(), first.getStart()));
    // compute from advisor
    assertTrue (!db.getTemporalAdvisor()->canPrecede(first.getId(),second.getId()));
    
    second.getStart()->reset();
    second.getEnd()->reset();

    first.getStart()->reset();
    first.getEnd()->reset();

    // restrict via a constraint

    std::vector<ConstrainedVariableId> temp;
    temp.push_back(first.getEnd());
    temp.push_back(second.getStart());

    ConstraintId beforeConstraint = ConstraintLibrary::createConstraint(LabelStr("precedes"),
									db.getConstraintEngine(),
									temp);
    assertTrue(beforeConstraint.isValid());

    res = ce.propagate();
    assertTrue(res);
    
    // compute from propagator directly
    res = tp->canPrecede(first.getEnd(), second.getStart());
    assertTrue (res);
    assertTrue (!tp->canPrecede(second.getEnd(), first.getStart()));

    // compute from advisor
    assertTrue (db.getTemporalAdvisor()->canPrecede(first.getId(),second.getId()));
    assertTrue (!db.getTemporalAdvisor()->canPrecede(second.getId(), first.getId()));

    delete (Constraint*) beforeConstraint;
    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testCanFitBetween() {
    DEFAULT_SETUP(ce,db,false);

    ObjectId timeline = (new Timeline(db.getId(), "Objects", LabelStr("o2")))->getId();
    assertTrue(!timeline.isNoId());

    db.close();

    IntervalToken token(db.getId(),
			"Objects.Predicate", 
			true,
			IntervalIntDomain(0, 10),
			IntervalIntDomain(0, 20),
			IntervalIntDomain(1, 1000));
    IntervalToken predecessor(db.getId(),
			      "Objects.Predicate", 
			      true,
			      IntervalIntDomain(0, 10),
			      IntervalIntDomain(0, 20),
			      IntervalIntDomain(1, 1000));
    IntervalToken successor(db.getId(),
			    "Objects.Predicate", 
			    true,
			    IntervalIntDomain(0, 10),
			    IntervalIntDomain(0, 20),
			    IntervalIntDomain(1, 1000));
    ce.propagate();

    // compute from propagator directly
    assertTrue (((TemporalPropagatorId)ce.getPropagatorByName(LabelStr("Temporal")))->canFitBetween(token.getStart(), token.getEnd(), predecessor.getEnd(), successor.getStart()));

    // compute from advisor
    assertTrue (db.getTemporalAdvisor()->canFitBetween(token.getId(), predecessor.getId(), successor.getId()));

    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testCanBeConcurrent() {
    DEFAULT_SETUP(ce,db,false);

    ObjectId timeline = (new Timeline(db.getId(), "Objects", LabelStr("o2")))->getId();
    assertTrue(!timeline.isNoId());

    db.close();

    IntervalToken t0(db.getId(),
		     "Objects.Predicate", 
		     true,
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000));

    IntervalToken t1(db.getId(),
		     "Objects.Predicate", 
		     true,
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000));

    IntervalToken t2(db.getId(),
		     "Objects.Predicate", 
		     true,
		     IntervalIntDomain(0, 10),
		     IntervalIntDomain(0, 20),
		     IntervalIntDomain(1, 1000));

    ce.propagate();

    // Check that they can co-incide, trivially.
    assertTrue(db.getTemporalAdvisor()->canBeConcurrent(t0.getId(), t1.getId()));

    // May 1 very tight, but still ok
    t0.getStart()->specify(1);
    t0.getEnd()->specify(2);
    assertTrue(ce.propagate());
    assertTrue(db.getTemporalAdvisor()->canBeConcurrent(t0.getId(), t1.getId()));

    // Make it too tight.
    t1.getEnd()->specify(10);
    ce.propagate();
    assertTrue(!db.getTemporalAdvisor()->canBeConcurrent(t0.getId(), t1.getId()));

    // Reset, but impose constraints
    t0.getStart()->reset();
    t0.getEnd()->reset();
    t1.getEnd()->reset();


    ConstraintId c0 = ConstraintLibrary::createConstraint(LabelStr("precedes"),
							  ce.getId(), 
							  makeScope(t0.getEnd(), t1.getStart()));


    ConstraintId c1 = ConstraintLibrary::createConstraint(LabelStr("precedes"),
							  ce.getId(),
							  makeScope(t1.getEnd(), t2.getStart()));

    assertTrue(ce.propagate());

    assertTrue(!db.getTemporalAdvisor()->canBeConcurrent(t0.getId(), t1.getId()));
    assertTrue(!db.getTemporalAdvisor()->canBeConcurrent(t1.getId(), t2.getId()));
    assertTrue(!db.getTemporalAdvisor()->canBeConcurrent(t0.getId(), t2.getId()));

    delete (Constraint*) c0;
    delete (Constraint*) c1;

    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testTokenStateChangeSynchronization() {
    DEFAULT_SETUP(ce,db,false);

    ObjectId timeline = (new Timeline(db.getId(), "Objects", LabelStr("o2")))->getId();
    assertTrue(!timeline.isNoId());

    db.close();

    // Allocate a token
    IntervalToken t1(db.getId(),
    		     "Objects.Predicate", 
    		     true,
    		     IntervalIntDomain(0, 10),
    		     IntervalIntDomain(0, 20),
    		     IntervalIntDomain(1, 1000));

    // Activate immediately. We will merge against it.
    t1.activate();

    // Allocate another
    IntervalToken t2(db.getId(), 
    		     "Objects.Predicate", 
    		     true,
    		     IntervalIntDomain(0, 10),
    		     IntervalIntDomain(0, 20),
    		     IntervalIntDomain(1, 1000));

    // Allocate a constraint on the inactive token, to constrain a timepoint
    Variable<IntervalIntDomain> v0(ce.getId(), IntervalIntDomain());
    EqualConstraint c0(LabelStr("eq"), LabelStr("Default"), ce.getId() , makeScope(t2.getEnd(), v0.getId()));

    // Conduct the merge.
    t2.merge(t1.getId());

    // Now changes on v0 should propagate to the end variable of t1.
    v0.specify(IntervalIntDomain(8, 10));
    assertTrue(t1.getEnd()->getDerivedDomain() == IntervalIntDomain(8, 10));

    // If we split again, expect that the restriction now applies to the end-point
    // of the inactive token
    t2.cancel();

    assertTrue(t2.getEnd()->getDerivedDomain() == IntervalIntDomain(8, 10));

    DEFAULT_TEARDOWN();
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
    		     IntervalIntDomain(0, 10),
    		     IntervalIntDomain(0, 20),
    		     IntervalIntDomain(1, 5),
		     "o2", false);
    t1.addParameter(IntervalIntDomain(0,1),"IntervalParam");
    t1.close();

    assertTrue(ce.propagate());

    // Activate immediately to trigger the rule.
    t1.activate();
    assertTrue(t1.getSlaves().size() == 1);

    assertTrue (ce.propagate());

    TokenId slave = *t1.getSlaves().begin();

    assertTrue(t1.getStart()->derivedDomain().getLowerBound() == 0);
    assertTrue(t1.getStart()->derivedDomain().getUpperBound() == 10);
    assertTrue(t1.getEnd()->derivedDomain().getLowerBound() == 1);
    assertTrue(t1.getEnd()->derivedDomain().getUpperBound() == 15);
    assertTrue(slave->getStart()->derivedDomain().getLowerBound() == 1);
    assertTrue(slave->getStart()->derivedDomain().getUpperBound() == 15);
    assertTrue(slave->getEnd()->derivedDomain().getLowerBound() == 2);
    assertTrue(slave->getEnd()->derivedDomain().getUpperBound() == 100);

    std::vector<ConstrainedVariableId> scope;
    scope.push_back(slave->getEnd());
    scope.push_back(t1.getParameters()[0]);
    ConstraintId culprit = ConstraintLibrary::createConstraint(LabelStr("leq"), ce.getId(), scope);

    assertTrue (!ce.propagate());

    assertTrue(t1.getStart()->derivedDomain().getLowerBound() == 3);
    assertTrue(t1.getStart()->derivedDomain().getUpperBound() == -2);
    assertTrue(t1.getEnd()->derivedDomain().getLowerBound() == 3);
    assertTrue(t1.getEnd()->derivedDomain().getUpperBound() == -2);
    assertTrue(slave->getStart()->derivedDomain().getLowerBound() == 3);
    assertTrue(slave->getStart()->derivedDomain().getUpperBound() == -2);
    assertTrue(slave->getEnd()->derivedDomain().getLowerBound() == 3);
    assertTrue(slave->getEnd()->derivedDomain().getUpperBound() == -2);

    t1.cancel();

    assertTrue(ce.propagate());

    assertTrue(t1.getStart()->derivedDomain().getLowerBound() == 0);
    assertTrue(t1.getStart()->derivedDomain().getUpperBound() == 10);
    assertTrue(t1.getEnd()->derivedDomain().getLowerBound() == 1);
    assertTrue(t1.getEnd()->derivedDomain().getUpperBound() == 15);

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

    ConstrainedVariableId v1 = (new Variable<IntervalIntDomain> (ce.getId(), domStart, true, "v1"))->getId();
    ConstrainedVariableId v2 = (new Variable<IntervalIntDomain> (ce.getId(), domDur, true, "v2"))->getId();
    ConstrainedVariableId v3 = (new Variable<IntervalIntDomain> (ce.getId(), domEnd, true, "v3"))->getId();
    ConstrainedVariableId v4 = (new Variable<IntervalIntDomain> (ce.getId(), domStart, true, "v4"))->getId();
    ConstrainedVariableId v5 = (new Variable<IntervalIntDomain> (ce.getId(), domDur, true, "v5"))->getId();
    ConstrainedVariableId v6 = (new Variable<IntervalIntDomain> (ce.getId(), domEnd, true, "v6"))->getId();

    v2->specify(IntervalIntDomain(5, 7));

    std::vector<ConstrainedVariableId> temp;
    temp.push_back(v1);
    temp.push_back(v2);
    temp.push_back(v3);
    ConstraintId duration1 = 
      ConstraintLibrary::createConstraint(LabelStr("temporalDistance"), ce.getId(), temp);

    assertTrue(!duration1.isNoId());

    temp.clear();
    temp.push_back(v4);
    temp.push_back(v5);
    temp.push_back(v6);
    ConstraintId duration2 = 
      ConstraintLibrary::createConstraint(LabelStr("temporalDistance"), ce.getId(), temp);

    assertTrue(!duration2.isNoId());

    temp.clear();
    temp.push_back(v3);
    temp.push_back(v4);
    ConstraintId beforeConstraint = 
      ConstraintLibrary::createConstraint(LabelStr("precedes"), ce.getId(), temp);

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
    ConstraintEngine ce;
    TemporalPropagator*
      tp = new TemporalPropagator(LabelStr("Temporal"), ce.getId());

    IntervalIntDomain domStart = IntervalIntDomain(1,10);
    IntervalIntDomain domEnd = IntervalIntDomain(0,1);
    IntervalIntDomain domDur = IntervalIntDomain(1,1);

    ConstrainedVariableId v1 = (new Variable<IntervalIntDomain> (ce.getId(), domStart, true, "v1"))->getId();
    ConstrainedVariableId v2 = (new Variable<IntervalIntDomain> (ce.getId(), domDur, true, "v2"))->getId();
    ConstrainedVariableId v3 = (new Variable<IntervalIntDomain> (ce.getId(), domEnd, true, "v3"))->getId();

    std::vector<ConstrainedVariableId> temp;
    temp.push_back(v1);
    temp.push_back(v2);
    temp.push_back(v3);
    ConstraintId constraint = 
      ConstraintLibrary::createConstraint(LabelStr("temporalDistance"),
                                          ce.getId(), temp);
    bool consistent = ce.propagate();
    std::vector<ConstrainedVariableId> fromvars;
    std::vector<ConstrainedVariableId> tovars;
    std::vector<long> lengths;
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


int main() {
  LockManager::instance().connect();
  LockManager::instance().lock();

  Schema::instance();
  initConstraintLibrary();

  for(int i=0;i<1;i++){
    runTestSuite(TemporalNetworkTest::test);
    runTestSuite(TemporalNetworkConstraintEngineOnlyTest::test);
    runTestSuite(TemporalPropagatorTest::test);
  }
  std::cout << "Finished" << std::endl;
  ConstraintLibrary::purgeAll();
  exit(0);
}
