/**
 * @file module-tests.cc
 * @author Conor McGann
 * @date August, 2003
 * @brief Read the source for details
 */
#include "ConstraintEngine.hh"
#include "AbstractVar.hh"
#include "AbstractDomain.hh"
#include "ConstraintFactory.hh"
#include "ConstraintLibrary.hh"
#include "../Libraries/IdTable.hh"

#include <iostream>
#include <cassert>

using namespace Prototype;
using namespace std;

class DefaultEngineAccessor{
public:
  static const ConstraintEngineId& instance(){
    if (s_instance.isNoId())
      s_instance = (new ConstraintEngine())->getId();

    return s_instance;
  }

  static void reset(){
    if(!s_instance.isNoId()){
      delete (ConstraintEngine*) s_instance;
      s_instance = ConstraintEngineId::noId();
    }
  }

private:
  static ConstraintEngineId s_instance;
};

ConstraintEngineId DefaultEngineAccessor::s_instance;

#define ENGINE DefaultEngineAccessor::instance()

#define runTest(test, name) { \
  cout << "      " << name; \
  bool result = test(); \
  DefaultEngineAccessor::reset(); \
  if(result && Europa::IdTable::size() == 0) \
    cout << " passed." << endl; \
  else \
    cout << " FAILED." << endl; \
}

#define runTestSuite(test, name) { \
  cout << name << "***************" << endl; \
  if(test()) \
    cout << name << " passed." << endl; \
  else \
    cout << name << " FAILED." << endl; \
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
    runTest(testBasicLabelOperations, "BasicLabelOperations");
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
    VariableImpl<IntervalIntDomain> v0(ENGINE, dom0);
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
    runTest(testForceInconsistency, "ForceInconsistency");
    runTest(testRepropagation, "Repropagation");
    runTest(testConstraintRemoval, "ConstraintRemoval");
    return true;
  }

private:

  static bool testAddEqualConstraint()
  {
    std::vector<ConstrainedVariableId> variables;
    VariableImpl<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v0.getId());
    VariableImpl<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, 1));
    variables.push_back(v1.getId());
    VariableImpl<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(0, 2));
    variables.push_back(v2.getId());
    AddEqualConstraint c0(ENGINE, variables);
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
    VariableImpl<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v0.getId());
    VariableImpl<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(-100, 1));
    variables.push_back(v1.getId());
    EqualConstraint c0(ENGINE, variables);
    ENGINE->propagate();
    assert(ENGINE->constraintConsistent());
    assert(v0.getDerivedDomain().getSingletonValue() == 1);
    assert(v1.getDerivedDomain().getSingletonValue() == 1);
    return true;
  }

  static bool testBasicPropagation(){
    std::vector<ConstrainedVariableId> variables;
    // v0 == v1
    VariableImpl<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v0.getId());
    VariableImpl<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v1.getId());
    EqualConstraint c0(ENGINE, variables);

    // v2 + v3 == v0
    variables.clear();
    VariableImpl<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(1, 4));
    variables.push_back(v2.getId());
    VariableImpl<IntervalIntDomain> v3(ENGINE, IntervalIntDomain(1, 1));
    variables.push_back(v3.getId());
    variables.push_back(v0.getId());
    AddEqualConstraint c1(ENGINE, variables);
    assert(!v0.getDerivedDomain().isEmpty());

    // v4 + v5 == v1
    variables.clear();
    VariableImpl<IntervalIntDomain> v4(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v4.getId());
    VariableImpl<IntervalIntDomain> v5(ENGINE, IntervalIntDomain(1, 1000));
    variables.push_back(v5.getId());
    variables.push_back(v1.getId());
    AddEqualConstraint c2(ENGINE, variables);

    ENGINE->propagate();
    assert(ENGINE->constraintConsistent());
    assert(!v4.getDerivedDomain().isEmpty());
    return true;
  }

  static bool testForceInconsistency(){
    std::vector<ConstrainedVariableId> variables;
    // v0 == v1
    VariableImpl<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v0.getId());
    VariableImpl<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v1.getId());
    EqualConstraint c0(ENGINE, variables);
    
    // v2 + v3 == v0
    variables.clear();
    VariableImpl<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(1, 1));
    variables.push_back(v2.getId());
    VariableImpl<IntervalIntDomain> v3(ENGINE, IntervalIntDomain(1, 1));
    variables.push_back(v3.getId());
    variables.push_back(v0.getId());
    AddEqualConstraint c1(ENGINE, variables);

    // v4 + v5 == v1
    variables.clear();
    VariableImpl<IntervalIntDomain> v4(ENGINE, IntervalIntDomain(2, 2));
    variables.push_back(v4.getId());
    VariableImpl<IntervalIntDomain> v5(ENGINE, IntervalIntDomain(2, 2));
    variables.push_back(v5.getId());
    variables.push_back(v1.getId());
    AddEqualConstraint c2(ENGINE, variables);
    
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
      VariableImpl<IntervalIntDomain>* id = (VariableImpl<IntervalIntDomain>*) (*it);
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
    VariableImpl<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v0.getId());
    VariableImpl<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v1.getId());
    EqualConstraint c0(ENGINE, variables);


    // v2 + v3 == v0
    variables.clear();
    VariableImpl<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v2.getId());
    VariableImpl<IntervalIntDomain> v3(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v3.getId());
    variables.push_back(v0.getId());
    AddEqualConstraint c1(ENGINE, variables);

    // v4 + v5 == v1
    variables.clear();
    VariableImpl<IntervalIntDomain> v4(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v4.getId());
    VariableImpl<IntervalIntDomain> v5(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v5.getId());
    variables.push_back(v1.getId());
    AddEqualConstraint c2(ENGINE, variables);

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
    VariableImpl<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v0.getId());
    VariableImpl<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v1.getId());
    ConstraintId c0((new EqualConstraint(ENGINE, variables))->getId());


    // v2 + v3 == v0
    variables.clear();
    VariableImpl<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v2.getId());
    VariableImpl<IntervalIntDomain> v3(ENGINE, IntervalIntDomain(1, 10));
    variables.push_back(v3.getId());
    variables.push_back(v0.getId());
    ConstraintId c1((new AddEqualConstraint(ENGINE, variables))->getId());

    ENGINE->propagate();
    assert(ENGINE->constraintConsistent());

    /*!< Show that we can simply delete a constraint and confirm that the system is still consistent */
    delete (Constraint*) c1;
    ENGINE->propagate();
    assert(ENGINE->constraintConsistent());

    variables.clear();
    VariableImpl<IntervalIntDomain> v4(ENGINE, IntervalIntDomain(1, 1));
    variables.push_back(v0.getId());
    variables.push_back(v4.getId());
    ConstraintId c2((new EqualConstraint(ENGINE, variables))->getId());
    ENGINE->propagate();
    assert(ENGINE->constraintConsistent());
    assert(v1.getDerivedDomain().getSingletonValue() == 1);

    delete (Constraint*) c2;
    ENGINE->propagate();
    assert(ENGINE->constraintConsistent());
    assert(v1.getDerivedDomain().getUpperBound() == 10);

    /*!< Add a constraint to force an inconsistency and show that consistency can be restored by removing the
      constraint */
    variables.clear();
    VariableImpl<IntervalIntDomain> v5(ENGINE, IntervalIntDomain(0, 0));
    variables.push_back(v0.getId());
    variables.push_back(v5.getId());
    ConstraintId c3((new EqualConstraint(ENGINE, variables))->getId());
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
