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
#include "StringDomain.hh"
#include "SymbolDomain.hh"
#include "NumericDomain.hh"

#include "TypeFactory.hh"
#include "EnumeratedTypeFactory.hh"

#include "ConstraintTesting.hh"

#include "module-tests.hh"

#include "LockManager.hh"

#include <iostream>
#include <vector>
#include <string>

#include <fstream>

using namespace EUROPA;

class DelegationTestConstraint : public Constraint {
public:
  DelegationTestConstraint(const LabelStr& name,
			   const LabelStr& propagatorName,
			   const ConstraintEngineId& constraintEngine,
			   const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables){s_instanceCount++;}
  ~DelegationTestConstraint(){s_instanceCount--;}

  void handleExecute(){
    s_executionCount++;
    ConstrainedVariableId var = getScope().front();
    assertTrue(!var->derivedDomain().isSingleton(), "Should be able to access derived domain during propagation safely.");
  }

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


class TestLockManager: public LockManager {
public:
  TestLockManager()
    : LockManager(), m_currentUser("TEST_HARNESS") {
  }

  const LabelStr& getCurrentUser() const {
    return(m_currentUser);
  }

private:
  const LabelStr m_currentUser;
};

class TypeFactoryTests {
public:
  static bool test() {
    runTest(testValueCreation);

    // This is needed by the next test and is done outside it to avoid the
    // need to purge the type factories, which might affect other tests.
    new EnumeratedTypeFactory("Locations", "Locations", LocationsBaseDomain());
    runTest(testDomainCreation);

    runTest(testVariableCreation);
    runTest(testVariableWithDomainCreation);
    return true; 
  }

  static bool testValueCreation(){
    IntervalIntDomain d0(5);
    int v0 = (int) TypeFactory::createValue(d0.getTypeName().c_str(), std::string("5"));
    assertTrue(d0.compareEqual(d0.getSingletonValue(), v0));

    IntervalDomain d1(2.3);
    double v1 = (double) TypeFactory::createValue(d1.getTypeName().c_str(), std::string("2.3"));
    assertTrue(d1.compareEqual(d1.getSingletonValue(), v1));

    BoolDomain d2(true);
    bool v2 = (bool) TypeFactory::createValue(d2.getTypeName().c_str(), std::string("true"));
    assertTrue(d2.compareEqual(d2.getSingletonValue(), v2));

    return true;
  }

  typedef SymbolDomain Locations;

  /**
   * Locations enumeration's base domain, as required by class TypeFactory.
   * @note Copied from System/test/basic-model-transaction.cc
   * as created from basic-model-transaction.nddl v1.3 with the NDDL compiler.
   */
  static const Locations& LocationsBaseDomain() {
    static Locations sl_enum("Locations");
    if (sl_enum.isOpen()) {
      sl_enum.insert(LabelStr("Hill"));
      sl_enum.insert(LabelStr("Rock"));
      sl_enum.insert(LabelStr("Lander"));
      sl_enum.close();
    }
    return(sl_enum);
  }

  static bool testDomainCreation() {
    const IntervalIntDomain & bd0 = dynamic_cast<const IntervalIntDomain &>(TypeFactory::baseDomain(IntervalIntDomain().getTypeName().c_str()));
    assertTrue(bd0.isMember(0));
    assertTrue(!bd0.isBool());
    const IntervalDomain & bd1 = dynamic_cast<const IntervalDomain &>(TypeFactory::baseDomain(IntervalDomain().getTypeName().c_str()));
    assertTrue(bd1.isMember(0.1));
    assertTrue(!bd1.isBool());
    const BoolDomain & bd2 = dynamic_cast<const BoolDomain &>(TypeFactory::baseDomain(BoolDomain().getTypeName().c_str()));
    assertTrue(bd2.isMember(false));
    assertTrue(bd2.isMember(true));
    assertTrue(bd2.isBool());
    assertTrue(LocationsBaseDomain().isMember(LabelStr("Hill")));
    assertTrue(LocationsBaseDomain().isMember(LabelStr("Rock")));
    assertTrue(LocationsBaseDomain().isMember(LabelStr("Lander")));
    assertTrue(!LocationsBaseDomain().isMember(LabelStr("true")));
    //!!This (and SymbolDomain) die with complaints of a "bad cast"
    //!!const Locations & loc0 = dynamic_cast<const Locations&>(TypeFactory::baseDomain("Locations"));
    const EnumeratedDomain & loc0 = dynamic_cast<const EnumeratedDomain &>(TypeFactory::baseDomain("Locations"));
    assertTrue(!loc0.isBool());
    assertTrue(loc0.isMember(LabelStr("Hill")));
    assertTrue(loc0.isMember(LabelStr("Rock")));
    assertTrue(loc0.isMember(LabelStr("Lander")));
    assertTrue(!loc0.isMember(LabelStr("true")));
    //!!The compiler complains about using Locations here when EnumeratedDomain is used above.
    //!!Locations *loc1 = loc0.copy();
    EnumeratedDomain *loc1 = loc0.copy();
    loc1->open();
    loc1->remove(LabelStr("Hill"));
    assertTrue(!loc1->isMember(LabelStr("Hill")));
    assertTrue(loc1->isMember(LabelStr("Rock")));
    assertTrue(loc1->isMember(LabelStr("Lander")));
    loc1->remove(LabelStr("Rock"));
    assertTrue(!loc1->isMember(LabelStr("Hill")));
    assertTrue(!loc1->isMember(LabelStr("Rock")));
    assertTrue(loc1->isMember(LabelStr("Lander")));
    loc1->insert(LabelStr("Hill"));
    assertTrue(loc1->isMember(LabelStr("Hill")));
    assertTrue(!loc1->isMember(LabelStr("Rock")));
    assertTrue(loc1->isMember(LabelStr("Lander")));
    loc1->remove(LabelStr("Lander"));
    assertTrue(loc1->isMember(LabelStr("Hill")));
    assertTrue(!loc1->isMember(LabelStr("Rock")));
    assertTrue(!loc1->isMember(LabelStr("Lander")));
    loc1->insert(LabelStr("Rock"));
    assertTrue(loc1->isMember(LabelStr("Hill")));
    assertTrue(loc1->isMember(LabelStr("Rock")));
    assertTrue(!loc1->isMember(LabelStr("Lander")));
    loc1->remove(LabelStr("Hill"));
    assertTrue(!loc1->isMember(LabelStr("Hill")));
    assertTrue(loc1->isMember(LabelStr("Rock")));
    assertTrue(!loc1->isMember(LabelStr("Lander")));
    delete loc1;
    return true;
  }

  static bool testVariableCreation(){
    ConstraintEngineId ce = (new ConstraintEngine())->getId();
    ConstrainedVariableId cv0 = TypeFactory::createVariable(IntervalIntDomain().getTypeName().c_str(), ce);
    assertTrue(cv0->baseDomain().getTypeName() == IntervalIntDomain().getTypeName());
    ConstrainedVariableId cv1 = TypeFactory::createVariable(IntervalDomain().getTypeName().c_str(), ce);
    assertTrue(cv1->baseDomain().getTypeName() == IntervalDomain().getTypeName());
    ConstrainedVariableId cv2 = TypeFactory::createVariable(BoolDomain().getTypeName().c_str(), ce);
    assertTrue(cv2->baseDomain().getTypeName() == BoolDomain().getTypeName());
    Entity::purgeStarted();
    delete (ConstraintEngine*) ce;
    Entity::purgeEnded();
    return true;
  }

  static bool testVariableWithDomainCreation(){
    IntervalIntDomain d0(5);
    IntervalDomain d1(2.3);
    BoolDomain d2(true);

    ConstraintEngineId ce = (new ConstraintEngine())->getId();
    ConstrainedVariableId cv0 = TypeFactory::createVariable(d0.getTypeName().c_str(), ce, d0);
    assertTrue(cv0->baseDomain() == d0);
    ConstrainedVariableId cv1 = TypeFactory::createVariable(d1.getTypeName().c_str(), ce, d1);
    assertTrue(cv1->baseDomain() == d1);
    ConstrainedVariableId cv2 = TypeFactory::createVariable(d2.getTypeName().c_str(), ce, d2);
    assertTrue(cv2->baseDomain() == d2);

    Entity::purgeStarted();
    delete (ConstraintEngine*) ce;
    Entity::purgeEnded();
    return true;
  }
};

class EntityTests {
public:
  static bool test(){
    return true;
  }
};

class ConstraintEngineTest
{
public:
  static bool test(){
    runTest(testDeallocationWithPurging);
    runTest(testInconsistentInitialVariableDomain);
    runTest(testVariableLookupByIndex);
    runTest(testGNATS_3133);
    return true;
  }

  static bool testDeallocationWithPurging(){
    ConstraintEngineId ce = (new ConstraintEngine())->getId();
    new DefaultPropagator(LabelStr("Default"), ce);

    // Set up a base domain
    NumericDomain intBaseDomain;
    intBaseDomain.insert(1);
    intBaseDomain.insert(2);
    intBaseDomain.insert(3);
    intBaseDomain.insert(4);
    intBaseDomain.insert(5);
    intBaseDomain.close();

    for(int i=0;i<100;i++){
      Id<Variable<NumericDomain > > v0 = (new Variable<NumericDomain> (ce, intBaseDomain))->getId();
      Id<Variable<NumericDomain > > v1 = (new Variable<NumericDomain>(ce, intBaseDomain))->getId();
      new EqualConstraint(LabelStr("EqualConstraint"), LabelStr("Default"), ce, makeScope(v0, v1));
    }

    assertTrue(ce->propagate());
    Entity::purgeStarted();
    delete (ConstraintEngine*) ce;
    Entity::purgeEnded();

    return true;
  }

  static bool testInconsistentInitialVariableDomain(){
    NumericDomain emptyDomain;
    emptyDomain.close();
    {
      Variable<NumericDomain> v0(ENGINE, emptyDomain);
      assertTrue(ENGINE->provenInconsistent()); // Should be immediately inconsistent!
    }
    assertTrue(ENGINE->propagate()); // Should be fixed by deletion of the variable

    return true;
  }

  static bool testVariableLookupByIndex(){
    std::vector<ConstrainedVariableId> vars;

    for(unsigned int i=0;i<10;i++){
      ConstrainedVariableId var = (new Variable<IntervalIntDomain> (ENGINE, IntervalIntDomain()))->getId();
      assertTrue(var == ENGINE->getVariable(i));
      assertTrue(ENGINE->getIndex(var) == i);
      vars.push_back(var);
    }

    cleanup(vars);
    return true;
  }

  /**
   * A single relaxation may not be enough to empty the variable. Want to ensure that
   * we correctly manage this case.
   */
  static bool testGNATS_3133(){
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(0, 10));
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(0, 10));
    Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(0, 10));
    Variable<IntervalIntDomain> v3(ENGINE, IntervalIntDomain(0, 10));

    Constraint* c0 = new EqualConstraint(LabelStr("EqualConstraint"), 
					 LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId()));

    Constraint* c1 = new EqualConstraint(LabelStr("EqualConstraint"), 
					 LabelStr("Default"), ENGINE, makeScope(v2.getId(), v3.getId()));

    Constraint* c2 = new EqualConstraint(LabelStr("EqualConstraint"), 
					 LabelStr("Default"), ENGINE, makeScope(v2.getId(), v3.getId()));

    v0.specify(1);
    v1.specify(2);
    v2.specify(3);
    v3.specify(3);
    assertFalse(ENGINE->propagate());

    v2.reset();
    assertTrue(ENGINE->provenInconsistent());
    assertFalse(ENGINE->propagate());
    v3.reset();
    assertFalse(ENGINE->propagate())
    assertTrue(ENGINE->provenInconsistent());;
    v0.reset();
    assertFalse(ENGINE->provenInconsistent());
    assertTrue(ENGINE->propagate());

    v0.specify(1);
    v1.specify(2);
    v2.specify(3);
    v3.specify(3);

    // Now delete constraints in the order that relaxes the empty variable last
    delete (Constraint*) c1;
    assertFalse(ENGINE->propagate());
    delete (Constraint*) c2;
    assertFalse(ENGINE->propagate());
    delete (Constraint*) c0;
    assertTrue(ENGINE->propagate());

    return true;
  }
};

class TestVariableListener: public ConstrainedVariableListener{
public:
  TestVariableListener(const ConstrainedVariableId& observedVar, const ConstrainedVariableId& managedVar)
    : ConstrainedVariableListener(observedVar), m_managedVar(managedVar){}
  ~TestVariableListener(){
    delete (ConstrainedVariable*) m_managedVar;
  }
private:
  ConstrainedVariableId m_managedVar;
};

class VariableTest
{
public:
  static bool test() {
    runTest(testAllocation);
    runTest(testMessaging);
    runTest(testDynamicVariable);
    runTest(testListener);
    runTest(testVariablesWithOpenDomains);
    runTest(testRestrictionScenarios);
    return true;
  }

private:

