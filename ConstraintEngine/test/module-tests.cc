/**
 * @file module-tests.cc
 * @author Conor McGann
 * @date August, 2003
 * @brief Read the source for details
 */
#include "TestSupport.hh"
#include "Variable.hh"
#include "Constraints.hh"
#include "ConstraintLibrary.hh"
#include "../Libraries/IdTable.hh"
#include "EquivalenceClassCollection.hh"
#include "EqualityConstraintPropagator.hh"

/* Include for domain management */
#include "AbstractDomain.hh"
#include "EnumeratedDomain.hh"
#include "LabelSet.hh"
#include "LabelStr.hh"
#include "IntervalIntDomain.hh"
#include "IntervalRealDomain.hh"

#include <iostream>
#include <cassert>
#include <bitset>
#include <vector>

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
  static int s_executionCount;
  static int s_instanceCount;
};

int DelegationTestConstraint::s_executionCount = 0;
int DelegationTestConstraint::s_instanceCount = 0;

class ChangeListener: public DomainListener {
public:
  ChangeListener(): m_change(NO_CHANGE){}

  void notifyChange(const ChangeType& change){
    m_change = change;
  }

  bool checkAndClearChange(ChangeType& change) {
    bool hasChanged = m_change != NO_CHANGE;
    change = m_change;
    m_change = NO_CHANGE;
    return (change != NO_CHANGE);
  }
private:
  bool m_changed;
  ChangeType m_change;
};

class EnumerationTest
{
public:
  static bool test(){
    runTest(testBasicLabelOperations, "BasicLabelOperations");
    runTest(testLabelSetAllocations, "LabelSetAllocations");
    runTest(testEquate, "LabelSet::equate");
    runTest(testValueRetrieval, "ValueRetrieval");
    runTest(testIntersection, "Intersection");
    runTest(testEnumerationOnly, "EnumerationOnly");
    return true;
  }
private:
  static bool testBasicLabelOperations() {
    int initialCount = Prototype::LabelStr::getSize();
    Prototype::LabelStr l1("L1");
    Prototype::LabelStr l2("L2");
    Prototype::LabelStr l3("L3");
    assert(l1 < l2 && l2 < l3);

    Prototype::LabelStr la("L");
    Prototype::LabelStr l4("L30");
    Prototype::LabelStr lb("L");
    assert(la == lb);
    assert(la < l1);
    assert(l4 > l3);

    assert(l1 < l2);
    assert(la == lb);

    Prototype::LabelStr copy1(l1);
    assert(l1 == copy1);
    assert (l2 != copy1);

    assert((Prototype::LabelStr::getSize() - initialCount) == 5);
    assert(l1.toString() == "L1");
    return true;
  }

