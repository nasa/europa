/**
 * @file module-tests.cc
 * @author Conor McGann
 * @date August, 2003
 * @brief Read the source for details
 */
#include "TestSupport.hh"
#include "Utils.hh"
#include "Variable.hh"
#include "Constraints.hh"
#include "ConstraintLibrary.hh"
#include "IdTable.hh"
#include "EquivalenceClassCollection.hh"
#include "EqualityConstraintPropagator.hh"

/* Include for domain management */
#include "AbstractDomain.hh"
#include "EnumeratedDomain.hh"
#include "LabelStr.hh"
#include "IntervalIntDomain.hh"
#include "BoolDomain.hh"
#include "Domain.hh"
#include "domain-tests.hh"

#include <iostream>
#include <vector>
#include <string>

using namespace Prototype;
using namespace std;

class DelegationTestConstraint: public Constraint{
public:
  DelegationTestConstraint(const LabelStr& name,
			   const LabelStr& propagatorName,
			   const ConstraintEngineId& constraintEngine,
			   const ConstrainedVariableId& variable,
			   const AbstractDomain&)
    : Constraint(name, propagatorName, constraintEngine, variable){s_instanceCount++;}
  ~DelegationTestConstraint(){s_instanceCount--;}
  void handleExecute(){s_executionCount++;}
  void handleExecute(const ConstrainedVariableId&,int, const DomainListener::ChangeType&){}
  bool canIgnore(const ConstrainedVariableId& variable, 
		 int argIndex, 
		 const DomainListener::ChangeType& changeType){
    if(changeType == DomainListener::SET)
      return true;
    return false;
  }

  static int s_executionCount;
  static int s_instanceCount;
};

int DelegationTestConstraint::s_executionCount = 0;
int DelegationTestConstraint::s_instanceCount = 0;


class TestListener: public ConstraintEngineListener{
public:
  TestListener(const ConstraintEngineId& ce):ConstraintEngineListener(ce){
    for (int i=0;i<ConstraintEngine::EVENT_COUNT;i++) m_events[i] = 0;
  }
  void notifyPropagationCommenced(){increment(ConstraintEngine::PROPAGATION_COMMENCED);}
  void notifyPropagationCompleted(){increment(ConstraintEngine::PROPAGATION_COMPLETED);}
  void notifyPropagationPreempted(){increment(ConstraintEngine::PROPAGATION_PREEMPTED);}
  void notifyAdded(const ConstraintId& constraint){increment(ConstraintEngine::CONSTRAINT_ADDED);}
  void notifyRemoved(const ConstraintId& constraint){increment(ConstraintEngine::CONSTRAINT_REMOVED);}
  void notifyExecuted(const ConstraintId& constraint){increment(ConstraintEngine::CONSTRAINT_EXECUTED);}
  void notifyAdded(const ConstrainedVariableId& variable){increment(ConstraintEngine::VARIABLE_ADDED);}
  void notifyRemoved(const ConstrainedVariableId& variable){increment(ConstraintEngine::VARIABLE_REMOVED);}
  void notifyChanged(const ConstrainedVariableId& variable, const DomainListener::ChangeType& changeType){increment(changeType);}

  int getCount(ConstraintEngine::Event event){return m_events[event];}
  void reset() {for(int i=0; i<ConstraintEngine::EVENT_COUNT;i++) m_events[i] = 0;}
private:
  void increment(int event){m_events[event] = m_events[event] + 1;}
  int m_events[ConstraintEngine::EVENT_COUNT];
};

class VariableTest
{
public:
  static bool test() {
    runTest(testAllocation);
    runTest(testMessaging);
    return true;
  }

private:
  static bool testAllocation(){
    IntervalIntDomain dom0(0, 1000);
    Variable<IntervalIntDomain> v0(ENGINE, dom0);
    const IntervalIntDomain& dom1 = v0.getBaseDomain();
    check_error(dom0 == dom1);
    check_error(v0.isValid());
    check_error(v0.canBeSpecified());
    Variable<IntervalIntDomain> v1(ENGINE, dom1, false, LabelStr("TEST VARIABLE"));
    check_error(!v1.canBeSpecified());
    check_error(v1.getName() == LabelStr("TEST VARIABLE"));
    check_error(v1.isValid());
    return true;
  }

