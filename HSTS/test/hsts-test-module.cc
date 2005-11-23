#include "hsts-test-module.hh"

/* HSTS Files */
#include "HSTSNoBranch.hh"
#include "HSTSNoBranchCondition.hh"
#include "HSTSPlanIdReader.hh"
#include "AtSubgoalRule.hh"

/* CBPlanner files */
#include "CBPlannerDefs.hh"
#include "Horizon.hh"
#include "DecisionManager.hh"
#include "Utils.hh"
#include "ConstrainedVariableDecisionPoint.hh"
#include "ObjectDecisionPoint.hh"
#include "TokenDecisionPoint.hh"
#include "OpenDecisionManager.hh"

/* Constraint Engine files */
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

/* PlanDatababse */
#include "PlanDatabase.hh"
#include "Object.hh"
#include "TokenVariable.hh"
#include "Timeline.hh"
#include "IntervalToken.hh"

/* Rules Engine*/
#include "RulesEngine.hh"

/* Miscellaneous */
#include "TestSupport.hh"
#include "DNPConstraints.hh"
#include "WeakDomainComparator.hh"
#include "Schema.hh"
#include "XMLUtils.hh"

#include "HSTSConstraintTesting.hh"

#include <iostream>
#include <string>
#include <fstream>

#define DEFAULT_SETUP(ce, db, autoClose) \
    ConstraintEngine ce; \
    hstsInitCBPTestSchema(); \
    PlanDatabase db(ce.getId(), Schema::instance()); \
    new DefaultPropagator(LabelStr("Default"), ce.getId()); \
    new DefaultPropagator(LabelStr("Temporal"), ce.getId()); \
    RulesEngine re(db.getId()); \
    Horizon hor(0,200); \
    OpenDecisionManager odm(db.getId()); \
    DecisionManager dm(db.getId(), odm.getId()); \
    if (autoClose) \
      db.close();

#define DEFAULT_TEARDOWN()	

