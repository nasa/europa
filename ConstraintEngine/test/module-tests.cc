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
#include "../Libraries/IdTable.hh"
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
#include <cassert>
#include <vector>
#include <sstream>
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
    const AbstractDomain& tempDomain = dom0;
    tempDomain >> std::cout;
    Variable<IntervalIntDomain> v0(ENGINE, dom0);
    const IntervalIntDomain& dom1 = v0.getBaseDomain();
    assert (dom0 == dom1);
    assert(v0.isValid());
    assert(v0.canBeSpecified());
    Variable<IntervalIntDomain> v1(ENGINE, dom1, false, LabelStr("TEST VARIABLE"));
    assert(!v1.canBeSpecified());
    assert(v1.getName() == LabelStr("TEST VARIABLE"));
    assert(v1.isValid());
    return true;
  }

  static bool testMessaging(){
    TestListener listener(ENGINE);

    // Add, Specify, Remove
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(0, 100));
      assert(listener.getCount(ConstraintEngine::SET) == 1);
      assert(listener.getCount(ConstraintEngine::VARIABLE_ADDED) == 1);
      v0.specify(IntervalIntDomain(3, 8));
      assert(listener.getCount(ConstraintEngine::SET) == 2);
      v0.specify(5);
      assert(listener.getCount(ConstraintEngine::SET) == 2);
      assert(listener.getCount(ConstraintEngine::SET_TO_SINGLETON) == 1);
    }
    assert(listener.getCount(ConstraintEngine::VARIABLE_REMOVED) == 1);

    // Bounds restriction messages for derived domain
    listener.reset();
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(0, 100));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(0, 10));
      EqualConstraint c0(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId()));
      ENGINE->propagate();
      assert(listener.getCount(ConstraintEngine::UPPER_BOUND_DECREASED) == 1);

      v0.specify(IntervalIntDomain(5, 10)); // Expect lower-bound-increased message
      assert(listener.getCount(ConstraintEngine::LOWER_BOUND_INCREASED) == 1);

      ENGINE->propagate(); // Expect another through propagation
      assert(listener.getCount(ConstraintEngine::LOWER_BOUND_INCREASED) == 2);

      v1.specify(IntervalIntDomain(6, 8)); // 
      assert(listener.getCount(ConstraintEngine::BOUNDS_RESTRICTED) == 1);

      ENGINE->propagate(); // Expect another through propagation
      assert(listener.getCount(ConstraintEngine::BOUNDS_RESTRICTED) == 2);

      v0.specify(7);
      ENGINE->propagate(); // Expect a RESTRICT_TO_SINGLETON event through propagation
      assert(listener.getCount(ConstraintEngine::RESTRICT_TO_SINGLETON) == 1);

      v0.reset(); // Expect a RESET message for v0 and a RELAXATION message for both variables
      assert(listener.getCount(ConstraintEngine::RESET) == 1);
      assert(listener.getCount(ConstraintEngine::RELAXED) == 2);
      assert(ENGINE->pending());

      v0.specify(0); // Expect EMPTIED
      ENGINE->propagate();
      assert(listener.getCount(ConstraintEngine::EMPTIED) == 1);
    }

    // Now tests message handling on Enumerated Domain
    listener.reset();
    {
      Variable<EnumeratedDomain> v0(ENGINE, EnumeratedDomain());
      v0.insert(1);
      v0.insert(3);
      v0.insert(5);
      v0.insert(10);
      assert(listener.getCount(ConstraintEngine::RELAXED) == 0); // Should not generate any of these messages while not closed
      v0.close();
      assert(listener.getCount(ConstraintEngine::CLOSED) == 1);
      assert(listener.getCount(ConstraintEngine::SET) == 1); // Expect to see specified domain cause 'set' on derived domain once closed.

      EnumeratedDomain d0;
      d0.insert(2);
      d0.insert(3);
      d0.insert(5);
      d0.insert(11);
      d0.close();
      Variable<EnumeratedDomain> v1(ENGINE, d0);
      assert(listener.getCount(ConstraintEngine::SET) == 2); // Expect to see specified domain cause 'set' immediately.

      EqualConstraint c0(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId()));
      ENGINE->propagate(); // Should see values removed from both variables domains. 
      assert(listener.getCount(ConstraintEngine::VALUE_REMOVED) == 2);
      v0.specify(3);
      assert(listener.getCount(ConstraintEngine::SET_TO_SINGLETON) == 1);
      v1.specify(5);
      assert(listener.getCount(ConstraintEngine::SET_TO_SINGLETON) == 2);
      ENGINE->propagate(); // Expect to see exactly one domain emptied
      assert(listener.getCount(ConstraintEngine::EMPTIED) == 1);
      v1.reset(); // Should now see 2 domains relaxed.
      assert(listener.getCount(ConstraintEngine::RELAXED) == 2);
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
    runTest(testBasicPropagation);
    runTest(testForceInconsistency);
    runTest(testRepropagation);
    runTest(testConstraintRemoval);
    runTest(testDelegation);
    runTest(testNotEqual);
    runTest(testMultEqualConstraint);
    runTest(testAddMultEqualConstraint);
    return true;
  }