  static bool testLabelSetAllocations(){
    std::list<Prototype::LabelStr> values;
    values.push_back(Prototype::LabelStr("L1"));
    values.push_back(Prototype::LabelStr("L4"));
    values.push_back(Prototype::LabelStr("L2"));
    values.push_back(Prototype::LabelStr("L5"));
    values.push_back(Prototype::LabelStr("L3"));

    ChangeListener l_listener;
    LabelSet ls0(values, true, l_listener.getId());
    assert(!ls0.isDynamic());

    Prototype::LabelStr l2("L2");
    assert(ls0.isMember(l2));
    DomainListener::ChangeType change;
    ls0.remove(l2);
    assert(l_listener.checkAndClearChange(change) && change == DomainListener::VALUE_REMOVED);
    assert(!ls0.isMember(l2));

    Prototype::LabelStr l3("L3");
    ls0.setToSingleton(l3);
    assert(ls0.isMember(l3));
    assert(ls0.getSize() == 1);

    LabelSet ls1(values, true);
    ls0 = ls1;
    assert(l_listener.checkAndClearChange(change) && change == DomainListener::RELAXED);
    assert(ls0 == ls1);
    return true;
  }
  static bool testEquate(){
    std::list<Prototype::LabelStr> baseValues;
    baseValues.push_back(Prototype::LabelStr("A"));
    baseValues.push_back(Prototype::LabelStr("B"));
    baseValues.push_back(Prototype::LabelStr("C"));
    baseValues.push_back(Prototype::LabelStr("D"));
    baseValues.push_back(Prototype::LabelStr("E"));
    baseValues.push_back(Prototype::LabelStr("F"));
    baseValues.push_back(Prototype::LabelStr("G"));
    baseValues.push_back(Prototype::LabelStr("H"));

    ChangeListener l_listener;
    LabelSet ls0(baseValues, true, l_listener.getId());
    LabelSet ls1(baseValues, true, l_listener.getId());

    assert(ls0 == ls1);
    assert(ls0.getSize() == 8);
    assert(ls0.equate(ls1) == false); // Implying no change occured

    Prototype::LabelStr lC("C");
    ls0.remove(lC);
    assert(!ls0.isMember(lC));
    assert(ls1.isMember(lC));
    assert(ls0.equate(ls1)); // It should have changed
    assert(!ls1.isMember(lC));

    LabelSet ls2(baseValues, true, l_listener.getId());
    ls2.remove(Prototype::LabelStr("A"));
    ls2.remove(Prototype::LabelStr("B"));
    ls2.remove(Prototype::LabelStr("C"));
    ls2.remove(Prototype::LabelStr("D"));
    ls2.remove(Prototype::LabelStr("E"));

    LabelSet ls3(baseValues, true, l_listener.getId());
    Prototype::LabelStr lA("A");
    Prototype::LabelStr lB("B");
    ls3.remove(lA);
    ls3.remove(lB);
    ls3.remove(lC);
    assert(ls2.equate(ls3));
    assert(ls2 == ls3);

    LabelSet ls4(baseValues, true, l_listener.getId());
    ls4.remove(Prototype::LabelStr("A"));
    ls4.remove(Prototype::LabelStr("B"));
    ls4.remove(Prototype::LabelStr("C"));
    ls4.remove(Prototype::LabelStr("D"));
    ls4.remove(Prototype::LabelStr("E"));

    LabelSet ls5(baseValues, true, l_listener.getId());
    ls5.remove(Prototype::LabelStr("F"));
    ls5.remove(Prototype::LabelStr("G"));
    ls5.remove(Prototype::LabelStr("H"));

    DomainListener::ChangeType change;
    ls4.equate(ls5);
    assert(l_listener.checkAndClearChange(change) && change == DomainListener::EMPTIED);
    assert(ls4.isEmpty() || ls5.isEmpty());
    assert(!(ls4.isEmpty() && ls5.isEmpty()));

    return true;
  }

  static bool testValueRetrieval(){
    std::list<Prototype::LabelStr> values;
    values.push_back(Prototype::LabelStr("A"));
    values.push_back(Prototype::LabelStr("B"));
    values.push_back(Prototype::LabelStr("C"));
    values.push_back(Prototype::LabelStr("D"));
    values.push_back(Prototype::LabelStr("E"));

    LabelSet l1(values, true);
    std::list<Prototype::LabelStr> results;
    l1.getValues(results);

    LabelSet l2(results, true);

    assert(l1 == l2);
    LabelStr lbl("C");
    l1.setToSingleton(lbl);
    assert(lbl == l1.getSingletonValue());
    return true;
  }