#define TEARDOWN()

  /**
   * @brief Creates the type specifications required for testing
   */
  void hstsInitCBPTestSchema(){
    const SchemaId& schema = Schema::instance();
    schema->reset();
    schema->addObjectType("Objects");

    schema->addPredicate("Objects.PredicateA");
    schema->addMember("Objects.PredicateA", IntervalIntDomain().getTypeName(), "IntervalIntParam");

    schema->addPredicate("Objects.PredicateB");
    schema->addPredicate("Objects.PredicateC");
    schema->addPredicate("Objects.PredicateD");
    schema->addPredicate("Objects.PADDED");

    schema->addPredicate("Objects.PredicateE");
    schema->addMember("Objects.PredicateE", IntervalIntDomain().getTypeName(), "param0");
    schema->addMember("Objects.PredicateE", IntervalDomain().getTypeName(), "param1");
    schema->addMember("Objects.PredicateE", LabelSet().getTypeName(), "param2");

    schema->addPredicate("Objects.P1");
    schema->addMember("Objects.P1", LabelSet().getTypeName(), "LabelSetParam0");
    schema->addMember("Objects.P1", LabelSet().getTypeName(), "LabelSetParam1");
    schema->addMember("Objects.P1", IntervalIntDomain().getTypeName(), "IntervalIntParam");

    schema->addPredicate("Objects.P1True");
    schema->addMember("Objects.P1True", BoolDomain().getTypeName(), "BoolParam");
    schema->addPredicate("Objects.P1False");
  }

  void hstsInitHeuristicsSchema(){
    const SchemaId& rover = Schema::instance();
    rover->reset();
    rover->addObjectType(LabelStr("Object"));
    rover->addObjectType(LabelStr("Timeline"), LabelStr("Object"));
    rover->addObjectType(LabelStr("NddlResource"));
    rover->addObjectType("Resource", "NddlResource");
    rover->addMember("Resource", "float", "initialCapacity");
    rover->addMember("Resource", "float", "levelLimitMin");
    rover->addMember("Resource", "float", "levelLimitMax");
    rover->addMember("Resource", "float", "productionRateMax");
    rover->addMember("Resource", "float", "productionMax");
    rover->addMember("Resource", "float", "consumptionRateMax");
    rover->addMember("Resource", "float", "consumptionMax");
    rover->addPredicate("Resource.change");
    rover->addMember("Resource.change", "float", "quantity");
    rover->addObjectType("UnaryResource", "Timeline");
    rover->addPredicate("UnaryResource.uses");
    rover->addObjectType("Battery", "Resource");
    rover->addObjectType("Location", "Object");
    // rover->isObjectType("Location");
    rover->addMember("Location", "string", "name");
    rover->addMember("Location", "int", "x");
    rover->addMember("Location", "int", "y");
    rover->addObjectType("Path", "Object");
    rover->addMember("Path", "string", "name");
    rover->addMember("Path", "Location", "from");
    rover->addMember("Path", "Location", "to");
    rover->addMember("Path", "float", "cost");
    rover->addObjectType("Navigator", "Timeline");
    rover->addPredicate("Navigator.At");
    rover->addMember("Navigator.At", "Location", "location");
    rover->addPredicate("Navigator.Going");
    rover->addMember("Navigator.Going", "Location", "from");
    rover->addMember("Navigator.Going", "Location", "to");
    rover->addObjectType("Commands", "Timeline");
    rover->addPredicate("Commands.TakeSample");
    rover->addMember("Commands.TakeSample", "Location", "rock");
    rover->addPredicate("Commands.PhoneHome");
    rover->addPredicate("Commands.PhoneLander");
    rover->addObjectType("Instrument", "Timeline");
    rover->addPredicate("Instrument.TakeSample");
    rover->addMember("Instrument.TakeSample", "Location", "rock");
    rover->addPredicate("Instrument.Place");
    rover->addMember("Instrument.Place", "Location", "rock");
    rover->addPredicate("Instrument.Stow");
    rover->addPredicate("Instrument.Unstow");
    rover->addPredicate("Instrument.Stowed");
    rover->addObjectType("Rover", "Object");
    rover->addMember("Rover", "Commands", "commands");
    rover->addMember("Rover", "Navigator", "navigator");
    rover->addMember("Rover", "Instrument", "instrument");
    rover->addMember("Rover", "Battery", "mainBattery");
    rover->addObjectType("PlannerConfig", "Timeline");
    rover->addMember("PlannerConfig", "int", "m_horizonStart");
    rover->addMember("PlannerConfig", "int", "m_horizonEnd");
    rover->addMember("PlannerConfig", "int", "m_maxPlannerSteps");
    rover->addPredicate("PlannerConfig.initialState");
    rover->addEnum("TokenStates");
    rover->addValue("TokenStates", LabelStr("INACTIVE"));
    rover->addValue("TokenStates", LabelStr("ACTIVE"));
    rover->addValue("TokenStates", LabelStr("MERGED"));
    rover->addValue("TokenStates", LabelStr("REJECTED"));
    rover->addMember("Navigator.Going", "Path", "p");
    rover->addMember("Commands.TakeSample", "Rover", "rovers");
    rover->addMember("Commands.TakeSample", "bool", "OR");
    rover->addMember("Instrument.TakeSample", "Rover", "rovers");
    rover->addMember("Instrument.Place", "Rover", "rovers");
    rover->addMember("Instrument.Unstow", "Rover", "rovers");
    rover->addMember("Instrument.Stow", "Rover", "rovers");

    // extra stuff to test
    rover->addObjectType("Telemetry", "Object");
    rover->addPredicate("Telemetry.Communicate");
    rover->addMember("Telemetry.Communicate", "int", "minutes");
    rover->addMember("Telemetry.Communicate", "float", "bandwidth");
    rover->addMember("Telemetry.Communicate", "bool", "encoded");
    rover->addMember("Telemetry.Communicate", "Mode", "mode");
    rover->addEnum("Mode");
    rover->addValue("Mode", LabelStr("high"));
    rover->addValue("Mode", LabelStr("medium-high"));
    rover->addValue("Mode", LabelStr("medium"));
    rover->addValue("Mode", LabelStr("medium-low"));
    rover->addValue("Mode", LabelStr("low"));
  }


class HSTSConstraintTest {
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
    DEFAULT_SETUP(ce,db,false);
    std::list<HSTSConstraintTestCase> tests;
    assertTrue(HSTSReadTestCases(getTestLoadLibraryPath() + std::string("/DNPTestCases"), tests) ||
               HSTSReadTestCases(std::string("HSTS/test/DNPTestCases"), tests));
    assertTrue(HSTSExecuteTestCases(ce.getId(), tests));
    DEFAULT_TEARDOWN();
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
    EqualConstraint c1(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, 
                       makeScope(v2.getId(), v3.getId()));
    ENGINE->propagate();
    assertTrue(ENGINE->constraintConsistent());
    assertTrue(v2.getDerivedDomain() == v3.getDerivedDomain());
    assertTrue(!v2.getDerivedDomain().isSingleton());

    LabelSet ls2(ls1);
    ls2.remove(EUROPA::LabelStr("E"));

    v2.restrictBaseDomain(ls2);
    ENGINE->propagate();
    assertTrue(!v3.getDerivedDomain().isMember(EUROPA::LabelStr("E")));

