/**
 * @file module-tests.cc
 * @author Conor McGann
 * @date August, 2003
 * @brief Read the source for details
 */
#include "ce-test-module.hh"
#include "TestSupport.hh"
#include "Utils.hh"
#include "Variable.hh"
#include "Constraints.hh"
#include "ConstraintFactory.hh"
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

#include "Engine.hh"
#include "ModuleConstraintEngine.hh"

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
    CPPUNIT_ASSERT_MESSAGE("Should be able to access derived domain during propagation safely.", !var->derivedDomain().isSingleton());
  }

  void handleExecute(const ConstrainedVariableId&,int, const DomainListener::ChangeType&){}

  static int s_executionCount;
  static int s_instanceCount;
};

int DelegationTestConstraint::s_executionCount = 0;
int DelegationTestConstraint::s_instanceCount = 0;

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

class CETestEngine : public EngineBase
{
  public:
	CETestEngine();
	virtual ~CETestEngine();
	const ConstraintEngineId& getConstraintEngine() const;

  protected:
	void createModules();
};

CETestEngine::CETestEngine()
{
    createModules();
    doStart();
    ConstraintEngine* ce = (ConstraintEngine*)getComponent("ConstraintEngine");
    ce->createValue("INT_INTERVAL", std::string("5"));
    ce->getCESchema()->registerFactory(
       (new EnumeratedTypeFactory("Locations", "Locations", LocationsBaseDomain()))->getId()
    );
    REGISTER_CONSTRAINT(ce->getCESchema(),DelegationTestConstraint, "TestOnly", "Default");
}

CETestEngine::~CETestEngine()
{
    doShutdown();
}

void CETestEngine::createModules()
{
    addModule((new ModuleConstraintEngine())->getId());
    addModule((new ModuleConstraintLibrary())->getId());
}

const ConstraintEngineId& CETestEngine::getConstraintEngine() const
{
    return ((ConstraintEngine*)getComponent("ConstraintEngine"))->getId();
}

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



class TypeFactoryTests {
public:
  static bool test() {
    EUROPA_runTest(testValueCreation);
    EUROPA_runTest(testDomainCreation);
    EUROPA_runTest(testVariableCreation);
    EUROPA_runTest(testVariableWithDomainCreation);
    return true;
  }

  static bool testValueCreation(){
      CETestEngine engine;
      ConstraintEngine* ce = (ConstraintEngine*)engine.getComponent("ConstraintEngine");

    IntervalIntDomain d0(5);
    int v0 = (int) ce->createValue(d0.getTypeName().c_str(), "5");
    CPPUNIT_ASSERT(d0.compareEqual(d0.getSingletonValue(), v0));

    IntervalDomain d1(2.3);
    double v1 = (double) ce->createValue(d1.getTypeName().c_str(), "2.3");
    CPPUNIT_ASSERT(d1.compareEqual(d1.getSingletonValue(), v1));

    BoolDomain d2(true);
    bool v2 = (bool) ce->createValue(d2.getTypeName().c_str(), "true");
    CPPUNIT_ASSERT(d2.compareEqual(d2.getSingletonValue(), v2));

    return true;
  }

  static bool testDomainCreation() {
      CETestEngine engine;
      CESchema* tfm = (CESchema*)engine.getComponent("CESchema");

    const IntervalIntDomain & bd0 = dynamic_cast<const IntervalIntDomain &>(tfm->baseDomain(IntervalIntDomain().getTypeName().c_str()));
    CPPUNIT_ASSERT(bd0.isMember(0));
    CPPUNIT_ASSERT(!bd0.isBool());
    const IntervalDomain & bd1 = dynamic_cast<const IntervalDomain &>(tfm->baseDomain(IntervalDomain().getTypeName().c_str()));
    CPPUNIT_ASSERT(bd1.isMember(0.1));
    CPPUNIT_ASSERT(!bd1.isBool());
    const BoolDomain & bd2 = dynamic_cast<const BoolDomain &>(tfm->baseDomain(BoolDomain().getTypeName().c_str()));
    CPPUNIT_ASSERT(bd2.isMember(false));
    CPPUNIT_ASSERT(bd2.isMember(true));
    CPPUNIT_ASSERT(bd2.isBool());
    CPPUNIT_ASSERT(LocationsBaseDomain().isMember(LabelStr("Hill")));
    CPPUNIT_ASSERT(LocationsBaseDomain().isMember(LabelStr("Rock")));
    CPPUNIT_ASSERT(LocationsBaseDomain().isMember(LabelStr("Lander")));
    CPPUNIT_ASSERT(!LocationsBaseDomain().isMember(LabelStr("true")));
    //!!This (and SymbolDomain) die with complaints of a "bad cast"
    //!!const Locations & loc0 = dynamic_cast<const Locations&>(tfm->baseDomain("Locations"));
    const EnumeratedDomain & loc0 = dynamic_cast<const EnumeratedDomain &>(tfm->baseDomain("Locations"));
    CPPUNIT_ASSERT(!loc0.isBool());
    CPPUNIT_ASSERT(loc0.isMember(LabelStr("Hill")));
    CPPUNIT_ASSERT(loc0.isMember(LabelStr("Rock")));
    CPPUNIT_ASSERT(loc0.isMember(LabelStr("Lander")));
    CPPUNIT_ASSERT(!loc0.isMember(LabelStr("true")));
    //!!The compiler complains about using Locations here when EnumeratedDomain is used above.
    //!!Locations *loc1 = loc0.copy();
    EnumeratedDomain *loc1 = loc0.copy();
    loc1->open();
    loc1->remove(LabelStr("Hill"));
    CPPUNIT_ASSERT(!loc1->isMember(LabelStr("Hill")));
    CPPUNIT_ASSERT(loc1->isMember(LabelStr("Rock")));
    CPPUNIT_ASSERT(loc1->isMember(LabelStr("Lander")));
    loc1->remove(LabelStr("Rock"));
    CPPUNIT_ASSERT(!loc1->isMember(LabelStr("Hill")));
    CPPUNIT_ASSERT(!loc1->isMember(LabelStr("Rock")));
    CPPUNIT_ASSERT(loc1->isMember(LabelStr("Lander")));
    loc1->insert(LabelStr("Hill"));
    CPPUNIT_ASSERT(loc1->isMember(LabelStr("Hill")));
    CPPUNIT_ASSERT(!loc1->isMember(LabelStr("Rock")));
    CPPUNIT_ASSERT(loc1->isMember(LabelStr("Lander")));
    loc1->remove(LabelStr("Lander"));
    CPPUNIT_ASSERT(loc1->isMember(LabelStr("Hill")));
    CPPUNIT_ASSERT(!loc1->isMember(LabelStr("Rock")));
    CPPUNIT_ASSERT(!loc1->isMember(LabelStr("Lander")));
    loc1->insert(LabelStr("Rock"));
    CPPUNIT_ASSERT(loc1->isMember(LabelStr("Hill")));
    CPPUNIT_ASSERT(loc1->isMember(LabelStr("Rock")));
    CPPUNIT_ASSERT(!loc1->isMember(LabelStr("Lander")));
    loc1->remove(LabelStr("Hill"));
    CPPUNIT_ASSERT(!loc1->isMember(LabelStr("Hill")));
    CPPUNIT_ASSERT(loc1->isMember(LabelStr("Rock")));
    CPPUNIT_ASSERT(!loc1->isMember(LabelStr("Lander")));
    delete loc1;
    return true;
  }

  static bool testVariableCreation(){
      CETestEngine engine;

      ConstraintEngineId ce = ((ConstraintEngine*)engine.getComponent("ConstraintEngine"))->getId();
      ConstrainedVariableId cv0 = ce->createVariable(IntervalIntDomain().getTypeName().c_str());
      CPPUNIT_ASSERT(cv0->baseDomain().getTypeName() == IntervalIntDomain().getTypeName());
      ConstrainedVariableId cv1 = ce->createVariable(IntervalDomain().getTypeName().c_str());
      CPPUNIT_ASSERT(cv1->baseDomain().getTypeName() == IntervalDomain().getTypeName());
      ConstrainedVariableId cv2 = ce->createVariable(BoolDomain().getTypeName().c_str());
      CPPUNIT_ASSERT(cv2->baseDomain().getTypeName() == BoolDomain().getTypeName());
      return true;
  }

  static bool testVariableWithDomainCreation(){
      CETestEngine engine;

      IntervalIntDomain d0(5);
      IntervalDomain d1(2.3);
      BoolDomain d2(true);

      ConstraintEngineId ce = ((ConstraintEngine*)engine.getComponent("ConstraintEngine"))->getId();
      ConstrainedVariableId cv0 = ce->createVariable(d0.getTypeName().c_str(), d0);
      CPPUNIT_ASSERT(cv0->baseDomain() == d0);
      ConstrainedVariableId cv1 = ce->createVariable(d1.getTypeName().c_str(), d1);
      CPPUNIT_ASSERT(cv1->baseDomain() == d1);
      ConstrainedVariableId cv2 = ce->createVariable(d2.getTypeName().c_str(), d2);
      CPPUNIT_ASSERT(cv2->baseDomain() == d2);
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
    EUROPA_runTest(testDeallocationWithPurging);
    EUROPA_runTest(testInconsistentInitialVariableDomain);
    EUROPA_runTest(testVariableLookupByIndex);
    EUROPA_runTest(testGNATS_3133);
    return true;
  }

  static bool testDeallocationWithPurging(){
      CETestEngine engine;
      ConstraintEngineId ce = ((ConstraintEngine*)engine.getComponent("ConstraintEngine"))->getId();

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

    CPPUNIT_ASSERT(ce->propagate());

    return true;
  }

  static bool testInconsistentInitialVariableDomain(){
    NumericDomain emptyDomain;
    emptyDomain.close();
    {
      Variable<NumericDomain> v0(ENGINE, emptyDomain);
      CPPUNIT_ASSERT(ENGINE->provenInconsistent()); // Should be immediately inconsistent!
    }
    CPPUNIT_ASSERT(ENGINE->propagate()); // Should be fixed by deletion of the variable

    return true;
  }