  static bool testIntersection(){
    std::list<Prototype::LabelStr> values;
    values.push_back(Prototype::LabelStr("A"));
    values.push_back(Prototype::LabelStr("B"));
    values.push_back(Prototype::LabelStr("C"));
    values.push_back(Prototype::LabelStr("D"));
    values.push_back(Prototype::LabelStr("E"));
    values.push_back(Prototype::LabelStr("F"));
    values.push_back(Prototype::LabelStr("G"));
    values.push_back(Prototype::LabelStr("H"));
    values.push_back(Prototype::LabelStr("I"));
    LabelSet ls1(values);

    LabelSet ls2(values);
    ls2.remove(Prototype::LabelStr("A"));
    ls2.remove(Prototype::LabelStr("C"));
    ls2.remove(Prototype::LabelStr("E"));
    assert(ls2.isSubsetOf(ls1));
    assert(!ls1.isSubsetOf(ls2));

    LabelSet ls3(ls1);

    ls1.intersect(ls2);
    assert(ls1 == ls2);
    assert(ls2.isSubsetOf(ls1));

    ls1 = ls3;
    assert(ls2.isSubsetOf(ls1));
    assert(ls1 == ls3);

    LabelSet ls4(values);
    ls4.remove(Prototype::LabelStr("A"));
    ls4.remove(Prototype::LabelStr("B"));
    ls4.remove(Prototype::LabelStr("C"));
    ls4.remove(Prototype::LabelStr("D"));
    ls4.remove(Prototype::LabelStr("E"));
    ls4.remove(Prototype::LabelStr("F"));
    ls4.remove(Prototype::LabelStr("G"));

    ls3.remove(Prototype::LabelStr("H"));
    ls3.remove(Prototype::LabelStr("I"));
    ls4.intersect(ls3);
    assert(ls4.isEmpty());
    return true;
  }

  /**
   * Note that the enumeration is pretty thoroughly tested through the LabelSet tests which build on it
   */
  static bool testEnumerationOnly(){
    std::list<double> values;
    values.push_back(-98.67);
    values.push_back(-0.01);
    values.push_back(1);
    values.push_back(2);
    values.push_back(10);
    values.push_back(11);

    EnumeratedDomain d0(values);
    EnumeratedDomain d1(values);
    assert(d0 == d1);
    assert(d0.isSubsetOf(d1));
    assert(d0.isMember(-98.67));
    d0.remove(-0.01);
    assert(!d0.isMember(-0.01));
    assert(d0.isSubsetOf(d1));
    assert(!d1.isSubsetOf(d0));

    return true;
  }
};

class DomainTest
{
public:
  static bool test() {
    runTest(testAllocation, "Allocation"); 
    runTest(testAssignment, "Assignment"); 
    runTest(testIntersection, "Intersection");  
    runTest(testSubset, "Subset");  
    return true;
  }

private:
  static bool testAllocation(){
    IntervalIntDomain intDomain(10, 20);
    assert(intDomain.isFinite());
    assert(!intDomain.isDynamic());
    IntervalIntDomain d1(intDomain);
    d1.empty();
    assert(d1.isEmpty());

    AbstractDomain& d2 = static_cast<AbstractDomain&>(intDomain);
    assert(!d2.isEmpty());

    IntervalIntDomain d3(static_cast<IntervalIntDomain&>(intDomain));
    IntervalIntDomain d4;
    assert( ! (d3 == d4));
    d3 = d4;
    assert(d3 == d4);
    return true;
  }

  static bool testAssignment(){
    ChangeListener l_listener;
    IntervalIntDomain dom0; // Will have very large default range
    IntervalIntDomain dom1(-100, 100, true, l_listener.getId());
    dom1 = dom0;
    DomainListener::ChangeType change;
    assert(l_listener.checkAndClearChange(change)  && change == DomainListener::RELAXED);
    assert(dom1.isSubsetOf(dom0));
    assert(dom0.isSubsetOf(dom1));
    assert(dom1 == dom0);

    IntervalIntDomain dom2(-300, 100);
    dom1.intersect(dom2);
    assert(l_listener.checkAndClearChange(change));
    assert(dom1 == dom2);
    dom1 = dom2;
    assert(!l_listener.checkAndClearChange(change));
    return true;
  }

