#include "HSTSModuleTests.hh"
#include "CBPlannerDefs.hh"
#include "Horizon.hh"
#include "DecisionManager.hh"
#include "Utils.hh"
#include "TestSupport.hh"
#include "HSTSNoBranch.hh"
#include "HSTSNoBranchCondition.hh"
#include "HSTSPlanIdReader.hh"

#include "DefaultPropagator.hh"
#include "ConstraintEngine.hh"
#include "Constraint.hh"
#include "Variable.hh"

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

#include "PlanDatabaseDefs.hh"
#include "PlanDatabase.hh"
#include "Schema.hh"
#include "Object.hh"
#include "EventToken.hh"
#include "TokenVariable.hh"
#include "Timeline.hh"

#include "RuleInstance.hh"
#include "RulesEngine.hh"
#include "Rule.hh"
#include "NddlRules.hh"

#include "DNPConstraints.hh"
#include "WeakDomainComparator.hh"

#include "test/ConstraintTesting.hh"

#include <iostream>
#include <string>

#define DEFAULT_SETUP(ce, db, autoClose) \
    ConstraintEngine ce; \
    initCBPTestSchema(); \
    PlanDatabase db(ce.getId(), Schema::instance()); \
    new DefaultPropagator(LabelStr("Default"), ce.getId()); \
    new DefaultPropagator(LabelStr("Temporal"), ce.getId()); \
    RulesEngine re(db.getId()); \
    Horizon hor(0,200); \
    DecisionManager dm(db.getId()); \
    if (autoClose) \
      db.close();

#define DEFAULT_TEARDOWN()

#define DEFAULT_SETUP_PLAN(ce, db, autoClose)			\
  ConstraintEngine ce;						\
  initCBPTestSchema();						\
  PlanDatabase db(ce.getId(), Schema::instance());		\
  new DefaultPropagator(LabelStr("Default"), ce.getId());	\
  new DefaultPropagator(LabelStr("Temporal"), ce.getId());	\
  RulesEngine re(db.getId());					\
  Horizon hor(0, 200);						\
  CBPlanner planner(db.getId(), hor.getId());			\
  if (autoClose)						\
    db.close();

#define DEFAULT_TEARDOWN_PLAN()

#define DEFAULT_SETUP_HEURISTICS()			\
  ConstraintEngine ce;					\
  initCBPTestSchema();					\
  PlanDatabase db(ce.getId(), Schema::instance());	\
  HSTSHeuristics heuristics(db.getId()); 

#define DEFAULT_TEARDOWN_HEURISTICS()

#define DEFAULT_SETUP_PLAN_HEURISTICS()				\
  ConstraintEngine ce;						\
  initCBPTestSchema();						\
  PlanDatabase db(ce.getId(), Schema::instance());		\
  new DefaultPropagator(LabelStr("Default"), ce.getId());	\
  new DefaultPropagator(LabelStr("Temporal"), ce.getId());	\
  RulesEngine re(db.getId());					\
  Horizon hor(0, 200);						\
  CBPlanner planner(db.getId(), hor.getId());			\
  HSTSHeuristics heuristics(db.getId()); 

#define DEFAULT_TEARDOWN_PLAN_HEURISTICS()

class ConstraintTest {
public:
  static bool test() {
    runTest(testDNPConstraints);
    //these are needed for the next test
    new EnumeratedTypeFactory("Color", "Color", ColorBaseDomain());
    new EnumeratedTypeFactory("Fruit", "Fruit", FruitBaseDomain());
    runTest(testEqualConstraint);
    return(true);
  }

private:
  /**
   * Test the DNP specific constraint functions.
   * @note Almost a copy of ConstraintEngine/test/module-tests.cc's testArbitraryCosntraints().
   */
  static bool testDNPConstraints() {
    DEFAULT_SETUP_PLAN_HEURISTICS();
    std::list<ConstraintTestCase> tests;
    assertTrue(readTestCases(std::string("DNPTestCases"), tests) ||
               readTestCases(std::string("HSTS/test/DNPTestCases"), tests));
    assertTrue(executeTestCases(ce.getId(), tests));
    DEFAULT_TEARDOWN_PLAN_HEURISTICS();
    return(true);
  }

  /**
   * Color and Fruit enumeration's base domains1, as required by class TypeFactory.
   * @note Copied from System/test/basic-types.cc
   * as created from basic-types.nddl with the NDDL compiler.
   */
  typedef SymbolDomain Fruit;

  static const Fruit& FruitBaseDomain(){
    static Fruit sl_enum("Fruit");
    if (sl_enum.isOpen()) {
      // Insert values to initialize
      sl_enum.insert(LabelStr("apple"));
      sl_enum.insert(LabelStr("orange"));
      sl_enum.insert(LabelStr("grape"));
      sl_enum.insert(LabelStr("banana"));
      sl_enum.close();
    }
    return(sl_enum);
  }

  typedef SymbolDomain Color;

