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

/* Rules Engine*/
#include "RulesEngine.hh"

/* Miscellaneous */
#include "TestSupport.hh"
#include "DNPConstraints.hh"
#include "WeakDomainComparator.hh"
#include "XMLUtils.hh"

/* Heuristics Engine */
#include "HeuristicsEngine.hh"
#include "Heuristic.hh"
#include "HeuristicsReader.hh"

#include "test/ConstraintTesting.hh"

#include <iostream>
#include <string>
#include <fstream>

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

#define SETUP_PLAN_HEURISTICS()			      \
  SETUP_HEURISTICS()                                  \
  planner.getDecisionManager()->getOpenDecisionManager()->initializeIfNeeded();

#define SETUP_HEURISTICS(heuristicsSource) READ_HEURISTICS(heuristicsSource, true)


#define READ_HEURISTICS(heuristicsSource, autoClose)		\
  ConstraintEngine ce;						\
  initCBPTestSchema();						\
  PlanDatabase db(ce.getId(), Schema::instance());		\
  new DefaultPropagator(LabelStr("Default"), ce.getId());	\
  new DefaultPropagator(LabelStr("Temporal"), ce.getId());	\
  RulesEngine re(db.getId());					\
  HeuristicsEngine heuristics(db.getId()); \
  initHeuristicsSchema(); \
  HeuristicsReader hreader(heuristics.getId()); \
  hreader.read(heuristicsSource, autoClose); \
  HSTSOpenDecisionManager odm(db.getId(), heuristics.getId());  \
  Horizon hor(0, 200);						\
  CBPlanner planner(db.getId(), hor.getId(), odm.getId());	

#define TEARDOWN()

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

/**
 * test the heuristics engine and associated components
 */
class HeuristicsEngineTest {
public:
  static bool test(){
    runTest(testBasicAllocation);
    runTest(testTokenMatching);
    runTest(testDynamicMatching);
    runTest(testMasterMatching);
    runTest(testTargetSelection);
    runTest(testVariableHeuristicConfiguration);
    runTest(testTokenHeuristicConfiguration);
    return true;
  }

private:
  /**
   * Ensure basic hook up and cleanup of Heuristics components
   */
  static bool testBasicAllocation(){
    DEFAULT_SETUP(ce,db,false);      
    Object o1(db.getId(), "Objects", "o1");
    db.close();               
  
    IntervalToken t1(db.getId(),  
                     "Objects.PredicateA",                                                     
                     true,                                                               
                     IntervalIntDomain(0, 10),                                           
                     IntervalIntDomain(0, 20),                                           
                     IntervalIntDomain(1, 1000));
                     
  
    IntervalToken t2(db.getId(),  
                     "Objects.PredicateB",                                                     
                     true,                                                               
                     IntervalIntDomain(0, 10),                                           
                     IntervalIntDomain(0, 20),                                           
                     IntervalIntDomain(1, 1000));

    HeuristicsEngine he(db.getId());

    // Still allowed to add heuristics
    Heuristic h0(he.getId(), "Objects.PredicateA", EMPTY_LABEL(), 11.23, true);
    new Heuristic(he.getId(), "Objects.PredicateB", EMPTY_LABEL(), 14.45, true);
    new Heuristic(he.getId(), "Objects.PredicateB", EMPTY_LABEL(), 20.78, true);

    assertTrue(he.getHeuristics().size() == 3, toString(he.getHeuristics().size())); 

    assertTrue(ce.propagate() && he.getHeuristicInstances().empty());

    he.initialize();

    assertTrue(ce.propagate() && he.getHeuristicInstances().size() == 3, toString(he.getHeuristicInstances().size()));

    // Ensures we get a hit with a single value
    assertTrue(he.getPriority(t1.getId()) == 11.23, 
	       toString(he.getPriority(t1.getId())));

    // Ensures we get the last allocated
    assertTrue(he.getPriority(t2.getId()) == 20.78, 
	       toString(he.getPriority(t2.getId())));

    // Allocte in a limited scope to force de-allocation afterwards
    {
      IntervalToken t3(db.getId(),  
		       "Objects.PredicateB",                                                     
		       true,                                                               
		       IntervalIntDomain(0, 10),                                           
		       IntervalIntDomain(0, 20),                                           
		       IntervalIntDomain(1, 1000));

      ce.propagate();

      assertTrue(he.getHeuristicInstances().size() == 5, toString(he.getHeuristicInstances().size()));
    }                

    // Correctly deallocated?
    assertTrue(he.getHeuristicInstances().size() == 3, toString(he.getHeuristicInstances().size()));

    // Token with no match
    IntervalToken t4(db.getId(),  
                     "Objects.PredicateC",                                                     
                     true,                                                               
                     IntervalIntDomain(0, 10),                                           
                     IntervalIntDomain(0, 20),                                           
                     IntervalIntDomain(1, 1000));


    assertTrue(ce.propagate() && he.getHeuristicInstances().size() == 3, toString(he.getHeuristicInstances().size()));

    // Ensure we get the default priority
    assertTrue(ce.propagate() && he.getPriority(t4.getId()) == he.getDefaultTokenPriority(), 
	       toString(he.getPriority(t4.getId())));

    DEFAULT_TEARDOWN();
    return true;
  }