  static bool testIntersection() {
    ChangeListener l_listener;
    IntervalIntDomain dom0(l_listener.getId()); // Will have very large default range

    // Execute intersection and verify results
    IntervalIntDomain dom1(-100, 100);
    dom0.intersect(dom1);
    DomainListener::ChangeType change;
    assert(l_listener.checkAndClearChange(change));
    assert(dom0 == dom1);
    
    // verify no change triggered if none should take place.
    dom0.intersect(dom1);
    assert(!l_listener.checkAndClearChange(change));

    // Verify only the upper bound changes
    IntervalIntDomain dom2(-200, 50);
    dom0.intersect(dom2);
    assert(l_listener.checkAndClearChange(change));
    assert(dom0.getLowerBound() == dom1.getLowerBound());
    assert(dom0.getUpperBound() == dom2.getUpperBound());
    
    // Make an intersection that leads to an empty domain
    IntervalIntDomain dom3(500, 1000);
    dom0.intersect(dom3);
    assert(l_listener.checkAndClearChange(change));
    assert(dom0.isEmpty());
    return true;
  }

  static bool testSubset(){
    IntervalRealDomain dom0(9.87, 34.54);
    IntervalRealDomain dom1(0, 100);
    assert(dom0.isSubsetOf(dom1));
    assert(! dom1.isSubsetOf(dom0));

    // Handle cases where domains are equal
    IntervalRealDomain dom2(dom0);
    assert(dom2 == dom0);
    assert(dom0.isSubsetOf(dom2));
    assert(dom2.isSubsetOf(dom0));

    // Handle case with no intersection
    IntervalRealDomain dom3(0, 9.8);
    assert(! dom3.isSubsetOf(dom0));
    assert(! dom0.isSubsetOf(dom3));

    // Handle case with partial intersection
    IntervalRealDomain dom4(0, 20);
    assert(! dom4.isSubsetOf(dom0));
    assert(! dom0.isSubsetOf(dom4));

    return true;
  }

  static bool testListener()
  {

    return true;
  }
};

class VariableTest
{
public:
  static bool test() {
    runTest(testAllocation, "Allocation");
    return true;
  }

private:
  static bool testAllocation(){
    IntervalIntDomain dom0(0, 1000);
    Variable<IntervalIntDomain> v0(ENGINE, dom0);
    const IntervalIntDomain& dom1 = v0.getBaseDomain();
    assert (dom0 == dom1);
    assert(v0.isValid());
    return true;
  }
};