  static bool testMessaging(){
    TestListener listener(ENGINE);

    // Add, Specify, Remove
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(0, 100));
      check_error(listener.getCount(ConstraintEngine::SET) == 1);
      check_error(listener.getCount(ConstraintEngine::VARIABLE_ADDED) == 1);
      v0.specify(IntervalIntDomain(3, 8));
      check_error(listener.getCount(ConstraintEngine::SET) == 2);
      v0.specify(5);
      check_error(listener.getCount(ConstraintEngine::SET) == 2);
      check_error(listener.getCount(ConstraintEngine::SET_TO_SINGLETON) == 1);
    }
    check_error(listener.getCount(ConstraintEngine::VARIABLE_REMOVED) == 1);

    // Bounds restriction messages for derived domain
    listener.reset();
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(0, 100));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(0, 10));
      EqualConstraint c0(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId()));
      ENGINE->propagate();
      check_error(listener.getCount(ConstraintEngine::UPPER_BOUND_DECREASED) == 1);

      v0.specify(IntervalIntDomain(5, 10)); // Expect lower-bound-increased message
      check_error(listener.getCount(ConstraintEngine::LOWER_BOUND_INCREASED) == 1);

      ENGINE->propagate(); // Expect another through propagation
      check_error(listener.getCount(ConstraintEngine::LOWER_BOUND_INCREASED) == 2);

      v1.specify(IntervalIntDomain(6, 8)); // 
      check_error(listener.getCount(ConstraintEngine::BOUNDS_RESTRICTED) == 1);

      ENGINE->propagate(); // Expect another through propagation
      check_error(listener.getCount(ConstraintEngine::BOUNDS_RESTRICTED) == 2);

      v0.specify(7);
      ENGINE->propagate(); // Expect a RESTRICT_TO_SINGLETON event through propagation
      check_error(listener.getCount(ConstraintEngine::RESTRICT_TO_SINGLETON) == 1);

      v0.reset(); // Expect a RESET message for v0 and a RELAXATION message for both variables
      check_error(listener.getCount(ConstraintEngine::RESET) == 1);
      check_error(listener.getCount(ConstraintEngine::RELAXED) == 2);
      check_error(ENGINE->pending());

      v0.specify(0); // Expect EMPTIED
      ENGINE->propagate();
      check_error(listener.getCount(ConstraintEngine::EMPTIED) == 1);
    }

    // Now tests message handling on Enumerated Domain
    listener.reset();
    {
      Variable<EnumeratedDomain> v0(ENGINE, EnumeratedDomain());
      v0.insert(1);
      v0.insert(3);
      v0.insert(5);
      v0.insert(10);
      check_error(listener.getCount(ConstraintEngine::RELAXED) == 0); // Should not generate any of these messages while not closed
      v0.close();
      check_error(listener.getCount(ConstraintEngine::CLOSED) == 1);
      check_error(listener.getCount(ConstraintEngine::SET) == 1); // Expect to see specified domain cause 'set' on derived domain once closed.

      EnumeratedDomain d0;
      d0.insert(2);
      d0.insert(3);
      d0.insert(5);
      d0.insert(11);
      d0.close();
      Variable<EnumeratedDomain> v1(ENGINE, d0);
      check_error(listener.getCount(ConstraintEngine::SET) == 2); // Expect to see specified domain cause 'set' immediately.

      EqualConstraint c0(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId()));
      ENGINE->propagate(); // Should see values removed from both variables domains. 
      check_error(listener.getCount(ConstraintEngine::VALUE_REMOVED) == 2);
      v0.specify(3);
      check_error(listener.getCount(ConstraintEngine::SET_TO_SINGLETON) == 1);
      v1.specify(5);
      check_error(listener.getCount(ConstraintEngine::SET_TO_SINGLETON) == 2);
      ENGINE->propagate(); // Expect to see exactly one domain emptied
      check_error(listener.getCount(ConstraintEngine::EMPTIED) == 1);
      v1.reset(); // Should now see 2 domains relaxed.
      check_error(listener.getCount(ConstraintEngine::RELAXED) == 2);
    }

    return true;
  }
};

class ConstraintTest
{
public:
  static bool test() {
    runTest(testSubsetConstraint);
    runTest(testAddEqualConstraint);
    runTest(testEqualConstraint);
    runTest(testLessThanEqualConstraint);
    runTest(testLessOrEqThanSumConstraint);
    runTest(testBasicPropagation);
    runTest(testForceInconsistency);
    runTest(testRepropagation);
    runTest(testConstraintRemoval);
    runTest(testDelegation);
    runTest(testNotEqual);
    runTest(testMultEqualConstraint);
    runTest(testAddMultEqualConstraint);
    runTest(testConstraintDeletion);
    return(true);
  }

private:

  static bool testSubsetConstraint() {
    std::list<Prototype::LabelStr> values;
    values.push_back(Prototype::LabelStr("A"));
    values.push_back(Prototype::LabelStr("B"));
    values.push_back(Prototype::LabelStr("C"));
    values.push_back(Prototype::LabelStr("D"));
    values.push_back(Prototype::LabelStr("E"));
    LabelSet ls0(values);
    values.pop_back();
    values.pop_back();
    LabelSet ls1(values);
    check_error(ls1.isSubsetOf(ls0));
    check_error(!(ls1 == ls0));

    Variable<LabelSet> v0(ENGINE, ls0);
    check_error(! (v0.getDerivedDomain() == ls1));
    SubsetOfConstraint c0(LabelStr("SubsetOf"), LabelStr("Default"), ENGINE, v0.getId(), ls1);
    ENGINE->propagate();
    check_error(ENGINE->constraintConsistent());
    check_error(v0.getDerivedDomain() == ls1);
    check_error(c0.executionCount() == 1);

    values.pop_back();
    LabelSet ls2(values);
    v0.specify(ls2);
    check_error(!ENGINE->pending()); // No change expected since it is a restriction
    check_error(!(v0.getDerivedDomain() == ls1));
    check_error(c0.executionCount() == 1);
    check_error(ENGINE->constraintConsistent());
    v0.reset();
    check_error(ENGINE->pending());
    ENGINE->propagate();
    check_error(ENGINE->constraintConsistent());
    check_error(v0.getDerivedDomain() == ls1);
    check_error(c0.executionCount() == 2);
    
    return true;
  }