  static bool testVariableLookupByIndex(){
    std::vector<ConstrainedVariableId> vars;

    for(unsigned int i=0;i<10;i++){
      ConstrainedVariableId var = (new Variable<IntervalIntDomain> (ENGINE, IntervalIntDomain()))->getId();
      CPPUNIT_ASSERT(var == ENGINE->getVariable(i));
      CPPUNIT_ASSERT(ENGINE->getIndex(var) == i);
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
    CPPUNIT_ASSERT(!ENGINE->propagate());

    v2.reset();
    //DEPRECATED (GNATS 3140):CPPUNIT_ASSERT(ENGINE->provenInconsistent());
    CPPUNIT_ASSERT(!ENGINE->propagate());
    v3.reset();
    CPPUNIT_ASSERT(!ENGINE->propagate());
    CPPUNIT_ASSERT(ENGINE->provenInconsistent());;
    v0.reset();
    v1.reset();
    CPPUNIT_ASSERT(!ENGINE->provenInconsistent());
    CPPUNIT_ASSERT(ENGINE->propagate());

    v0.specify(1);
    v1.specify(2);
    v2.specify(3);
    v3.specify(3);

    // Now delete constraints in the order that relaxes the empty variable last
    delete (Constraint*) c1;
    CPPUNIT_ASSERT(!ENGINE->propagate());
    delete (Constraint*) c2;
    CPPUNIT_ASSERT(!ENGINE->propagate());
    delete (Constraint*) c0;
    CPPUNIT_ASSERT(ENGINE->propagate());

    return true;
  }
};


// notifyDiscard is intended to notify the listener creator to delete the listener
// (which in fact must happen immediately, because the delete notifies the listenee, which therefore
// must still be around)
// Assumes listener has been created using new
class TestVariableListener: public ConstrainedVariableListener{
public:
  TestVariableListener(const ConstrainedVariableId& observedVar)
    : ConstrainedVariableListener(observedVar) {}
  void notifyDiscard() {
	  delete this;
  }

};

class VariableTest
{
public:
  static bool test() {
    EUROPA_runTest(testAllocation);
    EUROPA_runTest(testMessaging);
    EUROPA_runTest(testDynamicVariable);
    EUROPA_runTest(testListener);
    EUROPA_runTest(testVariablesWithOpenDomains);
    EUROPA_runTest(testRestrictionScenarios);
    EUROPA_runTest(testSpecification);
    return true;
  }

private:

  static bool testAllocation(){
    IntervalIntDomain dom0(0, 1000);
    Variable<IntervalIntDomain> v0(ENGINE, dom0);
    const IntervalIntDomain& dom1 = v0.getBaseDomain();
    CPPUNIT_ASSERT(dom0 == dom1);
    CPPUNIT_ASSERT(v0.isValid());
    CPPUNIT_ASSERT(v0.canBeSpecified());

    // Now restrict the base domain
    IntervalIntDomain dom2(3, 10);
    v0.restrictBaseDomain(dom2);
    CPPUNIT_ASSERT(v0.getDerivedDomain() == dom2);

    Variable<IntervalIntDomain> v1(ENGINE, dom1, false, false, LabelStr("TEST VARIABLE"));
    CPPUNIT_ASSERT(!v1.canBeSpecified());
    CPPUNIT_ASSERT(v1.getName() == LabelStr("TEST VARIABLE"));
    CPPUNIT_ASSERT(v1.isValid());
    return true;
  }

  static bool testMessaging(){
    TestListener listener(ENGINE);

    // Add, Specify, Remove
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(0, 100));
      CPPUNIT_ASSERT(listener.getCount(ConstraintEngine::VARIABLE_ADDED) == 1);
      v0.specify(5);
      CPPUNIT_ASSERT(listener.getCount(ConstraintEngine::SET_TO_SINGLETON) == 1);
    }
    CPPUNIT_ASSERT(listener.getCount(ConstraintEngine::VARIABLE_REMOVED) == 1);

    // Bounds restriction messages for derived domain
    listener.reset();
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(0, 100));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(0, 10));
      EqualConstraint c0(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId()));
      ENGINE->propagate();
      CPPUNIT_ASSERT(listener.getCount(ConstraintEngine::UPPER_BOUND_DECREASED) == 1);
      v0.specify(7);
      ENGINE->propagate(); // Expect a RESTRICT_TO_SINGLETON event through propagation
      CPPUNIT_ASSERT(listener.getCount(ConstraintEngine::RESTRICT_TO_SINGLETON) == 1);

      v0.reset(); // Expect a RESET message for v0 and a RELAXATION message for both variables
      CPPUNIT_ASSERT(listener.getCount(ConstraintEngine::RESET) == 1);
      CPPUNIT_ASSERT(listener.getCount(ConstraintEngine::RELAXED) == 2);
      CPPUNIT_ASSERT(ENGINE->pending());

      v0.specify(0); // Expect EMPTIED
      v1.specify(1); // Expect EMPTIED
      ENGINE->propagate();
      CPPUNIT_ASSERT(listener.getCount(ConstraintEngine::EMPTIED) == 1);
    }

    // Now tests message handling on Enumerated Domain
    listener.reset();
    {
      Variable<NumericDomain> v0(ENGINE, NumericDomain());
      v0.insert(1);
      v0.insert(3);
      v0.insert(5);
      v0.insert(10);
      CPPUNIT_ASSERT(listener.getCount(ConstraintEngine::RELAXED) == 4);
      v0.close();
      CPPUNIT_ASSERT(listener.getCount(ConstraintEngine::CLOSED) == 1);

      NumericDomain d0;
      d0.insert(2);
      d0.insert(3);
      d0.insert(5);
      d0.insert(11);
      d0.close();
      Variable<NumericDomain> v1(ENGINE, d0);

      EqualConstraint c0(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId()));
      ENGINE->propagate(); // Should see values removed from both variables domains.
      CPPUNIT_ASSERT(listener.getCount(ConstraintEngine::VALUE_REMOVED) == 2);
      v0.specify(3);
      CPPUNIT_ASSERT(listener.getCount(ConstraintEngine::SET_TO_SINGLETON) == 1);
      v1.specify(5);
      CPPUNIT_ASSERT(listener.getCount(ConstraintEngine::SET_TO_SINGLETON) == 2);
      ENGINE->propagate(); // Expect to see exactly one domain emptied
      CPPUNIT_ASSERT(listener.getCount(ConstraintEngine::EMPTIED) == 1);
      v1.reset(); // Should now see 2 domains relaxed.
      CPPUNIT_ASSERT(listener.getCount(ConstraintEngine::RELAXED) == 6);
    }

    return true;
  }

  static bool testDynamicVariable(){
    // Test empty on closure forces inconsistency
    {
      Variable<NumericDomain> v0(ENGINE,NumericDomain());
      CPPUNIT_ASSERT(ENGINE->propagate()); // No reason to be inconsistent
      v0.close(); // Should push empty event.
      CPPUNIT_ASSERT(ENGINE->provenInconsistent()); // Should be inconsistent
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
      EqualConstraint c0(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId()));
      CPPUNIT_ASSERT(ENGINE->propagate());
      CPPUNIT_ASSERT(v1.getDerivedDomain().getSize() == 3);

      // Now close v0, and we should see a restriction on v1
      v0.close();
      CPPUNIT_ASSERT(v1.getDerivedDomain().isSingleton());

      // insert further, and propagate again, the domain should grow by one
      v0.open();
      v0.insert(2);
      v0.close();
      CPPUNIT_ASSERT(v1.getDerivedDomain().getSize() == 2);

      // Open it and ensure that the engine is pending once again
      v0.open();
      CPPUNIT_ASSERT(ENGINE->pending());
      CPPUNIT_ASSERT(v1.getDerivedDomain().getSize() == 3);

      v0.specify(2);
      CPPUNIT_ASSERT(v0.lastDomain().isClosed());
      CPPUNIT_ASSERT(v1.getDerivedDomain().isSingleton());

      // Now insert and confirm it does not affect v1. Shoud insert to base domain but
      // not push to the specified or derived domain.
      v0.insert(3);
      CPPUNIT_ASSERT(v0.lastDomain().isSingleton()); // Still, since spec domain is closed.
      CPPUNIT_ASSERT(v1.getDerivedDomain().isSingleton());
      CPPUNIT_ASSERT(!v0.baseDomain().isClosed());
      CPPUNIT_ASSERT(v1.getDerivedDomain().isSingleton());

      // Now if we reset, the base domain will have 3 elements in it, so derived domain of v1 will also have 3 elements
      // in it since v0 should be relaxed to the base domain.
      v0.reset();
      CPPUNIT_ASSERT(v1.getDerivedDomain().getSize() == 3);
    }

    return true;
  }

  static bool testListener(){
    ConstrainedVariableId v0 = (new Variable<IntervalIntDomain>(ENGINE, IntervalIntDomain()))->getId();
    new TestVariableListener(v0); // deletes itself when v0 deleted
    delete (ConstrainedVariable*) v0;
    return true;
  }

  static bool testVariablesWithOpenDomains() {
    EnumeratedDomain e0(true, "Test");
    e0.insert(0); e0.insert(1); e0.insert(2); e0.insert(3);
    Variable<EnumeratedDomain> v0(ENGINE, e0);

    CPPUNIT_ASSERT(v0.baseDomain().isOpen());
    CPPUNIT_ASSERT(v0.derivedDomain().isOpen());

    v0.close();
    CPPUNIT_ASSERT(v0.baseDomain().isClosed());
    CPPUNIT_ASSERT(v0.derivedDomain().isClosed());

    v0.open();
    CPPUNIT_ASSERT(v0.baseDomain().isOpen());
    CPPUNIT_ASSERT(v0.derivedDomain().isOpen());

    e0.close();
    v0.specify(1);
    CPPUNIT_ASSERT(v0.baseDomain().isOpen());
    CPPUNIT_ASSERT(!v0.derivedDomain().isOpen());

    v0.restrictBaseDomain(e0);
    CPPUNIT_ASSERT(v0.baseDomain().isClosed());
    CPPUNIT_ASSERT(v0.isClosed());

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
    CPPUNIT_ASSERT(ENGINE->propagate());

    // Now v1's derived domain will also be a singleton. However, I want to restrict the base domain of v1 partially.
    v1.restrictBaseDomain(e1);
    CPPUNIT_ASSERT(ENGINE->propagate());

    // Now specify v1 to a singleton also, a different value than that already specified.
    v0.reset();
    v1.specify(3);
    v0.specify(1);
    CPPUNIT_ASSERT(!ENGINE->propagate());

    // Repair by reseting v0
    v0.reset();
    CPPUNIT_ASSERT(ENGINE->propagate());
    return true;
  }

  static bool testSpecification(){
    // Variable with a base domain that is a singleton
    {
      Variable<IntervalIntDomain> v(ENGINE, IntervalIntDomain(0, 0));
      CPPUNIT_ASSERT(v.isSpecified());
    }

    // Variable with a base domain that is left open but then closed
    {
      EnumeratedDomain e0(true, "Test");
      e0.insert(0);

      Variable<EnumeratedDomain> v(ENGINE, e0);
      CPPUNIT_ASSERT(!v.isSpecified());

      v.close();
      CPPUNIT_ASSERT(v.isSpecified());
      CPPUNIT_ASSERT(v.getSpecifiedValue() == 0);

      v.open();
      CPPUNIT_ASSERT(!v.isSpecified());
    }


    // Variable with a base domain that is left open and we reset
    {
      EnumeratedDomain e0(true, "Test");
      e0.insert(0);

      Variable<EnumeratedDomain> v(ENGINE, e0);
      CPPUNIT_ASSERT(!v.isSpecified());

      CPPUNIT_ASSERT(!v.isSpecified());
      v.specify(0);
      CPPUNIT_ASSERT(v.isSpecified());

      v.reset();
      CPPUNIT_ASSERT(!v.isSpecified());
    }

    // Restrict base domain
    {
      Variable<IntervalIntDomain> v(ENGINE, IntervalIntDomain(0, 10));
      CPPUNIT_ASSERT(!v.isSpecified());
      v.restrictBaseDomain(IntervalIntDomain(0, 0));
      CPPUNIT_ASSERT(v.isSpecified());
      CPPUNIT_ASSERT(v.getSpecifiedValue() == 0);
    }

    return true;
  }
};

