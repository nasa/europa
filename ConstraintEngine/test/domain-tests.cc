#include "TestSupport.hh"
#include "IntervalIntDomain.hh"
#include "IntervalRealDomain.hh"
#include "BoolDomain.hh"
#include "EnumeratedDomain.hh"
#include "LabelSet.hh"
#include "LabelStr.hh"
#include "DomainListener.hh"
#include "domain-tests.hh"

#include <sstream>

class ChangeListener: public DomainListener {
public:
  ChangeListener(): m_changed(false), m_change(RESET){}

  void notifyChange(const ChangeType& change){
    m_changed = true;
    m_change = change;
  }

  bool checkAndClearChange(ChangeType& change) {
    bool result = m_changed;
    change = m_change;
    m_changed = false;
    return (result);
  }

private:
  bool m_changed;
  ChangeType m_change;
};

class IntervalDomainTest
{
public:
  static bool test() {
    runTest(testAllocation); 
    runTest(testRelaxation); 
    runTest(testIntersection);  
    runTest(testSubset);
    runTest(testPrinting);
    runTest(testBoolDomain);
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
    d3.relax(d4);
    assert(d3 == d4);

    assert(d2 != d4);
    d2.relax(d4);
    assert(d2 == d4);
    return true;
  }

  static bool testRelaxation(){
    ChangeListener l_listener;
    IntervalIntDomain dom0; // Will have very large default range
    IntervalIntDomain dom1(-100, 100, true, l_listener.getId());
    dom1.relax(dom0);
    DomainListener::ChangeType change;
    assert(l_listener.checkAndClearChange(change)  && change == DomainListener::RELAXED);
    assert(dom1.isSubsetOf(dom0));
    assert(dom0.isSubsetOf(dom1));
    assert(dom1 == dom0);

    IntervalIntDomain dom2(-300, 100);
    dom1.intersect(dom2);
    assert(l_listener.checkAndClearChange(change));
    assert(dom1 == dom2);
    dom1.relax(dom2);
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

    IntervalRealDomain dom4(0.98, 101.23);
    IntervalRealDomain dom5(80, 120.44);
    IntervalRealDomain dom6(80, 101.23);
    dom4.equate(dom5);
    assert(dom4 == dom6);
    assert(dom5 == dom6);
    return true;
  }

  static bool testSubset(){
    IntervalIntDomain dom0(10, 35);
    IntervalRealDomain dom1(0, 101);
    assert(dom0.isSubsetOf(dom1));
    assert(! dom1.isSubsetOf(dom0));

    // Handle cases where domains are equal
    IntervalIntDomain dom2(dom0);
    assert(dom2 == dom0);
    assert(dom0.isSubsetOf(dom2));
    assert(dom2.isSubsetOf(dom0));

    // Handle case with no intersection
    IntervalIntDomain dom3(0, 9);
    assert(! dom3.isSubsetOf(dom0));
    assert(! dom0.isSubsetOf(dom3));

    // Handle case with partial intersection
    IntervalIntDomain dom4(0, 20);
    assert(! dom4.isSubsetOf(dom0));
    assert(! dom0.isSubsetOf(dom4));

    return true;
  }

  static bool testListener()
  {
    return true;
  }

  static bool testPrinting(){
    IntervalIntDomain d1(1, 100);
    std::stringstream ss1;
    d1 >> ss1;
    std::string actualString = ss1.str();
    std::string expectedString("INT_INTERVAL:CLOSED[1, 100]");
    assert(actualString == expectedString);
    return true;
  }

  static bool testBoolDomain(){
    BoolDomain dom0;
    assert(dom0.getSize() == 2);
    assert(dom0.getUpperBound() == true);
    assert(dom0.getLowerBound() == false);

    IntervalIntDomain dom1(0, 100);
    dom1.intersect(dom0);
    assert(dom1 == dom0);
    return true;
  }
};

class EnumeratedDomainTest{
public:
  static bool test(){
    runTest(testEnumerationOnly);
    runTest(testBasicLabelOperations);
    runTest(testLabelSetAllocations);
    runTest(testEquate);
    runTest(testValueRetrieval);
    runTest(testIntersection);
    return true;
  }
private:

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
    ls0.set(l3);
    assert(ls0.isMember(l3));
    assert(ls0.getSize() == 1);

    LabelSet ls1(values, true);
    ls0.relax(ls1);
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
    l1.set(lbl);
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

    ls1.relax(ls3);
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
};

class MixedTypeTest{
public:
  static bool test() {
    runTest(testEquality);
    runTest(testIntersection);
    runTest(testSubset);
    return true;
  }
private:
  static bool testEquality(){
    std::list<double> emptyList;
    EnumeratedDomain dom(emptyList, false);
    dom.insert(1.0);
    dom.insert(2.0);
    dom.close();

    EnumeratedDomain dom0(dom);
    dom0.set(1.0);

    IntervalRealDomain dom1(1.0, 1.0);
    assert(dom1 == dom0);
    assert(dom0 == dom1);

    IntervalIntDomain dom2(1, 1);
    assert(dom1 == dom2);

    dom0.reset(dom);
    IntervalIntDomain dom3(1, 2);
    assert(dom0 == dom3);
    return true;
  }

  static bool testIntersection(){
    std::list<double> emptyList;
    EnumeratedDomain dom0(emptyList, false);
    dom0.insert(0);
    dom0.insert(0.98);
    dom0.insert(1.0);
    dom0.insert(1.89);
    dom0.insert(2.98);
    dom0.insert(10);
    dom0.close();
    assert(dom0.getSize() == 6);
    IntervalIntDomain dom1(1, 8);
    EnumeratedDomain dom2(dom0);

    dom0.intersect(dom1);
    assert(dom0.getSize() == 1);
    assert(dom0.isMember(1.0));

    IntervalRealDomain dom3(1, 8);
    dom2.intersect(dom3);
    assert(dom2.getSize() == 3);

    BoolDomain dom4;
    dom2.intersect(dom4);
    assert(dom2.getSize() == 1);
    return true;
  }

  static bool testSubset(){
    std::list<double> emptyList;
    EnumeratedDomain dom0(emptyList, false);
    dom0.insert(0);
    dom0.insert(0.98);
    dom0.insert(1.0);
    dom0.insert(1.89);
    dom0.insert(2.98);
    dom0.insert(10);
    dom0.close();

    IntervalRealDomain dom1(0, 10);
    assert(dom0.isSubsetOf(dom1));

    IntervalIntDomain dom2(0, 10);
    assert(!dom0.isSubsetOf(dom2));

    dom0.remove(0.98);
    dom0.remove(1.89);
    dom0.remove(2.98);
    assert(dom0.isSubsetOf(dom2));

    assert(dom2.isSubsetOf(dom1));
    assert(!dom1.isSubsetOf(dom2));
    return true;
  }
};
bool DomainTests::test(){
  runTestSuite(IntervalDomainTest::test);
  runTestSuite(EnumeratedDomainTest::test);
  runTestSuite(MixedTypeTest::test);
  return true;
}