private:

  static bool testSubsetConstraint(){
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
    assert(ls1.isSubsetOf(ls0));
    assert(!(ls1 == ls0));

    Variable<LabelSet> v0(ENGINE, ls0);
    assert(! (v0.getDerivedDomain() == ls1));
    SubsetOfConstraint c0(LabelStr("SubsetOf"), LabelStr("Default"), ENGINE, v0.getId(), ls1);
    ENGINE->propagate();
    assert(ENGINE->constraintConsistent());
    assert(v0.getDerivedDomain() == ls1);
    assert(c0.executionCount() == 1);

    values.pop_back();
    LabelSet ls2(values);
    v0.specify(ls2);
    assert(!ENGINE->pending()); // No change expected since it is a restriction
    assert(!(v0.getDerivedDomain() == ls1));
    assert(c0.executionCount() == 1);
    assert(ENGINE->constraintConsistent());
    v0.reset();
    assert(ENGINE->pending());
    ENGINE->propagate();
    assert(ENGINE->constraintConsistent());
    assert(v0.getDerivedDomain() == ls1);
    assert(c0.executionCount() == 2);
    
    return true;
  }

  static bool testAddEqualConstraint()
  {
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, 1));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(0, 2));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      assert(ENGINE->constraintConsistent());
      assert(v0.getDerivedDomain().getSingletonValue() == 1);
      assert(v1.getDerivedDomain().getSingletonValue() == 1);
      assert(v2.getDerivedDomain().getSingletonValue() == 2);
    }

    // Now test mixed types
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
      Variable<IntervalDomain> v1(ENGINE, IntervalDomain(1.2, 2.8));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(0, 3));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      assert(ENGINE->constraintConsistent());
      assert(v0.getDerivedDomain().getSingletonValue() == 1);
      assert(v1.getDerivedDomain().getSingletonValue() == 2);
      assert(v2.getDerivedDomain().getSingletonValue() == 3);
    }

    // Now test special case of rounding with a singleton
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(0, 10));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(0, 10));
      Variable<IntervalDomain> v2(ENGINE, IntervalDomain(0.5, 0.5));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      assert(ENGINE->provenInconsistent());
    }

    // Now test special case of rounding with a singleton, with negative domain bounds
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(-10, 10));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(-10, 10));
      Variable<IntervalDomain> v2(ENGINE, IntervalDomain(0.01, 0.99));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      assert(ENGINE->provenInconsistent());
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
    assert(ENGINE->constraintConsistent());
    assert(v0.getDerivedDomain().getSingletonValue() == 1);
    assert(v1.getDerivedDomain().getSingletonValue() == 1);

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
    assert(ENGINE->constraintConsistent());
    assert(v2.getDerivedDomain() == v3.getDerivedDomain());
    assert(!v2.getDerivedDomain().isSingleton());

    LabelSet ls2(ls1);
    ls2.remove(Prototype::LabelStr("E"));

    v2.specify(ls2);
    ENGINE->propagate();
    assert(!v3.getDerivedDomain().isMember(Prototype::LabelStr("E")));

    Variable<LabelSet> v4(ENGINE, ls0);
    EqualConstraint c2(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v2.getId(), v4.getId()));
    ENGINE->propagate();
    assert(ENGINE->constraintConsistent());
    assert(v2.getDerivedDomain() == v3.getDerivedDomain());
    assert(v2.getDerivedDomain() == v4.getDerivedDomain());
    assert(v3.getDerivedDomain() == v4.getDerivedDomain());
    assert(v3.getDerivedDomain().getSingletonValue() == Prototype::LabelStr("A"));

    return true;
  }

  static bool testLessThanEqualConstraint()
  {
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 100));
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, 100));
    LessThanEqualConstraint c0(LabelStr("LessThanEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId()));
    ENGINE->propagate();
    assert(ENGINE->constraintConsistent());
    assert(v0.getDerivedDomain() == v1.getDerivedDomain());

    v0.specify(IntervalIntDomain(50, 100));
    assert(v1.getDerivedDomain().getLowerBound() == 50);
    IntervalIntDomain copy(v1.getDerivedDomain());
    v0.specify(IntervalIntDomain(50, 80));
    assert(v1.getDerivedDomain() == copy);
    v1.specify(IntervalIntDomain(60, 70));
    assert(v0.getDerivedDomain() == IntervalIntDomain(50, 70));
    v1.reset();
    assert(v0.getDerivedDomain() == IntervalIntDomain(50, 80));
    assert(v1.getDerivedDomain() == IntervalIntDomain(50, 100));
    return true;
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
    assert(!v0.getDerivedDomain().isEmpty());

    // v4 + v5 == v1
    Variable<IntervalIntDomain> v4(ENGINE, IntervalIntDomain(1, 10));
    Variable<IntervalIntDomain> v5(ENGINE, IntervalIntDomain(1, 1000));
    AddEqualConstraint c2(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v4.getId(), v5.getId(), v1.getId()));

    ENGINE->propagate();
    assert(ENGINE->constraintConsistent());
    assert(!v4.getDerivedDomain().isEmpty());
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
    assert(ENGINE->provenInconsistent());
    assert(v1.getDerivedDomain().isEmpty());
    assert(v2.getDerivedDomain().isEmpty());

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
      assert(id->getDerivedDomain().isEmpty());
      if(id->lastDomain().isEmpty())
	emptyCount++;
    }
    assert(emptyCount == 1);
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
    assert(ENGINE->constraintConsistent());
    v0.specify(IntervalIntDomain(8, 10));
    v1.specify(IntervalIntDomain(2, 7));
    assert(ENGINE->pending());

    ENGINE->propagate();
    assert(ENGINE->provenInconsistent());

    v0.reset();
    assert(ENGINE->pending());
    ENGINE->propagate();
    assert(ENGINE->constraintConsistent());

    /* Call reset on a constraint consistent network - not sure one would want to do this*/
    v1.reset();
    assert(ENGINE->pending()); /* Strictly speaking we know it is not inconsistent here since all we have done is relax a previously
				  consistent network. However, we have to propagate to find the new derived domains based on relaxed
				  domains.*/
    ENGINE->propagate();
    assert(ENGINE->constraintConsistent());
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
    assert(ENGINE->constraintConsistent());

    /* Show that we can simply delete a constraint and confirm that the system is still consistent */
    delete (Constraint*) c1;
    ENGINE->propagate();
    assert(ENGINE->constraintConsistent());

    variables.clear();
    Variable<IntervalIntDomain> v4(ENGINE, IntervalIntDomain(1, 1));
    variables.push_back(v0.getId());
    variables.push_back(v4.getId());
    ConstraintId c2((new EqualConstraint(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, variables))->getId());
    ENGINE->propagate();
    assert(ENGINE->constraintConsistent());
    assert(v1.getDerivedDomain().getSingletonValue() == 1);

    delete (Constraint*) c2;
    ENGINE->propagate();
    assert(ENGINE->constraintConsistent());
    assert(v1.getDerivedDomain().getUpperBound() == 10);

    /* Add a constraint to force an inconsistency and show that consistency can be restored by removing the
      constraint */
    variables.clear();
    Variable<IntervalIntDomain> v5(ENGINE, IntervalIntDomain(0, 0));
    variables.push_back(v0.getId());
    variables.push_back(v5.getId());
    ConstraintId c3((new EqualConstraint(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, variables))->getId());
    ENGINE->propagate();
    assert(ENGINE->provenInconsistent());
    delete (Constraint*) c3;
    ENGINE->propagate();
    assert(ENGINE->constraintConsistent());

    // Clean up remaining constraint
    delete (Constraint*) c0;
    ENGINE->propagate();
    assert(ENGINE->constraintConsistent());
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
    assert(ENGINE->constraintConsistent());
    assert(DelegationTestConstraint::s_instanceCount == 5);
    assert(DelegationTestConstraint::s_executionCount == 5);

    // Cause a change in the domain which will impact agenda, then deactivate a constraint and verify the correct execution count
    v0.specify(IntervalIntDomain(0, 900));
    c0->deactivate();
    assert(!c0->isActive());
    ENGINE->propagate();
    assert(ENGINE->constraintConsistent());
    assert(DelegationTestConstraint::s_instanceCount == 5);
    assert(DelegationTestConstraint::s_executionCount == 9);

    // Delete the delegate and verify instance counts and that the prior delegate has been reinstated and executed.
    delete (Constraint*) c1;
    c0->activate();
    ENGINE->propagate();
    assert(ENGINE->constraintConsistent());
    assert(DelegationTestConstraint::s_instanceCount == 4);
    assert(DelegationTestConstraint::s_executionCount == 10);

    // Now create a new instance and mark it for delegation only. Add remaining constraints as delegates
    ConstraintId c5 = ConstraintLibrary::createConstraint(LabelStr("TestOnly"), ENGINE, v0.getId(), IntervalIntDomain(0,0));
    c0->deactivate();
    c2->deactivate();
    c3->deactivate();
    c4->deactivate();
    assert(DelegationTestConstraint::s_instanceCount == 5);
    ENGINE->propagate();
    assert(DelegationTestConstraint::s_executionCount == 11);

    // Force propagation and confirm only one instance executes
    v0.specify(IntervalIntDomain(100, 900));
    ENGINE->propagate();
    assert(DelegationTestConstraint::s_executionCount == 12);

    // Now confirm correct handling of constraint deletions
    delete (Constraint*) c5;
    delete (Constraint*) c4;
    delete (Constraint*) c3;
    delete (Constraint*) c2;
    delete (Constraint*) c0;
    assert(DelegationTestConstraint::s_instanceCount == 0);
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
    assert(ENGINE->pending());
    assert(ENGINE->propagate());

    dom0.remove(2);
    v0.specify(dom0);
    assert(!ENGINE->pending()); // No propagation required

    v1.specify(3);
    assert(ENGINE->pending());
    assert(ENGINE->propagate());
    assert(v0.getDerivedDomain().getSingletonValue() == 1);
    assert(v2.getDerivedDomain().getSingletonValue() == 2);

    v0.reset();
    assert(ENGINE->pending());
    assert(ENGINE->propagate());
    assert(v0.getDerivedDomain() == v2.getDerivedDomain());
    return true;
  }

  static bool testMultEqualConstraint(){
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, 1));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(0, 2));
      MultEqualConstraint c0(LabelStr("MultEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      assert(ENGINE->constraintConsistent());
      assert(v0.getDerivedDomain() == v2.getDerivedDomain());
    }

    // Now test with 0 valued denominators and infinites
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(0, PLUS_INFINITY));
      Variable<IntervalDomain> v1(ENGINE, IntervalDomain(1, PLUS_INFINITY));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(MINUS_INFINITY, 6));
      MultEqualConstraint c0(LabelStr("MultEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      assert(ENGINE->constraintConsistent());
      assert(v0.getDerivedDomain().getUpperBound() == 6);
      assert(v2.getDerivedDomain().getLowerBound() == 0);
      assert(v1.getDerivedDomain().getUpperBound() == PLUS_INFINITY);
    }


    // Special case of negative values on LHS
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(-4, 10));
      Variable<IntervalDomain> v1(ENGINE, IntervalDomain(1, 10));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain());
      MultEqualConstraint c0(LabelStr("MultEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      assert(ENGINE->constraintConsistent());
      assert(v2.getDerivedDomain().getLowerBound() == -40);
    }

    return true;
  }

  static bool testAddMultEqualConstraint(){
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, 40));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(MINUS_INFINITY, 2));
      Variable<IntervalIntDomain> v3(ENGINE, IntervalIntDomain(-2, 6));
      AddMultEqualConstraint c0(LabelStr("AddMultEqualConstraint"), 
				LabelStr("Default"), 
				ENGINE, 
				makeScope(v0.getId(), v1.getId(), v2.getId(), v3.getId()));
      ENGINE->propagate();
      assert(ENGINE->constraintConsistent());
      assert(v2.getDerivedDomain().getLowerBound() == -1);
    }

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
    assert(v0.getDerivedDomain().getSingletonValue() == 1);
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
    assert(g0.getGraphCount() == 1);
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
    assert(g0.getGraphCount() == 1);
    int graphKey = g0.getGraphKey(v0.getId());
    assert(g0.getGraphKey(v1.getId()) == graphKey);
    assert(g0.getGraphKey(v2.getId()) == graphKey);

    Variable<IntervalIntDomain> v3(ENGINE, IntervalIntDomain(1, 100));
    Variable<IntervalIntDomain> v4(ENGINE, IntervalIntDomain(-100, 100));
    g0.addConnection(v2.getId(), v3.getId());
    g0.addConnection(v3.getId(), v4.getId());
    assert(g0.getGraphCount() == 1);
    assert(graphKey != g0.getGraphKey(v0.getId())); // Should have updated for all
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
    assert(g0.getGraphCount() == 1);

    // Cause a split by removing a connection in the middle
    g0.removeConnection(v3.getId(), v4.getId());
    assert(g0.getGraphCount() == 2);

    // Cause another split
    g0.removeConnection(v5.getId(), v6.getId());
    assert(g0.getGraphCount() == 3);

    // Test membership of resulting classes
    assert((g0.getGraphKey(v1.getId()) + g0.getGraphKey(v2.getId()) + g0.getGraphKey(v3.getId()))/3 == g0.getGraphKey(v0.getId()));
    assert(g0.getGraphKey(v4.getId()) == g0.getGraphKey(v5.getId()));
    assert((g0.getGraphKey(v6.getId()) + g0.getGraphKey(v7.getId()))/2   == g0.getGraphKey(v8.getId()));

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
    assert(g0.getGraphCount() == 3);
    assert((g0.getGraphKey(v1.getId()) + g0.getGraphKey(v2.getId()) + g0.getGraphKey(v3.getId()))/3 == g0.getGraphKey(v0.getId()));
    assert(g0.getGraphKey(v4.getId()) == g0.getGraphKey(v5.getId()));
    assert((g0.getGraphKey(v6.getId()) + g0.getGraphKey(v7.getId()))/2   == g0.getGraphKey(v8.getId()));

    // Add connectionto cause a merge
    g0.addConnection(v3.getId(), v4.getId());
    assert(g0.getGraphCount() == 2);
    assert((g0.getGraphKey(v1.getId()) + g0.getGraphKey(v2.getId()) + g0.getGraphKey(v3.getId()))/3 == g0.getGraphKey(v0.getId()));
    assert((g0.getGraphKey(v4.getId()) + g0.getGraphKey(v5.getId()))/2 == g0.getGraphKey(v0.getId()));
    assert((g0.getGraphKey(v6.getId()) + g0.getGraphKey(v7.getId()))/2   == g0.getGraphKey(v8.getId()));


    // Add connectionto cause a merge
    g0.addConnection(v5.getId(), v6.getId());
    assert(g0.getGraphCount() == 1);

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

      assert(v0.getDerivedDomain().getUpperBound() == 10);
      assert(v2.getDerivedDomain().getSingletonValue() == 10);

      variables.clear();
      variables.push_back(v3.getId());
      variables.push_back(v1.getId());
      EqualConstraint c2(LabelStr("EqualConstraint"), LabelStr("EquivalenceClass"), ce, variables);

      ce->propagate();
      assert(ce->constraintConsistent());
      assert(v0.getDerivedDomain().getSingletonValue() == 10);

      variables.clear();
      Variable<IntervalIntDomain> v4(ce, IntervalIntDomain(1, 9));
      variables.push_back(v3.getId());
      variables.push_back(v4.getId());
      ConstraintId c3((new EqualConstraint(LabelStr("EqualConstraint"), LabelStr("EquivalenceClass"), ce, variables))->getId());
      ce->propagate();
      assert(ce->provenInconsistent());

      delete (Constraint*) c3;
      assert(ce->pending());
      ce->propagate();
      assert(ce->constraintConsistent());
      assert(v0.getDerivedDomain().getSingletonValue() == 10);
    }
    delete (ConstraintEngine*) ce;
    return true;
  }
};

int main()
{
  initConstraintLibrary();
  REGISTER_UNARY(DelegationTestConstraint, "TestOnly", "Default");
  DomainTests::test();
  runTestSuite(VariableTest::test); 
  runTestSuite(ConstraintTest::test); 
  runTestSuite(FactoryTest::test);
  runTestSuite(EquivalenceClassTest::test);
  cout << "Finished" << endl;
}
