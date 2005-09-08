/* HSTS Files */
#include "HSTSNoBranch.hh"
#include "HSTSNoBranchCondition.hh"
#include "HSTSPlanIdReader.hh"
#include "HSTSHeuristicsReader.hh"
#include "HSTSOpenDecisionManager.hh"
#include "AtSubgoalRule.hh"

/* CBPlanner files */
#include "CBPlannerDefs.hh"
#include "Horizon.hh"
#include "DecisionManager.hh"
#include "Utils.hh"
#include "ConstrainedVariableDecisionPoint.hh"
#include "ObjectDecisionPoint.hh"
#include "TokenDecisionPoint.hh"

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
#include "Schema.hh"
#include "Object.hh"
#include "TokenVariable.hh"
#include "Timeline.hh"
#include "IntervalToken.hh"

/* Rules Engine Fules */
#include "RulesEngine.hh"

/* Miscellaneous */
#include "TestSupport.hh"
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
    OpenDecisionManager odm(db.getId()); \
    DecisionManager dm(db.getId(), odm.getId()); \
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
  new DefaultPropagator(LabelStr("Default"), ce.getId()); \
  new DefaultPropagator(LabelStr("Temporal"), ce.getId()); \
  HSTSHeuristics heuristics(db.getId()); \
  initHeuristicsSchema(); \
  HSTSHeuristicsReader hreader(heuristics.getNonConstId()); \
  hreader.read("../core/Heuristics-HSTS.xml");

#define DEFAULT_TEARDOWN_HEURISTICS()

#define DEFAULT_SETUP_PLAN_HEURISTICS()				\
  ConstraintEngine ce;						\
  initCBPTestSchema();						\
  PlanDatabase db(ce.getId(), Schema::instance());		\
  new DefaultPropagator(LabelStr("Default"), ce.getId());	\
  new DefaultPropagator(LabelStr("Temporal"), ce.getId());	\
  RulesEngine re(db.getId());					\
  HSTSHeuristics heuristics(db.getId()); \
  initHeuristicsSchema(); \
  HSTSHeuristicsReader hreader(heuristics.getNonConstId()); \
  hreader.read("../core/Heuristics-HSTS.xml"); \
  HSTSOpenDecisionManager odm(db.getId(), heuristics.getId());  \
  Horizon hor(0, 200);						\
  CBPlanner planner(db.getId(), hor.getId(), odm.getId());	

