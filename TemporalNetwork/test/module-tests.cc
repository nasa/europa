#include "TestSupport.hh"
#include "TemporalNetwork.hh"
#include "TemporalPropagator.hh"
#include "STNTemporalAdvisor.hh"
#include "TemporalAdvisor.hh"
#include "Schema.hh"
#include "PlanDatabase.hh"
#include "DbLogger.hh"
#include "CeLogger.hh"
#include "ObjectTokenRelation.hh"
#include "TemporalNetworkLogger.hh"
#include "IntervalToken.hh"
#include "Timeline.hh"
#include "Utils.hh"
#include "IntervalIntDomain.hh"
#include "TemporalConstraints.hh"

#include <iostream>
#include <string>
#include <list>

#define DEFAULT_SETUP(ce, db,  autoClose) \
    ConstraintEngine ce; \
    Schema::instance()->reset();\
    Schema::instance()->addObjectType("Objects"); \
    Schema::instance()->addPredicate("Objects.Predicate"); \
    PlanDatabase db(ce.getId(), Schema::instance());\
    new DefaultPropagator(LabelStr("Default"), ce.getId()); \
    new TemporalPropagator(LabelStr("Temporal"), ce.getId()); \
    db.setTemporalAdvisor((new STNTemporalAdvisor(ce.getPropagatorByName(LabelStr("Temporal"))))->getId()); \
    Id<DbLogger> dbLId; \
    if (loggingEnabled()) { \
      new CeLogger(std::cout, ce.getId()); \
      new TemporalNetworkLogger((const TemporalPropagatorId& )ce.getPropagatorByName(LabelStr("Temporal")), std::cout); \
      dbLId = (new DbLogger(std::cout, db.getId()))->getId(); \
    } \
    if (autoClose) \
      db.close();