  static bool testAllocation(){
    IntervalIntDomain dom0(0, 1000);
    Variable<IntervalIntDomain> v0(ENGINE, dom0);
    const IntervalIntDomain& dom1 = v0.getBaseDomain();
    assertTrue(dom0 == dom1);
    assertTrue(v0.isValid());
    assertTrue(v0.canBeSpecified());

    // Now restrict the base domain
    IntervalIntDomain dom2(3, 10);
    v0.restrictBaseDomain(dom2);
    assertTrue(v0.getDerivedDomain() == dom2);

    Variable<IntervalIntDomain> v1(ENGINE, dom1, false, LabelStr("TEST VARIABLE"));
    assertTrue(!v1.canBeSpecified());
    assertTrue(v1.getName() == LabelStr("TEST VARIABLE"));
    assertTrue(v1.isValid());
    return true;
  }

  static bool testMessaging(){
    TestListener listener(ENGINE);

    // Add, Specify, Remove
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(0, 100));
      assertTrue(listener.getCount(ConstraintEngine::SET) == 1);
      assertTrue(listener.getCount(ConstraintEngine::VARIABLE_ADDED) == 1);
      v0.specify(IntervalIntDomain(3, 8));
      assertTrue(listener.getCount(ConstraintEngine::SET) == 2);
      v0.specify(5);
      assertTrue(listener.getCount(ConstraintEngine::SET) == 2);
      assertTrue(listener.getCount(ConstraintEngine::SET_TO_SINGLETON) == 1);
    }
    assertTrue(listener.getCount(ConstraintEngine::VARIABLE_REMOVED) == 1);

    // Bounds restriction messages for derived domain
    listener.reset();
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(0, 100));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(0, 10));
      EqualConstraint c0(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId()));
      ENGINE->propagate();
      assertTrue(listener.getCount(ConstraintEngine::UPPER_BOUND_DECREASED) == 1);

      v0.specify(IntervalIntDomain(5, 10)); // Expect lower-bound-increased message
      assertTrue(listener.getCount(ConstraintEngine::LOWER_BOUND_INCREASED) == 1);

      ENGINE->propagate(); // Expect another through propagation
      assertTrue(listener.getCount(ConstraintEngine::LOWER_BOUND_INCREASED) == 2);

      v1.specify(IntervalIntDomain(6, 8)); // 
      assertTrue(listener.getCount(ConstraintEngine::BOUNDS_RESTRICTED) == 1);

      ENGINE->propagate(); // Expect another through propagation
      assertTrue(listener.getCount(ConstraintEngine::BOUNDS_RESTRICTED) == 2);

      v0.specify(7);
      ENGINE->propagate(); // Expect a RESTRICT_TO_SINGLETON event through propagation
      assertTrue(listener.getCount(ConstraintEngine::RESTRICT_TO_SINGLETON) == 1);

      v0.reset(); // Expect a RESET message for v0 and a RELAXATION message for both variables
      assertTrue(listener.getCount(ConstraintEngine::RESET) == 1);
      assertTrue(listener.getCount(ConstraintEngine::RELAXED) == 2);
      assertTrue(ENGINE->pending());

      v0.specify(0); // Expect EMPTIED
      ENGINE->propagate();
      assertTrue(listener.getCount(ConstraintEngine::EMPTIED) == 1);
    }

    // Now tests message handling on Enumerated Domain
    listener.reset();
    {
      Variable<NumericDomain> v0(ENGINE, NumericDomain());
      v0.insert(1);
      v0.insert(3);
      v0.insert(5);
      v0.insert(10);
      assertTrue(listener.getCount(ConstraintEngine::RELAXED) == 0); // Should not generate any of these messages while not closed
      v0.close();
      assertTrue(listener.getCount(ConstraintEngine::CLOSED) == 1);
      assert(listener.getCount(ConstraintEngine::SET) == 1); // Expect to see specified domain cause 'set' on derived domain once closed.

      NumericDomain d0;
      d0.insert(2);
      d0.insert(3);
      d0.insert(5);
      d0.insert(11);
      d0.close();
      Variable<NumericDomain> v1(ENGINE, d0);
      assertTrue(listener.getCount(ConstraintEngine::SET) == 2); // Expect to see specified domain cause 'set' immediately.

      EqualConstraint c0(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId()));
      ENGINE->propagate(); // Should see values removed from both variables domains. 
      assertTrue(listener.getCount(ConstraintEngine::VALUE_REMOVED) == 2);
      v0.specify(3);
      assertTrue(listener.getCount(ConstraintEngine::SET_TO_SINGLETON) == 1);
      v1.specify(5);
      assertTrue(listener.getCount(ConstraintEngine::SET_TO_SINGLETON) == 2);
      ENGINE->propagate(); // Expect to see exactly one domain emptied
      assertTrue(listener.getCount(ConstraintEngine::EMPTIED) == 1);
      v1.reset(); // Should now see 2 domains relaxed.
      assertTrue(listener.getCount(ConstraintEngine::RELAXED) == 2);
    }

    return true;
  }

  static bool testDynamicVariable(){
    // Test empty on closure forces inconsistency
    {
      Variable<NumericDomain> v0(ENGINE,NumericDomain());
      assertTrue(ENGINE->propagate()); // No reason to be inconsistent
      v0.close(); // Should push empty event.
      assertTrue(ENGINE->provenInconsistent()); // Should be inconsistent
    }

    // Test that insertion is possible for variables and that it is handled correctly through propagation
    {
      Variable<NumericDomain> v0(ENGINE, NumericDomain()); // The empty one.
      Variable<NumericDomain> v1(ENGINE, NumericDomain()); // The full one
      v0.insert(1); // The only value, leave it open.

      // Fill up v1 and close it.
      v1.insert(1);
      v1.insert(2);
      v1.insert(3);
      v1.close();

      // Post equality constraint between v0 and v1. It should not cause any restriction yet
      // since v0 has not been closed      
      //this test has been invalidated since we now propagate closed-ness, so the size should be 1
      EqualConstraint c0(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId()));
      assertTrue(ENGINE->propagate());
      assertTrue(v1.getDerivedDomain().getSize() == 1);

      // Now close v0, and we should see a restriction on v1
      v0.close();
      assertTrue(v1.getDerivedDomain().isSingleton());

      // insert further, and propagate again, the domain should grow by one
      v0.open();
      v0.insert(2);
      v0.close();
      assertTrue(v1.getDerivedDomain().getSize() == 2);

      // Open it and ensure that the engine is pending once again
      v0.open();
      assertTrue(ENGINE->pending());
      assertTrue(v1.getDerivedDomain().getSize() == 2);

      v0.specify(2);
      assertTrue(v0.specifiedDomain().isClosed());
      assertTrue(v1.getDerivedDomain().isSingleton());

      // Now insert and confirm it does not affect v1. Shoud insert to base domain but
      // not push to the specified or derived domain.
      v0.insert(3);
      assertTrue(v0.specifiedDomain().isSingleton()); // Still, since spec domain is closed.
      assertTrue(v1.getDerivedDomain().isSingleton());
      assertFalse(v0.baseDomain().isClosed());
      assertTrue(v1.getDerivedDomain().isSingleton());

      // Now if we reset, the base domain will have 3 elements in it, so derived domain of v1 will also have 3 elements
      // in it since v0 should be relaxed to the base domain.
      v0.reset();
      assertTrue(v1.getDerivedDomain().getSize() == 3);
    }

    return true;
  }

  static bool testListener(){
    ConstrainedVariableId v0 = (new Variable<IntervalIntDomain>(ENGINE, IntervalIntDomain()))->getId();
    ConstrainedVariableId v1 = (new Variable<IntervalIntDomain>(ENGINE, IntervalIntDomain()))->getId();
    new TestVariableListener(v0, v1);
    delete (ConstrainedVariable*) v0; // Should force deletion of all
    return true;
  }

  static bool testVariablesWithOpenDomains() {
    EnumeratedDomain e0(true, "Test");
    e0.insert(0); e0.insert(1); e0.insert(2); e0.insert(3);
    Variable<EnumeratedDomain> v0(ENGINE, e0);

    assertTrue(v0.baseDomain().isOpen());
    assertTrue(v0.specifiedDomain().isOpen());
    assertTrue(v0.derivedDomain().isOpen());

    v0.close();
    assertTrue(v0.baseDomain().isClosed());
    assertTrue(v0.specifiedDomain().isClosed());
    assertTrue(v0.derivedDomain().isClosed());

    v0.open();
    assertTrue(v0.baseDomain().isOpen());
    assertTrue(v0.specifiedDomain().isOpen());
    assertTrue(v0.derivedDomain().isOpen());

    e0.close();
    v0.specify(e0);
    assertTrue(v0.baseDomain().isOpen());
    assertTrue(v0.specifiedDomain().isClosed());
    
    v0.restrictBaseDomain(e0);
    assertTrue(v0.baseDomain().isClosed());
    assertTrue(v0.isClosed());

    return true;
  }

  static bool testRestrictionScenarios(){
    EnumeratedDomain e0(true, "Test");
    e0.insert(0); e0.insert(1); e0.insert(2); e0.insert(3);
    e0.close();

    EnumeratedDomain e1(true, "Test");
    e1.insert(1); e1.insert(3);
    e1.close();

    Variable<EnumeratedDomain> v0(ENGINE, e0);
    Variable<EnumeratedDomain> v1(ENGINE, e0);

    EqualConstraint c0(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId()));

    // Specify v0 and propagate
    v0.specify(1);
    assertTrue(ENGINE->propagate());

    // Now v1's derived domain will also be a singleton. However, I want to restrict the base domain of v1 partially.
    v1.restrictBaseDomain(e1);
    assertTrue(ENGINE->propagate());

    // Now specify v1 to a singleton also, a different value than that already specified.
    v1.specify(3);
    assertFalse(ENGINE->propagate());

    // Repair by reseting v0
    v0.reset();
    assertTrue(ENGINE->propagate());
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
    runTest(testDeactivation);
    runTest(testForceInconsistency);
    runTest(testRepropagation);
    runTest(testConstraintRemoval);
    runTest(testDelegation);
    runTest(testNotEqual);
    runTest(testMultEqualConstraint);
    runTest(testAddMultEqualConstraint);
    runTest(testEqualSumConstraint);
    runTest(testCondAllSameConstraint);
    runTest(testCondAllDiffConstraint);
    runTest(testConstraintDeletion);
    runTest(testArbitraryConstraints);
    runTest(testLockConstraint);
    runTest(testNegateConstraint);
    runTest(testUnaryQuery);
    runTest(testTestEqConstraint);
    runTest(testTestLessThanConstraint);
    runTest(testTestLEQConstraint);
    runTest(testLockManager);
    runTest(testGNATS_3075);
    return(true);
  }