#define DEFAULT_TEARDOWN_PLAN_HEURISTICS()


  /**
   * @brief Creates the type specifications required for testing
   */
  void initCBPTestSchema(){
    const SchemaId& schema = Schema::instance();
    schema->reset();
    schema->addObjectType("Objects");

    schema->addPredicate("Objects.PredicateA");
    schema->addMember("Objects.PredicateA", IntervalIntDomain().getTypeName(), "IntervalIntParam");

    schema->addPredicate("Objects.PredicateB");
    schema->addPredicate("Objects.PredicateC");
    schema->addPredicate("Objects.PredicateD");
    schema->addPredicate("Objects.PADDED");

    schema->addPredicate("Objects.P1");
    schema->addMember("Objects.P1", LabelSet().getTypeName(), "LabelSetParam0");
    schema->addMember("Objects.P1", LabelSet().getTypeName(), "LabelSetParam1");
    schema->addMember("Objects.P1", IntervalIntDomain().getTypeName(), "IntervalIntParam");

    schema->addPredicate("Objects.P1True");
    schema->addMember("Objects.P1True", BoolDomain().getTypeName(), "BoolParam");
    schema->addPredicate("Objects.P1False");
  }

  void initHeuristicsSchema(){
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

    v2.specify(ls2);
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

class ConditionTest {
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

class HeuristicsTest {
public:
  static bool test() {
    runTest(testDefaultInitialization);
    runTest(testTokenInitialization);
    runTest(testVariableInitialization);
    runTest(testReader);
    runTest(testHSTSPlanIdReader);
    runTest(testHSTSNoBranch);
    runTest(testHSTSHeuristicsAssembly);
    runTest(testDNPConstraints);
    runTest(testPreferredPriority);
    runTest(testHSTSHeuristicsStrict);
    runTest(testPriorities);
    return(true);
  }
private:

  static bool testDefaultInitialization() {
    
    DEFAULT_SETUP_HEURISTICS();
    heuristics.setDefaultPriorityPreference(HSTSHeuristics::HIGH);

    std::vector<std::pair<LabelStr,LabelStr> > domainSpecs;
    heuristics.setDefaultPriorityForTokenDPsWithParent(20.3, LabelStr("SEP_Thrust_Timer_SV.Max_Thrust_Time"), domainSpecs);

    heuristics.setDefaultPriorityForTokenDPs(10000.0);
    
    heuristics.setDefaultPriorityForConstrainedVariableDPs(10000.0);

    std::vector<LabelStr> states;
    std::vector<HSTSHeuristics::CandidateOrder> orders;
    states.push_back(Token::REJECTED);
    states.push_back(Token::MERGED);
    //    states.push_back(Token::DEFER);
    states.push_back(Token::ACTIVE);
    orders.push_back(HSTSHeuristics::NONE);
    orders.push_back(HSTSHeuristics::EARLY);
    //    orders.push_back(HSTSHeuristics::NONE);
    orders.push_back(HSTSHeuristics::EARLY);
    heuristics.setDefaultPreferenceForTokenDPs(states,orders);

    heuristics.setDefaultPreferenceForConstrainedVariableDPs(HSTSHeuristics::ASCENDING);

    DEFAULT_TEARDOWN_HEURISTICS();
    return true;
  }
  static bool testTokenInitialization() {
    
    DEFAULT_SETUP_HEURISTICS();
    std::vector<std::pair<LabelStr,LabelStr> > dsb;
    dsb.push_back(std::make_pair<LabelStr,LabelStr>(LabelStr("m_trans"),LabelStr("NO_RIGHT")));
    dsb.push_back(std::make_pair<LabelStr,LabelStr>(LabelStr("m_with"),LabelStr("CON")));
    dsb.push_back(std::make_pair<LabelStr,LabelStr>(LabelStr("m_sys"),LabelStr("DES")));
    std::vector<LabelStr> statesb;
    statesb.push_back(Token::REJECTED);
    statesb.push_back(Token::MERGED);
    //    statesb.push_back(LabelStr::DEFER);
    statesb.push_back(Token::ACTIVE);
    std::vector<HSTSHeuristics::CandidateOrder> ordersb;
    ordersb.push_back(HSTSHeuristics::NONE);
    ordersb.push_back(HSTSHeuristics::NEAR);
    //    ordersb.push_back(HSTSHeuristics::NONE);
    ordersb.push_back(HSTSHeuristics::MIN_FLEXIBLE);
    std::vector<std::pair<LabelStr,LabelStr> > domainSpecs;
    heuristics.setHeuristicsForToken(334.5, LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd"), dsb, HSTSHeuristics::ANY, NO_STRING,domainSpecs, statesb, ordersb);

    std::vector<std::pair<LabelStr,LabelStr> > dsc;
    dsc.push_back(std::make_pair<LabelStr,LabelStr>(LabelStr("m_trans"),LabelStr("NO_RIGHT")));
    dsc.push_back(std::make_pair<LabelStr,LabelStr>(LabelStr("m_with"),LabelStr("CON")));
    dsc.push_back(std::make_pair<LabelStr,LabelStr>(LabelStr("m_sys"),LabelStr("DES")));
    std::vector<LabelStr> statesc;
    std::vector<HSTSHeuristics::CandidateOrder> ordersc;
    statesc.push_back(Token::MERGED);
    statesc.push_back(Token::ACTIVE);
    ordersc.push_back(HSTSHeuristics::TGENERATOR);
    ordersc.push_back(HSTSHeuristics::LATE);
    heuristics.setHeuristicsForToken(6213.7, LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd"), dsc, HSTSHeuristics::AFTER, LabelStr("SEP_Thrust_Timer_SV.Max_Thrust_Time"), domainSpecs, statesc, ordersc);

    std::vector<std::pair<LabelStr,LabelStr> > dsd;
    dsd.push_back(std::make_pair<LabelStr,LabelStr>(LabelStr("m_trans"),LabelStr("NO_RIGHT")));
    dsd.push_back(std::make_pair<LabelStr,LabelStr>(LabelStr("m_with"),LabelStr("SIN")));
    dsd.push_back(std::make_pair<LabelStr,LabelStr>(LabelStr("m_sys"),LabelStr("RES")));
    std::vector<LabelStr> statesd;
    std::vector<HSTSHeuristics::CandidateOrder> ordersd;
    statesd.push_back(Token::ACTIVE);
    ordersd.push_back(HSTSHeuristics::EARLY);
    heuristics.setHeuristicsForToken(6213.7, LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd"), dsd, HSTSHeuristics::ANY, NO_STRING, domainSpecs, statesd, ordersd);

    /* this rightly produces a duplicate entry error in the code 
    std::vector<LabelStr> statese;
    std::vector<HSTSHeuristics::CandidateOrder> orderse;
    heuristics.setHeuristicsForToken(7652.4, ttd.getId(), HSTSHeuristics::ANY, TokenTypeId::noId(), statese, orderse);
    */

    DEFAULT_TEARDOWN_HEURISTICS();
    return true;
  }
  static bool testVariableInitialization() {
    
    DEFAULT_SETUP_HEURISTICS();
    /*  doesn't work because the schema doesn't agree with the data used
	here 
    std::vector<std::pair<LabelStr,LabelStr> > domainSpec;
    TokenType tt(LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd"),domainSpec);
    std::vector<LabelStr> aenums;
    heuristics.setHeuristicsForConstrainedVariable(443.6, LabelStr("with"), tt.getId(), HSTSHeuristics::ASCENDING, NO_STRING, aenums);

    aenums.push_back(LabelStr("SIN"));
    aenums.push_back(LabelStr("CON"));
    aenums.push_back(LabelStr("SAL"));
    aenums.push_back(LabelStr("PEP"));
    heuristics.setHeuristicsForConstrainedVariable(443.6, LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd.with"), TokenTypeId::noId(), HSTSHeuristics::ENUMERATION, NO_STRING, aenums);

    std::vector<LabelStr> emptyList;
    heuristics.setHeuristicsForConstrainedVariable(2269.3, LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Left.m_with"), TokenTypeId::noId(), HSTSHeuristics::VGENERATOR, LabelStr("Generator1"),emptyList);

    std::vector<LabelStr> benums;
    heuristics.setHeuristicsForConstrainedVariable(234.5, LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd.m_sys"), TokenTypeId::noId(), HSTSHeuristics::ENUMERATION, NO_STRING, benums);

    std::vector<LabelStr> cenums;
    cenums.push_back(LabelStr("CON"));
    //    LabelStr parentName = stripVariableName("Made_Up_Parent_SV.Made_Up_SV.Thrust_Left.m_with");
    LabelStr parentName = LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Left");
    std::vector<std::pair<LabelStr,LabelStr> > cds;
    cds.push_back(std::make_pair<LabelStr,LabelStr>(LabelStr("m_sys"),LabelStr("ON")));
    TokenType ctt(parentName, cds);
    heuristics.setHeuristicsForConstrainedVariable(6234.7, LabelStr("Made_Up_Parent_SV.Made_Up_SV.Thrust_Fwd.m_with"), ctt.getId(), HSTSHeuristics::ENUMERATION, NO_STRING, benums);
    */
    DEFAULT_TEARDOWN_HEURISTICS();
    return true;
  }
  static bool testReader() {
    
    DEFAULT_SETUP_HEURISTICS();
    assertTrue(heuristics.getDefaultPriorityPreference() == HSTSHeuristics::HIGH);
    assertTrue(heuristics.getDefaultPriorityForTokenDPsWithParent(LabelStr("Navigator.At")) == 1024.5);
    assertTrue(heuristics.getDefaultPriorityForTokenDPs() == 10.0);
    assertTrue(heuristics.getDefaultPriorityForConstrainedVariableDPs() == 5000.0);
    assertTrue(heuristics.getDefaultPreferenceForConstrainedVariableDPs() == HSTSHeuristics::DESCENDING);
    //everything else requires decision points and is tested in testPriorityImpl and testOrderingImpl

    DEFAULT_TEARDOWN_HEURISTICS();
    return true;
  }
  static bool testHSTSPlanIdReader() {
    
    DEFAULT_SETUP_HEURISTICS();
    initHeuristicsSchema();

    HSTSNoBranchId noBranchSpec(new HSTSNoBranch());
    HSTSPlanIdReader reader(noBranchSpec);
    reader.read("../core/NoBranch.pi");

    DEFAULT_TEARDOWN_HEURISTICS();
    return true;
  }
  static bool testHSTSNoBranch() {
    
    DEFAULT_SETUP_PLAN(ce,db,true);
    initHeuristicsSchema();

    HSTSNoBranchId noBranchSpec(new HSTSNoBranch());
    HSTSPlanIdReader reader(noBranchSpec);
    reader.read("../core/NoBranch.pi");

    DecisionManagerId dm = planner.getDecisionManager();
    HSTSNoBranchCondition cond(dm);
    assert(dm->getConditions().size() == 4);

    Variable<IntervalIntDomain> var1(ce.getId(), IntervalIntDomain(), true, LabelStr("Commands.TakeSample.rock"));
    Variable<IntervalIntDomain> var2(ce.getId(), IntervalIntDomain(), true, LabelStr("AnObj.APred.Var2"));

    cond.initialize(noBranchSpec);

    assert(!cond.test(var1.getId()));
    assert(cond.test(var2.getId()));

    const_cast<IntervalIntDomain&>(var1.getLastDomain()).intersect(IntervalIntDomain(0));

    assert(cond.test(var1.getId()));
    DEFAULT_TEARDOWN_PLAN();
    return true;
  }
  static bool testHSTSHeuristicsAssembly() {
    DEFAULT_SETUP_PLAN_HEURISTICS();

    HSTSNoBranchId noBranchSpec(new HSTSNoBranch());
    HSTSPlanIdReader pireader(noBranchSpec);
    pireader.read("../core/NoBranch.pi");

    DecisionManagerId& dm = planner.getDecisionManager();
    HSTSNoBranchCondition cond(dm);
    cond.initialize(noBranchSpec);

    Timeline com(db.getId(),LabelStr("Commands"),LabelStr("com1"));
    Timeline ins(db.getId(),LabelStr("Instrument"),LabelStr("ins1"));
    Timeline nav(db.getId(),LabelStr("Navigator"),LabelStr("nav1"));
    Timeline tel(db.getId(),LabelStr("Telemetry"),LabelStr("tel1"));

    Object loc1(db.getId(),LabelStr("Location"),LabelStr("Loc1"));
    Object loc2(db.getId(),LabelStr("Location"),LabelStr("Loc2"));
    Object loc3(db.getId(),LabelStr("Location"),LabelStr("Loc3"));
    Object loc4(db.getId(),LabelStr("Location"),LabelStr("Loc4"));
    Object loc5(db.getId(),LabelStr("Location"),LabelStr("Loc5"));

    db.close();

    std::list<ObjectId> results;
    db.getObjectsByType("Location",results);
    ObjectDomain allLocs(results,"Location");

    std::list<double> values;
    values.push_back(LabelStr("high"));
    values.push_back(LabelStr("medium-high"));
    values.push_back(LabelStr("medium"));
    values.push_back(LabelStr("medium-low"));
    values.push_back(LabelStr("low"));
    EnumeratedDomain allModes(values,false,"Mode");

    IntervalToken tok0(db.getId(),LabelStr("Telemetry.Communicate"), true, IntervalIntDomain(0,100), IntervalIntDomain(0,100), IntervalIntDomain(1,100), "tel1", false);
    tok0.addParameter(IntervalDomain("int"), LabelStr("minutes"));
    ConstrainedVariableId vmin = tok0.getVariable("minutes");
    vmin->specify(IntervalIntDomain(60,120));
    tok0.addParameter(IntervalDomain("float"), LabelStr("bandwidth"));
    ConstrainedVariableId vband = tok0.getVariable("bandwidth");
    vband->specify(IntervalDomain(500.3,1200.4));
    //    tok0.addParameter(BoolDomain(), LabelStr("encoded"));  token
    //    fails to recognize BoolDomain().
    tok0.addParameter(allModes, LabelStr("mode"));
    tok0.close();

    IntervalToken tok1(db.getId(),LabelStr("Commands.TakeSample"), true, IntervalIntDomain(0,100), IntervalIntDomain(0,100), IntervalIntDomain(1,100), "com1", false);
    tok1.addParameter(allLocs, LabelStr("rock"));
    tok1.close();
    ConstrainedVariableId vrock = tok1.getVariable("rock");
    vrock->specify(db.getObject("Loc3"));

    IntervalToken tok2(db.getId(),LabelStr("Instrument.TakeSample"), true, IntervalIntDomain(0,100), IntervalIntDomain(0,200), IntervalIntDomain(1,300), "ins1", false);
    tok2.addParameter(allLocs, LabelStr("rock"));
    tok2.close();

    IntervalToken tok3(db.getId(),LabelStr("Navigator.At"), true, IntervalIntDomain(0,100), IntervalIntDomain(0,200), IntervalIntDomain(1,300), "nav1", false);
    tok3.addParameter(allLocs, LabelStr("location"));
    tok3.close();

    IntervalToken tok4(db.getId(),LabelStr("Navigator.Going"), true, IntervalIntDomain(0,100), IntervalIntDomain(0,200), IntervalIntDomain(1,300), "nav1", false);
    tok4.addParameter(allLocs, LabelStr("from"));
    tok4.addParameter(allLocs, LabelStr("to"));
    tok4.close();

    IntervalToken tok5(db.getId(),LabelStr("Navigator.At"), true, IntervalIntDomain(0,100), IntervalIntDomain(0,200), IntervalIntDomain(1,300), "nav1", false);
    tok5.addParameter(allLocs, LabelStr("location"));
    tok5.close();
    ConstrainedVariableId vatloc = tok5.getVariable("location");
    vatloc->specify(db.getObject("Loc3"));

    AtSubgoalRule r("Navigator.At");

    CBPlanner::Status res = planner.run();
    assert(res == CBPlanner::PLAN_FOUND);
    planner.retract();
    DEFAULT_TEARDOWN_PLAN_HEURISTICS();
    return true;
  }

  static bool testPreferredPriority() {
    
    DEFAULT_SETUP_PLAN(ce,db,true);
    HSTSHeuristics heur(db.getId());
    Priority p1 = 0.0;
    Priority p2 = 1.0;

    heur.setDefaultPriorityPreference(HSTSHeuristics::LOW);
    check_error(heur.preferredPriority(p1, p2) == p1);
    heur.setDefaultPriorityPreference(HSTSHeuristics::HIGH);
    check_error(heur.preferredPriority(p1, p2) == p2);
    DEFAULT_TEARDOWN_PLAN();
    return true;
  }

  static bool testHSTSHeuristicsStrict() {
    DEFAULT_SETUP_PLAN_HEURISTICS();

    //read in the heuristics
    //prefers high
    //assigns 1024.5 priority to tokens with parent Navigator.At
    //assigns 100.25 priority to tokens with parent Navigator.Going
    //assigns 10.0 priority with preference to merge then activate for default token priority
    //assigns 5000.0 priority for default variable priority
    //assigns 6000.25 priority to variables named "to" which are parameters of Navigator.Going,
    //        with an initial parameter of Loc1
    //assigns 6000.5 priority to initial parameter of Commands.TakeSample
    //assigns 6000.25 priority to initial variables of Instrument.TakeSample
    //assigns 9000 priority to minutes parameter of Telemetry.Communicate
    //assigns 9000 priority to mode parameter of Telemetry.Communicate
    //assigns 443.7 priority to Navigator.At tokens
    //assigns 200.4 priority to Commands.TakeSample with an initial parameter equal to Loc3
    //assigns 10000.0 priority to Navigator.Going tokens with parameter 0 == Loc1, parameter 1 == Loc3
    //                that is after a Navigator.At

    //set up the database
    Timeline com(db.getId(),LabelStr("Commands"),LabelStr("com1"));
    Timeline ins(db.getId(),LabelStr("Instrument"),LabelStr("ins1"));
    Timeline nav(db.getId(),LabelStr("Navigator"),LabelStr("nav1"));
    Timeline tel(db.getId(),LabelStr("Telemetry"),LabelStr("tel1"));

    Object loc1(db.getId(),LabelStr("Location"),LabelStr("Loc1"));
    Object loc2(db.getId(),LabelStr("Location"),LabelStr("Loc2"));
    Object loc3(db.getId(),LabelStr("Location"),LabelStr("Loc3"));
    Object loc4(db.getId(),LabelStr("Location"),LabelStr("Loc4"));
    Object loc5(db.getId(),LabelStr("Location"),LabelStr("Loc5"));

    db.close();

    std::list<ObjectId> results;
    db.getObjectsByType("Location",results);
    ObjectDomain allLocs(results,"Location");

    std::list<double> values;
    values.push_back(LabelStr("high"));
    values.push_back(LabelStr("medium-high"));
    values.push_back(LabelStr("medium"));
    values.push_back(LabelStr("medium-low"));
    values.push_back(LabelStr("low"));
    EnumeratedDomain allModes(values,false,"Mode");

    //Telemetry.Communicate(minutes = [60 120], bandwidth = [500.3 1200.4], mode = {high medium-high medium medium-low low}, priority = 10.0
    IntervalToken tok0(db.getId(),LabelStr("Telemetry.Communicate"), true, IntervalIntDomain(1,100), IntervalIntDomain(1,100), IntervalIntDomain(1,100), "tel1", false);
    tok0.addParameter(IntervalDomain("int"), LabelStr("minutes"));
    ConstrainedVariableId vmin = tok0.getVariable("minutes");
    vmin->specify(IntervalIntDomain(60,120));
    tok0.addParameter(IntervalDomain("float"), LabelStr("bandwidth"));
    ConstrainedVariableId vband = tok0.getVariable("bandwidth");
    vband->specify(IntervalDomain(500.3,1200.4));
    //    tok0.addParameter(BoolDomain(), LabelStr("encoded"));  token
    //    fails to recognize BoolDomain().
    tok0.addParameter(allModes, LabelStr("mode"));
    tok0.close();

    //Commands.TakeSample(rock => {Loc1 Loc2 Loc3 Loc4})
    IntervalToken tok1(db.getId(),LabelStr("Commands.TakeSample"), true, IntervalIntDomain(1,100), IntervalIntDomain(1,100), IntervalIntDomain(1,100), "com1", false);
    tok1.addParameter(allLocs, LabelStr("rock"));
    tok1.close();
    //ConstrainedVariableId vrock = tok1.getVariable("rock");
    //vrock->specify(db.getObject("Loc3"));
    tok1.activate(); //activate to cause decision point to be cretaed for parameter, should have priority 6000.5

    //Instrument.TakeSample(rock = {Loc1 Loc2 Loc3 Loc4 Loc5}, priority 10.0
    IntervalToken tok2(db.getId(),LabelStr("Instrument.TakeSample"), true, IntervalIntDomain(1,100), IntervalIntDomain(1,200), IntervalIntDomain(1,300), "ins1", false);
    tok2.addParameter(allLocs, LabelStr("rock"));
    tok2.close();

    //Navigator.At(location = {Loc1 Loc2 Loc3 Loc4 Loc5}) priority 443.7
    IntervalToken tok3(db.getId(),LabelStr("Navigator.At"), true, IntervalIntDomain(1,100), IntervalIntDomain(1,200), IntervalIntDomain(1,300), "nav1", false);
    tok3.addParameter(allLocs, LabelStr("location"));
    tok3.close();

    //Navigator.Going(from = {Loc1 Loc2 Loc3 Loc4 Loc5} to = {Loc1 Loc2 Loc3 Loc4 Loc5}), priority 10.0
    IntervalToken tok4(db.getId(),LabelStr("Navigator.Going"), true, IntervalIntDomain(1,100), IntervalIntDomain(1,200), IntervalIntDomain(1,300), "nav1", false);
    tok4.addParameter(allLocs, LabelStr("from"));
    tok4.addParameter(allLocs, LabelStr("to"));
    tok4.close();

    //Navigator.At(location = {Loc3}), priority 443.7
    IntervalToken tok5(db.getId(),LabelStr("Navigator.At"), true, IntervalIntDomain(1,100), IntervalIntDomain(1,200), IntervalIntDomain(1,300), "nav1", false);
    tok5.addParameter(allLocs, LabelStr("location"));
    tok5.close();
    ConstrainedVariableId vatloc = tok5.getVariable("location");
    vatloc->specify(db.getObject("Loc3"));

    AtSubgoalRule r("Navigator.At");

    assert(ce.propagate());

    DecisionPointId d2 = odm.getNextDecision();
    assert(TokenDecisionPointId::convertable(d2));

    delete (DecisionPoint*) d2;
    DEFAULT_TEARDOWN_PLAN_HEURISTICS();
    return true;
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

  static bool testPriorities() {
    DEFAULT_SETUP_PLAN_HEURISTICS();

    Object loc1(db.getId(),LabelStr("Location"),LabelStr("Loc1"));
    Object loc3(db.getId(),LabelStr("Location"),LabelStr("Loc3"));

    Object nav(db.getId(), LabelStr("Navigator"), LabelStr("nav"));

    Object com(db.getId(), LabelStr("Commands"), LabelStr("com"));

    Object res(db.getId(), LabelStr("UnaryResource"), LabelStr("res"));

    db.close();

    std::list<ObjectId> results;
    db.getObjectsByType("Location",results);
    ObjectDomain allLocs(results,"Location");

    //Navigator.At(Location location);
    //Navigator.Going(Location from, Location to);
    //Commands.TakeSample(Location rock);
    
    //create an unknown variable, priority should be 5000.0
    Variable<IntervalIntDomain> randomVar(ce.getId(), IntervalIntDomain(1, 20), true, LabelStr("randomVar"));
    assert(heuristics.getPriorityForConstrainedVariable(randomVar.getId()) == 5000.0);

    //create Commands.TakeSample, first parameter should have priority = 6000.5
    IntervalToken takeSample(db.getId(), LabelStr("Commands.TakeSample"), false, 
			     IntervalIntDomain(), IntervalIntDomain(), 
			     IntervalIntDomain(1, 100), Token::noObject(), false);
    
    takeSample.addParameter(allLocs, LabelStr("rock"));
    takeSample.close();
    assert(heuristics.getPriorityForConstrainedVariable(takeSample.getParameters()[0]) == 6000.5);

    //create a token not in the heuristics, priority should be 10.0, order should be merge,activate (default match)
    IntervalToken randomTok(db.getId(), LabelStr("UnaryResource.uses"), false);
    assert(heuristics.getPriorityForObject(randomTok.getId()) == 10.0);

    //create a Navigator.At, priority should be 443.7 (simple predicate match)
    IntervalToken navAt(db.getId(), LabelStr("Navigator.At"), false, IntervalIntDomain(), IntervalIntDomain(), 
                        IntervalIntDomain(1, PLUS_INFINITY), Token::noObject(), false);
    navAt.addParameter(allLocs, LabelStr("location"));
    navAt.close();
    assert(heuristics.getPriorityForObject(navAt.getId()) == 443.7);
    navAt.activate();
 
    //create a Navigator.Going with a parent of Navigator.At, priority should be 1024.5 (simple parent match)
    IntervalToken navGoing(navAt.getId(), 
			   LabelStr("after"), 
			   LabelStr("Navigator.Going"), 
			   IntervalIntDomain(), IntervalIntDomain(), 
                           IntervalIntDomain(1, PLUS_INFINITY), Token::noObject(), false);

    navGoing.addParameter(allLocs, LabelStr("from"));
    navGoing.addParameter(allLocs, LabelStr("to"));
    navGoing.close();
    assert(heuristics.getPriorityForObject(navGoing.getId()) == 1024.5);

    //set first parameter of Commands.TakeSample to loc3, priority should be 200.4 (simple variable match)
    takeSample.getParameters()[0]->specify(loc3.getId());
    assert(heuristics.getPriorityForObject(takeSample.getId()) == 200.4);

    //set Navigator.Going "from" parameter to Loc1, parameter "to" should have priority 6000.25 (more complex variable match)
    navGoing.getParameters()[0]->specify(loc1.getId());
    assert(heuristics.getPriorityForConstrainedVariable(navGoing.getParameters()[1]) == 6000.25);

    //set Navigator.Going "to" parameter to Loc3, priority should be 10000 (parent relation match)
    navGoing.getParameters()[1]->specify(loc3.getId());
    assert(heuristics.getPriorityForObject(navGoing.getId()) == 10000.0);    


    takeSample.activate();
    IntervalToken testPreferMerge(takeSample.getId(), LabelStr("before"), 
				  LabelStr("Navigator.Going"), IntervalIntDomain(), IntervalIntDomain(),
				  IntervalIntDomain(1, PLUS_INFINITY), Token::noObject(), false);

    testPreferMerge.addParameter(allLocs, LabelStr("from"));
    testPreferMerge.addParameter(allLocs, LabelStr("to"));
    testPreferMerge.close();

    IntervalToken dummyForMerge(db.getId(), LabelStr("Navigator.Going"), false, IntervalIntDomain(), IntervalIntDomain(), 
                                IntervalIntDomain(1, PLUS_INFINITY), Token::noObject(), false);
    dummyForMerge.addParameter(allLocs, LabelStr("from"));
    dummyForMerge.addParameter(allLocs, LabelStr("to"));
    dummyForMerge.close();
    dummyForMerge.activate();

    assert(ce.propagate());

    // This is a test for full only heuristics and pruning
    TokenDecisionPoint preferMergeDP(DbClientId::noId(), 
				     testPreferMerge.getId(), 
				     planner.getDecisionManager()->getOpenDecisionManager());

    assert(heuristics.getPriorityForToken(testPreferMerge.getId()) == 3.14159);
    TokenDecisionPointId mergeDPId = (TokenDecisionPointId) preferMergeDP.getId();
    planner.getDecisionManager()->getOpenDecisionManager()->initializeChoices(mergeDPId);
    const std::vector<LabelStr>& choices = preferMergeDP.getChoices();
    assertTrue(choices.size() == 1, toString(choices.size()));
    assertTrue(choices[0] == Token::MERGED);
    DEFAULT_TEARDOWN_PLAN_HEURISTICS();
    return true;
  }
};

bool testWeakDomainComparator() {
  
  DEFAULT_SETUP(ce,db,false);
  initHeuristicsSchema();
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

class KeyMatcherTest {
 public:
  static bool test() {
    runTest(testKeyMatcher);
    runTest(testVariableMatch);
    runTest(testTokenMatch);
    return true;
  }
 private:
  static bool testKeyMatcher() {
    assert(KeyMatcher::keyMatches("Foo", "Foo"));
    assert(KeyMatcher::keyMatches("Foo:bar", "Foo:bar"));
    assert(!KeyMatcher::keyMatches("Foo:bar", "bar:Foo"));
    assert(KeyMatcher::keyMatches("Foo:bar:", "Foo:bar:"));
    assert(KeyMatcher::keyMatches("Foo:bar:", "Foo:bar"));
    assert(KeyMatcher::keyMatches("SUBGOAL:Foo:ANY:Bar", "SUBGOAL:Foo:ANY:Bar"));
    assert(KeyMatcher::keyMatches("SUBGOAL:Foo:AFTER:Bar", "SUBGOAL:Foo:ANY:Bar"));
    assert(KeyMatcher::keyMatches("SUBGOAL:Foo:BEFORE:Bar", "SUBGOAL:Foo:ANY:Bar"));
    assert(KeyMatcher::keyMatches("Foo:foo|true:bar|false", "Foo"));
    assert(KeyMatcher::keyMatches("Foo:foo|true:bar|false", "Foo:bar|false"));
    assert(KeyMatcher::keyMatches("SUBGOAL:Foo:::foo|true:BEFORE:Bar", "SUBGOAL:Foo:ANY:Bar")); 
    return true;
  }

  static bool testVariableMatch() {
    std::string simpleKey("foo:Foo");
    std::string predOneArgKey("foo:Foo:bar|true");
    std::string predParentKey("foo:SUBGOAL:Foo:ANY:Bar");
    std::string predParentArgKey("foo:SUBGOAL:Foo:bar|true:ANY:Bar");
    std::string predParentBothArgKey("foo:SUBGOAL:Foo:bar|true:ANY:Bar:baz|nargle");
    
    std::string match("foo:SUBGOAL:Foo::bar|true::BEFORE:Bar:baz|nargle::quux|argle");
    
    assert(KeyMatcher::keyMatches(match, simpleKey));
    assert(KeyMatcher::keyMatches(match, predOneArgKey));
    assert(KeyMatcher::keyMatches(match, predParentKey));
    assert(KeyMatcher::keyMatches(match, predParentArgKey));
    assert(KeyMatcher::keyMatches(match, predParentBothArgKey));
    return true;
  }

  static bool testTokenMatch() {
    std::string initialSimpleKey("INITIAL:Foo");
    std::string initialOneParam("INITIAL:Foo:bar|true");
    std::string initialTwoParam("INITIAL:Foo:bar|true:baz|true");
    std::string initialSkipParam("INITIAL:Foo:baz|true");

    std::string initialMatch("INITIAL:Foo:bar|true:baz|true:");
    
    assert(KeyMatcher::keyMatches(initialMatch, initialSimpleKey));
    assert(KeyMatcher::keyMatches(initialMatch, initialOneParam));
    assert(KeyMatcher::keyMatches(initialMatch, initialTwoParam));
    assert(KeyMatcher::keyMatches(initialMatch, initialSkipParam));

    std::string subgoalSimpleKey("SUBGOAL:Foo:ANY:Bar");
    std::string subgoalOneParam("SUBGOAL:Foo:foo|true:ANY:Bar");
    std::string subgoalTwoParam("SUBGOAL:Foo:foo|true:bar|ack:ANY:Bar");
    std::string subgoalSkipParam("SUBGOAL:Foo:bar|ack:ANY:Bar");
    std::string subgoalParentOneParam("SUBGOAL:Foo:ANY:Bar:bar|true");
    std::string subgoalParentTwoParam("SUBGOAL:Foo:ANY:Bar:bar|true:baz|ack");
    std::string subgoalParentSkipParam("SUBGOAL:Foo:ANY:Bar:baz|ack");
    std::string subgoalBothParam("SUBGOAL:Foo:foo|true:ANY:Bar:bar|true");
    std::string subgoalSkipBothParam("SUBGOAL:Foo:bar|ack:ANY:Bar:baz|ack");
    std::string subgoalEverything("SUBGOAL:Foo:foo|true:bar|ack:ANY:Bar:bar|true:baz|ack");

    std::string subgoalAnyMatch("SUBGOAL:Foo:foo|true:bar|ack:AFTER:Bar:bar|true:baz|ack:");
    
    assert(KeyMatcher::keyMatches(subgoalAnyMatch, subgoalSimpleKey));
    assert(KeyMatcher::keyMatches(subgoalAnyMatch, subgoalOneParam));
    assert(KeyMatcher::keyMatches(subgoalAnyMatch, subgoalTwoParam));
    assert(KeyMatcher::keyMatches(subgoalAnyMatch, subgoalSkipParam));
    assert(KeyMatcher::keyMatches(subgoalAnyMatch, subgoalParentOneParam));
    assert(KeyMatcher::keyMatches(subgoalAnyMatch, subgoalParentTwoParam));
    assert(KeyMatcher::keyMatches(subgoalAnyMatch, subgoalParentSkipParam));
    assert(KeyMatcher::keyMatches(subgoalAnyMatch, subgoalBothParam));
    assert(KeyMatcher::keyMatches(subgoalAnyMatch, subgoalSkipBothParam));
    assert(KeyMatcher::keyMatches(subgoalAnyMatch, subgoalEverything));

    assert(KeyMatcher::keyMatches("SUBGOAL:Foo:::foo|true:ANY:Bar", subgoalSimpleKey));

    assert(KeyMatcher::keyMatches("BEFORE", "ANY"));
    assert(KeyMatcher::keyMatches("AFTER", "ANY"));
    assert(KeyMatcher::keyMatches("SUBGOAL:Foo:BEFORE:Bar", subgoalSimpleKey));
    assert(KeyMatcher::keyMatches("SUBGOAL:Foo:AFTER:Bar", subgoalSimpleKey));
    assert(KeyMatcher::keyMatches("SUBGOAL:Foo:ANY:Bar", subgoalSimpleKey));

    assert(!KeyMatcher::keyMatches("SUBGOAL:Foo:BEFORE:Bar", "SUBGOAL:Foo:AFTER:Bar"));    
    assert(!KeyMatcher::keyMatches("SUBGOAL:Foo:AFTER:Bar", "SUBGOAL:Foo:BEFORE:Bar"));
    assert(!KeyMatcher::keyMatches("SUBGOAL:Foo:ANY:Bar", "SUBGOAL:Foo:BEFORE:Bar"));
    assert(!KeyMatcher::keyMatches("SUBGOAL:Foo:ANY:Bar", "SUBGOAL:Foo:AFTER:Bar"));
    
    assert(!KeyMatcher::keyMatches("SUBGOAL:Foo:foo|bar:ANY:Bar:bar|foo", "SUBGOAL:Foo:foo|foo:ANY:Bar:bar|bar"));

    return true;
  }
};


int main() {
  initConstraintEngine();

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
    runTestSuite(KeyMatcherTest::test);
    runTest(testWeakDomainComparator);
    //Use relaxed domain comparator that allows comparison of members of two different enum types
    WeakDomainComparator wdc;
    runTestSuite(ConstraintTest::test);
    runTestSuite(ConditionTest::test);
    runTestSuite(HeuristicsTest::test);
  }
  std::cout << "Finished" << std::endl;
  ConstraintLibrary::purgeAll();
  exit(0);
}