  /**
   * Test matching of tokens and Heuristics based on static matching data (rather than guard values)
   */
  static bool testTokenMatching(){
    DEFAULT_SETUP(ce,db,false);      
    Object o1(db.getId(), "Objects", "o1");
    db.close();               
  
    // Set up the heuristics
    HeuristicsEngine he(db.getId());
    Heuristic dontcare(he.getId(), "Objects.PredicateA", EMPTY_LABEL(), 1);
    Heuristic allSlaves(he.getId(), "Objects.PredicateA", EMPTY_LABEL(), 2, Heuristic::noGuards(), 
			"Objects.PredicateA", Heuristic::ALL);
    Heuristic before(he.getId(), "Objects.PredicateA", EMPTY_LABEL(), 3, Heuristic::noGuards(),
			"Objects.PredicateA", Heuristic::BEFORE);
    Heuristic other(he.getId(), "Objects.PredicateA", EMPTY_LABEL(), 4, Heuristic::noGuards(),
			"Objects.PredicateA", Heuristic::OTHER);

    std::vector< std::pair<unsigned int, double> > intLabelSetGuards;
    intLabelSetGuards.push_back(std::pair<unsigned int, double>(0, 12));
    intLabelSetGuards.push_back(std::pair<unsigned int, double>(2, LabelStr("A")));
    Heuristic dontcareIntLabelSetGuards(he.getId(), "Objects.PredicateE", EMPTY_LABEL(), 5, false, intLabelSetGuards);

    he.initialize();

    IntervalToken t0(db.getId(),  
		     "Objects.PredicateA",                                                     
		     true,                                                               
		     IntervalIntDomain(0, 10),                                           
		     IntervalIntDomain(0, 20),                                           
		     IntervalIntDomain(1, 1000));

    assertTrue(dontcare.canMatch(t0.getId()));
    assertFalse(allSlaves.canMatch(t0.getId()));
    assertFalse(before.canMatch(t0.getId()));
    assertFalse(other.canMatch(t0.getId()));
    assertFalse(dontcareIntLabelSetGuards.canMatch(t0.getId()));

    t0.activate();

    {
      IntervalToken t1(t0.getId(),
		       LabelStr("before"),
		       LabelStr("Objects.PredicateA"),                                                     
		       IntervalIntDomain(0, 10),                                           
		       IntervalIntDomain(0, 20),                                           
		       IntervalIntDomain(1, 1000));

      assertTrue(dontcare.canMatch(t1.getId()));
      assertTrue(allSlaves.canMatch(t1.getId()));
      assertTrue(before.canMatch(t1.getId()));
      assertFalse(other.canMatch(t1.getId()));
      assertFalse(dontcareIntLabelSetGuards.canMatch(t1.getId()));
    }

    {
      IntervalToken t1(t0.getId(),
		       LabelStr("containedBy"),
		       LabelStr("Objects.PredicateA"),                                                     
		       IntervalIntDomain(0, 10),                                           
		       IntervalIntDomain(0, 20),                                           
		       IntervalIntDomain(1, 1000));

      assertTrue(dontcare.canMatch(t1.getId()));
      assertTrue(allSlaves.canMatch(t1.getId()));
      assertFalse(before.canMatch(t1.getId()));
      assertTrue(other.canMatch(t1.getId()));
      assertFalse(dontcareIntLabelSetGuards.canMatch(t1.getId()));
    }

    // Match nothing
    {
      IntervalToken t1(t0.getId(),
		       LabelStr("containedBy"),
		       LabelStr("Objects.PredicateE"),                                                     
		       IntervalIntDomain(0, 10),                                           
		       IntervalIntDomain(0, 20),                                           
		       IntervalIntDomain(1, 1000));

      assertFalse(dontcare.canMatch(t1.getId()));
      assertFalse(allSlaves.canMatch(t1.getId()));
      assertFalse(before.canMatch(t1.getId()));
      assertFalse(other.canMatch(t1.getId()));
      assertFalse(dontcareIntLabelSetGuards.canMatch(t1.getId()));
    }

    {
      IntervalToken t1(t0.getId(),
		       LabelStr("after"),
		       LabelStr("Objects.PredicateE"),                                                     
		       IntervalIntDomain(0, 10),                                           
		       IntervalIntDomain(0, 20),                                           
		       IntervalIntDomain(1, 1000),
		       Token::noObject(),
		       false);

      ConstrainedVariableId param0 = t1.addParameter(IntervalIntDomain(), LabelStr("param0"));
      t1.addParameter(IntervalDomain(), LabelStr("param1"));
      LabelSet values;
      values.insert(LabelStr("A"));
      values.insert(LabelStr("B"));
      values.close();
      t1.addParameter(values, LabelStr("param2"));
      t1.close();

      assertFalse(dontcare.canMatch(t1.getId()));
      assertFalse(allSlaves.canMatch(t1.getId()));
      assertFalse(before.canMatch(t1.getId()));
      assertFalse(other.canMatch(t1.getId()));
      assertTrue(dontcareIntLabelSetGuards.canMatch(t1.getId()));

      // Restrict base domain to exclude the match
      param0->restrictBaseDomain(IntervalIntDomain(0, 1));
      assertFalse(dontcareIntLabelSetGuards.canMatch(t1.getId()));
    }

    DEFAULT_TEARDOWN();
    return true;
  }

