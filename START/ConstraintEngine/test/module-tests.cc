#include "ConstraintNetwork.hh"
#include "AbstractVar.hh"
#include "ConstraintFactory.hh"
#include "ConstraintLibrary.hh"

#include <iostream>
#include <cassert>

using namespace Prototype;
using namespace std;

#define runTest(test, name) { \
  cout << name; \
  if(test()) \
    cout << " passed." << endl; \
  else \
    cout << " failed." << endl; \
}

#define runTestSuite(test, name) { \
  cout << name << "***************" << endl; \
  if(test()) \
    cout << name << " passed." << endl; \
  else \
    cout << name << " failed." << endl; \
}


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

class LabelTest
{
public:
  static bool test(){
    testBasicLabelOperations();
    //runTest(testBasicLabelOperations, "BasicLabelOperations");
    runTest(testLabelSetAllocations, "LabelSetAllocations");
    runTest(testEquate, "LabelSet::equate");
    return true;
  }
private:
  static bool testBasicLabelOperations() {
    Prototype::LabelStr l1("L1");
    Prototype::LabelStr l2("L2");
    Prototype::LabelStr l3("L3");
    assert(l1.getKey() < l2.getKey() && l2.getKey() < l3.getKey());

    Prototype::LabelStr la("L");
    Prototype::LabelStr l4("L30");
    Prototype::LabelStr lb("L");
    assert(la.getKey() == lb.getKey());
    assert(la.getKey() < l1.getKey());
    assert(l4.getKey() > l3.getKey());

    assert(l1 < l2);
    assert(la == lb);

    Prototype::LabelStr copy1(l1);
    assert(l1 == copy1);
    assert (l2 != copy1);

    assert(Prototype::LabelStr::getSize() == 5);

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
    assert(ls0.getSize() == 5);

    Prototype::LabelStr l2("L2");
    assert(ls0.isMember(l2));
    DomainListener::ChangeType change;
    ls0.remove(l2);
    assert(l_listener.checkAndClearChange(change) && change == DomainListener::VALUE_REMOVED);
    assert(!ls0.isMember(l2));
    ls0.insert(l2);
    assert(l_listener.checkAndClearChange(change) && change == DomainListener::RELAXED);
    assert(ls0.isMember(l2));

    ls0.setToSingleton(l2);
    assert(ls0.isMember(l2));
    assert(ls0.getSize() == 1);
    return true;
  }
  static bool testEquate(){
    std::list<Prototype::LabelStr> values;
    values.push_back(Prototype::LabelStr("A"));
    values.push_back(Prototype::LabelStr("B"));
    values.push_back(Prototype::LabelStr("C"));
    values.push_back(Prototype::LabelStr("D"));
    values.push_back(Prototype::LabelStr("E"));

    ChangeListener l_listener;
    LabelSet ls0(values, true, l_listener.getId());
    LabelSet ls1(values, true, l_listener.getId());

    assert(ls0 == ls1);
    assert(ls0.getSize() == 5);
    assert(ls0.equate(ls1) == false); // Implying no change occured

    Prototype::LabelStr lC("C");
    ls0.remove(lC);
    assert(!ls0.isMember(lC));
    assert(ls1.isMember(lC));
    assert(ls0.equate(ls1)); // It should have changed
    assert(!ls1.isMember(lC));


    LabelSet ls2(values, true, l_listener.getId());
    values.push_back(Prototype::LabelStr("F"));
    values.push_back(Prototype::LabelStr("G"));
    values.push_back(Prototype::LabelStr("H"));
    LabelSet ls3(values, true, l_listener.getId());
    Prototype::LabelStr lA("A");
    Prototype::LabelStr lB("B");
    ls3.remove(lA);
    ls3.remove(lB);
    ls3.remove(lC);
    assert(ls2.equate(ls3));
    assert(ls2 == ls3);
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
    intDomain.empty();
    assert(intDomain.isEmpty());

    AbstractDomain& dom = static_cast<AbstractDomain&>(intDomain);
    assert(dom.isEmpty());

    IntervalIntDomain copied(static_cast<IntervalIntDomain&>(dom));
    IntervalIntDomain dom1;
    assert( ! (dom1 == copied));
    dom1 = copied;
    assert(dom1 == copied);
    return true;
  }