private:
  static bool testSubsetConstraint() {
    std::list<double> values;
    values.push_back(EUROPA::LabelStr("A"));
    values.push_back(EUROPA::LabelStr("B"));
    values.push_back(EUROPA::LabelStr("C"));
    values.push_back(EUROPA::LabelStr("D"));
    values.push_back(EUROPA::LabelStr("E"));
    LabelSet ls0(values);
    values.pop_back();
    values.pop_back();
    LabelSet ls1(values);
    assertTrue(ls1.isSubsetOf(ls0));
    assertTrue(!(ls1 == ls0));

    Variable<LabelSet> v0(ENGINE, ls0);
    LabelSet dom = v0.getDerivedDomain();
    assertTrue(dom == ls0 && !(dom == ls1));
    Variable<LabelSet> v1(ENGINE, ls1);
    SubsetOfConstraint c0(LabelStr("SubsetOf"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId()));
    ENGINE->propagate();
    assertTrue(ENGINE->constraintConsistent());

    assertTrue(v0.getDerivedDomain() == ls1);
    v1.specify(LabelStr("B"));
    assertTrue(v0.getDerivedDomain().getSingletonValue() == LabelStr("B"));
    v1.reset();
    assertTrue(v0.getDerivedDomain() == ls1);

    values.pop_back();
    LabelSet ls2(values);
    v0.specify(ls2);
    assertTrue(!ENGINE->pending()); // No change expected since it is a restriction.
    assertTrue(!(v0.getDerivedDomain() == ls1));
    assertTrue(ENGINE->constraintConsistent());
    v0.reset();
    assertTrue(ENGINE->pending());
    ENGINE->propagate();
    assertTrue(ENGINE->constraintConsistent());
    assertTrue(v0.getDerivedDomain() == ls1);
    
    return true;
  }

  static bool testAddEqualConstraint() {
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, 1));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(0, 2));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(v0.getDerivedDomain().getSingletonValue() == 1);
      assertTrue(v1.getDerivedDomain().getSingletonValue() == 1);
      assertTrue(v2.getDerivedDomain().getSingletonValue() == 2);
    }

    // Test to force negative values where A + B == C and C < A. Focus on computation of B
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(9, 29));
      Variable<IntervalDomain> v1(ENGINE, IntervalDomain());
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(8, 28));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(v1.getDerivedDomain().getLowerBound() == -21);
      assertTrue(v1.getDerivedDomain().getUpperBound() == 19);
    }

    // Now test mixed types
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
      Variable<IntervalDomain> v1(ENGINE, IntervalDomain(1.2, 2.8));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(0, 3));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(v0.getDerivedDomain().getSingletonValue() == 1);
      assertTrue(v1.getDerivedDomain().getSingletonValue() == 2);
      assertTrue(v2.getDerivedDomain().getSingletonValue() == 3);
    }

    // Now test special case of rounding with a singleton
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(0, 10));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(0, 10));
      Variable<IntervalDomain> v2(ENGINE, IntervalDomain(0.5, 0.5));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      assertTrue(ENGINE->provenInconsistent());
    }

    // Now test special case of rounding with negative domain bounds.
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(-10, 10));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(-10, 10));
      Variable<IntervalDomain> v2(ENGINE, IntervalDomain(0.01, 0.99));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      bool res = ENGINE->propagate();
      assertTrue(!res);
    }

    // Another, similar, case of rounding with negative domain bounds.
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(-10, 10));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(-10, 10));
      Variable<IntervalDomain> v2(ENGINE, IntervalDomain(0.01, 1.99));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      bool res = ENGINE->propagate();
      assertTrue(res);
      // Require correct result to be in v2's domain.
      assertTrue(v2.getDerivedDomain().isMember(1.0));
      // Following is false because implementation of AddEqualConstraint is not smart enough to deduce it.
      //assertTrue(v2.getDerivedDomain().getSingletonValue() == 1.0);
    }

    // Confirm correct result with all singletons.
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(-1, -1));
      Variable<IntervalDomain> v1(ENGINE, IntervalDomain(10.4, 10.4));
      Variable<IntervalDomain> v2(ENGINE, IntervalDomain(9.4, 9.4));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      bool res = ENGINE->propagate();
      assertTrue(res);
    }

    // Confirm inconsistency detected with all singletons.
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(-1, -1));
      Variable<IntervalDomain> v1(ENGINE, IntervalDomain(10.4, 10.4));
      Variable<IntervalDomain> v2(ENGINE, IntervalDomain(9.39, 9.39));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      bool res = ENGINE->propagate();
      assertTrue(!res);
    }

    // Obtain factors correct values for fixed result.
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(0, PLUS_INFINITY));
      Variable<IntervalDomain> v1(ENGINE, IntervalDomain(0, PLUS_INFINITY));
      Variable<IntervalDomain> v2(ENGINE, IntervalDomain(9.390, 9.390));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      bool res = ENGINE->propagate();
      assertTrue(res);
      assertTrue(v0.getDerivedDomain() == IntervalIntDomain(0, 9));
      assertTrue(v1.getDerivedDomain() == IntervalDomain(0.39, 9.39));
    }

    // Test handling with all infinites
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(MINUS_INFINITY, MINUS_INFINITY));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, PLUS_INFINITY));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(PLUS_INFINITY, PLUS_INFINITY));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      bool res = ENGINE->propagate();
      assertTrue(res);
      assertTrue(v0.getDerivedDomain() == IntervalIntDomain(MINUS_INFINITY, MINUS_INFINITY));
      assertTrue(v1.getDerivedDomain() == IntervalIntDomain(1, PLUS_INFINITY));
      assertTrue(v2.getDerivedDomain() == IntervalIntDomain(PLUS_INFINITY, PLUS_INFINITY));
    }

    // Test handling with infinites and non-infinites
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(10, PLUS_INFINITY));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, PLUS_INFINITY));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(MINUS_INFINITY, 100));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      bool res = ENGINE->propagate();
      assertTrue(res);
      assertTrue(v0.getDerivedDomain() == IntervalIntDomain(10, 99));
      assertTrue(v1.getDerivedDomain() == IntervalIntDomain(1, 90));
      assertTrue(v2.getDerivedDomain() == IntervalIntDomain(11, 100));
    }

    // Test propagating infinites: start + duration == end.
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(MINUS_INFINITY, PLUS_INFINITY));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(MINUS_INFINITY, PLUS_INFINITY));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      bool res = ENGINE->propagate();
      assertTrue(res);
      assertTrue(v0.getDerivedDomain() == IntervalIntDomain(MINUS_INFINITY, PLUS_INFINITY));
      assertTrue(v1.getDerivedDomain() == IntervalIntDomain(1));
      assertTrue(v2.getDerivedDomain() == IntervalIntDomain(MINUS_INFINITY, PLUS_INFINITY));
    }

    // Test that we can use the constraint on a variable that is present in the constraint more than once
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(10, 14));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(0, 1));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v0.getId()));
      bool res = ENGINE->propagate();
      assertTrue(res);
      v1.specify(1);
      assertFalse(ENGINE->propagate());
      v1.reset();
      assertTrue(ENGINE->propagate());
      v0.specify(11);
      assertTrue(ENGINE->propagate());
      assertTrue(v1.getDerivedDomain() == IntervalIntDomain(0));
    }

    return true;
  }

  static bool testEqualConstraint()
  {
    // Set up a base domain
    std::list<double> baseValues;
    baseValues.push_back(EUROPA::LabelStr("A"));
    baseValues.push_back(EUROPA::LabelStr("B"));
    baseValues.push_back(EUROPA::LabelStr("C"));
    baseValues.push_back(EUROPA::LabelStr("D"));
    baseValues.push_back(EUROPA::LabelStr("E"));
    LabelSet baseDomain(baseValues);

    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(-100, 1));
    EqualConstraint c0(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId()));
    ENGINE->propagate();
    assertTrue(ENGINE->constraintConsistent());
    assertTrue(v0.getDerivedDomain().getSingletonValue() == 1);
    assertTrue(v1.getDerivedDomain().getSingletonValue() == 1);

    LabelSet ls0(baseDomain);
    ls0.empty();
    ls0.open();
    ls0.insert(EUROPA::LabelStr("A"));
    ls0.close();

    LabelSet ls1(baseDomain);
    ls1.empty();
    ls1.open();
    ls1.insert(EUROPA::LabelStr("A"));
    ls1.insert(EUROPA::LabelStr("B"));
    ls1.insert(EUROPA::LabelStr("C"));
    ls1.insert(EUROPA::LabelStr("D"));
    ls1.insert(EUROPA::LabelStr("E"));
    ls1.close();

    Variable<LabelSet> v2(ENGINE, ls1);
    Variable<LabelSet> v3(ENGINE, ls1);
    EqualConstraint c1(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v2.getId(), v3.getId()));
    ENGINE->propagate();
    assertTrue(ENGINE->constraintConsistent());
    assertTrue(v2.getDerivedDomain() == v3.getDerivedDomain());
    assertTrue(!v2.getDerivedDomain().isSingleton());

    LabelSet ls2(ls1);
    ls2.remove(EUROPA::LabelStr("E"));

    v2.specify(ls2);
    ENGINE->propagate();
    assertTrue(!v3.getDerivedDomain().isMember(EUROPA::LabelStr("E")));

    Variable<LabelSet> v4(ENGINE, ls0);
    EqualConstraint c2(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v2.getId(), v4.getId()));
    ENGINE->propagate();
    assertTrue(ENGINE->constraintConsistent());
    assertTrue(v2.getDerivedDomain() == v3.getDerivedDomain());
    assertTrue(v2.getDerivedDomain() == v4.getDerivedDomain());
    assertTrue(v3.getDerivedDomain() == v4.getDerivedDomain());
    assertTrue(v3.getDerivedDomain().getSingletonValue() == EUROPA::LabelStr("A"));

    // Now test that equality is working correctly for dynamic domains
    {
      NumericDomain e0;
      e0.insert(1);
      e0.insert(2);
      e0.insert(3);

      NumericDomain e1;
      e1.insert(1);
      e1.insert(2);
      e1.insert(3);
      e1.insert(4);

      NumericDomain e2;
      e2.insert(5);
      
      assertTrue(e0.isOpen());
      assertTrue(e1.isOpen());
      assertTrue(e2.isOpen());
      // Leave domains dynamic

      Variable<NumericDomain> a(ENGINE, e0);
      Variable<NumericDomain> b(ENGINE, e1);
      Variable<NumericDomain> c(ENGINE, e2);
      EqualConstraint eq(LabelStr("EqualConstraint"), 
			 LabelStr("Default"), 
			 ENGINE, 
			 makeScope(a.getId(), b.getId(), c.getId()));
      assertTrue(ENGINE->propagate());

      //if they're all open, there should be no change
      assertTrue(a.lastDomain().isOpen());
      assertTrue(a.lastDomain() == e0);
      assertTrue(b.lastDomain().isOpen());
      assertTrue(b.lastDomain() == e1);
      assertTrue(c.lastDomain().isOpen());
      assertTrue(c.lastDomain() == e2);

      c.insert(3); //want to have a common element so there's no empty
      e2.insert(3);
      e2.remove(5);

      // Now close one
      b.close();
      e2.close(); //close the domain for comparison
      assertTrue(b.lastDomain().getSize() == 4);
      assertTrue(ENGINE->propagate());
     
      //all should be closed
      assertTrue(!a.lastDomain().isOpen());
      assertTrue(a.lastDomain() == e2);
      assertTrue(!b.lastDomain().isOpen());
      assertTrue(b.lastDomain() == e2);
      assertTrue(!c.lastDomain().isOpen());
      assertTrue(c.lastDomain() == e2);

    }

    // Create a fairly large multi-variable test that will ensure we handle the need for 2 passes.
    {
      Variable<IntervalIntDomain> a(ENGINE, IntervalIntDomain(0, 100));
      Variable<IntervalIntDomain> b(ENGINE, IntervalIntDomain(10, 90));
      Variable<IntervalIntDomain> c(ENGINE, IntervalIntDomain(20, 80));
      Variable<IntervalIntDomain> d(ENGINE, IntervalIntDomain(30, 70));
      std::vector<ConstrainedVariableId> scope;
      scope.push_back(a.getId());
      scope.push_back(b.getId());
      scope.push_back(c.getId());
      scope.push_back(d.getId());
      EqualConstraint eq(LabelStr("EqualConstraint"), 
			 LabelStr("Default"), 
			 ENGINE, 
			 scope);
      assertTrue(ENGINE->propagate());
      assertTrue(a.lastDomain() == IntervalIntDomain(30, 70));
    }
    return true;
  }

  static bool testLessThanEqualConstraint() {
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 100));
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, 100));
    LessThanEqualConstraint c0(LabelStr("LessThanEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId()));
    ENGINE->propagate();
    assertTrue(ENGINE->constraintConsistent());
    assertTrue(v0.getDerivedDomain() == v1.getDerivedDomain());

    v0.specify(IntervalIntDomain(50, 100));
    assertTrue(v1.getDerivedDomain().getLowerBound() == 50);
    IntervalIntDomain copy(v1.getDerivedDomain());
    v0.specify(IntervalIntDomain(50, 80));
    assertTrue(v1.getDerivedDomain() == copy);
    v1.specify(IntervalIntDomain(60, 70));
    assertTrue(v0.getDerivedDomain() == IntervalIntDomain(50, 70));
    v1.reset();
    assertTrue(v0.getDerivedDomain() == IntervalIntDomain(50, 80));
    assertTrue(v1.getDerivedDomain() == IntervalIntDomain(50, 100));

    // Handle propagation of infinities
    Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(2, PLUS_INFINITY));
    Variable<IntervalIntDomain> v3(ENGINE, IntervalIntDomain(MINUS_INFINITY, 100));
    LessThanEqualConstraint c2(LabelStr("LessThanEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v2.getId(), v3.getId()));
    bool res = ENGINE->propagate();
    assertTrue(res);
    assertTrue(v2.getDerivedDomain().getUpperBound() == 100);
    assertTrue(v3.getDerivedDomain().getLowerBound() == 2);

    // Handle restriction to singleton
    Variable<IntervalIntDomain> v4(ENGINE, IntervalIntDomain(0, 10));
    Variable<IntervalIntDomain> v5(ENGINE, IntervalIntDomain(5, 15));
    Variable<IntervalIntDomain> v6(ENGINE, IntervalIntDomain(0, 100));
    LessThanEqualConstraint c3(LabelStr("LessThanEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v4.getId(), v5.getId()));
    EqualConstraint c4(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v5.getId(), v6.getId()));
    res = ENGINE->propagate();
    assertTrue(res);
    v6.specify(9);
    res = ENGINE->propagate();
    assertTrue(res);
    assertTrue(v4.getDerivedDomain().getUpperBound() == 9);

    // Test with Numeric Domains
    NumericDomain realBaseDomain;
  
    realBaseDomain.insert(1.1);
    realBaseDomain.insert(2.2);
    realBaseDomain.insert(3.3);
 
    realBaseDomain.close();
 
    Variable<NumericDomain> realVar1(ENGINE, realBaseDomain);
    Variable<NumericDomain> realVar2(ENGINE, realBaseDomain);

    LessThanEqualConstraint cRealTest(LabelStr("LessThanEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(realVar2.getId(), realVar1.getId()));
    
    res = ENGINE->propagate();
    assertTrue(res);
    
    realVar2.specify(2.2);
    res = ENGINE->propagate();
    assertTrue(res);
    assertTrue(realVar2.getDerivedDomain().getUpperBound() == 2.2);

    return(true);
  }

  static bool testLessOrEqThanSumConstraint() {
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(0, 100));
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(0, 100));
    Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(0, 100));
    LessOrEqThanSumConstraint c0(LabelStr("LessOrEqThanSumConstraint"), LabelStr("Default"), ENGINE,
                                 makeScope(v0.getId(), v1.getId(), v2.getId()));
    bool res = ENGINE->propagate();
    assertTrue(res);
    assertTrue(ENGINE->constraintConsistent());
    assertTrue(v0.getDerivedDomain() == v1.getDerivedDomain());
    assertTrue(v1.getDerivedDomain() == v2.getDerivedDomain());

    v1.specify(IntervalIntDomain(0, 50));
    v2.specify(IntervalIntDomain(0, 40));
    assertTrue(v0.getDerivedDomain() == IntervalIntDomain(0, 90));
    v0.specify(IntervalIntDomain(60, 70));
    assertTrue(v1.getDerivedDomain() == IntervalIntDomain(20, 50));
    assertTrue(v2.getDerivedDomain() == IntervalIntDomain(10, 40));
    v1.reset();
    assertTrue(v0.getDerivedDomain() == IntervalIntDomain(60, 70));
    assertTrue(v1.getDerivedDomain() == IntervalIntDomain(20, 100));
    assertTrue(v2.getDerivedDomain() == IntervalIntDomain(0, 40));

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
    assertTrue(!v0.getDerivedDomain().isEmpty());

    // v4 + v5 == v1
    Variable<IntervalIntDomain> v4(ENGINE, IntervalIntDomain(1, 10));
    Variable<IntervalIntDomain> v5(ENGINE, IntervalIntDomain(1, 1000));
    AddEqualConstraint c2(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v4.getId(), v5.getId(), v1.getId()));

    ENGINE->propagate();
    assertTrue(ENGINE->constraintConsistent());
    assertTrue(!v4.getDerivedDomain().isEmpty());
    return true;
  }

  static bool testDeactivation(){
    // v0 == v1
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, 10));
    EqualConstraint c0(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId()));

    ENGINE->propagate();
    assertTrue(ENGINE->constraintConsistent());

    v0.deactivate();
    assertTrue(!c0.isActive());
    assertTrue(c0.deactivationCount() == 1);

    v1.deactivate();
    assertTrue(!c0.isActive());
    assertTrue(c0.deactivationCount() == 2);

    c0.deactivate();
    assertTrue(c0.deactivationCount() == 3);
    v0.undoDeactivation();
    v1.undoDeactivation();
    assertTrue(c0.deactivationCount() == 1);
    assertTrue(!c0.isActive());

    v1.deactivate();
    assertTrue(!c0.isActive());
    assertTrue(c0.deactivationCount() == 2);
    v1.undoDeactivation();
    assertTrue(c0.deactivationCount() == 1);
    c0.undoDeactivation();
    assertTrue(c0.isActive());

    // Now restrict the base domains to automatically deactivate
    v0.restrictBaseDomain(IntervalIntDomain(1, 1));
    assertTrue(c0.isActive());
    v1.restrictBaseDomain(IntervalIntDomain(1, 1));
    assertTrue(c0.isActive()); // Have not propagated yet!
    ENGINE->propagate();
    assertFalse(c0.isActive()); // Now we have propagated, so should be deactivated.

    // Make sure it stays deactive
    c0.undoDeactivation();
    assertFalse(c0.isActive());

    Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(1, 10));
    Variable<IntervalIntDomain> v3(ENGINE, IntervalIntDomain(1, 10));
    EqualConstraint c1(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v2.getId(), v3.getId()));
    ENGINE->propagate();
    v2.restrictBaseDomain(IntervalIntDomain(1, 1));
    v3.restrictBaseDomain(IntervalIntDomain(2, 2));

    // Now propagate. The constraint will not be deactivated.
    ENGINE->propagate();
    assertTrue(c1.isActive());

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
    assertTrue(ENGINE->provenInconsistent());
    assertTrue(v1.getDerivedDomain().isEmpty() || v2.getDerivedDomain().isEmpty());

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
      if(id->lastDomain().isEmpty())
	emptyCount++;
    }
    assertTrue(emptyCount == 1);
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
    assertTrue(ENGINE->constraintConsistent());
    v0.specify(IntervalIntDomain(8, 10));
    v1.specify(IntervalIntDomain(2, 7));
    assertTrue(ENGINE->pending());

    ENGINE->propagate();
    assertTrue(ENGINE->provenInconsistent());

    v0.reset();
    assertTrue(ENGINE->pending());
    ENGINE->propagate();
    assertTrue(ENGINE->constraintConsistent());

    /* Call reset on a constraint consistent network - not sure one would want to do this. */
    v1.reset();
    assertTrue(ENGINE->pending()); /* Strictly speaking we know it is not inconsistent here since all we have done is relax a previously
				  consistent network. However, we have to propagate to find the new derived domains based on relaxed
				  domains. */
    ENGINE->propagate();
    assertTrue(ENGINE->constraintConsistent());
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
    assertTrue(ENGINE->constraintConsistent());

    /* Show that we can simply delete a constraint and confirm that the system is still consistent. */
    delete (Constraint*) c1;
    ENGINE->propagate();
    assertTrue(ENGINE->constraintConsistent());

    variables.clear();
    Variable<IntervalIntDomain> v4(ENGINE, IntervalIntDomain(1, 1));
    variables.push_back(v0.getId());
    variables.push_back(v4.getId());
    ConstraintId c2((new EqualConstraint(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, variables))->getId());
    ENGINE->propagate();
    assertTrue(ENGINE->constraintConsistent());
    assertTrue(v1.getDerivedDomain().getSingletonValue() == 1);

    delete (Constraint*) c2;
    ENGINE->propagate();
    assertTrue(ENGINE->constraintConsistent());
    assertTrue(v1.getDerivedDomain().getUpperBound() == 10);

    /* Add a constraint to force an inconsistency and show that consistency can be restored by removing the
     * constraint. */
    variables.clear();
    Variable<IntervalIntDomain> v5(ENGINE, IntervalIntDomain(0, 0));
    variables.push_back(v0.getId());
    variables.push_back(v5.getId());
    ConstraintId c3((new EqualConstraint(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, variables))->getId());
    ENGINE->propagate();
    assertTrue(ENGINE->provenInconsistent());
    delete (Constraint*) c3;
    ENGINE->propagate();
    assertTrue(ENGINE->constraintConsistent());

    // Clean up remaining constraint
    delete (Constraint*) c0;
    ENGINE->propagate();
    assertTrue(ENGINE->constraintConsistent());
    return true;
  }

  static bool testDelegation(){
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(0, 1000));
    ConstraintId c0 = ConstraintLibrary::createConstraint(LabelStr("TestOnly"), ENGINE, makeScope(v0.getId()));
    ConstraintId c1 = ConstraintLibrary::createConstraint(LabelStr("TestOnly"), ENGINE, makeScope(v0.getId()));
    ConstraintId c2 = ConstraintLibrary::createConstraint(LabelStr("TestOnly"), ENGINE, makeScope(v0.getId()));
    ConstraintId c3 = ConstraintLibrary::createConstraint(LabelStr("TestOnly"), ENGINE, makeScope(v0.getId()));
    ConstraintId c4 = ConstraintLibrary::createConstraint(LabelStr("TestOnly"), ENGINE, makeScope(v0.getId()));
    ENGINE->propagate();
    assertTrue(ENGINE->constraintConsistent());
    assertTrue(DelegationTestConstraint::s_instanceCount == 5);
    assertTrue(DelegationTestConstraint::s_executionCount == 5);

    // Cause a change in the domain which will impact agenda, then deactivate a constraint and verify the correct execution count
    v0.specify(IntervalIntDomain(0, 900));
    c0->deactivate();
    assertTrue(!c0->isActive());
    ENGINE->propagate();
    assertTrue(ENGINE->constraintConsistent());
    assertTrue(DelegationTestConstraint::s_instanceCount == 5);
    assertTrue(DelegationTestConstraint::s_executionCount == 9);

    // Delete the delegate and verify instance counts and that the prior delegate has been reinstated and executed.
    delete (Constraint*) c1;
    c0->undoDeactivation();
    ENGINE->propagate();
    assertTrue(ENGINE->constraintConsistent());
    assertTrue(DelegationTestConstraint::s_instanceCount == 4);
    assertTrue(DelegationTestConstraint::s_executionCount == 10);

    // Now create a new instance and mark it for delegation only. Add remaining constraints as delegates
    ConstraintId c5 = ConstraintLibrary::createConstraint(LabelStr("TestOnly"), ENGINE, makeScope(v0.getId()));
    c0->deactivate();
    c2->deactivate();
    c3->deactivate();
    c4->deactivate();
    assertTrue(DelegationTestConstraint::s_instanceCount == 5);
    ENGINE->propagate();
    assertTrue(DelegationTestConstraint::s_executionCount == 11);

    // Force propagation and confirm only one instance executes
    v0.specify(IntervalIntDomain(100, 900));
    ENGINE->propagate();
    assertTrue(DelegationTestConstraint::s_executionCount == 12);

    // Now confirm correct handling of constraint deletions
    delete (Constraint*) c5;
    delete (Constraint*) c4;
    delete (Constraint*) c3;
    delete (Constraint*) c2;
    delete (Constraint*) c0;
    assertTrue(DelegationTestConstraint::s_instanceCount == 0);
    return true;
  }

  static bool testNotEqual(){
    NumericDomain dom0;
    dom0.insert(1);
    dom0.insert(2);
    dom0.insert(3);
    dom0.close();

    Variable<NumericDomain> v0(ENGINE, dom0);
    Variable<NumericDomain> v1(ENGINE, dom0);
    Variable<NumericDomain> v2(ENGINE, dom0);

    // Test not equals among variables and singletons
    IntervalIntDomain dom1(1);
    IntervalIntDomain dom2(2);
    NumericDomain dom3;
    dom3.insert(1);
    dom3.insert(2);
    dom3.insert(3);
    dom3.close();
    Variable<IntervalIntDomain> v3(ENGINE, dom1); 
    Variable<NumericDomain> v4(ENGINE, dom3);
    Variable<IntervalIntDomain> v5(ENGINE, dom2); 

    NotEqualConstraint c4(LabelStr("neq"), LabelStr("Default"), ENGINE, makeScope(v4.getId(), v5.getId()));
    NotEqualConstraint c3(LabelStr("neq"), LabelStr("Default"), ENGINE, makeScope(v4.getId(), v3.getId()));
    assertTrue(ENGINE->pending());
    bool res = ENGINE->propagate();
    assertTrue(res);
    assertTrue(v4.getDerivedDomain().isSingleton());
    assertTrue(v4.getDerivedDomain().getSingletonValue() == 3);

    // Test not equals among variables which are not singletons

    NotEqualConstraint c0(LabelStr("neq"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId()));
    NotEqualConstraint c1(LabelStr("neq"), LabelStr("Default"), ENGINE, makeScope(v1.getId(), v2.getId()));
    NotEqualConstraint c2(LabelStr("neq"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v2.getId()));
    assertTrue(ENGINE->pending());
    res = ENGINE->propagate();
    assertTrue(res);

    dom0.remove(2);
    v0.specify(dom0);
    assertTrue(!ENGINE->pending()); // No propagation required

    v1.specify(3);
    assertTrue(ENGINE->pending());
    res = ENGINE->propagate();
    assertTrue(res);
    assertTrue(v0.getDerivedDomain().getSingletonValue() == 1);
    assertTrue(v2.getDerivedDomain().getSingletonValue() == 2);

    v0.reset();
    assertTrue(ENGINE->pending());
    res = ENGINE->propagate();
    assertTrue(res);
    assertTrue(v0.getDerivedDomain() == v2.getDerivedDomain());
    return true;
  }

  static bool testMultEqualConstraint(){
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, 1));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(0, 2));
      MultEqualConstraint c0(LabelStr("MultEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(v0.getDerivedDomain() == v2.getDerivedDomain());
    }

    // Now test with 0 valued denominators and infinites
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(0, PLUS_INFINITY));
      Variable<IntervalDomain> v1(ENGINE, IntervalDomain(1, PLUS_INFINITY));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(MINUS_INFINITY, 6));
      MultEqualConstraint c0(LabelStr("MultEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(v0.getDerivedDomain().getUpperBound() == 6);
      assertTrue(v2.getDerivedDomain().getLowerBound() == 0);
      assertTrue(v1.getDerivedDomain().getUpperBound() == PLUS_INFINITY);
    }


    // Special case of negative values on LHS
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(-4, 10));
      Variable<IntervalDomain> v1(ENGINE, IntervalDomain(1, 10));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain());
      MultEqualConstraint c0(LabelStr("MultEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(v2.getDerivedDomain().getLowerBound() == -40);
    }

    // Part of test 157 of CLibTestCases, which incorrectly claimed to be
    // inconsistent during testing of a trial version of MultEqualConstraint.
    {
      Variable<IntervalDomain> v0(ENGINE, IntervalDomain(1.0));
      Variable<IntervalDomain> v1(ENGINE, IntervalDomain(MINUS_INFINITY, PLUS_INFINITY));
      Variable<IntervalDomain> v2(ENGINE, IntervalDomain(1.0, 2.0));
      MultEqualConstraint c0(LabelStr("MultEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(v1.getDerivedDomain() == IntervalDomain(1.0, 2.0));
    }
    // Signs tested
    {
      Variable<IntervalDomain> v0(ENGINE, IntervalDomain(-2.3, -1.8));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalDomain(1, PLUS_INFINITY));
      Variable<IntervalDomain> v2(ENGINE, IntervalDomain(-11.6, 12.4));
      MultEqualConstraint c0(LabelStr("MultEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(v0.getDerivedDomain() == IntervalDomain(-2.3, -1.8), v0.getDerivedDomain().toString());
      assertTrue(v1.getDerivedDomain() == IntervalIntDomain(1, 6), v1.getDerivedDomain().toString());
      assertTrue(v2.getDerivedDomain() == IntervalDomain(-11.6, -1.8), v2.getDerivedDomain().toString());
    }

    // Signs tested
    {
      Variable<IntervalDomain> v1(ENGINE, IntervalDomain(-2.3, -1.8));
      Variable<IntervalIntDomain> v0(ENGINE, IntervalDomain(1, PLUS_INFINITY));
      Variable<IntervalDomain> v2(ENGINE, IntervalDomain(-11.6, 12.4));
      MultEqualConstraint c0(LabelStr("MultEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(v1.getDerivedDomain() == IntervalDomain(-2.3, -1.8), v0.getDerivedDomain().toString());
      assertTrue(v0.getDerivedDomain() == IntervalIntDomain(1, 6), v1.getDerivedDomain().toString());
      assertTrue(v2.getDerivedDomain() == IntervalDomain(-11.6, -1.8), v2.getDerivedDomain().toString());
    }
    return true;
  }

  static bool testAddMultEqualConstraint() {
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
      bool res = ENGINE->propagate();
      assertTrue(res);
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
      bool res = ENGINE->propagate();
      assertTrue(!res);
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
      bool res = ENGINE->propagate();
      assertTrue(res);
      assertTrue(v2.getDerivedDomain().getSingletonValue() == 0);
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
      bool res = ENGINE->propagate();
      assertTrue(res);
      assertTrue(v0.getDerivedDomain() == IntervalIntDomain(1, 9));
      assertTrue(v2.getDerivedDomain() == IntervalDomain(1.0, 9.0));
    }


    return true;
  }

  static bool testEqualSumConstraint() {
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, 1));
    Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(0, 2));
    Variable<IntervalIntDomain> v3(ENGINE, IntervalIntDomain(0, 27));
    Variable<IntervalIntDomain> v4(ENGINE, IntervalIntDomain(0, 27));
    Variable<IntervalIntDomain> v5(ENGINE, IntervalIntDomain(0, 27));
    Variable<IntervalIntDomain> v6(ENGINE, IntervalIntDomain(0, 27));
    Variable<IntervalIntDomain> v7(ENGINE, IntervalIntDomain(0, 27));
    Variable<IntervalIntDomain> v8(ENGINE, IntervalIntDomain(0, 27));
    Variable<IntervalIntDomain> v9(ENGINE, IntervalIntDomain(0, 27));
    Variable<IntervalIntDomain> vA(ENGINE, IntervalIntDomain(0, 27));
    Variable<IntervalIntDomain> vB(ENGINE, IntervalIntDomain(0, 27));
    Variable<IntervalIntDomain> vC(ENGINE, IntervalIntDomain(0, 27));
    Variable<IntervalIntDomain> vD(ENGINE, IntervalIntDomain(0, 27));
    Variable<IntervalIntDomain> vE(ENGINE, IntervalIntDomain(0, 27));
    Variable<IntervalIntDomain> vF(ENGINE, IntervalIntDomain(0, 27));
    Variable<IntervalIntDomain> vG(ENGINE, IntervalIntDomain(0, 27));
    { // Duplicate first test case in testAddEqualConstraint(),
      //   but note args are in different order in scope to get same result
      EqualSumConstraint c0(LabelStr("EqualSumConstraint"), LabelStr("Default"), ENGINE, makeScope(v2.getId(), v0.getId(), v1.getId()));
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(v0.getDerivedDomain().getSingletonValue() == 1);
      assertTrue(v1.getDerivedDomain().getSingletonValue() == 1);
      assertTrue(v2.getDerivedDomain().getSingletonValue() == 2);
    }
    { // Same, but add another variable that will be constrained to 0
      std::vector<ConstrainedVariableId> scope;
      scope.push_back(v2.getId());
      scope.push_back(v0.getId());
      scope.push_back(v1.getId());
      scope.push_back(v3.getId());
      EqualSumConstraint c0(LabelStr("EqualSumConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(v0.getDerivedDomain().getSingletonValue() == 1);
      assertTrue(v1.getDerivedDomain().getSingletonValue() == 1);
      assertTrue(v2.getDerivedDomain().getSingletonValue() == 2);
      assertTrue(v3.getDerivedDomain().getSingletonValue() == 0);
    }
    { // Same, but add more variables that will be constrained to 0
      std::vector<ConstrainedVariableId> scope;
      scope.push_back(v2.getId());
      scope.push_back(v0.getId());
      scope.push_back(v1.getId());
      scope.push_back(v3.getId());
      scope.push_back(v4.getId());
      scope.push_back(v5.getId());
      scope.push_back(v6.getId());
      scope.push_back(v7.getId());
      scope.push_back(v8.getId());
      scope.push_back(v9.getId());
      scope.push_back(vA.getId());
      scope.push_back(vB.getId());
      scope.push_back(vC.getId());
      scope.push_back(vD.getId());
      scope.push_back(vE.getId());
      scope.push_back(vF.getId());
      scope.push_back(vG.getId());
      EqualSumConstraint c0(LabelStr("EqualSumConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(v0.getDerivedDomain().getSingletonValue() == 1);
      assertTrue(v1.getDerivedDomain().getSingletonValue() == 1);
      assertTrue(v2.getDerivedDomain().getSingletonValue() == 2);
      assertTrue(v3.getDerivedDomain().getSingletonValue() == 0);
      assertTrue(v4.getDerivedDomain().getSingletonValue() == 0);
      assertTrue(v5.getDerivedDomain().getSingletonValue() == 0);
      assertTrue(v6.getDerivedDomain().getSingletonValue() == 0);
      assertTrue(v7.getDerivedDomain().getSingletonValue() == 0);
      assertTrue(v8.getDerivedDomain().getSingletonValue() == 0);
      assertTrue(v9.getDerivedDomain().getSingletonValue() == 0);
      assertTrue(vA.getDerivedDomain().getSingletonValue() == 0);
      assertTrue(vB.getDerivedDomain().getSingletonValue() == 0);
      assertTrue(vC.getDerivedDomain().getSingletonValue() == 0);
      assertTrue(vD.getDerivedDomain().getSingletonValue() == 0);
      assertTrue(vE.getDerivedDomain().getSingletonValue() == 0);
      assertTrue(vF.getDerivedDomain().getSingletonValue() == 0);
      assertTrue(vG.getDerivedDomain().getSingletonValue() == 0);
    }
    return(true);
  }

  static bool testCondAllSameConstraint() {
    BoolDomain bothDom;
    Variable<BoolDomain> bothVar(ENGINE, bothDom);
    Variable<BoolDomain> falseVar(ENGINE, bothDom);
    falseVar.specify(false);
    Variable<BoolDomain> trueVar(ENGINE, bothDom);
    trueVar.specify(true);
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, 1));
    Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(0, 2));
    Variable<IntervalIntDomain> v3(ENGINE, IntervalIntDomain(0, 27));
    Variable<IntervalIntDomain> v4(ENGINE, IntervalIntDomain(0, 27));
    Variable<IntervalIntDomain> v5(ENGINE, IntervalIntDomain(0, 27));
    Variable<IntervalIntDomain> v6(ENGINE, IntervalIntDomain(0));
    Variable<IntervalIntDomain> v7(ENGINE, IntervalIntDomain(0));
    Variable<IntervalIntDomain> v8(ENGINE, IntervalIntDomain(0));
    Variable<IntervalIntDomain> v9(ENGINE, IntervalIntDomain(1));
    Variable<IntervalIntDomain> vA(ENGINE, IntervalIntDomain(11, 27));
    std::vector<ConstrainedVariableId> scope;
    {
      scope.push_back(bothVar.getId());
      scope.push_back(v0.getId());
      scope.push_back(v1.getId());
      CondAllSameConstraint c0(LabelStr("CondAllSameConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(!bothVar.getDerivedDomain().isSingleton());
      assertTrue(v0.getDerivedDomain() == IntervalIntDomain(1, 10));
      assertTrue(v1.getDerivedDomain().getSingletonValue() == 1);
    }
    assertTrue(!bothVar.getDerivedDomain().isSingleton());
    scope.clear();
    {
      scope.push_back(bothVar.getId());
      scope.push_back(v0.getId());
      scope.push_back(vA.getId());
      CondAllSameConstraint c0(LabelStr("CondAllSameConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(bothVar.getDerivedDomain() == BoolDomain(false));
    }
    assertTrue(!bothVar.getDerivedDomain().isSingleton());
    scope.clear();
    {
      scope.push_back(bothVar.getId());
      scope.push_back(v6.getId());
      scope.push_back(v7.getId());
      scope.push_back(v8.getId());
      CondAllSameConstraint c0(LabelStr("CondAllSameConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(bothVar.getDerivedDomain() == BoolDomain(true));
    }
    assertTrue(!bothVar.getDerivedDomain().isSingleton());
    scope.clear();
    {
      scope.push_back(bothVar.getId());
      scope.push_back(v2.getId());
      scope.push_back(v6.getId());
      scope.push_back(v7.getId());
      scope.push_back(v8.getId());
      CondAllSameConstraint c0(LabelStr("CondAllSameConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(!bothVar.getDerivedDomain().isSingleton());
    }
    assertTrue(!bothVar.getDerivedDomain().isSingleton());
    scope.clear();
    {
      scope.push_back(bothVar.getId());
      scope.push_back(v6.getId());
      scope.push_back(v7.getId());
      scope.push_back(v8.getId());
      scope.push_back(v2.getId());
      CondAllSameConstraint c0(LabelStr("CondAllSameConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(!bothVar.getDerivedDomain().isSingleton());
    }
    assertTrue(!bothVar.getDerivedDomain().isSingleton());
    scope.clear();
    {
      scope.push_back(bothVar.getId());
      scope.push_back(v6.getId());
      scope.push_back(v7.getId());
      scope.push_back(v2.getId());
      scope.push_back(v8.getId());
      CondAllSameConstraint c0(LabelStr("CondAllSameConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(!bothVar.getDerivedDomain().isSingleton());
    }
    assertTrue(!bothVar.getDerivedDomain().isSingleton());
    scope.clear();
    {
      scope.push_back(trueVar.getId());
      scope.push_back(v2.getId());
      scope.push_back(v6.getId());
      scope.push_back(v7.getId());
      CondAllSameConstraint c0(LabelStr("CondAllSameConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(v2.getDerivedDomain() == IntervalIntDomain(0));
    }
    scope.clear();
    {
      scope.push_back(trueVar.getId());
      scope.push_back(v2.getId());
      scope.push_back(v3.getId());
      scope.push_back(v6.getId());
      scope.push_back(v7.getId());
      CondAllSameConstraint c0(LabelStr("CondAllSameConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(v2.getDerivedDomain() == IntervalIntDomain(0));
      assertTrue(v3.getDerivedDomain() == IntervalIntDomain(0));
    }
    scope.clear();
    {
      scope.push_back(trueVar.getId());
      scope.push_back(v2.getId());
      scope.push_back(v3.getId());
      CondAllSameConstraint c0(LabelStr("CondAllSameConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(v2.getDerivedDomain() == IntervalIntDomain(0, 2));
      assertTrue(v3.getDerivedDomain() == IntervalIntDomain(0, 2));
    }
    scope.clear();
    {
      scope.push_back(trueVar.getId());
      scope.push_back(v1.getId());
      scope.push_back(v2.getId());
      scope.push_back(v3.getId());
      CondAllSameConstraint c0(LabelStr("CondAllSameConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(v2.getDerivedDomain() == IntervalIntDomain(1));
      assertTrue(v3.getDerivedDomain() == IntervalIntDomain(1));
    }
    assertTrue(v2.getDerivedDomain() == IntervalIntDomain(0, 2));
    assertTrue(v3.getDerivedDomain() == IntervalIntDomain(0, 27));
    scope.clear();
    {
      scope.push_back(falseVar.getId());
      scope.push_back(v3.getId());
      scope.push_back(v6.getId());
      scope.push_back(v7.getId());
      CondAllSameConstraint c0(LabelStr("CondAllSameConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(v3.getDerivedDomain().isSubsetOf(IntervalIntDomain(1, 27)));
    }
    scope.clear();
    {
      scope.push_back(falseVar.getId());
      scope.push_back(v6.getId());
      scope.push_back(v7.getId());
      CondAllSameConstraint c0(LabelStr("CondAllSameConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      assertTrue(!ENGINE->constraintConsistent());
    }
    ENGINE->propagate();
    assertTrue(ENGINE->constraintConsistent());
    scope.clear();
    {
      scope.push_back(falseVar.getId());
      scope.push_back(v2.getId());
      scope.push_back(v3.getId());
      CondAllSameConstraint c0(LabelStr("CondAllSameConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(v2.getDerivedDomain() == IntervalIntDomain(0, 2));
      assertTrue(v3.getDerivedDomain() == IntervalIntDomain(0, 27));
    }
    ENGINE->propagate();
    assertTrue(ENGINE->constraintConsistent());
    scope.clear();
    {
      scope.push_back(trueVar.getId());
      scope.push_back(v0.getId());
      scope.push_back(vA.getId());
      CondAllSameConstraint c0(LabelStr("CondAllSameConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      assertTrue(!ENGINE->constraintConsistent());
    }
    ENGINE->propagate();
    assertTrue(ENGINE->constraintConsistent());
    assertTrue(!bothVar.getDerivedDomain().isSingleton());
    scope.clear();
    return(true);
  }

  static bool testCondAllDiffConstraint() {
    BoolDomain bothDom;
    Variable<BoolDomain> bothVar(ENGINE, bothDom);
    Variable<BoolDomain> falseVar(ENGINE, bothDom);
    falseVar.specify(false);
    Variable<BoolDomain> trueVar(ENGINE, bothDom);
    trueVar.specify(true);
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, 1));
    Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(0, 2));
    Variable<IntervalIntDomain> v3(ENGINE, IntervalIntDomain(0, 27));
    Variable<IntervalIntDomain> v4(ENGINE, IntervalIntDomain(0, 27));
    Variable<IntervalIntDomain> v5(ENGINE, IntervalIntDomain(0, 27));
    Variable<IntervalIntDomain> v6(ENGINE, IntervalIntDomain(0));
    Variable<IntervalIntDomain> v7(ENGINE, IntervalIntDomain(0));
    Variable<IntervalIntDomain> v8(ENGINE, IntervalIntDomain(0));
    Variable<IntervalIntDomain> v9(ENGINE, IntervalIntDomain(1));
    Variable<IntervalIntDomain> vA(ENGINE, IntervalIntDomain(11, 27));
    Variable<IntervalIntDomain> vB(ENGINE, IntervalIntDomain(-1, 2));
    Variable<IntervalIntDomain> vC(ENGINE, IntervalIntDomain(10, 11));
    Variable<IntervalIntDomain> vD(ENGINE, IntervalIntDomain(10, 11));
    Variable<IntervalIntDomain> vE(ENGINE, IntervalIntDomain(10, 11));
    Variable<IntervalIntDomain> vF(ENGINE, IntervalIntDomain(0, 3));
    Variable<IntervalIntDomain> vG(ENGINE, IntervalIntDomain(9, 12));
    Variable<IntervalIntDomain> vH(ENGINE, IntervalIntDomain(3, 5));
    Variable<IntervalIntDomain> vI(ENGINE, IntervalIntDomain(0, 7));
    Variable<IntervalIntDomain> vJ(ENGINE, IntervalIntDomain(5, 9));
    Variable<IntervalIntDomain> vK(ENGINE, IntervalIntDomain(9));
    Variable<IntervalIntDomain> vL(ENGINE, IntervalIntDomain(5, 13));
    Variable<IntervalIntDomain> vM(ENGINE, IntervalIntDomain(5));
    Variable<IntervalIntDomain> vN(ENGINE, IntervalIntDomain(-1, 15));
    Variable<IntervalIntDomain> vO(ENGINE, IntervalIntDomain(-2, 6));
    Variable<IntervalIntDomain> vP(ENGINE, IntervalIntDomain(-2));
    Variable<IntervalIntDomain> vQ(ENGINE, IntervalIntDomain(15));
    std::vector<ConstrainedVariableId> scope;
    {
      scope.push_back(bothVar.getId());
      scope.push_back(v0.getId());
      scope.push_back(v1.getId());
      CondAllDiffConstraint c0(LabelStr("CondAllDiffConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(!bothVar.getDerivedDomain().isSingleton());
      assertTrue(v0.getDerivedDomain() == IntervalIntDomain(1, 10));
      assertTrue(v1.getDerivedDomain().getSingletonValue() == 1);
    }
    scope.clear();
    {
      scope.push_back(bothVar.getId());
      scope.push_back(v0.getId());
      scope.push_back(v6.getId());
      scope.push_back(vA.getId());
      CondAllDiffConstraint c0(LabelStr("CondAllDiffConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(bothVar.getDerivedDomain().getSingletonValue());
      assertTrue(v0.getDerivedDomain() == IntervalIntDomain(1, 10));
      assertTrue(v6.getDerivedDomain().getSingletonValue() == 0);
      assertTrue(vA.getDerivedDomain() == IntervalIntDomain(11, 27));
    }
    assertTrue(!bothVar.getDerivedDomain().isSingleton());
    scope.clear();
    {
      scope.push_back(bothVar.getId());
      scope.push_back(v0.getId());
      scope.push_back(v6.getId());
      scope.push_back(v9.getId());
      scope.push_back(vA.getId());
      CondAllDiffConstraint c0(LabelStr("CondAllDiffConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(!bothVar.getDerivedDomain().isSingleton());
      assertTrue(v0.getDerivedDomain() == IntervalIntDomain(1, 10));
      assertTrue(v6.getDerivedDomain().getSingletonValue() == 0);
      assertTrue(v9.getDerivedDomain().getSingletonValue() == 1);
      assertTrue(vA.getDerivedDomain() == IntervalIntDomain(11, 27));
    }
    scope.clear();
    {
      scope.push_back(trueVar.getId());
      scope.push_back(v0.getId()); // 1 10 so far
      scope.push_back(v2.getId()); // 1 10, 0 2
      scope.push_back(v6.getId()); // 1 10, 1 2, 0
      scope.push_back(v9.getId()); // 3 10, 2, 0, 1
      scope.push_back(vA.getId()); // 3 10, 2, 0, 1, 11 27
      scope.push_back(vB.getId()); // 3 10, 2, 0, 1, 11 27, -1
      CondAllDiffConstraint c0(LabelStr("CondAllDiffConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(trueVar.getDerivedDomain().isSingleton());
      assertTrue(v0.getDerivedDomain() == IntervalIntDomain(3, 10));
      assertTrue(v2.getDerivedDomain().getSingletonValue() == 2);
      assertTrue(v6.getDerivedDomain().getSingletonValue() == 0);
      assertTrue(v9.getDerivedDomain().getSingletonValue() == 1);
      assertTrue(vA.getDerivedDomain() == IntervalIntDomain(11, 27));
      assertTrue(vB.getDerivedDomain().getSingletonValue() == -1);
    }
    assertTrue(v0.getDerivedDomain() == IntervalIntDomain(1, 10));
    assertTrue(v2.getDerivedDomain() == IntervalIntDomain(0, 2));
    assertTrue(vB.getDerivedDomain() == IntervalIntDomain(-1, 2));
    scope.clear();
    {
      scope.push_back(falseVar.getId());
      scope.push_back(v0.getId()); // 1 10
      scope.push_back(v6.getId()); // 1 10, 0 (inconsistent)
      scope.push_back(v9.getId()); // 1, 0, 1
      scope.push_back(vA.getId()); // 1, 0, 1, 11 27
      CondAllDiffConstraint c0(LabelStr("CondAllDiffConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      // This next restriction is correct but the current implementation
      //   does not enforce it, as it requires checking all pairs to see
      //   that two individual vars are the only ones that overlap and
      //   therefore must be equal.
      // assertTrue(v0.getDerivedDomain().getSingletonValue() == 1);
      assertTrue(v6.getDerivedDomain().getSingletonValue() == 0);
      assertTrue(v9.getDerivedDomain().getSingletonValue() == 1);
      assertTrue(vA.getDerivedDomain() == IntervalIntDomain(11, 27));
    }
    scope.clear();
    {
      scope.push_back(bothVar.getId());
      scope.push_back(v6.getId()); // 0
      scope.push_back(v7.getId()); // 0, 0 -> false; rest irrelevant
      scope.push_back(v8.getId());
      scope.push_back(v9.getId());
      scope.push_back(vA.getId());
      CondAllDiffConstraint c0(LabelStr("CondAllDiffConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(!bothVar.getDerivedDomain().getSingletonValue());
      assertTrue(v6.getDerivedDomain() == IntervalIntDomain(0));
      assertTrue(v7.getDerivedDomain() == IntervalIntDomain(0));
      assertTrue(v8.getDerivedDomain() == IntervalIntDomain(0));
      assertTrue(v9.getDerivedDomain() == IntervalIntDomain(1));
      assertTrue(vA.getDerivedDomain() == IntervalIntDomain(11, 27));
    }
    assertTrue(!bothVar.getDerivedDomain().isSingleton());
    scope.clear();
    {
      scope.push_back(bothVar.getId());
      scope.push_back(vC.getId()); // 10 11
      scope.push_back(vD.getId()); // 10 11, 10 11
      CondAllDiffConstraint c0(LabelStr("CondAllDiffConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(!bothVar.getDerivedDomain().isSingleton());
    }
    scope.clear();
    {
      scope.push_back(bothVar.getId());
      scope.push_back(vC.getId()); // 10 11
      scope.push_back(vD.getId()); // 10 11, 10 11
      scope.push_back(vE.getId()); // 10 11, 10 11, 10 11 -> false: three vars but only two values
      CondAllDiffConstraint c0(LabelStr("CondAllDiffConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(!bothVar.getDerivedDomain().getSingletonValue());
    }
    assertTrue(!bothVar.getDerivedDomain().isSingleton());
    scope.clear();
    {
      scope.push_back(trueVar.getId());
      scope.push_back(v0.getId()); // 1 10
      scope.push_back(vA.getId()); // 1 10, 11 27
      scope.push_back(vB.getId()); // 1 10, 11 27, -1 2
      scope.push_back(vC.getId()); // 1 10, 11 27, -1 2, 10 11
      scope.push_back(vD.getId()); // 1 10, 11 27, -1 2, 10 11, 10 11
      CondAllDiffConstraint c0(LabelStr("CondAllDiffConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      // 10 could be removed from v0 and 11 from vA, but current
      //   implementation doesn't check for such things.
    }
    scope.clear();
    {
      scope.push_back(trueVar.getId());
      scope.push_back(v0.getId()); // 1 10 so far
      scope.push_back(v1.getId()); // 2 10, 1 (since condition is true, remove singletons from all others)
      scope.push_back(v2.getId()); // 2 10, 1, 0 2
      scope.push_back(v3.getId()); // 2 10, 1, 0 2, 0 27
      scope.push_back(v4.getId()); // 2 10, 1, 0 2, 0 27, 0 27
      scope.push_back(v5.getId()); // 2 10, 1, 0 2, 0 27, 0 27, 0 27
      scope.push_back(v6.getId()); // 3 10, 1, 2, 3 27, 3 27, 3 27, 0
      scope.push_back(vA.getId()); // 3 10, 1, 2, 3 27, 3 27, 3 27, 0, 11 27
      scope.push_back(vB.getId()); // 3 10, 1, 2, 3 27, 3 27, 3 27, 0, 11 27, -1
      scope.push_back(vC.getId()); // 3 10, 1, 2, 3 27, 3 27, 3 27, 0, 11 27, -1, 10 11
      scope.push_back(vF.getId()); // 4 10, 1, 2, 4 27, 4 27, 4 27, 0, 11 27, -1, 10 11, 3
      scope.push_back(vG.getId()); // 4 10, 1, 2, 4 27, 4 27, 4 27, 0, 11 27, -1, 10 11, 3, 9 12
      scope.push_back(vH.getId()); // 4 10, 1, 2, 4 27, 4 27, 4 27, 0, 11 27, -1, 10 11, 3, 9 12, 4 5
      scope.push_back(vI.getId()); // 4 10, 1, 2, 4 27, 4 27, 4 27, 0, 11 27, -1, 10 11, 3, 9 12, 4 5, 4 7
      scope.push_back(vJ.getId()); // 4 10, 1, 2, 4 27, 4 27, 4 27, 0, 11 27, -1, 10 11, 3, 9 12, 4 5, 4 7, 5 9
      scope.push_back(vK.getId()); // 4 10, 1, 2, 4 27, 4 27, 4 27, 0, 11 27, -1, 10 11, 3, 10 12, 4 5, 4 7, 5 8, 9
      scope.push_back(vL.getId()); // 4 10, 1, 2, 4 27, 4 27, 4 27, 0, 11 27, -1, 10 11, 3, 10 12, 4 5, 4 7, 5 8, 9, 5 13
      scope.push_back(vM.getId()); // 6 10, 1, 2, 6 27, 6 27, 6 27, 0, 11 27, -1, 10 11, 3, 10 12, 4, 6 7, 6 8, 9, 6 13, 5
      scope.push_back(vN.getId()); // 6 10, 1, 2, 6 27, 6 27, 6 27, 0, 11 27, -1, 10 11, 3, 10 12, 4, 6 7, 6 8, 9, 6 13, 5, 6 15
      scope.push_back(vO.getId()); // 6 10, 1, 2, 6 27, 6 27, 6 27, 0, 11 27, -1, 10 11, 3, 10 12, 4, 6 7, 6 8, 9, 6 13, 5, 6 15, -2 6
      scope.push_back(vP.getId()); // 10, 1, 2, 14 27, 14 27, 14 27, 0, 14 27, -1, 11, 3, 12, 4, 7, 8, 9, 13, 5, 14 15, 6, -2
      scope.push_back(vQ.getId()); // 10, 1, 2, 16 27, 16 27, 16 27, 0, 16 27, -1, 11, 3, 12, 4, 7, 8, 9, 13, 5, 14, 6, -2, 15
      CondAllDiffConstraint c0(LabelStr("CondAllDiffConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(v0.getDerivedDomain() == IntervalIntDomain(10));
      assertTrue(v1.getDerivedDomain() == IntervalIntDomain(1));
      assertTrue(v2.getDerivedDomain() == IntervalIntDomain(2));
      assertTrue(v3.getDerivedDomain() == IntervalIntDomain(16, 27));
      assertTrue(v4.getDerivedDomain() == IntervalIntDomain(16, 27));
      assertTrue(v5.getDerivedDomain() == IntervalIntDomain(16, 27));
      assertTrue(v6.getDerivedDomain() == IntervalIntDomain(0));
      assertTrue(vA.getDerivedDomain() == IntervalIntDomain(16, 27));
      assertTrue(vB.getDerivedDomain() == IntervalIntDomain(-1));
      assertTrue(vC.getDerivedDomain() == IntervalIntDomain(11));
      assertTrue(vF.getDerivedDomain() == IntervalIntDomain(3));
      assertTrue(vG.getDerivedDomain() == IntervalIntDomain(12));
      assertTrue(vH.getDerivedDomain() == IntervalIntDomain(4));
      assertTrue(vI.getDerivedDomain() == IntervalIntDomain(7));
      assertTrue(vJ.getDerivedDomain() == IntervalIntDomain(8));
      assertTrue(vK.getDerivedDomain() == IntervalIntDomain(9));
      assertTrue(vL.getDerivedDomain() == IntervalIntDomain(13));
      assertTrue(vM.getDerivedDomain() == IntervalIntDomain(5));
      assertTrue(vN.getDerivedDomain() == IntervalIntDomain(14));
      assertTrue(vO.getDerivedDomain() == IntervalIntDomain(6));
      assertTrue(vP.getDerivedDomain() == IntervalIntDomain(-2));
      assertTrue(vQ.getDerivedDomain() == IntervalIntDomain(15));
    }
    scope.clear();
    {
      scope.push_back(falseVar.getId());
      scope.push_back(v6.getId()); // 0
      scope.push_back(v7.getId()); // 0
      CondAllDiffConstraint c0(LabelStr("CondAllDiffConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(v6.getDerivedDomain().getSingletonValue() == 0);
      assertTrue(v7.getDerivedDomain().getSingletonValue() == 0);
    }
    scope.clear();
    {
      scope.push_back(falseVar.getId());
      scope.push_back(v3.getId()); // 0 27
      scope.push_back(v4.getId()); // 0 27
      CondAllDiffConstraint c0(LabelStr("CondAllDiffConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      assertTrue(v3.getDerivedDomain() == IntervalIntDomain(0, 27));
      assertTrue(v4.getDerivedDomain() == IntervalIntDomain(0, 27));
    }
    scope.clear();
    {
      scope.push_back(falseVar.getId());
      scope.push_back(vP.getId()); // -2
      scope.push_back(vF.getId()); // 0 3
      scope.push_back(vI.getId()); // 0 7
      scope.push_back(vC.getId()); // 10 11
      CondAllDiffConstraint c0(LabelStr("CondAllDiffConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      assertTrue(ENGINE->constraintConsistent());
      // Only vF and vI overlap, so they have to be equal.
      assertTrue(vF.getDerivedDomain() == IntervalIntDomain(0, 3));
      // This next restriction is correct but the current implementation
      //   does not enforce it, as it requires checking all pairs to see
      //   that two individual vars are the only ones that overlap and
      //   therefore must be equal.
      // assertTrue(vI.getDerivedDomain() == IntervalIntDomain(0, 3));
    }
    return(true);
  }

  static bool testConstraintDeletion() {
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
    bool res = ENGINE->propagate();
    assertTrue(!res);

    // Reset, and delete constraint, but it should not matter
    v1.reset();
    delete (Constraint*) c2;

    // Confirm still inconsistent
    res = ENGINE->propagate();
    assertTrue(!res);

    delete (Constraint*) c0;
    delete (Constraint*) c1;
    return true;
  }

  /**
   * @brief Run arbtrary constraints with arbitrary input domains,
   * comparing the propagated domains with the expected output domains.
   */
  static bool testArbitraryConstraints() {
    // Input to this test: a list of constraint calls and expected output domains.
    std::list<ConstraintTestCase> tests;

    // This kind of information can also be read from a file, as below.
    std::string constraintName("Equal");
    std::list<AbstractDomain*> domains;
    domains.push_back(new IntervalIntDomain(1, 10)); // first input domain
    domains.push_back(new IntervalIntDomain(2, 10)); // expected value of first output domain
    domains.push_back(new IntervalIntDomain(2, 11)); // second input domain
    domains.push_back(new IntervalIntDomain(2, 10)); // expected value of second output domain
    tests.push_back(ConstraintTestCase(constraintName, __FILE__, __LINE__, std::list<AbstractDomain*>(domains)));

    // Try reading "test cases" file of NewPlan/ModuleTests/ConstraintLibrary/testCLib,
    //   committed here as CLibTestCases after some minor editing to use '[]' for all
    //   numeric domains since Europa1 prints some of those using '{}' syntax and
    //   Europa2 treats as intervals all numeric domains that aren't explicitly
    //   identified as enumerations.
    // "NewTestCases" was written while tracking down the bugs in both
    //   Europa2 (PLASMA) and in Europa1 (NewPlan) discovered while testing
    //   "CLibTestCases".
    // For each file, try twice with different relative paths since we don't know what
    //   the current working directory is.
    assertTrue(readTestCases(std::string("NewTestCases"), tests) ||
               readTestCases(std::string("ConstraintEngine/test/NewTestCases"), tests));
    assertTrue(readTestCases(std::string("CLibTestCases"), tests) ||
               readTestCases(std::string("ConstraintEngine/test/CLibTestCases"), tests));

    return(executeTestCases(ENGINE, tests));
  }

  static bool testLockConstraint() {
    LabelSet lockDomain;
    lockDomain.insert(EUROPA::LabelStr("A"));
    lockDomain.insert(EUROPA::LabelStr("B"));
    lockDomain.insert(EUROPA::LabelStr("C"));
    lockDomain.insert(EUROPA::LabelStr("D"));
    lockDomain.close();

    LabelSet baseDomain;
    baseDomain.insert(EUROPA::LabelStr("A"));
    baseDomain.insert(EUROPA::LabelStr("B"));
    baseDomain.insert(EUROPA::LabelStr("C"));
    baseDomain.insert(EUROPA::LabelStr("D"));
    baseDomain.insert(EUROPA::LabelStr("E"));
    baseDomain.close();

    // Set up variable with base domain - will exceed lock domain
    Variable<LabelSet> v0(ENGINE, baseDomain);
    assertTrue(v0.getDerivedDomain() != lockDomain);
    Variable<LabelSet> v1(ENGINE, lockDomain);

    // Post constraint, and ensure it is propagated to equality with lock domain
    LockConstraint c0(LabelStr("Lock"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId()));
    assertTrue(ENGINE->propagate());
    assertTrue(v0.getDerivedDomain() == lockDomain);

    // Now specify to a restricted value, and ensure an inconsistency
    v0.specify(EUROPA::LabelStr("C"));
    assertTrue(!ENGINE->propagate());

    // Now ensure that we can rty again, without changing anything, and get the same result.
    assertTrue(!ENGINE->propagate());

    // Reset the variable and ensure we can repropagate it correctly
    v0.reset();
    assertTrue(ENGINE->propagate());
    assertTrue(v0.getDerivedDomain() == lockDomain);

    return true;
  }

  static bool testNegateConstraint() {
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain());
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain());
    NegateConstraint c0(LabelStr("NegateConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId()));
    ENGINE->propagate();
    assertTrue(ENGINE->constraintConsistent());
    assertTrue(v0.getDerivedDomain() == IntervalIntDomain(0, PLUS_INFINITY));
    assertTrue(v1.getDerivedDomain() == IntervalIntDomain(MINUS_INFINITY, 0));

    v0.specify(IntervalIntDomain(20, 30));
    assertTrue(v1.getDerivedDomain() == IntervalIntDomain(-30, -20));

    v1.specify(IntervalIntDomain(-22, -21));
    assertTrue(v0.getDerivedDomain() == IntervalIntDomain(21, 22));

    v0.specify(21);
    assertTrue(v1.getDerivedDomain().getSingletonValue() == -21);

    return true;
  }

  static bool testUnaryQuery() {
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain());
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain());
    NegateConstraint c0(LabelStr("NegateConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId()));
    assertTrue(!c0.isUnary());

    Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(1));
    NegateConstraint c1(LabelStr("NegateConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v2.getId()));
    assertTrue(c1.isUnary());

    return true;
  }

  static bool testTestEqConstraint() {
    {
      EnumeratedDomain baseDomain(true, "ENUM");
      baseDomain.insert(1);
      baseDomain.insert(2);
      baseDomain.insert(3);
      baseDomain.insert(4);
      baseDomain.close();

      Variable<BoolDomain> test(ENGINE, BoolDomain());
      Variable<EnumeratedDomain> arg1(ENGINE, baseDomain);
      Variable<EnumeratedDomain> arg2(ENGINE, baseDomain);
      TestEQ c1(LabelStr("TestEq"),
		LabelStr("Default"),
		ENGINE, 
		makeScope(test.getId(), arg1.getId(), arg2.getId()));
      assert(ENGINE->propagate());

      assert(ENGINE->propagate());
      arg1.specify(4);
      arg2.specify(4);
      assert(ENGINE->propagate());
      assert(test.lastDomain().getSingletonValue() == 1);

      arg1.reset();
      arg1.specify(3);
      assert(ENGINE->propagate());
      assert(test.lastDomain().getSingletonValue() == 0);

      arg2.reset();
      test.specify(1);
      arg2.specify(1);
      assert(!ENGINE->propagate());
    }
    
    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain());
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(5));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(5));
      TestEQ c0(LabelStr("TestEQ"), LabelStr("Default"), 
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      assertTrue(v0.getDerivedDomain().isTrue());
    }
    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain());
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(5));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(6));
      TestEQ c0(LabelStr("TestEQ"), LabelStr("Default"), 
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      assertTrue(v0.getDerivedDomain().isFalse());
    }
    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain());
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(5,10));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(5, 20));
      TestEQ c0(LabelStr("TestEQ"), LabelStr("Default"), 
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      assertTrue(!v0.getDerivedDomain().isSingleton());
      v1.specify(7);
      v2.specify(7);
      ENGINE->propagate();
      assertTrue(v0.getDerivedDomain().isTrue());
      v1.reset();
      ENGINE->propagate();
      assertTrue(!v0.getDerivedDomain().isSingleton());      
    }
    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain());
      Variable<LabelSet> v1(ENGINE, LabelSet(LabelStr("C")));
      Variable<LabelSet> v2(ENGINE, LabelSet(LabelStr("C")));
      TestEQ c0("TestEQ", "Default", 
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      assertTrue(v0.getDerivedDomain().isTrue());
    }
    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain());
      Variable<LabelSet> v1(ENGINE, LabelSet(LabelStr("C")));
      Variable<LabelSet> v2(ENGINE, LabelSet(LabelStr("E")));
      TestEQ c0(LabelStr("TestEQ"), LabelStr("Default"), 
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      assertTrue(v0.getDerivedDomain().isFalse());
    }
    return true;
  }

  static bool testTestLessThanConstraint(){
    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain());
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(5));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(6));
      TestLessThan c0(LabelStr("TestLessThan"), LabelStr("Default"), 
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      assertTrue(v0.getDerivedDomain().isTrue());
    }

    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain());
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(5));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(5));
      TestLessThan c0(LabelStr("TestLessThan"), LabelStr("Default"), 
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      assertTrue(v0.getDerivedDomain().isFalse());
    }

    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain());
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(5, 10));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(5));
      TestLessThan c0(LabelStr("TestLessThan"), LabelStr("Default"), 
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      assertTrue(v0.getDerivedDomain().isFalse());
    }

    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain());
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(4, 10));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(11, 20));
      TestLessThan c0(LabelStr("TestLessThan"), LabelStr("Default"), 
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      assertTrue(v0.getDerivedDomain().isTrue());
    }

    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain());
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(4, 10));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(8, 20));
      TestLessThan c0(LabelStr("TestLessThan"), LabelStr("Default"), 
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      assertTrue(!v0.getDerivedDomain().isSingleton());
    }

    return true;
  }

  static bool testTestLEQConstraint(){
    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain());
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(5));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(5));

      TestLEQ c0(LabelStr("TestLEQ"), LabelStr("Default"), 
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();

      assertTrue(v0.getDerivedDomain().isTrue());
    }

    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain());
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(5));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(4));

      TestLEQ c0(LabelStr("TestLEQ"), LabelStr("Default"), 
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();

      assertTrue(v0.getDerivedDomain().isFalse());
    }

    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain());
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(4));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(6));

      TestLEQ c0(LabelStr("TestLEQ"), LabelStr("Default"), 
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      assertTrue(ENGINE->propagate());
      assertTrue(v0.getDerivedDomain().isTrue());
    }

    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain());
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(0, 10));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(0, 10));

      TestLEQ c0(LabelStr("TestLEQ"), LabelStr("Default"), 
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));

      assertTrue(ENGINE->propagate());

      assertTrue(!v0.getDerivedDomain().isSingleton());
    }

    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain(false));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(0, 10));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(11, 20));

      TestLEQ c0(LabelStr("TestLEQ"), LabelStr("Default"), 
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));

      assertTrue(!ENGINE->propagate());
    }

    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain(true));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(5, 10));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(2, 4));

      TestLEQ c0(LabelStr("TestLEQ"), LabelStr("Default"), 
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));

      assertTrue(!ENGINE->propagate());
    }

    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain());
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(0, 10));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(11, 20));

      TestLEQ c0(LabelStr("TestLEQ"), LabelStr("Default"), 
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));

      assertTrue(ENGINE->propagate());
      assertTrue(v0.getDerivedDomain().isTrue());
    }

    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain());
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(10, 20));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(0, 9));

      TestLEQ c0(LabelStr("TestLEQ"), LabelStr("Default"), 
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));

      assertTrue(ENGINE->propagate());
      assertTrue(v0.getDerivedDomain().isFalse());
    }

    return true;
  }

  static bool testLockManager() {
    assertTrue(LockManager::instance().getCurrentUser() == LabelStr("ANONYMOUS"));

    // Ensure new instance overwrites old
    {
      LockManager std_manager;
      assertTrue(&std_manager == &LockManager::instance());
    }

    // Now allocate the test allocator and confirm it is assigned
    TestLockManager testManager;
    assertTrue(LockManager::instance().getCurrentUser() == LabelStr("TEST_HARNESS"));
    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain());
      Variable<LabelSet> v1(ENGINE, LabelSet(LabelStr("C")));
      Variable<LabelSet> v2(ENGINE, LabelSet(LabelStr("E")));
      TestEQ c0(LabelStr("TestEQ"), LabelStr("Default"), 
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));

      // Now ensure the appropriate identifer has been passed to the constraint
      assertTrue(c0.getCreatedBy() == LabelStr("TEST_HARNESS"));
    }

    return true;
  }

  /**
   * @brief Derived from an example from Nicola
   */
  static bool testGNATS_3075(){
    Variable<IntervalDomain> goalBox_leftBottomX(ENGINE, IntervalDomain());
    Variable<IntervalDomain> goalBox_leftBottomY(ENGINE, IntervalDomain());
    Variable<IntervalDomain> goalBox_rightTopX(ENGINE, IntervalDomain());
    Variable<IntervalDomain> goalBox_rightTopY(ENGINE, IntervalDomain());
    Variable<IntervalDomain> goalBoxTolerance(ENGINE, IntervalDomain(0.5, 0.5));
    Variable<IntervalDomain> goalX(ENGINE, IntervalDomain(0.1, 0.1));
    Variable<IntervalDomain> goalY(ENGINE, IntervalDomain(-0.2, -0.2));
    Variable<IntervalDomain> start_x(ENGINE, IntervalDomain());
    Variable<IntervalDomain> start_y(ENGINE, IntervalDomain());

    AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, 
			  makeScope(goalBox_leftBottomX.getId(), goalBoxTolerance.getId(), goalX.getId()));

    AddEqualConstraint c1(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, 
			  makeScope(goalBox_leftBottomY.getId(), goalBoxTolerance.getId(), goalY.getId()));

    AddEqualConstraint c2(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, 
			  makeScope(goalX.getId(), goalBoxTolerance.getId(), goalBox_rightTopX.getId()));

    AddEqualConstraint c3(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, 
			  makeScope(goalY.getId(), goalBoxTolerance.getId(), goalBox_rightTopY.getId()));

    Variable<BoolDomain> right_of_goalBox_left(ENGINE, BoolDomain());
    TestLEQ c4(LabelStr("TestLEQ"), LabelStr("Default"), 
	      ENGINE, makeScope(right_of_goalBox_left.getId(), goalBox_leftBottomX.getId(), start_x.getId()));

    Variable<BoolDomain> left_of_goalBox_right(ENGINE, BoolDomain());
    TestLEQ c5(LabelStr("TestLEQ"), LabelStr("Default"), 
	      ENGINE, makeScope(left_of_goalBox_right.getId(), start_x.getId(), goalBox_rightTopX.getId()));

    Variable<BoolDomain> over_goalBox_bottom(ENGINE, BoolDomain());
    TestLEQ c6(LabelStr("TestLEQ"), LabelStr("Default"), 
	      ENGINE, makeScope(over_goalBox_bottom.getId(), start_y.getId(), goalBox_leftBottomY.getId()));

    Variable<BoolDomain> under_goalBox_bottom(ENGINE, BoolDomain());
    TestLEQ c7(LabelStr("TestLEQ"), LabelStr("Default"), 
	      ENGINE, makeScope(under_goalBox_bottom.getId(), start_y.getId(), goalBox_rightTopY.getId()));

    assert(ENGINE->propagate());
    return true;
  }

}; // class ConstraintTest