  /**
   * Test dynamic matching against guard values
   */
  static bool testDynamicMatching() {
    DEFAULT_SETUP(ce,db,false);      
    Object o1(db.getId(), "Objects", "o1");
    db.close();               
  
    // Set up the heuristics
    HeuristicsEngine he(db.getId());

    std::vector< std::pair<unsigned int, double> > guards_12_A;
    guards_12_A.push_back(std::pair<unsigned int, double>(0, 12));
    guards_12_A.push_back(std::pair<unsigned int, double>(2, LabelStr("A")));
    Heuristic h_12_A(he.getId(), "Objects.PredicateE", EMPTY_LABEL(), 5, true, guards_12_A);

    std::vector< std::pair<unsigned int, double> > guards_12_B;
    guards_12_B.push_back(std::pair<unsigned int, double>(0, 12));
    guards_12_B.push_back(std::pair<unsigned int, double>(2, LabelStr("B")));
    Heuristic h_12_B(he.getId(), "Objects.PredicateE", EMPTY_LABEL(), 10,true,  guards_12_B);

    std::vector< std::pair<unsigned int, double> > guards_40_B;
    guards_40_B.push_back(std::pair<unsigned int, double>(0, 40));
    guards_40_B.push_back(std::pair<unsigned int, double>(2, LabelStr("B")));
    Heuristic h_40_B(he.getId(), "Objects.PredicateE", EMPTY_LABEL(), 15, true, guards_40_B);

    he.initialize();

    // Set up the token
    IntervalToken t0(db.getId(),  
		     "Objects.PredicateE",                                                     
		     true,                                                               
		     IntervalIntDomain(0, 10),                                           
		     IntervalIntDomain(0, 20),                                           
		     IntervalIntDomain(1, 1000),
		     Token::noObject(),
		     false);

    ConstrainedVariableId param0 = t0.addParameter(IntervalIntDomain(), LabelStr("param0"));
    t0.addParameter(IntervalDomain(), LabelStr("param1"));
    LabelSet values;
    values.insert(LabelStr("A"));
    values.insert(LabelStr("B"));
    values.close();
    ConstrainedVariableId param2 = t0.addParameter(values, LabelStr("param2"));
    t0.close();

    // veryfy we get the default priority
    assertTrue(ce.propagate() && he.getPriority(t0.getId()) == he.getDefaultTokenPriority());

    // Bind and check - only 1 of t guards will be hit
    param2->specify(LabelStr("A"));
    ce.propagate();
    assertTrue(he.getPriority(t0.getId()) == he.getDefaultTokenPriority());

    // Bind the second to fire 12 A
    param0->specify(12);
    ce.propagate();
    assertTrue(he.getPriority(t0.getId()) == 5);

    // Fire 12 B
    param2->reset();
    ce.propagate();
    assertTrue(he.getPriority(t0.getId()) == he.getDefaultTokenPriority());
    param2->specify(LabelStr("B"));
    ce.propagate();
    assertTrue(he.getPriority(t0.getId()) == 10);

    param0->reset();
    ce.propagate();
    assertTrue(he.getPriority(t0.getId()) == he.getDefaultTokenPriority());
    param0->specify(40);
    ce.propagate();
    assertTrue(he.getPriority(t0.getId()) == 15);

    param0->reset();
    param2->reset();
    param0->specify(12);
    param2->specify(LabelStr("A"));
    ce.propagate();
    assertTrue(he.getPriority(t0.getId()) == 5);
    DEFAULT_TEARDOWN();
    return true;
  }