class ConstraintTest
{
public:
  static bool test() {
    EUROPA_runTest(testGNATS_3181);
    EUROPA_runTest(testUnaryConstraint);
    EUROPA_runTest(testAddEqualConstraint);
    EUROPA_runTest(testLessThanEqualConstraint);
    EUROPA_runTest(testLessOrEqThanSumConstraint);
    EUROPA_runTest(testBasicPropagation);
    EUROPA_runTest(testDeactivation);
    EUROPA_runTest(testForceInconsistency);
    EUROPA_runTest(testRepropagation);
    EUROPA_runTest(testConstraintRemoval);
    EUROPA_runTest(testDelegation);
    EUROPA_runTest(testNotEqual);
    EUROPA_runTest(testMultEqualConstraint);
    EUROPA_runTest(testAddMultEqualConstraint);
    EUROPA_runTest(testEqualSumConstraint);
    EUROPA_runTest(testCondAllSameConstraint);
    EUROPA_runTest(testCondAllDiffConstraint);
    EUROPA_runTest(testConstraintDeletion);
    EUROPA_runTest(testArbitraryConstraints);
    EUROPA_runTest(testLockConstraint);
    EUROPA_runTest(testNegateConstraint);
    EUROPA_runTest(testUnaryQuery);
    EUROPA_runTest(testTestEqConstraint);
    EUROPA_runTest(testTestLessThanConstraint);
    EUROPA_runTest(testTestLEQConstraint);
    EUROPA_runTest(testGNATS_3075);
    return(true);
  }

private:
  static bool testGNATS_3181(){
    std::list<double> values;
    values.push_back(1);
    values.push_back(2);
    values.push_back(3);
    values.push_back(4);
    values.push_back(5);

    EnumeratedDomain dom(values, true, "ANY");
    dom.open();

    // All domains closed should be a no-op
    Variable<EnumeratedDomain> v0(ENGINE, dom);

    // Remove 1 and add another
    dom.remove(1);
    dom.insert(6);
    Variable<EnumeratedDomain> v1(ENGINE, dom);

    EqualConstraint c0(LabelStr("EqualConstraint"), LabelStr("Default"),
		       ENGINE, makeScope(v0.getId(), v1.getId()));

    CPPUNIT_ASSERT(ENGINE->propagate());

    CPPUNIT_ASSERT_MESSAGE(v0.toString(), !v0.isClosed());
    CPPUNIT_ASSERT_MESSAGE(v1.toString(), !v1.isClosed());
    CPPUNIT_ASSERT(!(v0.lastDomain() == v1.lastDomain()));

    // Close 1 variable. It should prune values, but not propagate closure
    v1.close();
    CPPUNIT_ASSERT(ENGINE->propagate());
    CPPUNIT_ASSERT_MESSAGE(v0.toString(), !v0.isClosed());
    CPPUNIT_ASSERT_MESSAGE(v1.toString(), v1.isClosed());
    CPPUNIT_ASSERT_MESSAGE(v0.toString() + " == " + v1.toString(), !(v0.lastDomain() == v1.lastDomain()) );

    // Value test to ensure the restriction has occurred and that we have retained the inequality also.
    // Basically one way propagation.
    CPPUNIT_ASSERT_MESSAGE(v0.lastDomain().toString(), !v0.lastDomain().isMember(1) );
    CPPUNIT_ASSERT_MESSAGE(v1.toString(), v1.lastDomain().isMember(6));

    // Re-open the variable, it should no longer propagate the equality
    return true;
  }