class ConstraintFactoryTest
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
    assertTrue(v0.getDerivedDomain().getSingletonValue() == 1);
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
    assertTrue(g0.getGraphCount() == 1);
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
    assertTrue(g0.getGraphCount() == 1);
    int graphKey = g0.getGraphKey(v0.getId());
    assertTrue(g0.getGraphKey(v1.getId()) == graphKey);
    assertTrue(g0.getGraphKey(v2.getId()) == graphKey);

    Variable<IntervalIntDomain> v3(ENGINE, IntervalIntDomain(1, 100));
    Variable<IntervalIntDomain> v4(ENGINE, IntervalIntDomain(-100, 100));
    g0.addConnection(v2.getId(), v3.getId());
    g0.addConnection(v3.getId(), v4.getId());
    assertTrue(g0.getGraphCount() == 1);
    assertTrue(graphKey != g0.getGraphKey(v0.getId())); // Should have updated for all
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
    assertTrue(g0.getGraphCount() == 1);

    // Cause a split by removing a connection in the middle
    g0.removeConnection(v3.getId(), v4.getId());
    assertTrue(g0.getGraphCount() == 2);

    // Cause another split
    g0.removeConnection(v5.getId(), v6.getId());
    assertTrue(g0.getGraphCount() == 3);

    // Test membership of resulting classes
    assertTrue((g0.getGraphKey(v1.getId()) + g0.getGraphKey(v2.getId()) + g0.getGraphKey(v3.getId()))/3 == g0.getGraphKey(v0.getId()));
    assertTrue(g0.getGraphKey(v4.getId()) == g0.getGraphKey(v5.getId()));
    assertTrue((g0.getGraphKey(v6.getId()) + g0.getGraphKey(v7.getId()))/2   == g0.getGraphKey(v8.getId()));

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
    assertTrue(g0.getGraphCount() == 3);
    assertTrue((g0.getGraphKey(v1.getId()) + g0.getGraphKey(v2.getId()) + g0.getGraphKey(v3.getId()))/3 == g0.getGraphKey(v0.getId()));
    assertTrue(g0.getGraphKey(v4.getId()) == g0.getGraphKey(v5.getId()));
    assertTrue((g0.getGraphKey(v6.getId()) + g0.getGraphKey(v7.getId()))/2   == g0.getGraphKey(v8.getId()));

    // Add connectionto cause a merge
    g0.addConnection(v3.getId(), v4.getId());
    assertTrue(g0.getGraphCount() == 2);
    assertTrue((g0.getGraphKey(v1.getId()) + g0.getGraphKey(v2.getId()) + g0.getGraphKey(v3.getId()))/3 == g0.getGraphKey(v0.getId()));
    assertTrue((g0.getGraphKey(v4.getId()) + g0.getGraphKey(v5.getId()))/2 == g0.getGraphKey(v0.getId()));
    assertTrue((g0.getGraphKey(v6.getId()) + g0.getGraphKey(v7.getId()))/2   == g0.getGraphKey(v8.getId()));


    // Add connectionto cause a merge
    g0.addConnection(v5.getId(), v6.getId());
    assertTrue(g0.getGraphCount() == 1);

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

      assertTrue(v0.getDerivedDomain().getUpperBound() == 10);
      assertTrue(v2.getDerivedDomain().getSingletonValue() == 10);

      variables.clear();
      variables.push_back(v3.getId());
      variables.push_back(v1.getId());
      EqualConstraint c2(LabelStr("EqualConstraint"), LabelStr("EquivalenceClass"), ce, variables);

      ce->propagate();
      assertTrue(ce->constraintConsistent());
      assertTrue(v0.getDerivedDomain().getSingletonValue() == 10);

      variables.clear();
      Variable<IntervalIntDomain> v4(ce, IntervalIntDomain(1, 9));
      variables.push_back(v3.getId());
      variables.push_back(v4.getId());
      ConstraintId c3((new EqualConstraint(LabelStr("EqualConstraint"), LabelStr("EquivalenceClass"), ce, variables))->getId());
      ce->propagate();
      assertTrue(ce->provenInconsistent());

      delete (Constraint*) c3;
      assertTrue(ce->pending());
      ce->propagate();
      assertTrue(ce->constraintConsistent());
      assertTrue(v0.getDerivedDomain().getSingletonValue() == 10);
    }
    delete (ConstraintEngine*) ce;
    return(true);
  }
};

int main() {
  LockManager::instance().connect();
  LockManager::instance().lock();

  initConstraintEngine();
  initConstraintLibrary();
  REGISTER_CONSTRAINT(DelegationTestConstraint, "TestOnly", "Default");
  TypeFactory::createValue("INT_INTERVAL", std::string("5"));
  runTestSuite(DomainTests::test);
  runTestSuite(TypeFactoryTests::test);
  runTestSuite(EntityTests::test);
  runTestSuite(ConstraintEngineTest::test);
  runTestSuite(VariableTest::test); 
  runTestSuite(ConstraintTest::test); 
  runTestSuite(ConstraintFactoryTest::test);
  runTestSuite(EquivalenceClassTest::test);
  std::cout << "Finished" << std::endl;
  ConstraintLibrary::purgeAll();
  exit(0);
}