  static bool testAssignment(){
    ChangeListener l_listener;
    IntervalIntDomain dom0(l_listener.getId()); // Will have very large default range
    IntervalIntDomain dom1(-100, 100);
    dom0 = dom1;
    DomainListener::ChangeType change;
    assert(!l_listener.checkAndClearChange(change)); // No change event generated
    IntervalIntDomain dom2(-300, 100);
    dom0 = dom2;
    assert(l_listener.checkAndClearChange(change) && change == DomainListener::RELAXED);
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
    VariableImpl<IntervalIntDomain> v0(dom0);
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
    runTest(testAddEqualConstraint, "AddEqualConstraint");
    runTest(testEqualConstraint, "EqualConstraint");
    runTest(testBasicPropagation, "BasicPropagation");
    return true;
  }

private:

  static bool testAddEqualConstraint()
  {
    std::vector<ConstrainedVariableId> variables;
    VariableImpl<IntervalIntDomain> v0(IntervalIntDomain(1, 10));
    variables.push_back(v0.forConstraint());
    VariableImpl<IntervalIntDomain> v1(IntervalIntDomain(1, 1));
    variables.push_back(v1.forConstraint());
    VariableImpl<IntervalIntDomain> v2(IntervalIntDomain(0, 2));
    variables.push_back(v2.forConstraint());
    AddEqualConstraint c0(variables);
    assert(c0.execute());
    assert(v0.getDerivedDomain().getSingletonValue() == 1);
    assert(v1.getDerivedDomain().getSingletonValue() == 1);
    assert(v2.getDerivedDomain().getSingletonValue() == 2);
    return true;
  }

  static bool testEqualConstraint()
  {
    std::vector<ConstrainedVariableId> variables;
    VariableImpl<IntervalIntDomain> v0(IntervalIntDomain(1, 10));
    variables.push_back(v0.forConstraint());
    VariableImpl<IntervalIntDomain> v1(IntervalIntDomain(-100, 1));
    variables.push_back(v1.forConstraint());
    EqualConstraint c0(variables);
    assert(c0.execute());
    assert(v0.getDerivedDomain().getSingletonValue() == 1);
    assert(v1.getDerivedDomain().getSingletonValue() == 1);
    return true;
  }


  static bool testBasicPropagation(){
    std::vector<ConstrainedVariableId> variables;
    // v0 == v1
    VariableImpl<IntervalIntDomain> v0(IntervalIntDomain(1, 10));
    variables.push_back(v0.forConstraint());
    VariableImpl<IntervalIntDomain> v1(IntervalIntDomain(1, 10));
    variables.push_back(v1.forConstraint());
    EqualConstraint c0(variables);

    // v2 + v3 == v0
    variables.clear();
    VariableImpl<IntervalIntDomain> v2(IntervalIntDomain(1, 4));
    variables.push_back(v2.forConstraint());
    VariableImpl<IntervalIntDomain> v3(IntervalIntDomain(1, 1));
    variables.push_back(v3.forConstraint());
    variables.push_back(v0.forConstraint());
    AddEqualConstraint c1(variables);

    // v4 + v5 == v1
    variables.clear();
    VariableImpl<IntervalIntDomain> v4(IntervalIntDomain(1, 10));
    variables.push_back(v4.forConstraint());
    VariableImpl<IntervalIntDomain> v5(IntervalIntDomain(1, 1000));
    variables.push_back(v5.forConstraint());
    variables.push_back(v1.forConstraint());
    AddEqualConstraint c2(variables);

    ProceduralConstraintPropagator p;
    p.addConstraint(c0.getId());
    p.addConstraint(c1.getId());
    p.addConstraint(c2.getId());
    p.execute();

    cout << "v5.ub = " << v5.getDerivedDomain().getUpperBound() << endl;
    return true;
  }
};

class FactoryTest
{
public:
  static bool test() {
    runTest(test1, "test1");
    return true;
  }

private:
  static bool test1(){
    ConstraintFactory factory("sum:3:int:int:double");
    return true;
  }
};

class IntegrationTest
{
public:
  static bool test() {
    runTest(domainConversion, "domainConversion");
    return true;
  }

private:
  static bool domainConversion(){
    Europa::Domain intDomain(Europa::intSort, 10, 20);
    IntervalIntDomain dom0(intDomain);
    Europa::Domain intDomain2 = dom0.makeDomain();
    assert(intDomain == intDomain2);
    assert(intDomain.isFinite());
    IntervalIntDomain dom1(intDomain2);
    assert(dom0 == dom1);
    return true;
  }
};

int main()
{
  runTestSuite(LabelTest::test, "LabelTests"); 
  runTestSuite(DomainTest::test, "DomainTests");  
  runTestSuite(VariableTest::test, "VariableTests"); 
  runTestSuite(ConstraintTest::test, "ConstraintTests"); 
  runTestSuite(FactoryTest::test, "FactoryTests");  
  runTestSuite(IntegrationTest::test, "IntegrationTests"); 
  cout << "Finished" << endl;
}