    Variable<LabelSet> v4(ENGINE, ls0);
    EqualConstraint c2(LabelStr("EqualConstraint"), LabelStr("Default"), ENGINE, 
                       makeScope(v2.getId(), v4.getId()));
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
      // Leave domains dynamic

      Variable<NumericDomain> a(ENGINE, e0);
      Variable<NumericDomain> b(ENGINE, e1);
      Variable<NumericDomain> c(ENGINE, e2);
      EqualConstraint eq(LabelStr("EqualConstraint"),
                         LabelStr("Default"),
                         ENGINE,
                         makeScope(a.getId(), b.getId(), c.getId()));
      assertTrue(ENGINE->propagate());

      // Now close one only. Should not change anything else.
      b.close();
      assertTrue(b.lastDomain().getSize() == 4);
      assertTrue(!ENGINE->propagate());

     //  // Close another, should see partial restriction
//       a.close();
//       assertTrue(a.lastDomain().getSize() == 3);
//       assertTrue(ENGINE->propagate());
//       assertTrue(a.lastDomain().getSize() == 3);
//       assertTrue(b.lastDomain().getSize() == 3);

//       // By closing the final variables domain
//       c.close();
//       assertTrue(!ENGINE->propagate());
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
      assertTrue(ENGINE->propagate());
    }

    return true;
  }

};

class HSTSConditionTest {
public:
  static bool test() {
    runTest(testHSTSNoBranchCondition);
    return(true);
  }
private:
  static bool testHSTSNoBranchCondition() {
    DEFAULT_SETUP(ce,db,false);
    HSTSNoBranchCondition cond(dm.getId());
    assert(dm.getConditions().size() == 1);

    HSTSNoBranchId noBranchSpec(new HSTSNoBranch());

    const LabelStr var1Name("AnObj.APred.Var1");

    noBranchSpec->addNoBranch(var1Name);

    Variable<IntervalIntDomain> var1(ce.getId(), IntervalIntDomain(), true, var1Name);
    Variable<IntervalIntDomain> var2(ce.getId(), IntervalIntDomain(), true, LabelStr("AnObj.APred.Var2"));

    //std::cout << " var1 name = " << var1.getName().c_str() << std::endl;
    //    std::cout << " var2 name = " << var2.getName().c_str() << std::endl;

    cond.initialize(noBranchSpec);

    assert(!cond.test(var1.getId()));
    assert(cond.test(var2.getId()));

    noBranchSpec.remove();

    DEFAULT_TEARDOWN();
    return true;
  }

};


bool testWeakDomainComparator() {
  
  DEFAULT_SETUP(ce,db,false);
  hstsInitHeuristicsSchema();
  DomainComparator();
  IntervalDomain i1;
  IntervalIntDomain i2;

  assertTrue(DomainComparator::getComparator().canCompare(i1, i2));

  Object loc1(db.getId(),LabelStr("Location"),LabelStr("Loc1"));
  Object loc3(db.getId(),LabelStr("Location"),LabelStr("Loc3"));
    
  std::list<ObjectId> results1;
  db.getObjectsByType("Location",results1);
  ObjectDomain allLocs(results1,"Location");


  Object nav(db.getId(), LabelStr("Navigator"), LabelStr("nav"));

  std::list<ObjectId> results2;
  db.getObjectsByType("Navigator",results2);
  ObjectDomain allNavs(results2,"Navigator");

  assertTrue(!DomainComparator::getComparator().canCompare(i1, allLocs));
  assertTrue(!DomainComparator::getComparator().canCompare(allLocs, allNavs));

  WeakDomainComparator wdc;


  assertTrue(DomainComparator::getComparator().canCompare(i1, i2));
  assertTrue(!DomainComparator::getComparator().canCompare(i1, allLocs));
  assertTrue(DomainComparator::getComparator().canCompare(allLocs, allNavs));
  DEFAULT_TEARDOWN();
  return true;
}

void HSTSModuleTests::runTests(std::string path) {
  initConstraintEngine();
  setTestLoadLibraryPath(path);
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
    runTest(testWeakDomainComparator);
    //Use relaxed domain comparator that allows comparison of members of two different enum types
    WeakDomainComparator wdc;
    runTestSuite(HSTSConstraintTest::test);
    runTestSuite(HSTSConditionTest::test);
  }
  std::cout << "Finished" << std::endl;
  ConstraintLibrary::purgeAll();
  uninitConstraintLibrary();

  const SchemaId& schema = Schema::instance();  
  // set back to regular domain comparator (from weak domain comparator).
  DomainComparator* p = (DomainComparator*) schema;
  DomainComparator::setComparator( p );  

}
  