  static bool testUnaryConstraint(){
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(-10, 10));
      UnaryConstraint c0(IntervalDomain(4, 6), v0.getId());
      CPPUNIT_ASSERT_MESSAGE(v0.toString(), v0.getDerivedDomain() == IntervalIntDomain(4, 6));
      v0.specify(5);
      CPPUNIT_ASSERT(v0.constraintConsistent());
      v0.reset();
      CPPUNIT_ASSERT_MESSAGE(v0.toString(), v0.getDerivedDomain() == IntervalIntDomain(4, 6));
      c0.discard();
      CPPUNIT_ASSERT_MESSAGE(v0.toString(), v0.lastDomain() == IntervalIntDomain(-10, 10));
    }

    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(-10, 10));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(-10, 10));
      UnaryConstraint c0(IntervalDomain(4, 6), v0.getId());
      UnaryConstraint c1("UNARY", "Default", ENGINE, makeScope(v1.getId()));
      c1.getId()->setSource(c0.getId());
      CPPUNIT_ASSERT_MESSAGE(v0.toString(), v1.getDerivedDomain() == IntervalIntDomain(4, 6));
    }

    return true;
  }

  static bool testAddEqualConstraint() {

    // Now test special case of rounding with negative domain bounds.
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(-10, 10));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(-10, 10));
      Variable<IntervalDomain> v2(ENGINE, IntervalDomain(0.01, 0.99));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      bool res = ENGINE->propagate();
      CPPUNIT_ASSERT(!res);
    }

    // Another, similar, case of rounding with negative domain bounds.
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(-10, 10));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(-10, 10));
      Variable<IntervalDomain> v2(ENGINE, IntervalDomain(0.01, 1.99));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      bool res = ENGINE->propagate();
      CPPUNIT_ASSERT(res);
      // Require correct result to be in v2's domain.
      CPPUNIT_ASSERT(v2.getDerivedDomain().isMember(1.0));
      // Following is false because implementation of AddEqualConstraint is not smart enough to deduce it.
      //CPPUNIT_ASSERT(v2.getDerivedDomain().getSingletonValue() == 1.0);
    }

    // Confirm correct result with all singletons.
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(-1, -1));
      Variable<IntervalDomain> v1(ENGINE, IntervalDomain(10.4, 10.4));
      Variable<IntervalDomain> v2(ENGINE, IntervalDomain(9.4, 9.4));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      bool res = ENGINE->propagate();
      CPPUNIT_ASSERT(res);
    }

    // Confirm inconsistency detected with all singletons.
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(-1, -1));
      Variable<IntervalDomain> v1(ENGINE, IntervalDomain(10.4, 10.4));
      Variable<IntervalDomain> v2(ENGINE, IntervalDomain(9.39, 9.39));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      bool res = ENGINE->propagate();
      CPPUNIT_ASSERT(!res);
    }

    // Obtain factors correct values for fixed result.
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(0, PLUS_INFINITY));
      Variable<IntervalDomain> v1(ENGINE, IntervalDomain(0, PLUS_INFINITY));
      Variable<IntervalDomain> v2(ENGINE, IntervalDomain(9.390, 9.390));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      bool res = ENGINE->propagate();
      CPPUNIT_ASSERT(res);
      CPPUNIT_ASSERT(v0.getDerivedDomain() == IntervalIntDomain(0, 9));
      CPPUNIT_ASSERT(v1.getDerivedDomain() == IntervalDomain(0.39, 9.39));
    }

    // Test handling with all infinites
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(MINUS_INFINITY, MINUS_INFINITY));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, PLUS_INFINITY));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(PLUS_INFINITY, PLUS_INFINITY));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      bool res = ENGINE->propagate();
      CPPUNIT_ASSERT(res);
      CPPUNIT_ASSERT(v0.getDerivedDomain() == IntervalIntDomain(MINUS_INFINITY, MINUS_INFINITY));
      CPPUNIT_ASSERT(v1.getDerivedDomain() == IntervalIntDomain(1, PLUS_INFINITY));
      CPPUNIT_ASSERT(v2.getDerivedDomain() == IntervalIntDomain(PLUS_INFINITY, PLUS_INFINITY));
    }

    // Test handling with infinites and non-infinites
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(10, PLUS_INFINITY));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, PLUS_INFINITY));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(MINUS_INFINITY, 100));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      bool res = ENGINE->propagate();
      CPPUNIT_ASSERT(res);
      CPPUNIT_ASSERT(v0.getDerivedDomain() == IntervalIntDomain(10, 99));
      CPPUNIT_ASSERT(v1.getDerivedDomain() == IntervalIntDomain(1, 90));
      CPPUNIT_ASSERT(v2.getDerivedDomain() == IntervalIntDomain(11, 100));
    }

    // Test propagating infinites: start + duration == end.
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(MINUS_INFINITY, PLUS_INFINITY));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(MINUS_INFINITY, PLUS_INFINITY));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      bool res = ENGINE->propagate();
      CPPUNIT_ASSERT(res);
      CPPUNIT_ASSERT(v0.getDerivedDomain() == IntervalIntDomain(MINUS_INFINITY, PLUS_INFINITY));
      CPPUNIT_ASSERT(v1.getDerivedDomain() == IntervalIntDomain(1));
      CPPUNIT_ASSERT(v2.getDerivedDomain() == IntervalIntDomain(MINUS_INFINITY, PLUS_INFINITY));
    }

    // Test that we can use the constraint on a variable that is present in the constraint more than once
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(10, 14));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(0, 1));
      AddEqualConstraint c0(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v0.getId()));
      bool res = ENGINE->propagate();
      CPPUNIT_ASSERT(res);
      v1.specify(1);
      CPPUNIT_ASSERT(!ENGINE->propagate());
      v1.reset();
      CPPUNIT_ASSERT(ENGINE->propagate());
      v0.specify(11);
      CPPUNIT_ASSERT(ENGINE->propagate());
      CPPUNIT_ASSERT(v1.getDerivedDomain() == IntervalIntDomain(0));
    }

    return true;
  }

  static bool testLessThanEqualConstraint() {/* TO DO
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 100));
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, 100));
    LessThanEqualConstraint c0(LabelStr("LessThanEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId()));
    ENGINE->propagate();
    CPPUNIT_ASSERT(ENGINE->constraintConsistent());
    CPPUNIT_ASSERT(v0.getDerivedDomain() == v1.getDerivedDomain());

    v0.specify(IntervalIntDomain(50, 100));
    CPPUNIT_ASSERT(v1.getDerivedDomain().getLowerBound() == 50);
    IntervalIntDomain copy(v1.getDerivedDomain());
    v0.specify(IntervalIntDomain(50, 80));
    CPPUNIT_ASSERT(v1.getDerivedDomain() == copy);
    v1.specify(IntervalIntDomain(60, 70));
    CPPUNIT_ASSERT(v0.getDerivedDomain() == IntervalIntDomain(50, 70));
    v1.reset();
    CPPUNIT_ASSERT(v0.getDerivedDomain() == IntervalIntDomain(50, 80));
    CPPUNIT_ASSERT(v1.getDerivedDomain() == IntervalIntDomain(50, 100));

    // Handle propagation of infinities
    Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(2, PLUS_INFINITY));
    Variable<IntervalIntDomain> v3(ENGINE, IntervalIntDomain(MINUS_INFINITY, 100));
    LessThanEqualConstraint c2(LabelStr("LessThanEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v2.getId(), v3.getId()));
    bool res = ENGINE->propagate();
    CPPUNIT_ASSERT(res);
    CPPUNIT_ASSERT(v2.getDerivedDomain().getUpperBound() == 100);
    CPPUNIT_ASSERT(v3.getDerivedDomain().getLowerBound() == 2);

    // Handle restriction to singleton
    Variable<IntervalIntDomain> v4(ENGINE, IntervalIntDomain(0, 10));
    Variable<IntervalIntDomain> v5(ENGINE, IntervalIntDomain(5, 15));
    Variable<IntervalIntDomain> v6(ENGINE, IntervalIntDomain(0, 100));
    LessThanEqualConstraint c3(LabelStr("LessThanEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v4.getId(), v5.getId()));
    EqualConstraint c4(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v5.getId(), v6.getId()));
    res = ENGINE->propagate();
    CPPUNIT_ASSERT(res);
    v6.specify(9);
    res = ENGINE->propagate();
    CPPUNIT_ASSERT(res);
    CPPUNIT_ASSERT(v4.getDerivedDomain().getUpperBound() == 9);

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
    CPPUNIT_ASSERT(res);

    realVar2.specify(2.2);
    res = ENGINE->propagate();
    CPPUNIT_ASSERT(res);
    CPPUNIT_ASSERT(realVar2.getDerivedDomain().getUpperBound() == 2.2);
					     */
    return(true);
  }

  static bool testLessOrEqThanSumConstraint() {/* TODO
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(0, 100));
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(0, 100));
    Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(0, 100));
    LessOrEqThanSumConstraint c0(LabelStr("LessOrEqThanSumConstraint"), LabelStr("Default"), ENGINE,
                                 makeScope(v0.getId(), v1.getId(), v2.getId()));
    bool res = ENGINE->propagate();
    CPPUNIT_ASSERT(res);
    CPPUNIT_ASSERT(ENGINE->constraintConsistent());
    CPPUNIT_ASSERT(v0.getDerivedDomain() == v1.getDerivedDomain());
    CPPUNIT_ASSERT(v1.getDerivedDomain() == v2.getDerivedDomain());

    v1.specify(IntervalIntDomain(0, 50));
    v2.specify(IntervalIntDomain(0, 40));
    CPPUNIT_ASSERT(v0.getDerivedDomain() == IntervalIntDomain(0, 90));
    v0.specify(IntervalIntDomain(60, 70));
    CPPUNIT_ASSERT(v1.getDerivedDomain() == IntervalIntDomain(20, 50));
    CPPUNIT_ASSERT(v2.getDerivedDomain() == IntervalIntDomain(10, 40));
    v1.reset();
    CPPUNIT_ASSERT(v0.getDerivedDomain() == IntervalIntDomain(60, 70));
    CPPUNIT_ASSERT(v1.getDerivedDomain() == IntervalIntDomain(20, 100));
    CPPUNIT_ASSERT(v2.getDerivedDomain() == IntervalIntDomain(0, 40));

    // @todo Lots more ...
    */
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
    CPPUNIT_ASSERT(!v0.getDerivedDomain().isEmpty());

    // v4 + v5 == v1
    Variable<IntervalIntDomain> v4(ENGINE, IntervalIntDomain(1, 10));
    Variable<IntervalIntDomain> v5(ENGINE, IntervalIntDomain(1, 1000));
    AddEqualConstraint c2(LabelStr("AddEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v4.getId(), v5.getId(), v1.getId()));

    ENGINE->propagate();
    CPPUNIT_ASSERT(ENGINE->constraintConsistent());
    CPPUNIT_ASSERT(!v4.getDerivedDomain().isEmpty());
    return true;
  }

  static bool testDeactivation(){
    // v0 == v1
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, 10));
    EqualConstraint c0(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId()));

    ENGINE->propagate();
    CPPUNIT_ASSERT(ENGINE->constraintConsistent());

    v0.deactivate();
    CPPUNIT_ASSERT(!c0.isActive());
    CPPUNIT_ASSERT(c0.deactivationCount() == 1);

    v1.deactivate();
    CPPUNIT_ASSERT(!c0.isActive());
    CPPUNIT_ASSERT(c0.deactivationCount() == 2);

    c0.deactivate();
    CPPUNIT_ASSERT(c0.deactivationCount() == 3);
    v0.undoDeactivation();
    v1.undoDeactivation();
    CPPUNIT_ASSERT(c0.deactivationCount() == 1);
    CPPUNIT_ASSERT(!c0.isActive());

    v1.deactivate();
    CPPUNIT_ASSERT(!c0.isActive());
    CPPUNIT_ASSERT(c0.deactivationCount() == 2);
    v1.undoDeactivation();
    CPPUNIT_ASSERT(c0.deactivationCount() == 1);
    c0.undoDeactivation();
    CPPUNIT_ASSERT(c0.isActive());

    // Now restrict the base domains to automatically deactivate
    v0.restrictBaseDomain(IntervalIntDomain(1, 1));
    CPPUNIT_ASSERT(c0.isActive());
    v1.restrictBaseDomain(IntervalIntDomain(1, 1));
    CPPUNIT_ASSERT(c0.isActive()); // Have not propagated yet!
    ENGINE->propagate();
    CPPUNIT_ASSERT(!c0.isActive()); // Now we have propagated, so should be deactivated.

    // Make sure it stays deactive
    c0.undoDeactivation();
    CPPUNIT_ASSERT(!c0.isActive());

    Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(1, 10));
    Variable<IntervalIntDomain> v3(ENGINE, IntervalIntDomain(1, 10));
    EqualConstraint c1(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v2.getId(), v3.getId()));
    ENGINE->propagate();
    v2.restrictBaseDomain(IntervalIntDomain(1, 1));
    v3.restrictBaseDomain(IntervalIntDomain(2, 2));

    // Now propagate. The constraint will not be deactivated.
    ENGINE->propagate();
    CPPUNIT_ASSERT(c1.isActive());

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
    CPPUNIT_ASSERT(ENGINE->provenInconsistent());
    CPPUNIT_ASSERT(v1.getDerivedDomain().isEmpty() || v2.getDerivedDomain().isEmpty());

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
    CPPUNIT_ASSERT(emptyCount == 1);
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
    /* TODO
    CPPUNIT_ASSERT(ENGINE->constraintConsistent());
    v0.specify(IntervalIntDomain(8, 10));
    v1.specify(IntervalIntDomain(2, 7));
    CPPUNIT_ASSERT(ENGINE->pending());

    ENGINE->propagate();
    CPPUNIT_ASSERT(ENGINE->provenInconsistent());
    */
    v0.reset();
    CPPUNIT_ASSERT(ENGINE->pending());
    ENGINE->propagate();
    CPPUNIT_ASSERT(ENGINE->constraintConsistent());

    /* Call reset on a constraint consistent network - not sure one would want to do this. */
    v1.reset();
    CPPUNIT_ASSERT(ENGINE->pending()); /* Strictly speaking we know it is not inconsistent here since all we have done is relax a previously
				  consistent network. However, we have to propagate to find the new derived domains based on relaxed
				  domains. */
    ENGINE->propagate();
    CPPUNIT_ASSERT(ENGINE->constraintConsistent());
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
    CPPUNIT_ASSERT(ENGINE->constraintConsistent());

    /* Show that we can simply delete a constraint and confirm that the system is still consistent. */
    delete (Constraint*) c1;
    ENGINE->propagate();
    CPPUNIT_ASSERT(ENGINE->constraintConsistent());

    variables.clear();
    Variable<IntervalIntDomain> v4(ENGINE, IntervalIntDomain(1, 1));
    variables.push_back(v0.getId());
    variables.push_back(v4.getId());
    ConstraintId c2((new EqualConstraint(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, variables))->getId());
    ENGINE->propagate();
    CPPUNIT_ASSERT(ENGINE->constraintConsistent());
    CPPUNIT_ASSERT(v1.getDerivedDomain().getSingletonValue() == 1);

    delete (Constraint*) c2;
    ENGINE->propagate();
    CPPUNIT_ASSERT(ENGINE->constraintConsistent());
    CPPUNIT_ASSERT(v1.getDerivedDomain().getUpperBound() == 10);

    /* Add a constraint to force an inconsistency and show that consistency can be restored by removing the
     * constraint. */
    variables.clear();
    Variable<IntervalIntDomain> v5(ENGINE, IntervalIntDomain(0, 0));
    variables.push_back(v0.getId());
    variables.push_back(v5.getId());
    ConstraintId c3((new EqualConstraint(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, variables))->getId());
    ENGINE->propagate();
    CPPUNIT_ASSERT(ENGINE->provenInconsistent());
    delete (Constraint*) c3;
    ENGINE->propagate();
    CPPUNIT_ASSERT(ENGINE->constraintConsistent());

    // Clean up remaining constraint
    delete (Constraint*) c0;
    ENGINE->propagate();
    CPPUNIT_ASSERT(ENGINE->constraintConsistent());
    return true;
  }

  static bool testDelegation(){
      CETestEngine eng;
      const ConstraintEngineId& engine=((ConstraintEngine*)eng.getComponent("ConstraintEngine"))->getId();

    Variable<IntervalIntDomain> v0(engine, IntervalIntDomain(0, 1000));
    ConstraintId c0 = engine->createConstraint(LabelStr("TestOnly"), makeScope(v0.getId()));
    ConstraintId c1 = engine->createConstraint(LabelStr("TestOnly"), makeScope(v0.getId()));
    ConstraintId c2 = engine->createConstraint(LabelStr("TestOnly"), makeScope(v0.getId()));
    ConstraintId c3 = engine->createConstraint(LabelStr("TestOnly"), makeScope(v0.getId()));
    ConstraintId c4 = engine->createConstraint(LabelStr("TestOnly"), makeScope(v0.getId()));
    engine->propagate();
    CPPUNIT_ASSERT(engine->constraintConsistent());
    CPPUNIT_ASSERT(DelegationTestConstraint::s_instanceCount == 5);
    CPPUNIT_ASSERT(DelegationTestConstraint::s_executionCount == 5);

    // Cause a change in the domain which will impact agenda, then deactivate a constraint and verify the correct execution count
    v0.restrictBaseDomain(IntervalIntDomain(0, 900));
    c0->deactivate();
    CPPUNIT_ASSERT(!c0->isActive());
    engine->propagate();
    CPPUNIT_ASSERT(engine->constraintConsistent());
    CPPUNIT_ASSERT(DelegationTestConstraint::s_instanceCount == 5);
    CPPUNIT_ASSERT(DelegationTestConstraint::s_executionCount == 9);

    // Delete the delegate and verify instance counts and that the prior delegate has been reinstated and executed.
    delete (Constraint*) c1;
    c0->undoDeactivation();
    engine->propagate();
    CPPUNIT_ASSERT(engine->constraintConsistent());
    CPPUNIT_ASSERT(DelegationTestConstraint::s_instanceCount == 4);
    CPPUNIT_ASSERT(DelegationTestConstraint::s_executionCount == 10);

    // Now create a new instance and mark it for delegation only. Add remaining constraints as delegates
    ConstraintId c5 = engine->createConstraint(LabelStr("TestOnly"), makeScope(v0.getId()));
    c0->deactivate();
    c2->deactivate();
    c3->deactivate();
    c4->deactivate();
    CPPUNIT_ASSERT(DelegationTestConstraint::s_instanceCount == 5);
    engine->propagate();
    CPPUNIT_ASSERT(DelegationTestConstraint::s_executionCount == 11);

    // Force propagation and confirm only one instance executes
    v0.restrictBaseDomain(IntervalIntDomain(100, 900));
    engine->propagate();
    CPPUNIT_ASSERT(DelegationTestConstraint::s_executionCount == 12);

    // Now confirm correct handling of constraint deletions
    delete (Constraint*) c5;
    delete (Constraint*) c4;
    delete (Constraint*) c3;
    delete (Constraint*) c2;
    delete (Constraint*) c0;
    CPPUNIT_ASSERT(DelegationTestConstraint::s_instanceCount == 0);
    return true;
  }

  static bool testNotEqual(){/* TODO
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
    CPPUNIT_ASSERT(ENGINE->pending());
    bool res = ENGINE->propagate();
    CPPUNIT_ASSERT(res);
    CPPUNIT_ASSERT(v4.getDerivedDomain().isSingleton());
    CPPUNIT_ASSERT(v4.getDerivedDomain().getSingletonValue() == 3);

    // Test not equals among variables which are not singletons

    NotEqualConstraint c0(LabelStr("neq"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId()));
    NotEqualConstraint c1(LabelStr("neq"), LabelStr("Default"), ENGINE, makeScope(v1.getId(), v2.getId()));
    NotEqualConstraint c2(LabelStr("neq"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v2.getId()));
    CPPUNIT_ASSERT(ENGINE->pending());
    res = ENGINE->propagate();
    CPPUNIT_ASSERT(res);

    dom0.remove(2);
    v0.specify(dom0);
    CPPUNIT_ASSERT(!ENGINE->pending()); // No propagation required

    v1.specify(3);
    CPPUNIT_ASSERT(ENGINE->pending());
    res = ENGINE->propagate();
    CPPUNIT_ASSERT(res);
    CPPUNIT_ASSERT(v0.getDerivedDomain().getSingletonValue() == 1);
    CPPUNIT_ASSERT(v2.getDerivedDomain().getSingletonValue() == 2);

    v0.reset();
    CPPUNIT_ASSERT(ENGINE->pending());
    res = ENGINE->propagate();
    CPPUNIT_ASSERT(res);
    CPPUNIT_ASSERT(v0.getDerivedDomain() == v2.getDerivedDomain());
			     */
    return true;
  }

  static bool testMultEqualConstraint(){
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, 1));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(0, 2));
      MultEqualConstraint c0(LabelStr("MultEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
      CPPUNIT_ASSERT(v0.getDerivedDomain() == v2.getDerivedDomain());
    }

    // Now test with 0 valued denominators and infinites
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(0, PLUS_INFINITY));
      Variable<IntervalDomain> v1(ENGINE, IntervalDomain(1, PLUS_INFINITY));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(MINUS_INFINITY, 6));
      MultEqualConstraint c0(LabelStr("MultEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
      CPPUNIT_ASSERT(v0.getDerivedDomain().getUpperBound() == 6);
      CPPUNIT_ASSERT(v2.getDerivedDomain().getLowerBound() == 0);
      CPPUNIT_ASSERT(v1.getDerivedDomain().getUpperBound() == PLUS_INFINITY);
    }


    // Special case of negative values on LHS
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(-4, 10));
      Variable<IntervalDomain> v1(ENGINE, IntervalDomain(1, 10));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain());
      MultEqualConstraint c0(LabelStr("MultEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
      CPPUNIT_ASSERT(v2.getDerivedDomain().getLowerBound() == -40);
    }

    // Part of test 157 of CLibTestCases, which incorrectly claimed to be
    // inconsistent during testing of a trial version of MultEqualConstraint.
    {
      Variable<IntervalDomain> v0(ENGINE, IntervalDomain(1.0));
      Variable<IntervalDomain> v1(ENGINE, IntervalDomain(MINUS_INFINITY, PLUS_INFINITY));
      Variable<IntervalDomain> v2(ENGINE, IntervalDomain(1.0, 2.0));
      MultEqualConstraint c0(LabelStr("MultEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
      CPPUNIT_ASSERT(v1.getDerivedDomain() == IntervalDomain(1.0, 2.0));
    }
    // Signs tested
    {
      Variable<IntervalDomain> v0(ENGINE, IntervalDomain(-2.3, -1.8));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(1, PLUS_INFINITY));
      Variable<IntervalDomain> v2(ENGINE, IntervalDomain(-11.6, 12.4));
      MultEqualConstraint c0(LabelStr("MultEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
      CPPUNIT_ASSERT_MESSAGE(v0.getDerivedDomain().toString(), v0.getDerivedDomain() == IntervalDomain(-2.3, -1.8));
      CPPUNIT_ASSERT_MESSAGE(v1.getDerivedDomain().toString(), v1.getDerivedDomain() == IntervalIntDomain(1, 6));
      CPPUNIT_ASSERT_MESSAGE(v2.getDerivedDomain().toString(), v2.getDerivedDomain() == IntervalDomain(-11.6, -1.8));
    }

    // Signs tested
    {
      Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, PLUS_INFINITY));
      Variable<IntervalDomain> v1(ENGINE, IntervalDomain(-2.3, -1.8));
      Variable<IntervalDomain> v2(ENGINE, IntervalDomain(-11.6, 12.4));
      MultEqualConstraint c0(LabelStr("MultEqualConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
      CPPUNIT_ASSERT_MESSAGE(v0.getDerivedDomain().toString(), v0.getDerivedDomain() == IntervalIntDomain(1, 6));
      CPPUNIT_ASSERT_MESSAGE(v1.getDerivedDomain().toString(), v1.getDerivedDomain() == IntervalDomain(-2.3, -1.8));
      CPPUNIT_ASSERT_MESSAGE(v2.getDerivedDomain().toString(), v2.getDerivedDomain() == IntervalDomain(-11.6, -1.8));
    }

    // Special test for roundig errors
    IntervalDomain dom;
    MultEqualConstraint::updateMinAndMax(dom, 3.225, 3.225, -1, -1);
    IntervalDomain expectedResult(-1.0 / 3.225, -1.0 / 3.225);
    CPPUNIT_ASSERT_MESSAGE(dom.toString() + " != " + expectedResult.toString(), dom == expectedResult);
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
      CPPUNIT_ASSERT(res);
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
      CPPUNIT_ASSERT(!res);
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
      CPPUNIT_ASSERT(res);
      CPPUNIT_ASSERT(v2.getDerivedDomain().getSingletonValue() == 0);
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
      CPPUNIT_ASSERT(res);
      CPPUNIT_ASSERT(v0.getDerivedDomain() == IntervalIntDomain(1, 9));
      CPPUNIT_ASSERT(v2.getDerivedDomain() == IntervalDomain(1.0, 9.0));
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
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
      CPPUNIT_ASSERT(v0.getDerivedDomain().getSingletonValue() == 1);
      CPPUNIT_ASSERT(v1.getDerivedDomain().getSingletonValue() == 1);
      CPPUNIT_ASSERT(v2.getDerivedDomain().getSingletonValue() == 2);
    }
    { // Same, but add another variable that will be constrained to 0
      std::vector<ConstrainedVariableId> scope;
      scope.push_back(v2.getId());
      scope.push_back(v0.getId());
      scope.push_back(v1.getId());
      scope.push_back(v3.getId());
      EqualSumConstraint c0(LabelStr("EqualSumConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
      CPPUNIT_ASSERT(v0.getDerivedDomain().getSingletonValue() == 1);
      CPPUNIT_ASSERT(v1.getDerivedDomain().getSingletonValue() == 1);
      CPPUNIT_ASSERT(v2.getDerivedDomain().getSingletonValue() == 2);
      CPPUNIT_ASSERT(v3.getDerivedDomain().getSingletonValue() == 0);
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
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
      CPPUNIT_ASSERT(v0.getDerivedDomain().getSingletonValue() == 1);
      CPPUNIT_ASSERT(v1.getDerivedDomain().getSingletonValue() == 1);
      CPPUNIT_ASSERT(v2.getDerivedDomain().getSingletonValue() == 2);
      CPPUNIT_ASSERT(v3.getDerivedDomain().getSingletonValue() == 0);
      CPPUNIT_ASSERT(v4.getDerivedDomain().getSingletonValue() == 0);
      CPPUNIT_ASSERT(v5.getDerivedDomain().getSingletonValue() == 0);
      CPPUNIT_ASSERT(v6.getDerivedDomain().getSingletonValue() == 0);
      CPPUNIT_ASSERT(v7.getDerivedDomain().getSingletonValue() == 0);
      CPPUNIT_ASSERT(v8.getDerivedDomain().getSingletonValue() == 0);
      CPPUNIT_ASSERT(v9.getDerivedDomain().getSingletonValue() == 0);
      CPPUNIT_ASSERT(vA.getDerivedDomain().getSingletonValue() == 0);
      CPPUNIT_ASSERT(vB.getDerivedDomain().getSingletonValue() == 0);
      CPPUNIT_ASSERT(vC.getDerivedDomain().getSingletonValue() == 0);
      CPPUNIT_ASSERT(vD.getDerivedDomain().getSingletonValue() == 0);
      CPPUNIT_ASSERT(vE.getDerivedDomain().getSingletonValue() == 0);
      CPPUNIT_ASSERT(vF.getDerivedDomain().getSingletonValue() == 0);
      CPPUNIT_ASSERT(vG.getDerivedDomain().getSingletonValue() == 0);
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
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
      CPPUNIT_ASSERT(!bothVar.getDerivedDomain().isSingleton());
      CPPUNIT_ASSERT(v0.getDerivedDomain() == IntervalIntDomain(1, 10));
      CPPUNIT_ASSERT(v1.getDerivedDomain().getSingletonValue() == 1);
    }
    CPPUNIT_ASSERT(!bothVar.getDerivedDomain().isSingleton());
    scope.clear();
    {
      scope.push_back(bothVar.getId());
      scope.push_back(v0.getId());
      scope.push_back(vA.getId());
      CondAllSameConstraint c0(LabelStr("CondAllSameConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
      CPPUNIT_ASSERT(bothVar.getDerivedDomain() == BoolDomain(false));
    }
    CPPUNIT_ASSERT(!bothVar.getDerivedDomain().isSingleton());
    scope.clear();
    {
      scope.push_back(bothVar.getId());
      scope.push_back(v6.getId());
      scope.push_back(v7.getId());
      scope.push_back(v8.getId());
      CondAllSameConstraint c0(LabelStr("CondAllSameConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
      CPPUNIT_ASSERT(bothVar.getDerivedDomain() == BoolDomain(true));
    }
    CPPUNIT_ASSERT(!bothVar.getDerivedDomain().isSingleton());
    scope.clear();
    {
      scope.push_back(bothVar.getId());
      scope.push_back(v2.getId());
      scope.push_back(v6.getId());
      scope.push_back(v7.getId());
      scope.push_back(v8.getId());
      CondAllSameConstraint c0(LabelStr("CondAllSameConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
      CPPUNIT_ASSERT(!bothVar.getDerivedDomain().isSingleton());
    }
    CPPUNIT_ASSERT(!bothVar.getDerivedDomain().isSingleton());
    scope.clear();
    {
      scope.push_back(bothVar.getId());
      scope.push_back(v6.getId());
      scope.push_back(v7.getId());
      scope.push_back(v8.getId());
      scope.push_back(v2.getId());
      CondAllSameConstraint c0(LabelStr("CondAllSameConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
      CPPUNIT_ASSERT(!bothVar.getDerivedDomain().isSingleton());
    }
    CPPUNIT_ASSERT(!bothVar.getDerivedDomain().isSingleton());
    scope.clear();
    {
      scope.push_back(bothVar.getId());
      scope.push_back(v6.getId());
      scope.push_back(v7.getId());
      scope.push_back(v2.getId());
      scope.push_back(v8.getId());
      CondAllSameConstraint c0(LabelStr("CondAllSameConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
      CPPUNIT_ASSERT(!bothVar.getDerivedDomain().isSingleton());
    }
    CPPUNIT_ASSERT(!bothVar.getDerivedDomain().isSingleton());
    scope.clear();
    {
      scope.push_back(trueVar.getId());
      scope.push_back(v2.getId());
      scope.push_back(v6.getId());
      scope.push_back(v7.getId());
      CondAllSameConstraint c0(LabelStr("CondAllSameConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
      CPPUNIT_ASSERT(v2.getDerivedDomain() == IntervalIntDomain(0));
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
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
      CPPUNIT_ASSERT(v2.getDerivedDomain() == IntervalIntDomain(0));
      CPPUNIT_ASSERT(v3.getDerivedDomain() == IntervalIntDomain(0));
    }
    scope.clear();
    {
      scope.push_back(trueVar.getId());
      scope.push_back(v2.getId());
      scope.push_back(v3.getId());
      CondAllSameConstraint c0(LabelStr("CondAllSameConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
      CPPUNIT_ASSERT(v2.getDerivedDomain() == IntervalIntDomain(0, 2));
      CPPUNIT_ASSERT(v3.getDerivedDomain() == IntervalIntDomain(0, 2));
    }
    scope.clear();
    {
      scope.push_back(trueVar.getId());
      scope.push_back(v1.getId());
      scope.push_back(v2.getId());
      scope.push_back(v3.getId());
      CondAllSameConstraint c0(LabelStr("CondAllSameConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
      CPPUNIT_ASSERT(v2.getDerivedDomain() == IntervalIntDomain(1));
      CPPUNIT_ASSERT(v3.getDerivedDomain() == IntervalIntDomain(1));
    }
    CPPUNIT_ASSERT(v2.getDerivedDomain() == IntervalIntDomain(0, 2));
    CPPUNIT_ASSERT(v3.getDerivedDomain() == IntervalIntDomain(0, 27));
    scope.clear();
    {
      scope.push_back(falseVar.getId());
      scope.push_back(v3.getId());
      scope.push_back(v6.getId());
      scope.push_back(v7.getId());
      CondAllSameConstraint c0(LabelStr("CondAllSameConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
      CPPUNIT_ASSERT(v3.getDerivedDomain().isSubsetOf(IntervalIntDomain(1, 27)));
    }
    scope.clear();
    {
      scope.push_back(falseVar.getId());
      scope.push_back(v6.getId());
      scope.push_back(v7.getId());
      CondAllSameConstraint c0(LabelStr("CondAllSameConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      CPPUNIT_ASSERT(!ENGINE->constraintConsistent());
    }
    ENGINE->propagate();
    CPPUNIT_ASSERT(ENGINE->constraintConsistent());
    scope.clear();
    {
      scope.push_back(falseVar.getId());
      scope.push_back(v2.getId());
      scope.push_back(v3.getId());
      CondAllSameConstraint c0(LabelStr("CondAllSameConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
      CPPUNIT_ASSERT(v2.getDerivedDomain() == IntervalIntDomain(0, 2));
      CPPUNIT_ASSERT(v3.getDerivedDomain() == IntervalIntDomain(0, 27));
    }
    ENGINE->propagate();
    CPPUNIT_ASSERT(ENGINE->constraintConsistent());
    scope.clear();
    {
      scope.push_back(trueVar.getId());
      scope.push_back(v0.getId());
      scope.push_back(vA.getId());
      CondAllSameConstraint c0(LabelStr("CondAllSameConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      CPPUNIT_ASSERT(!ENGINE->constraintConsistent());
    }
    ENGINE->propagate();
    CPPUNIT_ASSERT(ENGINE->constraintConsistent());
    CPPUNIT_ASSERT(!bothVar.getDerivedDomain().isSingleton());
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
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
      CPPUNIT_ASSERT(!bothVar.getDerivedDomain().isSingleton());
      CPPUNIT_ASSERT(v0.getDerivedDomain() == IntervalIntDomain(1, 10));
      CPPUNIT_ASSERT(v1.getDerivedDomain().getSingletonValue() == 1);
    }
    scope.clear();
    {
      scope.push_back(bothVar.getId());
      scope.push_back(v0.getId());
      scope.push_back(v6.getId());
      scope.push_back(vA.getId());
      CondAllDiffConstraint c0(LabelStr("CondAllDiffConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
      CPPUNIT_ASSERT(bothVar.getDerivedDomain().getSingletonValue());
      CPPUNIT_ASSERT(v0.getDerivedDomain() == IntervalIntDomain(1, 10));
      CPPUNIT_ASSERT(v6.getDerivedDomain().getSingletonValue() == 0);
      CPPUNIT_ASSERT(vA.getDerivedDomain() == IntervalIntDomain(11, 27));
    }
    CPPUNIT_ASSERT(!bothVar.getDerivedDomain().isSingleton());
    scope.clear();
    {
      scope.push_back(bothVar.getId());
      scope.push_back(v0.getId());
      scope.push_back(v6.getId());
      scope.push_back(v9.getId());
      scope.push_back(vA.getId());
      CondAllDiffConstraint c0(LabelStr("CondAllDiffConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
      CPPUNIT_ASSERT(!bothVar.getDerivedDomain().isSingleton());
      CPPUNIT_ASSERT(v0.getDerivedDomain() == IntervalIntDomain(1, 10));
      CPPUNIT_ASSERT(v6.getDerivedDomain().getSingletonValue() == 0);
      CPPUNIT_ASSERT(v9.getDerivedDomain().getSingletonValue() == 1);
      CPPUNIT_ASSERT(vA.getDerivedDomain() == IntervalIntDomain(11, 27));
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
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
      CPPUNIT_ASSERT(trueVar.getDerivedDomain().isSingleton());
      CPPUNIT_ASSERT(v0.getDerivedDomain() == IntervalIntDomain(3, 10));
      CPPUNIT_ASSERT(v2.getDerivedDomain().getSingletonValue() == 2);
      CPPUNIT_ASSERT(v6.getDerivedDomain().getSingletonValue() == 0);
      CPPUNIT_ASSERT(v9.getDerivedDomain().getSingletonValue() == 1);
      CPPUNIT_ASSERT(vA.getDerivedDomain() == IntervalIntDomain(11, 27));
      CPPUNIT_ASSERT(vB.getDerivedDomain().getSingletonValue() == -1);
    }
    CPPUNIT_ASSERT(v0.getDerivedDomain() == IntervalIntDomain(1, 10));
    CPPUNIT_ASSERT(v2.getDerivedDomain() == IntervalIntDomain(0, 2));
    CPPUNIT_ASSERT(vB.getDerivedDomain() == IntervalIntDomain(-1, 2));
    scope.clear();
    {
      scope.push_back(falseVar.getId());
      scope.push_back(v0.getId()); // 1 10
      scope.push_back(v6.getId()); // 1 10, 0 (inconsistent)
      scope.push_back(v9.getId()); // 1, 0, 1
      scope.push_back(vA.getId()); // 1, 0, 1, 11 27
      CondAllDiffConstraint c0(LabelStr("CondAllDiffConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
      // This next restriction is correct but the current implementation
      //   does not enforce it, as it requires checking all pairs to see
      //   that two individual vars are the only ones that overlap and
      //   therefore must be equal.
      // CPPUNIT_ASSERT(v0.getDerivedDomain().getSingletonValue() == 1);
      CPPUNIT_ASSERT(v6.getDerivedDomain().getSingletonValue() == 0);
      CPPUNIT_ASSERT(v9.getDerivedDomain().getSingletonValue() == 1);
      CPPUNIT_ASSERT(vA.getDerivedDomain() == IntervalIntDomain(11, 27));
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
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
      CPPUNIT_ASSERT(!bothVar.getDerivedDomain().getSingletonValue());
      CPPUNIT_ASSERT(v6.getDerivedDomain() == IntervalIntDomain(0));
      CPPUNIT_ASSERT(v7.getDerivedDomain() == IntervalIntDomain(0));
      CPPUNIT_ASSERT(v8.getDerivedDomain() == IntervalIntDomain(0));
      CPPUNIT_ASSERT(v9.getDerivedDomain() == IntervalIntDomain(1));
      CPPUNIT_ASSERT(vA.getDerivedDomain() == IntervalIntDomain(11, 27));
    }
    CPPUNIT_ASSERT(!bothVar.getDerivedDomain().isSingleton());
    scope.clear();
    {
      scope.push_back(bothVar.getId());
      scope.push_back(vC.getId()); // 10 11
      scope.push_back(vD.getId()); // 10 11, 10 11
      CondAllDiffConstraint c0(LabelStr("CondAllDiffConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
      CPPUNIT_ASSERT(!bothVar.getDerivedDomain().isSingleton());
    }
    scope.clear();
    {
      scope.push_back(bothVar.getId());
      scope.push_back(vC.getId()); // 10 11
      scope.push_back(vD.getId()); // 10 11, 10 11
      scope.push_back(vE.getId()); // 10 11, 10 11, 10 11 -> false: three vars but only two values
      CondAllDiffConstraint c0(LabelStr("CondAllDiffConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
      CPPUNIT_ASSERT(!bothVar.getDerivedDomain().getSingletonValue());
    }
    CPPUNIT_ASSERT(!bothVar.getDerivedDomain().isSingleton());
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
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
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
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
      CPPUNIT_ASSERT(v0.getDerivedDomain() == IntervalIntDomain(10));
      CPPUNIT_ASSERT(v1.getDerivedDomain() == IntervalIntDomain(1));
      CPPUNIT_ASSERT(v2.getDerivedDomain() == IntervalIntDomain(2));
      CPPUNIT_ASSERT(v3.getDerivedDomain() == IntervalIntDomain(16, 27));
      CPPUNIT_ASSERT(v4.getDerivedDomain() == IntervalIntDomain(16, 27));
      CPPUNIT_ASSERT(v5.getDerivedDomain() == IntervalIntDomain(16, 27));
      CPPUNIT_ASSERT(v6.getDerivedDomain() == IntervalIntDomain(0));
      CPPUNIT_ASSERT(vA.getDerivedDomain() == IntervalIntDomain(16, 27));
      CPPUNIT_ASSERT(vB.getDerivedDomain() == IntervalIntDomain(-1));
      CPPUNIT_ASSERT(vC.getDerivedDomain() == IntervalIntDomain(11));
      CPPUNIT_ASSERT(vF.getDerivedDomain() == IntervalIntDomain(3));
      CPPUNIT_ASSERT(vG.getDerivedDomain() == IntervalIntDomain(12));
      CPPUNIT_ASSERT(vH.getDerivedDomain() == IntervalIntDomain(4));
      CPPUNIT_ASSERT(vI.getDerivedDomain() == IntervalIntDomain(7));
      CPPUNIT_ASSERT(vJ.getDerivedDomain() == IntervalIntDomain(8));
      CPPUNIT_ASSERT(vK.getDerivedDomain() == IntervalIntDomain(9));
      CPPUNIT_ASSERT(vL.getDerivedDomain() == IntervalIntDomain(13));
      CPPUNIT_ASSERT(vM.getDerivedDomain() == IntervalIntDomain(5));
      CPPUNIT_ASSERT(vN.getDerivedDomain() == IntervalIntDomain(14));
      CPPUNIT_ASSERT(vO.getDerivedDomain() == IntervalIntDomain(6));
      CPPUNIT_ASSERT(vP.getDerivedDomain() == IntervalIntDomain(-2));
      CPPUNIT_ASSERT(vQ.getDerivedDomain() == IntervalIntDomain(15));
    }
    scope.clear();
    {
      scope.push_back(falseVar.getId());
      scope.push_back(v6.getId()); // 0
      scope.push_back(v7.getId()); // 0
      CondAllDiffConstraint c0(LabelStr("CondAllDiffConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
      CPPUNIT_ASSERT(v6.getDerivedDomain().getSingletonValue() == 0);
      CPPUNIT_ASSERT(v7.getDerivedDomain().getSingletonValue() == 0);
    }
    scope.clear();
    {
      scope.push_back(falseVar.getId());
      scope.push_back(v3.getId()); // 0 27
      scope.push_back(v4.getId()); // 0 27
      CondAllDiffConstraint c0(LabelStr("CondAllDiffConstraint"), LabelStr("Default"), ENGINE, scope);
      ENGINE->propagate();
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
      CPPUNIT_ASSERT(v3.getDerivedDomain() == IntervalIntDomain(0, 27));
      CPPUNIT_ASSERT(v4.getDerivedDomain() == IntervalIntDomain(0, 27));
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
      CPPUNIT_ASSERT(ENGINE->constraintConsistent());
      // Only vF and vI overlap, so they have to be equal.
      CPPUNIT_ASSERT(vF.getDerivedDomain() == IntervalIntDomain(0, 3));
      // This next restriction is correct but the current implementation
      //   does not enforce it, as it requires checking all pairs to see
      //   that two individual vars are the only ones that overlap and
      //   therefore must be equal.
      // CPPUNIT_ASSERT(vI.getDerivedDomain() == IntervalIntDomain(0, 3));
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
    CPPUNIT_ASSERT(!res);

    // Reset, and delete constraint, but it should not matter
    v1.reset();
    delete (Constraint*) c2;

    // Confirm still inconsistent
    res = ENGINE->propagate();
    CPPUNIT_ASSERT(!res);

    delete (Constraint*) c0;
    delete (Constraint*) c1;
    return true;
  }

  /**
   * @brief Run arbtrary constraints with arbitrary input domains,
   * comparing the propagated domains with the expected output domains.
   */
  static bool testArbitraryConstraints() {
      CETestEngine testEngine;
    // Input to this test: a list of constraint calls and expected output domains.
    std::list<ConstraintTestCase> tests;

    // This kind of information can also be read from a file, as below.
    std::string constraintName("Equal");
    std::list<AbstractDomain*> domains;
    domains.push_back(new IntervalIntDomain(1, 10)); // first input domain
    domains.push_back(new IntervalIntDomain(2, 10)); // expected value of first output domain
    domains.push_back(new IntervalIntDomain(2, 11)); // second input domain
    domains.push_back(new IntervalIntDomain(2, 10)); // expected value of second output domain
    tests.push_back(ConstraintTestCase(constraintName, __FILE__, "1", std::list<AbstractDomain*>(domains)));

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
    CPPUNIT_ASSERT(readTestCases(getTestLoadLibraryPath() + std::string("/NewTestCases.xml"), tests) ||
               readTestCases(std::string("ConstraintEngine/test/NewTestCases.xml"), tests));

    CPPUNIT_ASSERT(readTestCases(getTestLoadLibraryPath() + std::string("/CLibTestCases.xml"), tests) ||
               readTestCases(std::string("ConstraintEngine/test/CLibTestCases.xml"), tests));

    return(executeTestCases(testEngine.getConstraintEngine(), tests));
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
    CPPUNIT_ASSERT(v0.getDerivedDomain() != lockDomain);
    Variable<LabelSet> v1(ENGINE, lockDomain);

    // Post constraint, and ensure it is propagated to equality with lock domain
    LockConstraint c0(LabelStr("Lock"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId()));
    CPPUNIT_ASSERT(ENGINE->propagate());
    CPPUNIT_ASSERT(v0.getDerivedDomain() == lockDomain);

    // Now specify to a restricted value, and ensure an inconsistency
    v0.specify(EUROPA::LabelStr("C"));
    CPPUNIT_ASSERT(!ENGINE->propagate());

    // Now ensure that we can rty again, without changing anything, and get the same result.
    CPPUNIT_ASSERT(!ENGINE->propagate());

    // Reset the variable and ensure we can repropagate it correctly
    v0.reset();
    CPPUNIT_ASSERT(ENGINE->propagate());
    CPPUNIT_ASSERT(v0.getDerivedDomain() == lockDomain);

    return true;
  }

  static bool testNegateConstraint() {
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain());
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(MINUS_INFINITY, 0));
    NegateConstraint c0(LabelStr("NegateConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId()));
    ENGINE->propagate();
    CPPUNIT_ASSERT(ENGINE->constraintConsistent());
    CPPUNIT_ASSERT(v0.getDerivedDomain() == IntervalIntDomain(0, PLUS_INFINITY));
    CPPUNIT_ASSERT(v1.getDerivedDomain() == IntervalIntDomain(MINUS_INFINITY, 0));

    v0.restrictBaseDomain(IntervalIntDomain(20, 30));
    CPPUNIT_ASSERT(v1.getDerivedDomain() == IntervalIntDomain(-30, -20));

    v1.restrictBaseDomain(IntervalIntDomain(-22, -21));
    CPPUNIT_ASSERT(v0.getDerivedDomain() == IntervalIntDomain(21, 22));

    v0.specify(21);
    CPPUNIT_ASSERT(v1.getDerivedDomain().getSingletonValue() == -21);
    return true;
  }

  static bool testUnaryQuery() {
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain());
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain());
    NegateConstraint c0(LabelStr("NegateConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v1.getId()));
    CPPUNIT_ASSERT(!c0.isUnary());

    Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(1));
    NegateConstraint c1(LabelStr("NegateConstraint"), LabelStr("Default"), ENGINE, makeScope(v0.getId(), v2.getId()));
    CPPUNIT_ASSERT(c1.isUnary());

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
      CPPUNIT_ASSERT(v0.getDerivedDomain().isTrue());
    }
    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain());
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(5));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(6));
      TestEQ c0(LabelStr("TestEQ"), LabelStr("Default"),
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      CPPUNIT_ASSERT(v0.getDerivedDomain().isFalse());
    }
    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain());
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(5,10));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(5, 20));
      TestEQ c0(LabelStr("TestEQ"), LabelStr("Default"),
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      CPPUNIT_ASSERT(!v0.getDerivedDomain().isSingleton());
      v1.specify(7);
      v2.specify(7);
      ENGINE->propagate();
      CPPUNIT_ASSERT(v0.getDerivedDomain().isTrue());
      v1.reset();
      ENGINE->propagate();
      CPPUNIT_ASSERT(!v0.getDerivedDomain().isSingleton());
    }
    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain());
      Variable<LabelSet> v1(ENGINE, LabelSet(LabelStr("C")));
      Variable<LabelSet> v2(ENGINE, LabelSet(LabelStr("C")));
      TestEQ c0("TestEQ", "Default",
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      CPPUNIT_ASSERT(v0.getDerivedDomain().isTrue());
    }
    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain());
      Variable<LabelSet> v1(ENGINE, LabelSet(LabelStr("C")));
      Variable<LabelSet> v2(ENGINE, LabelSet(LabelStr("E")));
      TestEQ c0(LabelStr("TestEQ"), LabelStr("Default"),
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      CPPUNIT_ASSERT(v0.getDerivedDomain().isFalse());
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
      CPPUNIT_ASSERT(v0.getDerivedDomain().isTrue());
    }

    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain());
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(5));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(5));
      TestLessThan c0(LabelStr("TestLessThan"), LabelStr("Default"),
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      CPPUNIT_ASSERT(v0.getDerivedDomain().isFalse());
    }

    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain());
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(5, 10));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(5));
      TestLessThan c0(LabelStr("TestLessThan"), LabelStr("Default"),
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      CPPUNIT_ASSERT(v0.getDerivedDomain().isFalse());
    }

    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain());
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(4, 10));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(11, 20));
      TestLessThan c0(LabelStr("TestLessThan"), LabelStr("Default"),
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      CPPUNIT_ASSERT(v0.getDerivedDomain().isTrue());
    }

    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain());
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(4, 10));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(8, 20));
      TestLessThan c0(LabelStr("TestLessThan"), LabelStr("Default"),
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();
      CPPUNIT_ASSERT(!v0.getDerivedDomain().isSingleton());
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

      CPPUNIT_ASSERT(v0.getDerivedDomain().isTrue());
    }

    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain());
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(5));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(4));

      TestLEQ c0(LabelStr("TestLEQ"), LabelStr("Default"),
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      ENGINE->propagate();

      CPPUNIT_ASSERT(v0.getDerivedDomain().isFalse());
    }

    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain());
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(4));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(6));

      TestLEQ c0(LabelStr("TestLEQ"), LabelStr("Default"),
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));
      CPPUNIT_ASSERT(ENGINE->propagate());
      CPPUNIT_ASSERT(v0.getDerivedDomain().isTrue());
    }

    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain());
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(0, 10));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(0, 10));

      TestLEQ c0(LabelStr("TestLEQ"), LabelStr("Default"),
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));

      CPPUNIT_ASSERT(ENGINE->propagate());

      CPPUNIT_ASSERT(!v0.getDerivedDomain().isSingleton());
    }

    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain(false));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(0, 10));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(11, 20));

      TestLEQ c0(LabelStr("TestLEQ"), LabelStr("Default"),
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));

      CPPUNIT_ASSERT(!ENGINE->propagate());
    }

    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain(true));
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(5, 10));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(2, 4));

      TestLEQ c0(LabelStr("TestLEQ"), LabelStr("Default"),
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));

      CPPUNIT_ASSERT(!ENGINE->propagate());
    }

    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain());
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(0, 10));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(11, 20));

      TestLEQ c0(LabelStr("TestLEQ"), LabelStr("Default"),
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));

      CPPUNIT_ASSERT(ENGINE->propagate());
      CPPUNIT_ASSERT(v0.getDerivedDomain().isTrue());
    }

    {
      Variable<BoolDomain> v0(ENGINE, BoolDomain());
      Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(10, 20));
      Variable<IntervalIntDomain> v2(ENGINE, IntervalIntDomain(0, 9));

      TestLEQ c0(LabelStr("TestLEQ"), LabelStr("Default"),
		ENGINE, makeScope(v0.getId(), v1.getId(), v2.getId()));

      CPPUNIT_ASSERT(ENGINE->propagate());
      CPPUNIT_ASSERT(v0.getDerivedDomain().isFalse());
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
    EUROPA_runTest(testAllocation);
    return true;
  }