class ConstraintTest
{
public:
  static bool test() {
    runTest(testAddEqualConstraint, "SubsetOfConstraint");
    runTest(testAddEqualConstraint, "AddEqualConstraint");
    runTest(testEqualConstraint, "EqualConstraint");
    runTest(testBasicPropagation, "BasicPropagation");
    runTest(testForceInconsistency, "ForceInconsistency");
    runTest(testRepropagation, "Repropagation");
    runTest(testConstraintRemoval, "ConstraintRemoval");
    runTest(testDelegation, "TestDelegation");
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
    assert(ENGINE->pending());
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
    std::vector<ConstrainedVariableId> variables;
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v0.getId());
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, 1));
    variables.push_back(v1.getId());
    Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(0, 2));
    variables.push_back(v2.getId());
    AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, variables);
    ENGINE->propagate();
    assert(ENGINE->constraintConsistent());
    assert(v0.getDerivedDomain().getSingletonValue() == 1);
    assert(v1.getDerivedDomain().getSingletonValue() == 1);
    assert(v2.getDerivedDomain().getSingletonValue() == 2);
    return true;
  }

  static bool testEqualConstraint()
  {
    std::vector<ConstrainedVariableId> variables;
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v0.getId());
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(-100, 1));
    variables.push_back(v1.getId());
    EqualConstraint c0(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, variables);
    ENGINE->propagate();
    assert(ENGINE->constraintConsistent());
    assert(v0.getDerivedDomain().getSingletonValue() == 1);
    assert(v1.getDerivedDomain().getSingletonValue() == 1);


    std::list<Prototype::LabelStr> values;
    values.push_back(Prototype::LabelStr("A"));
    LabelSet ls0(values);
    values.push_back(Prototype::LabelStr("B"));
    values.push_back(Prototype::LabelStr("C"));
    values.push_back(Prototype::LabelStr("D"));
    values.push_back(Prototype::LabelStr("E"));
    LabelSet ls1(values);


    variables.clear();
    Variable<LabelSet> v2(ENGINE, ls1);
    variables.push_back(v2.getId());
    Variable<LabelSet> v3(ENGINE, ls1);
    variables.push_back(v3.getId());
    EqualConstraint c1(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, variables);
    ENGINE->propagate();
    assert(ENGINE->constraintConsistent());
    assert(v2.getDerivedDomain() == v3.getDerivedDomain());
    assert(!v2.getDerivedDomain().isSingleton());

    values.pop_back();
    LabelSet ls2(values);
    v2.specify(ls2);
    ENGINE->propagate();
    assert(!v3.getDerivedDomain().isMember(Prototype::LabelStr("E")));

    variables.clear();
    Variable<LabelSet> v4(ENGINE, ls0);
    variables.push_back(v2.getId());
    variables.push_back(v4.getId());
    EqualConstraint c2(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, variables);
    ENGINE->propagate();
    assert(ENGINE->constraintConsistent());
    assert(v2.getDerivedDomain() == v3.getDerivedDomain());
    assert(v2.getDerivedDomain() == v4.getDerivedDomain());
    assert(v3.getDerivedDomain() == v4.getDerivedDomain());
    assert(v3.getDerivedDomain().getSingletonValue() == Prototype::LabelStr("A"));

    return true;
  }

  static bool testBasicPropagation(){
    std::vector<ConstrainedVariableId> variables;
    // v0 == v1
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v0.getId());
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v1.getId());
    EqualConstraint c0(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, variables);

    // v2 + v3 == v0
    variables.clear();
    Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(1, 4));
    variables.push_back(v2.getId());
    Variable<IntervalIntDomain> v3(ENGINE, IntervalIntDomain(1, 1));
    variables.push_back(v3.getId());
    variables.push_back(v0.getId());
    AddEqualConstraint c1(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, variables);
    assert(!v0.getDerivedDomain().isEmpty());

    // v4 + v5 == v1
    variables.clear();
    Variable<IntervalIntDomain> v4(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v4.getId());
    Variable<IntervalIntDomain> v5(ENGINE, IntervalIntDomain(1, 1000));
    variables.push_back(v5.getId());
    variables.push_back(v1.getId());
    AddEqualConstraint c2(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, variables);

    ENGINE->propagate();
    assert(ENGINE->constraintConsistent());
    assert(!v4.getDerivedDomain().isEmpty());
    return true;
  }

  static bool testForceInconsistency(){
    std::vector<ConstrainedVariableId> variables;
    // v0 == v1
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v0.getId());
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v1.getId());
    EqualConstraint c0(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, variables);
    
    // v2 + v3 == v0
    variables.clear();
    Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(1, 1));
    variables.push_back(v2.getId());
    Variable<IntervalIntDomain> v3(ENGINE, IntervalIntDomain(1, 1));
    variables.push_back(v3.getId());
    variables.push_back(v0.getId());
    AddEqualConstraint c1(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, variables);

    // v4 + v5 == v1
    variables.clear();
    Variable<IntervalIntDomain> v4(ENGINE, IntervalIntDomain(2, 2));
    variables.push_back(v4.getId());
    Variable<IntervalIntDomain> v5(ENGINE, IntervalIntDomain(2, 2));
    variables.push_back(v5.getId());
    variables.push_back(v1.getId());
    AddEqualConstraint c2(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, variables);
    
    ENGINE->propagate();
    assert(ENGINE->provenInconsistent());
    assert(v1.getDerivedDomain().isEmpty());
    assert(v2.getDerivedDomain().isEmpty());

    variables.clear();
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
    std::vector<ConstrainedVariableId> variables;
    // v0 == v1
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v0.getId());
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v1.getId());
    EqualConstraint c0(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, variables);


    // v2 + v3 == v0
    variables.clear();
    Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v2.getId());
    Variable<IntervalIntDomain> v3(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v3.getId());
    variables.push_back(v0.getId());
    AddEqualConstraint c1(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, variables);

    // v4 + v5 == v1
    variables.clear();
    Variable<IntervalIntDomain> v4(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v4.getId());
    Variable<IntervalIntDomain> v5(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v5.getId());
    variables.push_back(v1.getId());
    AddEqualConstraint c2(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, variables);

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

    // Cause a change in the domain which will impoact agenda, then deactivate a constraint and verify the correct execution count
    v0.specify(IntervalIntDomain(0, 900));
    c0->deactivate(c1);
    assert(!c0->isActive());
    ENGINE->propagate();
    assert(ENGINE->constraintConsistent());
    assert(DelegationTestConstraint::s_instanceCount == 5);
    assert(DelegationTestConstraint::s_executionCount == 9);

    // Delete the delegate and verify instance counts and that the prior delegate has been reinstated and executed.
    delete (Constraint*) c1;
    assert(c0->isActive());
    ENGINE->propagate();
    assert(ENGINE->constraintConsistent());
    assert(DelegationTestConstraint::s_instanceCount == 4);
    assert(DelegationTestConstraint::s_executionCount == 10);

    // Now create a new instance and mark it for delegation only. Add remaining constraints as delegates
    ConstraintId c5 = ConstraintLibrary::createConstraint(LabelStr("TestOnly"), ENGINE, v0.getId(), IntervalIntDomain(0,0));
    c5->markForDelegationOnly();
    c0->deactivate(c5);
    c2->deactivate(c5);
    c3->deactivate(c5);
    c4->deactivate(c5);
    assert(DelegationTestConstraint::s_instanceCount == 5);
    ENGINE->propagate();
    assert(DelegationTestConstraint::s_executionCount == 11);

    // Force propagation and confirm only one instance executes
    v0.specify(IntervalIntDomain(100, 900));
    ENGINE->propagate();
    assert(DelegationTestConstraint::s_executionCount == 12);

    // Now confirm correct handling of constraint deletions
    delete (Constraint*) c4;
    delete (Constraint*) c3;
    delete (Constraint*) c2;
    assert(DelegationTestConstraint::s_instanceCount == 2);
    delete (Constraint*) c0;
    assert(DelegationTestConstraint::s_instanceCount == 0);
    return true;
  }
};