  static bool testAddEqualConstraint() {
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, 1));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(0, 2));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      check_error(ENGINE->constraintConsistent());
      check_error(v0.getDerivedDomain().getSingletonValue() == 1);
      check_error(v1.getDerivedDomain().getSingletonValue() == 1);
      check_error(v2.getDerivedDomain().getSingletonValue() == 2);
    }

    // Now test mixed types
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
      Variable<IntervalDomain> v1(ENGINE, IntervalDomain(1.2, 2.8));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(0, 3));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      check_error(ENGINE->constraintConsistent());
      check_error(v0.getDerivedDomain().getSingletonValue() == 1);
      check_error(v1.getDerivedDomain().getSingletonValue() == 2);
      check_error(v2.getDerivedDomain().getSingletonValue() == 3);
    }

    // Now test special case of rounding with a singleton
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(0, 10));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(0, 10));
      Variable<IntervalDomain> v2(ENGINE, IntervalDomain(0.5, 0.5));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      check_error(ENGINE->provenInconsistent());
    }

    // Now test special case of rounding with a singleton, with negative domain bounds
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(-10, 10));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(-10, 10));
      Variable<IntervalDomain> v2(ENGINE, IntervalDomain(0.01, 0.99));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      check_error(!ENGINE->propagate());
    }

    // Confirm correct result will all singletons
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(-1, -1));
      Variable<IntervalDomain> v1(ENGINE, IntervalDomain(10.4, 10.4));
      Variable<IntervalDomain> v2(ENGINE, IntervalDomain(9.4, 9.4));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      check_error(ENGINE->propagate());
    }

    // Confirm inconsistency detected with all singletons
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(-1, -1));
      Variable<IntervalDomain> v1(ENGINE, IntervalDomain(10.4, 10.4));
      Variable<IntervalDomain> v2(ENGINE, IntervalDomain(9.39, 9.39));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      check_error(!ENGINE->propagate());
    }

    // Obtain factors correct values for fixed result
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(0, PLUS_INFINITY));
      Variable<IntervalDomain> v1(ENGINE, IntervalDomain(0, PLUS_INFINITY));
      Variable<IntervalDomain> v2(ENGINE, IntervalDomain(9.390, 9.390));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      check_error(ENGINE->propagate());
      check_error(v0.getDerivedDomain() == IntervalIntDomain(0, 9));
      check_error(v1.getDerivedDomain() == IntervalDomain(0.39, 9.39));
    }

    // Test handling with all infinites
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(MINUS_INFINITY, MINUS_INFINITY));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, PLUS_INFINITY));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(PLUS_INFINITY, PLUS_INFINITY));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      check_error(ENGINE->propagate());
      check_error(v0.getDerivedDomain() == IntervalIntDomain(MINUS_INFINITY, MINUS_INFINITY));
      check_error(v1.getDerivedDomain() == IntervalIntDomain(1, PLUS_INFINITY));
      check_error(v2.getDerivedDomain() == IntervalIntDomain(PLUS_INFINITY, PLUS_INFINITY));
    }

    // Test handling with infinites and non-infinites
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(10, PLUS_INFINITY));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, PLUS_INFINITY));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(MINUS_INFINITY, 100));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      check_error(ENGINE->propagate());
      check_error(v0.getDerivedDomain() == IntervalIntDomain(10, 99));
      check_error(v1.getDerivedDomain() == IntervalIntDomain(1, 90));
      check_error(v2.getDerivedDomain() == IntervalIntDomain(11, 100));
    }

    // Test propagating infinites: start + duration == end.
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(MINUS_INFINITY, PLUS_INFINITY));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(MINUS_INFINITY, PLUS_INFINITY));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      check_error(ENGINE->propagate());
      check_error(v0.getDerivedDomain() == IntervalIntDomain(MINUS_INFINITY, PLUS_INFINITY));
      check_error(v1.getDerivedDomain() == IntervalIntDomain(1));
      check_error(v2.getDerivedDomain() == IntervalIntDomain(MINUS_INFINITY, PLUS_INFINITY));
    }

    return true;
  }

  static bool testEqualConstraint()
  {
    // Set up a base domain
    std::list<Prototype::LabelStr> baseValues;
    baseValues.push_back(Prototype::LabelStr("A"));
    baseValues.push_back(Prototype::LabelStr("B"));
    baseValues.push_back(Prototype::LabelStr("C"));
    baseValues.push_back(Prototype::LabelStr("D"));
    baseValues.push_back(Prototype::LabelStr("E"));
    LabelSet baseDomain(baseValues);

    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(-100, 1));
    EqualConstraint c0(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId()));
    ENGINE->propagate();
    check_error(ENGINE->constraintConsistent());
    check_error(v0.getDerivedDomain().getSingletonValue() == 1);
    check_error(v1.getDerivedDomain().getSingletonValue() == 1);

    LabelSet ls0(baseDomain);
    ls0.empty();
    ls0.insert(Prototype::LabelStr("A"));

    LabelSet ls1(baseDomain);
    ls1.empty();
    ls1.insert(Prototype::LabelStr("A"));
    ls1.insert(Prototype::LabelStr("B"));
    ls1.insert(Prototype::LabelStr("C"));
    ls1.insert(Prototype::LabelStr("D"));
    ls1.insert(Prototype::LabelStr("E"));

    Variable<LabelSet> v2(ENGINE, ls1);
    Variable<LabelSet> v3(ENGINE, ls1);
    EqualConstraint c1(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v2.getId(), v3.getId()));
    ENGINE->propagate();
    check_error(ENGINE->constraintConsistent());
    check_error(v2.getDerivedDomain() == v3.getDerivedDomain());
    check_error(!v2.getDerivedDomain().isSingleton());

    LabelSet ls2(ls1);
    ls2.remove(Prototype::LabelStr("E"));

    v2.specify(ls2);
    ENGINE->propagate();
    check_error(!v3.getDerivedDomain().isMember(Prototype::LabelStr("E")));

    Variable<LabelSet> v4(ENGINE, ls0);
    EqualConstraint c2(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v2.getId(), v4.getId()));
    ENGINE->propagate();
    check_error(ENGINE->constraintConsistent());
    check_error(v2.getDerivedDomain() == v3.getDerivedDomain());
    check_error(v2.getDerivedDomain() == v4.getDerivedDomain());
    check_error(v3.getDerivedDomain() == v4.getDerivedDomain());
    check_error(v3.getDerivedDomain().getSingletonValue() == Prototype::LabelStr("A"));

    return true;
  }

  static bool testLessThanEqualConstraint() {
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 100));
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, 100));
    LessThanEqualConstraint c0(LabelStr("LessThanEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId()));
    ENGINE->propagate();
    check_error(ENGINE->constraintConsistent());
    check_error(v0.getDerivedDomain() == v1.getDerivedDomain());

    v0.specify(IntervalIntDomain(50, 100));
    check_error(v1.getDerivedDomain().getLowerBound() == 50);
    IntervalIntDomain copy(v1.getDerivedDomain());
    v0.specify(IntervalIntDomain(50, 80));
    check_error(v1.getDerivedDomain() == copy);
    v1.specify(IntervalIntDomain(60, 70));
    check_error(v0.getDerivedDomain() == IntervalIntDomain(50, 70));
    v1.reset();
    check_error(v0.getDerivedDomain() == IntervalIntDomain(50, 80));
    check_error(v1.getDerivedDomain() == IntervalIntDomain(50, 100));

    // Handle propagation of infinities
    Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(2, PLUS_INFINITY));
    Variable<IntervalIntDomain> v3(ENGINE, IntervalIntDomain(MINUS_INFINITY, 100));
    LessThanEqualConstraint c2(LabelStr("LessThanEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v2.getId(), v3.getId()));
    check_error(ENGINE->propagate());
    check_error(v2.getDerivedDomain().getUpperBound() == 100);
    check_error(v3.getDerivedDomain().getLowerBound() == 2);

    // Handle restriction to singleton
    Variable<IntervalIntDomain> v4(ENGINE, IntervalIntDomain(0, 10));
    Variable<IntervalIntDomain> v5(ENGINE, IntervalIntDomain(5, 15));
    Variable<IntervalIntDomain> v6(ENGINE, IntervalIntDomain(0, 100));
    LessThanEqualConstraint c3(LabelStr("LessThanEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v4.getId(), v5.getId()));
    EqualConstraint c4(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v5.getId(), v6.getId()));
    check_error(ENGINE->propagate());
    v6.specify(9);
    check_error(ENGINE->propagate());
    check_error(v4.getDerivedDomain().getUpperBound() == 9);

    return(true);
  }

  static bool testLessOrEqThanSumConstraint() {
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(0, 100));
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(0, 100));
    Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(0, 100));
    LessOrEqThanSumConstraint c0(LabelStr("LessOrEqThanSumConstraint"), LabelStr("Default"), ENGINE,
                                 makeScope(v0.getId(), v1.getId(), v2.getId()));
    check_error(ENGINE->propagate());
    check_error(ENGINE->constraintConsistent());
    check_error(v0.getDerivedDomain() == v1.getDerivedDomain());
    check_error(v1.getDerivedDomain() == v2.getDerivedDomain());

    v1.specify(IntervalIntDomain(0, 50));
    v2.specify(IntervalIntDomain(0, 40));
    check_error(v0.getDerivedDomain() == IntervalIntDomain(0, 90));
    v0.specify(IntervalIntDomain(60, 70));
    check_error(v1.getDerivedDomain() == IntervalIntDomain(20, 50));
    check_error(v2.getDerivedDomain() == IntervalIntDomain(10, 40));
    v1.reset();
    check_error(v0.getDerivedDomain() == IntervalIntDomain(60, 70));
    check_error(v1.getDerivedDomain() == IntervalIntDomain(20, 100));
    check_error(v2.getDerivedDomain() == IntervalIntDomain(0, 40));

    // @todo Lots more ...

    return(true);
  }

  static bool testBasicPropagation(){
    // v0 == v1
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, 10));
    EqualConstraint c0(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId()));

    // v2 + v3 == v0
    Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(1, 4));
    Variable<IntervalIntDomain> v3(ENGINE, IntervalIntDomain(1, 1));
    AddEqualConstraint c1(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v2.getId(), v3.getId(), v0.getId()));
    check_error(!v0.getDerivedDomain().isEmpty());

    // v4 + v5 == v1
    Variable<IntervalIntDomain> v4(ENGINE, IntervalIntDomain(1, 10));
    Variable<IntervalIntDomain> v5(ENGINE, IntervalIntDomain(1, 1000));
    AddEqualConstraint c2(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v4.getId(), v5.getId(), v1.getId()));

    ENGINE->propagate();
    check_error(ENGINE->constraintConsistent());
    check_error(!v4.getDerivedDomain().isEmpty());
    return true;
  }

  static bool testForceInconsistency(){
    // v0 == v1
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, 10));
    EqualConstraint c0(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId()));
    
    // v2 + v3 == v0
    Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(1, 1));
    Variable<IntervalIntDomain> v3(ENGINE, IntervalIntDomain(1, 1));
    AddEqualConstraint c1(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v2.getId(), v3.getId(), v0.getId()));

    // v4 + v5 == v1
    Variable<IntervalIntDomain> v4(ENGINE, IntervalIntDomain(2, 2));
    Variable<IntervalIntDomain> v5(ENGINE, IntervalIntDomain(2, 2));
    AddEqualConstraint c2(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v4.getId(), v5.getId(), v1.getId()));
    
    ENGINE->propagate();
    check_error(ENGINE->provenInconsistent());
    check_error(v1.getDerivedDomain().isEmpty());
    check_error(v2.getDerivedDomain().isEmpty());

    std::vector<ConstrainedVariableId> variables;
    variables.push_back(v0.getId());
    variables.push_back(v1.getId());
    variables.push_back(v2.getId());
    variables.push_back(v3.getId());
    variables.push_back(v4.getId());
    variables.push_back(v5.getId());

    int emptyCount(0);
    for(std::vector<ConstrainedVariableId>::iterator it = variables.begin(); it != variables.end(); ++it){
      Variable<IntervalIntDomain>* id = (Variable<IntervalIntDomain>*) (*it);
      check_error(id->getDerivedDomain().isEmpty());
      if(id->lastDomain().isEmpty())
	emptyCount++;
    }
    check_error(emptyCount == 1);
    return true;
  }

  static bool testRepropagation()
  {
    // v0 == v1
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, 10));
    EqualConstraint c0(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId()));


    // v2 + v3 == v0
    Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(1, 10));
    Variable<IntervalIntDomain> v3(ENGINE, IntervalIntDomain(1, 10));
    AddEqualConstraint c1(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v2.getId(), v3.getId(), v0.getId()));

    // v4 + v5 == v1
    Variable<IntervalIntDomain> v4(ENGINE, IntervalIntDomain(1, 10));
    Variable<IntervalIntDomain> v5(ENGINE, IntervalIntDomain(1, 10));
    AddEqualConstraint c2(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v4.getId(), v5.getId(), v1.getId()));

    ENGINE->propagate();
    check_error(ENGINE->constraintConsistent());
    v0.specify(IntervalIntDomain(8, 10));
    v1.specify(IntervalIntDomain(2, 7));
    check_error(ENGINE->pending());

    ENGINE->propagate();
    check_error(ENGINE->provenInconsistent());

    v0.reset();
    check_error(ENGINE->pending());
    ENGINE->propagate();
    check_error(ENGINE->constraintConsistent());

    /* Call reset on a constraint consistent network - not sure one would want to do this*/
    v1.reset();
    check_error(ENGINE->pending()); /* Strictly speaking we know it is not inconsistent here since all we have done is relax a previously
				  consistent network. However, we have to propagate to find the new derived domains based on relaxed
				  domains.*/
    ENGINE->propagate();
    check_error(ENGINE->constraintConsistent());
    return true;
  }

  static bool testConstraintRemoval()
  {
    std::vector<ConstrainedVariableId> variables;
    // v0 == v1
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v0.getId());
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v1.getId());
    ConstraintId c0((new EqualConstraint(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, variables))->getId());


    // v2 + v3 == v0
    variables.clear();
    Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v2.getId());
    Variable<IntervalIntDomain> v3(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v3.getId());
    variables.push_back(v0.getId());
    ConstraintId c1((new AddEqualConstraint(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, variables))->getId());

    ENGINE->propagate();
    check_error(ENGINE->constraintConsistent());

    /* Show that we can simply delete a constraint and confirm that the system is still consistent */
    delete (Constraint*) c1;
    ENGINE->propagate();
    check_error(ENGINE->constraintConsistent());

    variables.clear();
    Variable<IntervalIntDomain> v4(ENGINE, IntervalIntDomain(1, 1));
    variables.push_back(v0.getId());
    variables.push_back(v4.getId());
    ConstraintId c2((new EqualConstraint(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, variables))->getId());
    ENGINE->propagate();
    check_error(ENGINE->constraintConsistent());
    check_error(v1.getDerivedDomain().getSingletonValue() == 1);

    delete (Constraint*) c2;
    ENGINE->propagate();
    check_error(ENGINE->constraintConsistent());
    check_error(v1.getDerivedDomain().getUpperBound() == 10);

    /* Add a constraint to force an inconsistency and show that consistency can be restored by removing the
      constraint */
    variables.clear();
    Variable<IntervalIntDomain> v5(ENGINE, IntervalIntDomain(0, 0));
    variables.push_back(v0.getId());
    variables.push_back(v5.getId());
    ConstraintId c3((new EqualConstraint(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, variables))->getId());
    ENGINE->propagate();
    check_error(ENGINE->provenInconsistent());
    delete (Constraint*) c3;
    ENGINE->propagate();
    check_error(ENGINE->constraintConsistent());

    // Clean up remaining constraint
    delete (Constraint*) c0;
    ENGINE->propagate();
    check_error(ENGINE->constraintConsistent());
    return true;
  }

  static bool testDelegation(){
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(0, 1000));
    ConstraintId c0 = ConstraintLibrary::createConstraint(LabelStr("TestOnly"), ENGINE, v0.getId(), IntervalIntDomain(0,0));
    ConstraintId c1 = ConstraintLibrary::createConstraint(LabelStr("TestOnly"), ENGINE, v0.getId(), IntervalIntDomain(0,0));
    ConstraintId c2 = ConstraintLibrary::createConstraint(LabelStr("TestOnly"), ENGINE, v0.getId(), IntervalIntDomain(0,0));
    ConstraintId c3 = ConstraintLibrary::createConstraint(LabelStr("TestOnly"), ENGINE, v0.getId(), IntervalIntDomain(0,0));
    ConstraintId c4 = ConstraintLibrary::createConstraint(LabelStr("TestOnly"), ENGINE, v0.getId(), IntervalIntDomain(0,0));
    ENGINE->propagate();
    check_error(ENGINE->constraintConsistent());
    check_error(DelegationTestConstraint::s_instanceCount == 5);
    check_error(DelegationTestConstraint::s_executionCount == 5);

    // Cause a change in the domain which will impact agenda, then deactivate a constraint and verify the correct execution count
    v0.specify(IntervalIntDomain(0, 900));
    c0->deactivate();
    check_error(!c0->isActive());
    ENGINE->propagate();
    check_error(ENGINE->constraintConsistent());
    check_error(DelegationTestConstraint::s_instanceCount == 5);
    check_error(DelegationTestConstraint::s_executionCount == 9);

    // Delete the delegate and verify instance counts and that the prior delegate has been reinstated and executed.
    delete (Constraint*) c1;
    c0->activate();
    ENGINE->propagate();
    check_error(ENGINE->constraintConsistent());
    check_error(DelegationTestConstraint::s_instanceCount == 4);
    check_error(DelegationTestConstraint::s_executionCount == 10);

    // Now create a new instance and mark it for delegation only. Add remaining constraints as delegates
    ConstraintId c5 = ConstraintLibrary::createConstraint(LabelStr("TestOnly"), ENGINE, v0.getId(), IntervalIntDomain(0,0));
    c0->deactivate();
    c2->deactivate();
    c3->deactivate();
    c4->deactivate();
    check_error(DelegationTestConstraint::s_instanceCount == 5);
    ENGINE->propagate();
    check_error(DelegationTestConstraint::s_executionCount == 11);

    // Force propagation and confirm only one instance executes
    v0.specify(IntervalIntDomain(100, 900));
    ENGINE->propagate();
    check_error(DelegationTestConstraint::s_executionCount == 12);

    // Now confirm correct handling of constraint deletions
    delete (Constraint*) c5;
    delete (Constraint*) c4;
    delete (Constraint*) c3;
    delete (Constraint*) c2;
    delete (Constraint*) c0;
    check_error(DelegationTestConstraint::s_instanceCount == 0);
    return true;
  }

  static bool testNotEqual(){
    EnumeratedDomain dom0;
    dom0.insert(1);
    dom0.insert(2);
    dom0.insert(3);
    dom0.close();

    Variable<EnumeratedDomain> v0(ENGINE, dom0);
    Variable<EnumeratedDomain> v1(ENGINE, dom0);
    Variable<EnumeratedDomain> v2(ENGINE, dom0);

    NotEqualConstraint c0(LabelStr("neq"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId()));
    NotEqualConstraint c1(LabelStr("neq"), LabelStr("Default"), ENGINE, makeScope(v1.getId(), v2.getId()));
    NotEqualConstraint c2(LabelStr("neq"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v2.getId()));
    check_error(ENGINE->pending());
    check_error(ENGINE->propagate());

    dom0.remove(2);
    v0.specify(dom0);
    check_error(!ENGINE->pending()); // No propagation required

    v1.specify(3);
    check_error(ENGINE->pending());
    check_error(ENGINE->propagate());
    check_error(v0.getDerivedDomain().getSingletonValue() == 1);
    check_error(v2.getDerivedDomain().getSingletonValue() == 2);

    v0.reset();
    check_error(ENGINE->pending());
    check_error(ENGINE->propagate());
    check_error(v0.getDerivedDomain() == v2.getDerivedDomain());
    return true;
  }

  static bool testMultEqualConstraint(){
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, 1));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(0, 2));
      MultEqualConstraint c0(LabelStr("MultEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      check_error(ENGINE->constraintConsistent());
      check_error(v0.getDerivedDomain() == v2.getDerivedDomain());
    }

    // Now test with 0 valued denominators and infinites
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(0, PLUS_INFINITY));
      Variable<IntervalDomain> v1(ENGINE, IntervalDomain(1, PLUS_INFINITY));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(MINUS_INFINITY, 6));
      MultEqualConstraint c0(LabelStr("MultEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      check_error(ENGINE->constraintConsistent());
      check_error(v0.getDerivedDomain().getUpperBound() == 6);
      check_error(v2.getDerivedDomain().getLowerBound() == 0);
      check_error(v1.getDerivedDomain().getUpperBound() == PLUS_INFINITY);
    }


    // Special case of negative values on LHS
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(-4, 10));
      Variable<IntervalDomain> v1(ENGINE, IntervalDomain(1, 10));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain());
      MultEqualConstraint c0(LabelStr("MultEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      check_error(ENGINE->constraintConsistent());
      check_error(v2.getDerivedDomain().getLowerBound() == -40);
    }

    return true;
  }

  static bool testAddMultEqualConstraint(){
    // 1 + 2 * 3 == 7
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 1));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(2, 2));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(3, 3));
      Variable<IntervalIntDomain> v3(ENGINE, IntervalIntDomain(7, 7));
      AddMultEqualConstraint c0(LabelStr("AddMultEqualConstraint"), 
				LabelStr("Default"), 
				ENGINE, 
				makeScope(v0.getId(), v1.getId(), v2.getId(), v3.getId()));
      check_error(ENGINE->propagate());
    }

    // 1 + 2 * 3 == 8 => empty
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 1));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(2, 2));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(3, 3));
      Variable<IntervalIntDomain> v3(ENGINE, IntervalIntDomain(8, 8));
      AddMultEqualConstraint c0(LabelStr("AddMultEqualConstraint"), 
				LabelStr("Default"), 
				ENGINE, 
				makeScope(v0.getId(), v1.getId(), v2.getId(), v3.getId()));
      check_error(!ENGINE->propagate());
    }

    // 1 + 1 * [-infty 0] = 1 -> 1 + 1 * 0 = 1
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 1));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, 1));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(MINUS_INFINITY, 0));
      Variable<IntervalIntDomain> v3(ENGINE, IntervalIntDomain(1, 1));
      AddMultEqualConstraint c0(LabelStr("AddMultEqualConstraint"), 
				LabelStr("Default"), 
				ENGINE, 
				makeScope(v0.getId(), v1.getId(), v2.getId(), v3.getId()));
      check_error(ENGINE->propagate());
      check_error(v2.getDerivedDomain().getSingletonValue() == 0);
    }

    // [1.0 10.0] + 1.0 * [1.0 10.0] = 10.0 ->  [1.0 9.0] + 1.0 * [1.0 9.0] = 10.0
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
      Variable<IntervalDomain> v1(ENGINE, IntervalDomain(1.0, 1.0));
      Variable<IntervalDomain> v2(ENGINE, IntervalDomain(1.0, 10.0));
      Variable<IntervalDomain> v3(ENGINE, IntervalDomain(10.0, 10.0));
      AddMultEqualConstraint c0(LabelStr("AddMultEqualConstraint"), 
				LabelStr("Default"), 
				ENGINE, 
				makeScope(v0.getId(), v1.getId(), v2.getId(), v3.getId()));
      check_error(ENGINE->propagate());
      check_error(v0.getDerivedDomain() == IntervalIntDomain(1, 9));
      check_error(v2.getDerivedDomain() == IntervalDomain(1.0, 9.0));
    }


    return true;
  }

  static bool testConstraintDeletion(){
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, 100));
    Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(10, 100));
    Variable<IntervalIntDomain> v3(ENGINE, IntervalIntDomain());

    ConstraintId c0 = (new EqualConstraint(LabelStr("eq"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId())))->getId();
    ConstraintId c1 = (new EqualConstraint(LabelStr("eq"), LabelStr("Default"), ENGINE, makeScope(v1.getId(), v2.getId())))->getId();
    ConstraintId c2 = (new EqualConstraint(LabelStr("eq"), LabelStr("Default"), ENGINE, makeScope(v2.getId(), v3.getId())))->getId();

    // Force an inconsistency
    v0.specify(1);
    v1.specify(1);
    check_error(!ENGINE->propagate());

    // Reset, and delete constraint, but it should not matter
    v1.reset();
    delete (Constraint*) c2;

    // Confirm still inconsistent
    check_error(!ENGINE->propagate());

    delete (Constraint*) c0;
    delete (Constraint*) c1;
    return true;
  }
};