private:
  static bool testAllocation(){
      CETestEngine testEngine;
      const ConstraintEngineId& ce=((ConstraintEngine*)testEngine.getComponent("ConstraintEngine"))->getId();

    std::vector<ConstrainedVariableId> variables;
    // v0 == v1
    Variable<IntervalIntDomain> v0(ce, IntervalIntDomain(1, 10));
    variables.push_back(v0.getId());
    Variable<IntervalIntDomain> v1(ce, IntervalIntDomain(1, 1));
    variables.push_back(v1.getId());
    ConstraintId c0 = ce->createConstraint(LabelStr("Equal"), variables);
    ce->propagate();
    CPPUNIT_ASSERT(v0.getDerivedDomain().getSingletonValue() == 1);
    delete (Constraint*) c0;
    return true;
  }
};

class EquivalenceClassTest{
public:
  static bool test() {
    EUROPA_runTest(testBasicAllocation);
    EUROPA_runTest(testConstructionOfSingleGraph);
    EUROPA_runTest(testSplittingOfSingleGraph);
    EUROPA_runTest(testMultiGraphMerging);
    EUROPA_runTest(testEqualityConstraintPropagator);
    return true;
  }

private:
  static bool testBasicAllocation(){
    Variable<IntervalIntDomain> v0(ENGINE, IntervalIntDomain(1, 10));
    Variable<IntervalIntDomain> v1(ENGINE, IntervalIntDomain(2, 8));
    EquivalenceClassCollection g0;
    g0.addConnection(v0.getId(), v1.getId());
    CPPUNIT_ASSERT(g0.getGraphCount() == 1);
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
    CPPUNIT_ASSERT(g0.getGraphCount() == 1);
    int graphKey = g0.getGraphKey(v0.getId());
    CPPUNIT_ASSERT(g0.getGraphKey(v1.getId()) == graphKey);
    CPPUNIT_ASSERT(g0.getGraphKey(v2.getId()) == graphKey);

    Variable<IntervalIntDomain> v3(ENGINE, IntervalIntDomain(1, 100));
    Variable<IntervalIntDomain> v4(ENGINE, IntervalIntDomain(-100, 100));
    g0.addConnection(v2.getId(), v3.getId());
    g0.addConnection(v3.getId(), v4.getId());
    CPPUNIT_ASSERT(g0.getGraphCount() == 1);
    CPPUNIT_ASSERT(graphKey != g0.getGraphKey(v0.getId())); // Should have updated for all
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
    CPPUNIT_ASSERT(g0.getGraphCount() == 1);

    // Cause a split by removing a connection in the middle
    g0.removeConnection(v3.getId(), v4.getId());
    CPPUNIT_ASSERT(g0.getGraphCount() == 2);

    // Cause another split
    g0.removeConnection(v5.getId(), v6.getId());
    CPPUNIT_ASSERT(g0.getGraphCount() == 3);

    // Test membership of resulting classes
    CPPUNIT_ASSERT((g0.getGraphKey(v1.getId()) + g0.getGraphKey(v2.getId()) + g0.getGraphKey(v3.getId()))/3 == g0.getGraphKey(v0.getId()));
    CPPUNIT_ASSERT(g0.getGraphKey(v4.getId()) == g0.getGraphKey(v5.getId()));
    CPPUNIT_ASSERT((g0.getGraphKey(v6.getId()) + g0.getGraphKey(v7.getId()))/2   == g0.getGraphKey(v8.getId()));

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
    CPPUNIT_ASSERT(g0.getGraphCount() == 3);
    CPPUNIT_ASSERT((g0.getGraphKey(v1.getId()) + g0.getGraphKey(v2.getId()) + g0.getGraphKey(v3.getId()))/3 == g0.getGraphKey(v0.getId()));
    CPPUNIT_ASSERT(g0.getGraphKey(v4.getId()) == g0.getGraphKey(v5.getId()));
    CPPUNIT_ASSERT((g0.getGraphKey(v6.getId()) + g0.getGraphKey(v7.getId()))/2   == g0.getGraphKey(v8.getId()));

    // Add connectionto cause a merge
    g0.addConnection(v3.getId(), v4.getId());
    CPPUNIT_ASSERT(g0.getGraphCount() == 2);
    CPPUNIT_ASSERT((g0.getGraphKey(v1.getId()) + g0.getGraphKey(v2.getId()) + g0.getGraphKey(v3.getId()))/3 == g0.getGraphKey(v0.getId()));
    CPPUNIT_ASSERT((g0.getGraphKey(v4.getId()) + g0.getGraphKey(v5.getId()))/2 == g0.getGraphKey(v0.getId()));
    CPPUNIT_ASSERT((g0.getGraphKey(v6.getId()) + g0.getGraphKey(v7.getId()))/2   == g0.getGraphKey(v8.getId()));


    // Add connectionto cause a merge
    g0.addConnection(v5.getId(), v6.getId());
    CPPUNIT_ASSERT(g0.getGraphCount() == 1);

    return true;
  }