  /**
   * Proves the heuristics are selective to master gaurds.
   */
  static bool testMasterMatching(){

    DEFAULT_SETUP(ce,db,false);      
    Object o1(db.getId(), "Objects", "o1");
    db.close();               
  
    // Set up the heuristics
    HeuristicsEngine he(db.getId());

    std::vector< std::pair<unsigned int, double> > guards;
    guards.push_back(std::pair<unsigned int, double>(0, 12));
    guards.push_back(std::pair<unsigned int, double>(2, LabelStr("A")));

    // Heuristic guarded on the master. Predicate match on both, and one must be before the other
    Heuristic h0(he.getId(), "Objects.PredicateE", EMPTY_LABEL(), 5, Heuristic::noGuards(), 
		 "Objects.PredicateE", Heuristic::BEFORE, guards);

    he.initialize();

    // Set up the token
    IntervalToken master0(db.getId(),  
			  "Objects.PredicateE",                                                     
			  true,                                                               
			  IntervalIntDomain(0, 10),                                           
			  IntervalIntDomain(0, 20),                                           
			  IntervalIntDomain(1, 1000),
			  Token::noObject(),
			  false);

    ConstrainedVariableId param0 = master0.addParameter(IntervalIntDomain(), LabelStr("param0"));
    master0.addParameter(IntervalDomain(), LabelStr("param1"));
    LabelSet values;
    values.insert(LabelStr("A"));
    values.insert(LabelStr("B"));
    values.close();
    ConstrainedVariableId param2 = master0.addParameter(values, LabelStr("param2"));
    master0.close();

    assertFalse(h0.canMatch(master0.getId()));

    master0.activate();

    IntervalToken slave0(master0.getId(),
			 LabelStr("before"),
			 LabelStr("Objects.PredicateE"),                                                     
			 IntervalIntDomain(0, 10),                                           
			 IntervalIntDomain(0, 20),                                           
			 IntervalIntDomain(1, 1000));

    // It should match
    assertTrue(h0.canMatch(slave0.getId()));

    // But not fire
    assertTrue( ce.propagate() && he.getPriority(slave0.getId()) == he.getDefaultTokenPriority());

    // Bind and check - only 1 of the guards will be hit
    param2->specify(LabelStr("A"));
    ce.propagate();
    assertTrue(he.getPriority(slave0.getId()) == he.getDefaultTokenPriority());

    // Bind the second to fire 12 A
    param0->specify(12);
    ce.propagate();
    assertTrue(he.getPriority(slave0.getId()) == 5);

    // Reset and fire B
    param0->reset();
    param2->reset();
    param0->specify(12);
    param2->specify(LabelStr("B"));
    ce.propagate();
    assertTrue(he.getPriority(slave0.getId()) == he.getDefaultTokenPriority());
    DEFAULT_TEARDOWN();
    return true;
  }