  static const Color& ColorBaseDomain(){
    static Color sl_enum("Color");
    if (sl_enum.isOpen()) {
      // Insert values to initialize
      sl_enum.insert(LabelStr("red"));
      sl_enum.insert(LabelStr("orange"));
      sl_enum.insert(LabelStr("green"));
      sl_enum.insert(LabelStr("blue"));
      sl_enum.insert(LabelStr("purple"));
      sl_enum.close();
    }
    return(sl_enum);
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
    EqualConstraint c0(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, 
                       makeScope(v0.getId(), v1.getId()));
    ENGINE->propagate();
    assert(ENGINE->constraintConsistent());
    assert(v0.getDerivedDomain().getSingletonValue() == 1);
    assert(v1.getDerivedDomain().getSingletonValue() == 1);

    LabelSet ls0(baseDomain);
    ls0.empty();
    ls0.insert(EUROPA::LabelStr("A"));

    LabelSet ls1(baseDomain);
    ls1.empty();
    ls1.insert(EUROPA::LabelStr("A"));
    ls1.insert(EUROPA::LabelStr("B"));
    ls1.insert(EUROPA::LabelStr("C"));
    ls1.insert(EUROPA::LabelStr("D"));
    ls1.insert(EUROPA::LabelStr("E"));

    Variable<LabelSet> v2(ENGINE, ls1);
    Variable<LabelSet> v3(ENGINE, ls1);
    EqualConstraint c1(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, 
                       makeScope(v2.getId(), v3.getId()));
    ENGINE->propagate();
    assert(ENGINE->constraintConsistent());
    assert(v2.getDerivedDomain() == v3.getDerivedDomain());
    assert(!v2.getDerivedDomain().isSingleton());

    LabelSet ls2(ls1);
    ls2.remove(EUROPA::LabelStr("E"));

    v2.specify(ls2);
    ENGINE->propagate();
    assert(!v3.getDerivedDomain().isMember(EUROPA::LabelStr("E")));

    Variable<LabelSet> v4(ENGINE, ls0);
    EqualConstraint c2(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, 
                       makeScope(v2.getId(), v4.getId()));
    ENGINE->propagate();
    assert(ENGINE->constraintConsistent());
    assert(v2.getDerivedDomain() == v3.getDerivedDomain());
    assert(v2.getDerivedDomain() == v4.getDerivedDomain());
    assert(v3.getDerivedDomain() == v4.getDerivedDomain());
    assert(v3.getDerivedDomain().getSingletonValue() == EUROPA::LabelStr("A"));

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
      // Leave domains dynamic

      Variable<NumericDomain> a(ENGINE, e0);
      Variable<NumericDomain> b(ENGINE, e1);
      Variable<NumericDomain> c(ENGINE, e2);
      EqualConstraint eq(LabelStr("EqualConstraint"),
                         LabelStr("Default"),
                         ENGINE,
                         makeScope(a.getId(), b.getId(), c.getId()));
      assert(ENGINE->propagate());

      // Now close one only. Should not change anything else.
      b.close();
      assert(b.lastDomain().getSize() == 4);
      assert(ENGINE->propagate());

      // Close another, should see partial restriction
      a.close();
      assert(a.lastDomain().getSize() == 3);
      assert(ENGINE->propagate());
      assert(a.lastDomain().getSize() == 3);
      assert(b.lastDomain().getSize() == 3);

      // By closing the final variables domain
      c.close();
      assert(!ENGINE->propagate());
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
      assert(ENGINE->propagate());
      assert(a.lastDomain() == IntervalIntDomain(30, 70));
    }


    // Create a test that requires the comparison of members of two different enum types
    {
      const EnumeratedDomain & color0 = dynamic_cast<const EnumeratedDomain &>(
                                         TypeFactory::baseDomain("Color"));
      const EnumeratedDomain & fruit0 = dynamic_cast<const EnumeratedDomain &>(
                                         TypeFactory::baseDomain("Fruit"));
      assertTrue(color0.isMember(LabelStr("red")));
      assertTrue(fruit0.isMember(LabelStr("apple")));

      Variable<EnumeratedDomain> x(ENGINE, ColorBaseDomain());
      Variable<EnumeratedDomain> y(ENGINE, FruitBaseDomain());

      std::vector<ConstrainedVariableId> scope;
      scope.push_back(x.getId());
      scope.push_back(y.getId());
      EqualConstraint eq(LabelStr("EqualConstraint"),
                         LabelStr("Default"),
                         ENGINE,
                         scope);
      assert(ENGINE->propagate());
    }

    return true;
  }

};

class ConditionTest {
public:
  static bool test() {
    runTest(testHSTSNoBranchCondition);
    return(true);
  }
private:
  static bool testHSTSNoBranchCondition() {
    bool retval = false;
    DEFAULT_SETUP(ce,db,false);
    retval = testHSTSNoBranchConditionImpl(ce,db,dm);
    DEFAULT_TEARDOWN();
    return retval;
  }

};