  static bool testEqualityConstraintPropagator(){
      CETestEngine engine;
      ConstraintEngineId ce = ((ConstraintEngine*)engine.getComponent("ConstraintEngine"))->getId();

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

      CPPUNIT_ASSERT(v0.getDerivedDomain().getUpperBound() == 10);
      CPPUNIT_ASSERT(v2.getDerivedDomain().getSingletonValue() == 10);

      variables.clear();
      variables.push_back(v3.getId());
      variables.push_back(v1.getId());
      EqualConstraint c2(LabelStr("EqualConstraint"), LabelStr("EquivalenceClass"), ce, variables);

      ce->propagate();
      CPPUNIT_ASSERT(ce->constraintConsistent());
      CPPUNIT_ASSERT(v0.getDerivedDomain().getSingletonValue() == 10);

      variables.clear();
      Variable<IntervalIntDomain> v4(ce, IntervalIntDomain(1, 9));
      variables.push_back(v3.getId());
      variables.push_back(v4.getId());
      ConstraintId c3((new EqualConstraint(LabelStr("EqualConstraint"), LabelStr("EquivalenceClass"), ce, variables))->getId());
      ce->propagate();
      CPPUNIT_ASSERT(ce->provenInconsistent());

      delete (Constraint*) c3;
      CPPUNIT_ASSERT(ce->pending());
      ce->propagate();
      CPPUNIT_ASSERT(ce->constraintConsistent());
      CPPUNIT_ASSERT(v0.getDerivedDomain().getSingletonValue() == 10);
    }
    return(true);
  }
};

void ConstraintEngineModuleTests::cppSetup(void)
{
    setTestLoadLibraryPath(".");
}

void ConstraintEngineModuleTests::domainTests(void)
{
    DomainTests::test();
}

void ConstraintEngineModuleTests::typeFactoryTests(void)
{
    TypeFactoryTests::test();
}

void ConstraintEngineModuleTests::entityTests(void)
{
    EntityTests::test();
}

void ConstraintEngineModuleTests::constraintEngineTests(void)
{
    ConstraintEngineTest::test();
}

void ConstraintEngineModuleTests::variableTests(void)
{
    VariableTest::test();
}

void ConstraintEngineModuleTests::constraintTests(void)
{
    ConstraintTest::test();
}

void ConstraintEngineModuleTests::constraintFactoryTests(void)
{
    ConstraintFactoryTest::test();
}

void ConstraintEngineModuleTests::equivalenceClassTests(void)
{
    EquivalenceClassTest::test();
}