  /**
   * Test dynamic matching against guard values
   */
  static bool testTargetSelection() {
    DEFAULT_SETUP(ce,db,false);      
    Object o1(db.getId(), "Objects", "o1");
    db.close();               
  
    // Set up the heuristics
    HeuristicsEngine he(db.getId());

    std::vector< std::pair<unsigned int, double> > guards;
    guards.push_back(std::pair<unsigned int, double>(0, 12));
    guards.push_back(std::pair<unsigned int, double>(2, LabelStr("A")));
    Heuristic h0(he.getId(), "Objects.PredicateE", LabelStr("start"), 5, true, guards);
    Heuristic h1(he.getId(), "Objects.PredicateE", LabelStr("param1"), 10, true, guards);

    he.initialize();

    // Set up the token
    IntervalToken t0(db.getId(),  
		     "Objects.PredicateE",                                                     
		     true,                                                               
		     IntervalIntDomain(0, 10),                                           
		     IntervalIntDomain(0, 20),                                           
		     IntervalIntDomain(1, 1000),
		     Token::noObject(),
		     false);

    ConstrainedVariableId param0 = t0.addParameter(IntervalIntDomain(), LabelStr("param0"));
    ConstrainedVariableId param1 = t0.addParameter(IntervalDomain(), LabelStr("param1"));
    LabelSet values;
    values.insert(LabelStr("A"));
    values.insert(LabelStr("B"));
    values.close();
    ConstrainedVariableId param2 = t0.addParameter(values, LabelStr("param2"));
    t0.close();

    // veryfy we get the default priority
    assertTrue(ce.propagate() && he.getPriority(t0.getId()) == he.getDefaultTokenPriority());

    // Bind
    param0->specify(12);
    param2->specify(LabelStr("A"));
    ce.propagate();

    // Confirm
    assertTrue(he.getPriority(t0.getId()) == he.getDefaultTokenPriority());
    assertTrue(he.getPriority(t0.getStart()) == 5);
    assertTrue(he.getPriority(param1) == 10);
    DEFAULT_TEARDOWN();
    return true;
  }

  static bool testVariableHeuristicConfiguration(){
    return testHeuristicConfiguration("VariableHeuristics.xml");
  }

  static bool testTokenHeuristicConfiguration(){
    return testHeuristicConfiguration("TokenHeuristics.xml");
  }

  static bool testHeuristicConfiguration(const char* source){
    DEFAULT_SETUP(ce,db,false);      
    initHeuristicsSchema();
    db.close();               
  
    std::string outputFile(source);
    outputFile = outputFile + ".out";
    std::ofstream ofs(outputFile.c_str());

    // Set up the heuristics
    HeuristicsEngine he(db.getId());
    TiXmlElement* configXml = initXml(source);
    assertTrue(configXml != NULL, "Bad test input data.");
    for (TiXmlElement * child = configXml->FirstChildElement(); 
	 child != NULL; 
	 child = child->NextSiblingElement()) {
      HeuristicId heuristic = HeuristicsReader::createHeuristic(he.getId(), *child);
      ofs << heuristic->toString() << std::endl;
      assertTrue(heuristic.isValid());
    }

    ofs.close();
    DEFAULT_TEARDOWN();
    return true;
  }
};

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
    DEFAULT_SETUP(ce,db,false);
    std::list<ConstraintTestCase> tests;
    assertTrue(readTestCases(std::string("DNPTestCases"), tests) ||
               readTestCases(std::string("HSTS/test/DNPTestCases"), tests));
    assertTrue(executeTestCases(ce.getId(), tests));
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
    runTest(testHSTSPlanIdReader);
    runTest(testHSTSNoBranch);
    runTest(testHSTSHeuristicsAssembly);
    runTest(testHSTSHeuristicsStrict);
    runTest(testPriorities);
    return(true);
  }