class HeuristicsTest {
public:
  static bool test() {
    runTest(testTokenType);
    runTest(testDefaultInitialization);
    runTest(testTokenInitialization);
    runTest(testVariableInitialization);
    runTest(testReader);
    runTest(testHSTSPlanIdReader);
    runTest(testHSTSNoBranch);
    runTest(testHSTSHeuristicsAssembly);
    runTest(testDNPConstraints);
    return(true);
  }
private:
  static bool testTokenType() {
    bool retval = false;
    DEFAULT_SETUP_HEURISTICS();
    retval = testTokenTypeImpl(heuristics);
    DEFAULT_TEARDOWN_HEURISTICS();
    return retval;
  }
  static bool testDefaultInitialization() {
    bool retval = false;
    DEFAULT_SETUP_HEURISTICS();
    retval = testDefaultInitializationImpl(heuristics);
    DEFAULT_TEARDOWN_HEURISTICS();
    return retval;
  }
  static bool testTokenInitialization() {
    bool retval = false;
    DEFAULT_SETUP_HEURISTICS();
    retval = testTokenInitializationImpl(heuristics);
    DEFAULT_TEARDOWN_HEURISTICS();
    return retval;
  }
  static bool testVariableInitialization() {
    bool retval = false;
    DEFAULT_SETUP_HEURISTICS();
    retval = testVariableInitializationImpl(heuristics);
    DEFAULT_TEARDOWN_HEURISTICS();
    return retval;
  }
  static bool testReader() {
    bool retval = false;
    DEFAULT_SETUP_HEURISTICS();
    retval = testReaderImpl(heuristics);
    DEFAULT_TEARDOWN_HEURISTICS();
    return retval;
  }
  static bool testHSTSPlanIdReader() {
    bool retval = false;
    DEFAULT_SETUP_HEURISTICS();
    retval = testHSTSPlanIdReaderImpl();
    DEFAULT_TEARDOWN_HEURISTICS();
    return retval;
  }
  static bool testHSTSNoBranch() {
    bool retval = false;
    DEFAULT_SETUP_PLAN(ce,db,true);
    retval = testHSTSNoBranchImpl(ce, db, planner);
    DEFAULT_TEARDOWN_PLAN();
    return retval;
  }
  static bool testHSTSHeuristicsAssembly() {
    bool retval = false;
    DEFAULT_SETUP_PLAN_HEURISTICS();
    retval = testHSTSHeuristicsAssemblyImpl(ce, db, planner, heuristics);
    DEFAULT_TEARDOWN_PLAN_HEURISTICS();
    return retval;
  }

  /**
   * Test the DNP specific constraint functions.
   * @note Almost a copy of ConstraintEngine/test/module-tests.cc's testArbitraryCosntraints().
   */
  static bool testDNPConstraints() {
    DEFAULT_SETUP_PLAN_HEURISTICS();
    std::list<ConstraintTestCase> tests;
    assertTrue(readTestCases(std::string("DNPTestCases"), tests) ||
               readTestCases(std::string("HSTS/test/DNPTestCases"), tests));
    assertTrue(executeTestCases(ce.getId(), tests));
    DEFAULT_TEARDOWN_PLAN_HEURISTICS();
    return(true);
  }
};

int main() {
  //Use relaxed domain comparator that allows comparison of members of two different enum types
  WeakDomainComparator* wdc = new WeakDomainComparator();
  DomainComparator::initialize((DomainComparator*)wdc);
  warn("DomainComparator relaxed with WeakDomainComparator");

  Schema::instance();


  //!!initConstraintEngine(); // Needed ?
  //!!initConstraintLibrary(); // Needed ?  May interfere with some of the later register constraint calls.

  REGISTER_CONSTRAINT(EqualConstraint, "eq", "Default");
  REGISTER_CONSTRAINT(EqualConstraint, "Equal", "Default");
  REGISTER_CONSTRAINT(LessThanConstraint, "lt", "Default");
  REGISTER_CONSTRAINT(SubsetOfConstraint, "SubsetOf", "Default");

  // Register the DNP constraint functions.
  REGISTER_CONSTRAINT(BOUNDS_PLAYBACK_START_STORAGE, "BOUNDS_PLAYBACK_START_STORAGE", "Default");
  REGISTER_CONSTRAINT(BOUNDS_PLAYBACK_END_STORAGE, "BOUNDS_PLAYBACK_END_STORAGE", "Default");
  REGISTER_CONSTRAINT(BOUNDS_RECORD_END_STORAGE, "BOUNDS_RECORD_END_STORAGE", "Default");
  REGISTER_CONSTRAINT(BOUNDS_RECORD_START_STORAGE, "BOUNDS_RECORD_START_STORAGE", "Default");
  REGISTER_CONSTRAINT(COMPUTE_PLAYBACK_DURATION, "COMPUTE_PLAYBACK_DURATION", "Default");
  REGISTER_CONSTRAINT(FIGURE_EARLIER_OP_IDS, "FIGURE_EARLIER_OP_IDS", "Default");

  //!!Add calls to readTestCases(), etc., from ConstraintEngine/test/module-tests.cc

  for (int i = 0; i < 1; i++) {
    runTestSuite(ConstraintTest::test);
    runTestSuite(ConditionTest::test);
    runTestSuite(HeuristicsTest::test);
  }
  std::cout << "Finished" << std::endl;
  ConstraintLibrary::purgeAll();
  exit(0);
}