class FactoryTest
{
public:
  static bool test() {
    runTest(testAllocation, "testAllocation");
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
    runTest(testBasicAllocation, "BasicAllocation");
    runTest(testConstructionOfSingleGraph, "ConstructionOfSingleGraph");
    runTest(testSplittingOfSingleGraph, "SplittingOfSingleGraph");
    runTest(testMultiGraphMerging, "MultiGraphMerging");
    runTest(testEqualityConstraintPropagator, "EqualityConstraintPropagator");
    return true;
  }

private:
  static bool testBasicAllocation(){
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(2, 8));
    EquivalenceClassCollection g0;
    g0.addConnection(v0.getId(), v1.getId());
    assert(g0.getGraphCount() == 1);
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

void testBitVector(){
  bitset<20> bitvec;
  bitvec.set();
  assert(bitvec.any());
  bitvec.flip();
  assert(bitvec.none());
  bitvec.set(8);
  assert(bitvec.any());
  cout << "BitVector Test Passed" << endl;
}

int main()
{
  initConstraintLibrary();
  REGISTER_UNARY(DelegationTestConstraint, "TestOnly", "Default");
  runTestSuite(EnumerationTest::test, "LabelTests"); 
  runTestSuite(DomainTest::test, "DomainTests");  
  runTestSuite(VariableTest::test, "VariableTests"); 
  runTestSuite(ConstraintTest::test, "ConstraintTests"); 
  runTestSuite(FactoryTest::test, "FactoryTests");
  runTestSuite(EquivalenceClassTest::test, "EquivalenceClassTests");
  testBitVector();
  cout << "Finished" << endl;
}