#define DEFAULT_TEARDOWN() \
    delete (DbLogger*) dbLId;

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
    assert(delta == 0 && epsilon == 0);

    tn.calcDistanceBounds(origin, origin, delta, epsilon);
    assert(delta == 0 && epsilon == 0);
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
    assert(res);

    Time dist_lb, dist_ub;
    tn.calcDistanceBounds(c_start, b_end, dist_lb, dist_ub);
    assert(dist_lb > 0);

    // Force failure where b meets c
    TemporalConstraintId b_meets_c = tn.addTemporalConstraint(b_end, c_start, 0, 0);
    res = tn.isConsistent();
    assert(!res);

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
    assert(!res);

    tn.removeTemporalConstraint(fromage);
    tn.removeTemporalConstraint(tango);

    res = tn.isConsistent();
    assert(res); // Consistency restored

    TemporalConstraintId c0 = tn.addTemporalConstraint(y, x, -200, g_infiniteTime());
    TemporalConstraintId c1 = tn.addTemporalConstraint(x, z, 0, g_infiniteTime());
    TemporalConstraintId c2 = tn.addTemporalConstraint(z, y, (Time)0, g_infiniteTime());
    TemporalConstraintId c3 = tn.addTemporalConstraint(x, y, 200, g_infiniteTime());

    res = tn.isConsistent();
    assert(res);

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
    runTest(testSynchronization);
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
    assert(!timeline.isNoId());

    db.close();

    IntervalToken t1(db.getId(),
    		     "Objects.Predicate", 
    		     true,
    		     IntervalIntDomain(0, 10),
    		     IntervalIntDomain(0, 20),
    		     IntervalIntDomain(1, 1000));

    t1.getDuration()->specify(IntervalIntDomain(5, 7));
    assert(t1.getEnd()->getDerivedDomain().getLowerBound() == 5);
    assert(t1.getEnd()->getDerivedDomain().getUpperBound() == 17);

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
    assert(!beforeConstraint.isNoId());

    assert(t1.getStart()->getDerivedDomain().getLowerBound() == 0);
    assert(t1.getStart()->getDerivedDomain().getUpperBound() == 5);
    assert(t1.getEnd()->getDerivedDomain().getLowerBound() == 5);
    assert(t1.getEnd()->getDerivedDomain().getUpperBound() == 10);
    assert(t2.getStart()->getDerivedDomain().getLowerBound() == 5);
    assert(t2.getStart()->getDerivedDomain().getUpperBound() == 10);
    assert(t2.getEnd()->getDerivedDomain().getLowerBound() == 6);
    assert(t2.getEnd()->getDerivedDomain().getUpperBound() == 20);

    delete (Constraint*) beforeConstraint;
    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testCanPrecede() {
    DEFAULT_SETUP(ce,db,false);

    ObjectId timeline = (new Timeline(db.getId(), "Objects", LabelStr("o2")))->getId();
    assert(!timeline.isNoId());

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
    assert (tp->canPrecede(first.getEnd(), second.getStart()));
    assert (tp->canPrecede(second.getEnd(), first.getStart()));

    // compute from advisor
    assert (db.getTemporalAdvisor()->canPrecede(first.getId(),second.getId()));

    // restrict via specifying the domain

    IntervalIntDomain dom(21, 31);
    first.getStart()->specify(dom);
    first.getEnd()->specify(dom);

    IntervalIntDomain dom2(1, 20);
    second.getStart()->specify(dom2);
    second.getEnd()->specify(dom2);

    bool res = ce.propagate();
    assert(res);
    
    // compute from propagator directly
    assert (!tp->canPrecede(first.getEnd(), second.getStart()));
    assert (tp->canPrecede(second.getEnd(), first.getStart()));
    // compute from advisor
    assert (!db.getTemporalAdvisor()->canPrecede(first.getId(),second.getId()));
    
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
    assert(beforeConstraint.isValid());

    res = ce.propagate();
    assert(res);
    
    // compute from propagator directly
    res = tp->canPrecede(first.getEnd(), second.getStart());
    assert (res);
    assert (!tp->canPrecede(second.getEnd(), first.getStart()));

    // compute from advisor
    assert (db.getTemporalAdvisor()->canPrecede(first.getId(),second.getId()));
    assert (!db.getTemporalAdvisor()->canPrecede(second.getId(), first.getId()));

    delete (Constraint*) beforeConstraint;
    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testCanFitBetween() {
    DEFAULT_SETUP(ce,db,false);

    ObjectId timeline = (new Timeline(db.getId(), "Objects", LabelStr("o2")))->getId();
    assert(!timeline.isNoId());

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
    assert (((TemporalPropagatorId)ce.getPropagatorByName(LabelStr("Temporal")))->canFitBetween(token.getStart(), token.getEnd(), predecessor.getEnd(), successor.getStart()));

    // compute from advisor
    assert (db.getTemporalAdvisor()->canFitBetween(token.getId(), predecessor.getId(), successor.getId()));

    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testCanBeConcurrent() {
    DEFAULT_SETUP(ce,db,false);

    ObjectId timeline = (new Timeline(db.getId(), "Objects", LabelStr("o2")))->getId();
    assert(!timeline.isNoId());

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
    assert(db.getTemporalAdvisor()->canBeConcurrent(t0.getId(), t1.getId()));

    // May 1 very tight, but still ok
    t0.getStart()->specify(1);
    t0.getEnd()->specify(2);
    assert(ce.propagate());
    assert(db.getTemporalAdvisor()->canBeConcurrent(t0.getId(), t1.getId()));

    // Make it too tight.
    t1.getEnd()->specify(10);
    ce.propagate();
    assert(!db.getTemporalAdvisor()->canBeConcurrent(t0.getId(), t1.getId()));

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

    assert(ce.propagate());

    assert(!db.getTemporalAdvisor()->canBeConcurrent(t0.getId(), t1.getId()));
    assert(!db.getTemporalAdvisor()->canBeConcurrent(t1.getId(), t2.getId()));
    assert(!db.getTemporalAdvisor()->canBeConcurrent(t0.getId(), t2.getId()));

    delete (Constraint*) c0;
    delete (Constraint*) c1;

    DEFAULT_TEARDOWN();
    return true;
  }
  static bool testSynchronization() {
    DEFAULT_SETUP(ce,db,false);

    ObjectId timeline = (new Timeline(db.getId(), "Objects", LabelStr("o2")))->getId();
    assert(!timeline.isNoId());

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
    assert(t1.getEnd()->getDerivedDomain() == IntervalIntDomain(8, 10));

    // If we split again, expect that the restriction now applies to the end-point
    // of the inactive token
    t2.cancel();

    assert(t2.getEnd()->getDerivedDomain() == IntervalIntDomain(8, 10));

    DEFAULT_TEARDOWN();
    return true;
  }
};

int main() {
  Schema::instance();
  initConstraintLibrary();

  // Special designations for temporal relations
  REGISTER_CONSTRAINT(PrecedesConstraint, "precedes", "Temporal");
  REGISTER_CONSTRAINT(TemporalDistanceConstraint, "StartEndDurationRelation", "Temporal");
  REGISTER_CONSTRAINT(ObjectTokenRelation, "ObjectTokenRelation", "Default");

  for(int i=0;i<1;i++){
    runTestSuite(TemporalNetworkTest::test);
    runTestSuite(TemporalPropagatorTest::test);
  }
  std::cout << "Finished" << std::endl;
  ConstraintLibrary::purgeAll();
}