class FactoryTest
{
public:
  static bool test() {
    runTest(testAllocation);
    return true;
  }

private:
  static bool testAllocation(){
    std::vector<ConstrainedVariableId> variables;
    // v0 == v1
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v0.getId());
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, 1));
    variables.push_back(v1.getId());
    ConstraintId c0 = ConstraintLibrary::createConstraint(LabelStr("Equal"), ENGINE, variables);    
    ENGINE->propagate();
    check_error(v0.getDerivedDomain().getSingletonValue() == 1);
    delete (Constraint*) c0;
    return true;
  }
};

class EquivalenceClassTest{
public:
  static bool test() {
    runTest(testBasicAllocation);
    runTest(testConstructionOfSingleGraph);
    runTest(testSplittingOfSingleGraph);
    runTest(testMultiGraphMerging);
    runTest(testEqualityConstraintPropagator);
    return true;
  }

private:
  static bool testBasicAllocation(){
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(2, 8));
    EquivalenceClassCollection g0;
    g0.addConnection(v0.getId(), v1.getId());
    check_error(g0.getGraphCount() == 1);
    v0.specify(10);
    return true;
  }

  static bool testConstructionOfSingleGraph(){
    EquivalenceClassCollection g0;
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(2, 8));
    Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(8, 20));
    g0.addConnection(v0.getId(), v1.getId());
    g0.addConnection(v1.getId(), v2.getId());
    check_error(g0.getGraphCount() == 1);
    int graphKey = g0.getGraphKey(v0.getId());
    check_error(g0.getGraphKey(v1.getId()) == graphKey);
    check_error(g0.getGraphKey(v2.getId()) == graphKey);

    Variable<IntervalIntDomain> v3(ENGINE, IntervalIntDomain(1, 100));
    Variable<IntervalIntDomain> v4(ENGINE, IntervalIntDomain(-100, 100));
    g0.addConnection(v2.getId(), v3.getId());
    g0.addConnection(v3.getId(), v4.getId());
    check_error(g0.getGraphCount() == 1);
    check_error(graphKey != g0.getGraphKey(v0.getId())); // Should have updated for all
    return true;
  }

  static bool testSplittingOfSingleGraph(){
    EquivalenceClassCollection g0;
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(2, 8));
    Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(8, 20));
    Variable<IntervalIntDomain> v3(ENGINE, IntervalIntDomain(1, 100));
    Variable<IntervalIntDomain> v4(ENGINE, IntervalIntDomain(-100, 100));
    Variable<IntervalIntDomain> v5(ENGINE, IntervalIntDomain(-100, 1000));
    Variable<IntervalIntDomain> v6(ENGINE, IntervalIntDomain(-100, 100));
    Variable<IntervalIntDomain> v7(ENGINE, IntervalIntDomain(-100, 100));
    Variable<IntervalIntDomain> v8(ENGINE, IntervalIntDomain(-100, 100));
    g0.addConnection(v0.getId(), v1.getId());
    g0.addConnection(v1.getId(), v2.getId());
    g0.addConnection(v2.getId(), v3.getId());
    g0.addConnection(v3.getId(), v4.getId());
    g0.addConnection(v4.getId(), v5.getId());
    g0.addConnection(v5.getId(), v6.getId());
    g0.addConnection(v6.getId(), v7.getId());
    g0.addConnection(v7.getId(), v8.getId());
    check_error(g0.getGraphCount() == 1);

    // Cause a split by removing a connection in the middle
    g0.removeConnection(v3.getId(), v4.getId());
    check_error(g0.getGraphCount() == 2);

    // Cause another split
    g0.removeConnection(v5.getId(), v6.getId());
    check_error(g0.getGraphCount() == 3);

    // Test membership of resulting classes
    check_error((g0.getGraphKey(v1.getId()) + g0.getGraphKey(v2.getId()) + g0.getGraphKey(v3.getId()))/3 == g0.getGraphKey(v0.getId()));
    check_error(g0.getGraphKey(v4.getId()) == g0.getGraphKey(v5.getId()));
    check_error((g0.getGraphKey(v6.getId()) + g0.getGraphKey(v7.getId()))/2   == g0.getGraphKey(v8.getId()));

    return true;
  }

  static bool testMultiGraphMerging(){
    EquivalenceClassCollection g0;
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(2, 8));
    Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(8, 20));
    Variable<IntervalIntDomain> v3(ENGINE, IntervalIntDomain(1, 100));
    Variable<IntervalIntDomain> v4(ENGINE, IntervalIntDomain(-100, 100));
    Variable<IntervalIntDomain> v5(ENGINE, IntervalIntDomain(-100, 1000));
    Variable<IntervalIntDomain> v6(ENGINE, IntervalIntDomain(-100, 100));
    Variable<IntervalIntDomain> v7(ENGINE, IntervalIntDomain(-100, 100));
    Variable<IntervalIntDomain> v8(ENGINE, IntervalIntDomain(-100, 100));
    // First group
    g0.addConnection(v0.getId(), v1.getId());
    g0.addConnection(v1.getId(), v2.getId());
    g0.addConnection(v2.getId(), v3.getId());

    // Second group
    g0.addConnection(v4.getId(), v5.getId());

    // Third group
    g0.addConnection(v6.getId(), v7.getId());
    g0.addConnection(v7.getId(), v8.getId());

    // Test resulting classes
    check_error(g0.getGraphCount() == 3);
    check_error((g0.getGraphKey(v1.getId()) + g0.getGraphKey(v2.getId()) + g0.getGraphKey(v3.getId()))/3 == g0.getGraphKey(v0.getId()));
    check_error(g0.getGraphKey(v4.getId()) == g0.getGraphKey(v5.getId()));
    check_error((g0.getGraphKey(v6.getId()) + g0.getGraphKey(v7.getId()))/2   == g0.getGraphKey(v8.getId()));

    // Add connectionto cause a merge
    g0.addConnection(v3.getId(), v4.getId());
    check_error(g0.getGraphCount() == 2);
    check_error((g0.getGraphKey(v1.getId()) + g0.getGraphKey(v2.getId()) + g0.getGraphKey(v3.getId()))/3 == g0.getGraphKey(v0.getId()));
    check_error((g0.getGraphKey(v4.getId()) + g0.getGraphKey(v5.getId()))/2 == g0.getGraphKey(v0.getId()));
    check_error((g0.getGraphKey(v6.getId()) + g0.getGraphKey(v7.getId()))/2   == g0.getGraphKey(v8.getId()));


    // Add connectionto cause a merge
    g0.addConnection(v5.getId(), v6.getId());
    check_error(g0.getGraphCount() == 1);

    return true;
  }

  static bool testEqualityConstraintPropagator(){
    ConstraintEngineId ce((new ConstraintEngine())->getId());
    new EqualityConstraintPropagator(LabelStr("EquivalenceClass"), ce);
    {
      std::vector<ConstrainedVariableId> variables;
      // v0 == v1
      Variable<IntervalIntDomain> v0(ce, IntervalIntDomain(1, 10));
      variables.push_back(v0.getId());
      Variable<IntervalIntDomain> v1(ce, IntervalIntDomain(-100, 100));
      variables.push_back(v1.getId());
      EqualConstraint c0(LabelStr("EqualConstraint"), LabelStr("EquivalenceClass"), ce, variables);
      ce->propagate();

      variables.clear();
      Variable<IntervalIntDomain> v2(ce, IntervalIntDomain(8, 10));
      variables.push_back(v2.getId());
      Variable<IntervalIntDomain> v3(ce, IntervalIntDomain(10, 200));
      variables.push_back(v3.getId());
      EqualConstraint c1(LabelStr("EqualConstraint"), LabelStr("EquivalenceClass"), ce, variables);
      ce->propagate();

      check_error(v0.getDerivedDomain().getUpperBound() == 10);
      check_error(v2.getDerivedDomain().getSingletonValue() == 10);

      variables.clear();
      variables.push_back(v3.getId());
      variables.push_back(v1.getId());
      EqualConstraint c2(LabelStr("EqualConstraint"), LabelStr("EquivalenceClass"), ce, variables);

      ce->propagate();
      check_error(ce->constraintConsistent());
      check_error(v0.getDerivedDomain().getSingletonValue() == 10);

      variables.clear();
      Variable<IntervalIntDomain> v4(ce, IntervalIntDomain(1, 9));
      variables.push_back(v3.getId());
      variables.push_back(v4.getId());
      ConstraintId c3((new EqualConstraint(LabelStr("EqualConstraint"), LabelStr("EquivalenceClass"), ce, variables))->getId());
      ce->propagate();
      check_error(ce->provenInconsistent());

      delete (Constraint*) c3;
      check_error(ce->pending());
      ce->propagate();
      check_error(ce->constraintConsistent());
      check_error(v0.getDerivedDomain().getSingletonValue() == 10);
    }
    delete (ConstraintEngine*) ce;
    return(true);
  }
};

int main() {
  initConstraintLibrary();
  REGISTER_UNARY(DelegationTestConstraint, "TestOnly", "Default");
  DomainTests::test();
  runTestSuite(VariableTest::test); 
  runTestSuite(ConstraintTest::test); 
  runTestSuite(FactoryTest::test);
  runTestSuite(EquivalenceClassTest::test);
  cout << "Finished" << endl;
}