private:

  static bool testDefaultInitialization() {
    READ_HEURISTICS("HSTSAssemblyHeuristics.xml", false);
    initHeuristicsSchema();
    std::vector< GuardEntry > domainSpecs;
    heuristics.setDefaultPriorityForTokenDPsWithParent(20.3, LabelStr("Commands.TakeSample"), domainSpecs);
    heuristics.setDefaultPriorityForTokenDPs(10000.0);
    
    heuristics.setDefaultPriorityForConstrainedVariableDPs(10000.0);

    std::vector<LabelStr> states;
    std::vector<TokenHeuristic::CandidateOrder> orders;
    states.push_back(Token::REJECTED);
    states.push_back(Token::MERGED);
    //    states.push_back(Token::DEFER);
    states.push_back(Token::ACTIVE);
    orders.push_back(TokenHeuristic::NONE);
    orders.push_back(TokenHeuristic::EARLY);
    //    orders.push_back(HSTSHeuristics::NONE);
    orders.push_back(TokenHeuristic::EARLY);
    heuristics.setDefaultPreferenceForTokenDPs(states,orders);

    heuristics.setDefaultPreferenceForConstrainedVariableDPs(VariableHeuristic::ASCENDING);

    TEARDOWN();
    return true;
  }

  static bool testHSTSPlanIdReader() { 
    DEFAULT_SETUP(ce,db,true);
    initHeuristicsSchema();

    HSTSNoBranchId noBranchSpec(new HSTSNoBranch());
    HSTSPlanIdReader reader(noBranchSpec);
    reader.read("../core/NoBranch.pi");

    DEFAULT_TEARDOWN();
    return true;
  }
  static bool testHSTSNoBranch() {    
    SETUP_HEURISTICS("HSTSAssemblyHeuristics.xml");

    HSTSNoBranchId noBranchSpec(new HSTSNoBranch());
    HSTSPlanIdReader reader(noBranchSpec);
    reader.read("../core/NoBranch.pi");

    DecisionManagerId dm = planner.getDecisionManager();
    HSTSNoBranchCondition cond(dm);
    assert(dm->getConditions().size() == 3);

    Variable<IntervalIntDomain> var1(ce.getId(), IntervalIntDomain(), true, LabelStr("Commands.TakeSample.rock"));
    Variable<IntervalIntDomain> var2(ce.getId(), IntervalIntDomain(), true, LabelStr("AnObj.APred.Var2"));

    cond.initialize(noBranchSpec);

    assert(!cond.test(var1.getId()));
    assert(cond.test(var2.getId()));

    const_cast<IntervalIntDomain&>(var1.getLastDomain()).intersect(IntervalIntDomain(0));

    assert(cond.test(var1.getId()));
    TEARDOWN();
    return true;
  }
  static bool testHSTSHeuristicsAssembly() {
    SETUP_HEURISTICS("HSTSAssemblyHeuristics.xml");

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

    planner.run();

    //CBPlanner::Status res = planner.run();
    //assert(res == CBPlanner::PLAN_FOUND);
    planner.retract();
    TEARDOWN();
    return true;
  }

  static bool testHSTSHeuristicsStrict() {
    SETUP_HEURISTICS("HSTSAssemblyHeuristics.xml");

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
    ConstrainedVariableId mode = tok0.addParameter(allModes, LabelStr("mode"));
    tok0.close();
    ce.propagate();
    assertTrue(heuristics.getPriority(mode) ==  9000, toString(heuristics.getPriority(mode)));
    assertTrue(heuristics.getPriority(tok0.getId()) ==  10, toString(heuristics.getPriority(tok0.getId())));

    //Commands.TakeSample(rock => {Loc1 Loc2 Loc3 Loc4})
    IntervalToken tok1(db.getId(),LabelStr("Commands.TakeSample"), true, IntervalIntDomain(1,100), IntervalIntDomain(1,100), IntervalIntDomain(1,100), "com1", false);
    ConstrainedVariableId rock = tok1.addParameter(allLocs, LabelStr("rock"));
    tok1.close();
    ce.propagate();
    assertTrue(heuristics.getPriority(rock) ==  6000.5, toString(heuristics.getPriority(rock)));

    tok1.activate();

    //Instrument.TakeSample(rock = {Loc1 Loc2 Loc3 Loc4 Loc5}, priority 10.0
    IntervalToken tok2(db.getId(),LabelStr("Instrument.TakeSample"), true, IntervalIntDomain(1,100), IntervalIntDomain(1,200), IntervalIntDomain(1,300), "ins1", false);
    rock = tok2.addParameter(allLocs, LabelStr("rock"));
    tok2.close();
    ce.propagate();
    assertTrue(heuristics.getPriority(rock) ==  6000.25, toString(heuristics.getPriority(rock)));
	      
    //Navigator.At(location = {Loc1 Loc2 Loc3 Loc4 Loc5}) priority 443.7
    IntervalToken tok3(db.getId(),LabelStr("Navigator.At"), true, IntervalIntDomain(1,100), IntervalIntDomain(1,200), IntervalIntDomain(1,300), "nav1", false);
    ConstrainedVariableId location = tok3.addParameter(allLocs, LabelStr("location"));
    tok3.close();
    ce.propagate();

    assertTrue(heuristics.getPriority(tok3.getId()) ==  443.7, toString(heuristics.getPriority(tok3.getId())));
    assertTrue(heuristics.getPriority(location) ==  5000, toString(heuristics.getPriority(location)));

    //Navigator.Going(from = {Loc1 Loc2 Loc3 Loc4 Loc5} to = {Loc1 Loc2 Loc3 Loc4 Loc5}), priority 
    IntervalToken tok4(db.getId(),LabelStr("Navigator.Going"), true, IntervalIntDomain(1,100), IntervalIntDomain(1,200), IntervalIntDomain(1,300), "nav1", false);
    ConstrainedVariableId from = tok4.addParameter(allLocs, LabelStr("from"));
    ConstrainedVariableId to = tok4.addParameter(allLocs, LabelStr("to"));
    tok4.close();
    ce.propagate();

    assertTrue(heuristics.getPriority(tok4.getId()) ==  100.25, toString(heuristics.getPriority(tok4.getId())));
    assertTrue(heuristics.getPriority(from) ==  5000, toString(heuristics.getPriority(from)));
    assertTrue(heuristics.getPriority(to) ==  5000, toString(heuristics.getPriority(to)));

    //Navigator.At(location = {Loc3}), priority 443.7
    IntervalToken tok5(db.getId(),LabelStr("Navigator.At"), true, IntervalIntDomain(1,100), IntervalIntDomain(1,200), IntervalIntDomain(1,300), "nav1", false);
    location = tok5.addParameter(allLocs, LabelStr("location"));
    tok5.close();
    ce.propagate();
    assertTrue(heuristics.getPriority(tok5.getId()) ==  443.7, toString(heuristics.getPriority(tok5.getId())));
	       
    location->specify(db.getObject("Loc5"));
    ce.propagate();
    assertTrue(heuristics.getPriority(tok5.getId()) ==  7.23, toString(heuristics.getPriority(tok5.getId())));

    AtSubgoalRule r("Navigator.At");

    assert(ce.propagate());

    TEARDOWN();
    return true;
  }

  static bool testPriorities() {
    SETUP_HEURISTICS("HSTSAssemblyHeuristics.xml");

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
    ce.propagate();
    assert(heuristics.getPriority(randomVar.getId()) == 5000.0);

    //create Commands.TakeSample, first parameter should have priority = 6000.5
    IntervalToken takeSample(db.getId(), LabelStr("Commands.TakeSample"), false, 
			     IntervalIntDomain(), IntervalIntDomain(), 
			     IntervalIntDomain(1, 100), Token::noObject(), false);
    
    takeSample.addParameter(allLocs, LabelStr("rock"));
    takeSample.close();
    ce.propagate();
    assert(heuristics.getPriority(takeSample.getParameters()[0]) == 6000.5);

    //create a token not in the heuristics, priority should be 10.0, order should be merge,activate (default match)
    IntervalToken randomTok(db.getId(), LabelStr("UnaryResource.uses"), false);
    ce.propagate();
    assert(heuristics.getPriority(randomTok.getId()) == 10.0);

    //create a Navigator.At, priority should be 443.7 (simple predicate match)
    IntervalToken navAt(db.getId(), LabelStr("Navigator.At"), false, IntervalIntDomain(), IntervalIntDomain(), 
                        IntervalIntDomain(1, PLUS_INFINITY), Token::noObject(), false);
    navAt.addParameter(allLocs, LabelStr("location"));
    navAt.close();
    ce.propagate();
    assert(heuristics.getPriority(navAt.getId()) == 443.7);
    navAt.activate();

    takeSample.activate();

    //create a Navigator.Going with a parent of TakeSample, priority should be 3.14159
    IntervalToken navGoing(takeSample.getId(), 
			   LabelStr("before"), 
			   LabelStr("Navigator.Going"), 
			   IntervalIntDomain(), IntervalIntDomain(), 
                           IntervalIntDomain(1, PLUS_INFINITY), Token::noObject(), false);

    navGoing.addParameter(allLocs, LabelStr("from"));
    navGoing.addParameter(allLocs, LabelStr("to"));
    navGoing.close();
    ce.propagate();
    assertTrue(heuristics.getPriority(navGoing.getId()) == 3.14159, toString(heuristics.getPriority(navGoing.getId())));

    //set first parameter of Commands.TakeSample to loc3, priority should be 200.4 (simple variable match)
    takeSample.getParameters()[0]->specify(loc3.getId());
    ce.propagate();
    assert(heuristics.getPriority(takeSample.getId()) == 200.4);

    //set Navigator.Going "from" parameter to Loc1, parameter "to" should have priority 6000.25 (more complex variable match)
    navGoing.getParameters()[0]->specify(loc1.getId());
    ce.propagate();
    assert(heuristics.getPriority(navGoing.getParameters()[1]) == 6000.25);

    //set Navigator.Going "to" parameter to Loc3. Should not change, because of the master relation being 'before'
    navGoing.getParameters()[1]->specify(loc3.getId());
    ce.propagate();
    assert(heuristics.getPriority(navGoing.getParameters()[1]) == 6000.25);

    {
      //create a Navigator.Going with a parent of TakeSample, priority should be 3.14159
      IntervalToken newNavGoing(takeSample.getId(), 
				LabelStr("after"), 
				LabelStr("Navigator.Going"), 
				IntervalIntDomain(), IntervalIntDomain(), 
				IntervalIntDomain(1, PLUS_INFINITY), Token::noObject(), false);

      newNavGoing.addParameter(allLocs, LabelStr("from"));
      newNavGoing.addParameter(allLocs, LabelStr("to"));
      newNavGoing.close();
      newNavGoing.getParameters()[0]->specify(loc1.getId());
      newNavGoing.getParameters()[1]->specify(loc3.getId());
      ce.propagate();
      assert(heuristics.getPriority(newNavGoing.getId()) == 10000.0);
    }

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

    assert(heuristics.getPriority(testPreferMerge.getId()) == 3.14159);
    TokenDecisionPointId mergeDPId = (TokenDecisionPointId) preferMergeDP.getId();
    planner.getDecisionManager()->getOpenDecisionManager()->initializeChoices(mergeDPId);
    const std::vector<LabelStr>& choices = preferMergeDP.getChoices();
    assertTrue(choices.size() == 1, toString(choices.size()));
    assertTrue(choices[0] == Token::MERGED);
    TEARDOWN();
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
    runTestSuite(HeuristicsEngineTest::test);
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
