#include "IntervalIntDomain.hh"
#include "IntervalDomain.hh"
#include "DefaultPropagator.hh"
#include "Utils.hh"
#include "StringDomain.hh"
#include "PlanDatabase.hh"
#include "Schema.hh"
#include "Object.hh"
#include "Timeline.hh"
#include "IntervalToken.hh"
#include "TokenVariable.hh"

/* Miscellaneous */
#include "TestSupport.hh"
#include "XMLUtils.hh"
#include "Utils.hh"

/* Heuristics Engine */
#include "HeuristicsEngine.hh"
#include "Heuristic.hh"
#include "HeuristicsReader.hh"

#include <list>
#include <vector>
#include <iostream>
#include <string>
#include <fstream>


#define DEFAULT_SETUP(ce, db, autoClose) \
    ConstraintEngine ce; \
    initCBPTestSchema(); \
    PlanDatabase db(ce.getId(), Schema::instance()); \
    new DefaultPropagator(LabelStr("Default"), ce.getId()); \
    new DefaultPropagator(LabelStr("Temporal"), ce.getId()); \
    if (autoClose) \
      db.close();

#define DEFAULT_TEARDOWN()

#define SETUP_HEURISTICS(heuristicsSource) \
  READ_HEURISTICS(heuristicsSource, true)


#define READ_HEURISTICS(heuristicsSource, autoClose)		\
  initHeuristicsSchema();                                       \
  ConstraintEngine ce;						\
  PlanDatabase db(ce.getId(), Schema::instance());		\
  new DefaultPropagator(LabelStr("Default"), ce.getId());	\
  new DefaultPropagator(LabelStr("Temporal"), ce.getId());	\
  HeuristicsEngine heuristics(db.getId()); \
  HeuristicsReader hreader(heuristics.getId()); \
  hreader.read(heuristicsSource, autoClose);
	
#define TEARDOWN()


/**
 * @brief Creates the type specifications required for testing
 */
void initCBPTestSchema(){
  const SchemaId& schema = Schema::instance();
  schema->reset();
  schema->addObjectType("Object");

  schema->addPredicate("Object.PredicateA");
  schema->addMember("Object.PredicateA", IntervalIntDomain().getTypeName(), "IntervalIntParam");

  schema->addPredicate("Object.PredicateB");
  schema->addPredicate("Object.PredicateC");
  schema->addPredicate("Object.PredicateD");
  schema->addPredicate("Object.PADDED");

  schema->addPredicate("Object.PredicateE");
  schema->addMember("Object.PredicateE", IntervalIntDomain().getTypeName(), "param0");
  schema->addMember("Object.PredicateE", IntervalDomain().getTypeName(), "param1");
  schema->addMember("Object.PredicateE", LabelSet().getTypeName(), "param2");

  schema->addPredicate("Object.P1");
  schema->addMember("Object.P1", LabelSet().getTypeName(), "LabelSetParam0");
  schema->addMember("Object.P1", LabelSet().getTypeName(), "LabelSetParam1");
  schema->addMember("Object.P1", IntervalIntDomain().getTypeName(), "IntervalIntParam");

  schema->addPredicate("Object.P1True");
  schema->addMember("Object.P1True", BoolDomain().getTypeName(), "BoolParam");
  schema->addPredicate("Object.P1False");
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
    Object o1(db.getId(), "Object", "o1");
    db.close();               
  
    IntervalToken t1(db.getId(),  
                     "Object.PredicateA",                                                     
                     true,                                                               
                     IntervalIntDomain(0, 10),                                           
                     IntervalIntDomain(0, 20),                                           
                     IntervalIntDomain(1, 1000));
                     
  
    IntervalToken t2(db.getId(),  
                     "Object.PredicateB",                                                     
                     true,                                                               
                     IntervalIntDomain(0, 10),                                           
                     IntervalIntDomain(0, 20),                                           
                     IntervalIntDomain(1, 1000));

    HeuristicsEngine he(db.getId());

    // Still allowed to add heuristics
    Heuristic h0(he.getId(), "Object.PredicateA", EMPTY_LABEL(), 11.23, true);
    new Heuristic(he.getId(), "Object.PredicateB", EMPTY_LABEL(), 14.45, true);
    new Heuristic(he.getId(), "Object.PredicateB", EMPTY_LABEL(), 20.78, true);

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
		       "Object.PredicateB",                                                     
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
                     "Object.PredicateC",                                                     
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
    Object o1(db.getId(), "Object", "o1");
    db.close();               
  
    // Set up the heuristics
    HeuristicsEngine he(db.getId());
    Heuristic dontcare(he.getId(), "Object.PredicateA", EMPTY_LABEL(), 1);
    Heuristic allSlaves(he.getId(), "Object.PredicateA", EMPTY_LABEL(), 2, Heuristic::noGuards(), 
			"Object.PredicateA", Heuristic::ALL);
    Heuristic before(he.getId(), "Object.PredicateA", EMPTY_LABEL(), 3, Heuristic::noGuards(),
			"Object.PredicateA", Heuristic::BEFORE);
    Heuristic other(he.getId(), "Object.PredicateA", EMPTY_LABEL(), 4, Heuristic::noGuards(),
			"Object.PredicateA", Heuristic::OTHER);

    std::vector< std::pair<unsigned int, double> > intLabelSetGuards;
    intLabelSetGuards.push_back(std::pair<unsigned int, double>(0, 12));
    intLabelSetGuards.push_back(std::pair<unsigned int, double>(2, LabelStr("A")));
    Heuristic dontcareIntLabelSetGuards(he.getId(), "Object.PredicateE", EMPTY_LABEL(), 5, false, intLabelSetGuards);

    he.initialize();

    IntervalToken t0(db.getId(),  
		     "Object.PredicateA",                                                     
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
		       LabelStr("Object.PredicateA"),                                                     
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
		       LabelStr("Object.PredicateA"),                                                     
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
		       LabelStr("Object.PredicateE"),                                                     
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
		       LabelStr("Object.PredicateE"),                                                     
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
    Object o1(db.getId(), "Object", "o1");
    db.close();               
  
    // Set up the heuristics
    HeuristicsEngine he(db.getId());

    std::vector< std::pair<unsigned int, double> > guards_12_A;
    guards_12_A.push_back(std::pair<unsigned int, double>(0, 12));
    guards_12_A.push_back(std::pair<unsigned int, double>(2, LabelStr("A")));
    Heuristic h_12_A(he.getId(), "Object.PredicateE", EMPTY_LABEL(), 5, true, guards_12_A);

    std::vector< std::pair<unsigned int, double> > guards_12_B;
    guards_12_B.push_back(std::pair<unsigned int, double>(0, 12));
    guards_12_B.push_back(std::pair<unsigned int, double>(2, LabelStr("B")));
    Heuristic h_12_B(he.getId(), "Object.PredicateE", EMPTY_LABEL(), 10,true,  guards_12_B);

    std::vector< std::pair<unsigned int, double> > guards_40_B;
    guards_40_B.push_back(std::pair<unsigned int, double>(0, 40));
    guards_40_B.push_back(std::pair<unsigned int, double>(2, LabelStr("B")));
    Heuristic h_40_B(he.getId(), "Object.PredicateE", EMPTY_LABEL(), 15, true, guards_40_B);

    he.initialize();

    // Set up the token
    IntervalToken t0(db.getId(),  
		     "Object.PredicateE",                                                     
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
    Object o1(db.getId(), "Object", "o1");
    db.close();               
  
    // Set up the heuristics
    HeuristicsEngine he(db.getId());

    std::vector< std::pair<unsigned int, double> > guards;
    guards.push_back(std::pair<unsigned int, double>(0, 12));
    guards.push_back(std::pair<unsigned int, double>(2, LabelStr("A")));

    // Heuristic guarded on the master. Predicate match on both, and one must be before the other
    Heuristic h0(he.getId(), "Object.PredicateE", EMPTY_LABEL(), 5, Heuristic::noGuards(), 
		 "Object.PredicateE", Heuristic::BEFORE, guards);

    he.initialize();

    // Set up the token
    IntervalToken master0(db.getId(),  
			  "Object.PredicateE",                                                     
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
			 LabelStr("Object.PredicateE"),                                                     
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
    Object o1(db.getId(), "Object" , "o1");
    db.close();               
  
    // Set up the heuristics
    HeuristicsEngine he(db.getId());

    std::vector< std::pair<unsigned int, double> > guards;
    guards.push_back(std::pair<unsigned int, double>(0, 12));
    guards.push_back(std::pair<unsigned int, double>(2, LabelStr("A")));
    Heuristic h0(he.getId(), "Object.PredicateE", LabelStr("start"), 5, true, guards);
    Heuristic h1(he.getId(), "Object.PredicateE", LabelStr("param1"), 10, true, guards);

    he.initialize();

    // Set up the token
    IntervalToken t0(db.getId(),  
		     "Object.PredicateE",                                                     
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

class HeuristicsTest {
public:
  static bool test() {
    runTest(testDefaultInitialization);
    runTest(testHSTSHeuristicsAssembly);
    runTest(testHSTSHeuristicsStrict);
    runTest(testPriorities);
    runTest(testTokenOrderingCalculations);
    runTest(testTokenOrdering);
    runTest(testTokenOrderingChoices);
    runTest(testVariableValueOrderingChoices);
    return(true);
  }

private:

  static bool testDefaultInitialization() {
    READ_HEURISTICS("HSTSHeuristics.xml", false);
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

  static bool testHSTSHeuristicsAssembly() {
    SETUP_HEURISTICS("HSTSHeuristics.xml");

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

    TEARDOWN();
    return true;
  }

  static bool testHSTSHeuristicsStrict() {
    SETUP_HEURISTICS("HSTSHeuristics.xml");

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

    assert(ce.propagate());

    TEARDOWN();
    return true;
  }

  static bool testPriorities() {
    SETUP_HEURISTICS("HSTSHeuristics.xml");

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
    TEARDOWN();
    return true;
  }

  static bool testTokenOrderingCalculations() {
    DEFAULT_SETUP(ce,db,false);      
    Object o1(db.getId(), "Object", "o1");
    db.close();               

    IntervalToken t0(db.getId(),  
		     "Object.PredicateA",                                                     
		     true,                                                               
		     IntervalIntDomain(-12, 10),                                           
		     IntervalIntDomain(-10, 8),                                           
		     IntervalIntDomain(1, PLUS_INFINITY));

    assertTrue(TokenHeuristic::midpoint(t0.getId()) == -2, toString(TokenHeuristic::midpoint(t0.getId())));

    // Same tokens have zero distance
    assertTrue(TokenHeuristic::absoluteDistance(t0.getId(), t0.getId()) == 0, 
	       toString(TokenHeuristic::absoluteDistance(t0.getId(), t0.getId())));

    IntervalToken t1(db.getId(),  
		     "Object.PredicateA",                                                     
		     true,                                                               
		     IntervalIntDomain(0, 10),                                           
		     IntervalIntDomain(1, 49),                                           
		     IntervalIntDomain(1, 1000));

    assertTrue(TokenHeuristic::midpoint(t1.getId()) == 24, toString(TokenHeuristic::midpoint(t1.getId())));

    // Different tokens
    assertTrue(TokenHeuristic::absoluteDistance(t0.getId(), t1.getId()) == 26, 
	       toString(TokenHeuristic::absoluteDistance(t0.getId(), t1.getId())));

    // Order reversed
    assertTrue(TokenHeuristic::absoluteDistance(t1.getId(), t0.getId()) == 26, 
	       toString(TokenHeuristic::absoluteDistance(t1.getId(), t0.getId())));

    return true;
  }



  static int earliest(const TokenId& orderingChoice){
    return (int) orderingChoice->getStart()->lastDomain().getLowerBound();
  }

  static int earliest(const Token& orderingChoice){
    return earliest(orderingChoice.getId());
  }

  static bool testTokenOrdering(){
    DEFAULT_SETUP(ce,db,false);      
    Object o1(db.getId(), "Object", "o1");
    db.close();               

    std::vector<TokenId> tokensToOrder;

    IntervalToken referenceToken(db.getId(),  
				 "Object.PredicateA",                                                     
				 true,                                                               
				 IntervalIntDomain(-50, -50),                                           
				 IntervalIntDomain(0, 0),                                           
				 IntervalIntDomain(1, PLUS_INFINITY));

    // Allocate tokens to span before and after the refrence token, but not symmetrically so we can
    // be specific in picking best and worst
    const int baseTime(-500);
    const int increment(50);
    const int tokenCount(20);
    int newBase(baseTime);
    for(int i = 0; i < tokenCount; i++){
      newBase = baseTime + i * increment;
      TokenId token = (new IntervalToken(db.getId(),  
					 "Object.PredicateA",                                                     
					 true,                                                               
					 IntervalIntDomain(newBase, newBase),
					 IntervalIntDomain(newBase + increment, newBase + increment),
					 IntervalIntDomain(1, PLUS_INFINITY)))->getId();
      tokensToOrder.push_back(token);
    }

    // LATE: Ensure the earliest token is the last in the sequence. and latest is first
    TokenHeuristic::orderTokens(tokensToOrder, referenceToken.getId(), TokenHeuristic::LATE);
    assertTrue(earliest(tokensToOrder.back()) == baseTime);
    assertTrue(earliest(tokensToOrder[0]) == baseTime + (increment * (tokenCount-1)),
	       toString(earliest(tokensToOrder.back()))); 

    // EARLY && NONE: Ensure the opposite
    TokenHeuristic::orderTokens(tokensToOrder, referenceToken.getId(), TokenHeuristic::EARLY);
    assertTrue(earliest(tokensToOrder.back()) == baseTime + (increment * (tokenCount-1)));
    assertTrue(earliest(tokensToOrder[0]) == baseTime,
	       toString(earliest(tokensToOrder.back()))); 

    TokenHeuristic::orderTokens(tokensToOrder, referenceToken.getId(), TokenHeuristic::NONE);
    assertTrue(earliest(tokensToOrder.back()) == baseTime + (increment * (tokenCount-1)));
    assertTrue(earliest(tokensToOrder[0]) == baseTime,
	       toString(earliest(tokensToOrder.back()))); 

    // NEAR: First should be the same bounds. Latest should be either extreme.
    TokenHeuristic::orderTokens(tokensToOrder, referenceToken.getId(), TokenHeuristic::NEAR);
    assertTrue(earliest(tokensToOrder[0]) ==  earliest(referenceToken));
    assertTrue(abs((int) (earliest(tokensToOrder.back()))) == abs(baseTime + increment),
	       toString(earliest(tokensToOrder.back())));

    // FAR: The opposite of near
    TokenHeuristic::orderTokens(tokensToOrder, referenceToken.getId(), TokenHeuristic::FAR);
    assertTrue(earliest(tokensToOrder.back()) == earliest(referenceToken));
    assertTrue(abs((int) (earliest(tokensToOrder[0]))) == abs(baseTime + increment),
	       toString(earliest(tokensToOrder[0])));

    cleanup(tokensToOrder);
    return true;
  }

  static int earliest(const OrderingChoice& orderingChoice){
    return earliest(orderingChoice.second.second);
  }

  static bool testTokenOrderingChoices(){
    DEFAULT_SETUP(ce,db,false);      
    Object o1(db.getId(), "Object", "o1");
    db.close();               

    std::vector< OrderingChoice > tokensToOrder;
    std::vector<TokenId> tokensToDelete;

    IntervalToken referenceToken(db.getId(),  
				 "Object.PredicateA",                                                     
				 true,                                                               
				 IntervalIntDomain(-50, -50),                                           
				 IntervalIntDomain(0, 0),                                           
				 IntervalIntDomain(1, PLUS_INFINITY));

    // Allocate tokens to span before and after the refrence token, but not symmetrically so we can
    // be specific in picking best and worst
    const int baseTime(-500);
    const int increment(50);
    const int tokenCount(20);
    int newBase(baseTime);
    for(int i = 0; i < tokenCount; i++){
      newBase = baseTime + i * increment;
      TokenId token = (new IntervalToken(db.getId(),  
					 "Object.PredicateA",                                                     
					 true,                                                               
					 IntervalIntDomain(newBase, newBase),
					 IntervalIntDomain(newBase + increment, newBase + increment),
					 IntervalIntDomain(1, PLUS_INFINITY)))->getId();
      OrderingChoice choice(o1.getId(), std::pair<TokenId, TokenId>(referenceToken.getId(), token));
      tokensToOrder.push_back(choice);
      tokensToDelete.push_back(token);
    }


    // LATE: Ensure the earliest token is the last in the sequence. and latest is first
    TokenHeuristic::orderTokens(tokensToOrder, referenceToken.getId(), TokenHeuristic::LATE);
    assertTrue(earliest(tokensToOrder.back()) == baseTime);
    assertTrue(earliest(tokensToOrder[0]) == baseTime + (increment * (tokenCount-1)),
	       toString(earliest(tokensToOrder.back()))); 

    // EARLY && NONE: Ensure the opposite
    TokenHeuristic::orderTokens(tokensToOrder, referenceToken.getId(), TokenHeuristic::EARLY);
    assertTrue(earliest(tokensToOrder.back()) == baseTime + (increment * (tokenCount-1)));
    assertTrue(earliest(tokensToOrder[0]) == baseTime,
	       toString(earliest(tokensToOrder.back()))); 

    TokenHeuristic::orderTokens(tokensToOrder, referenceToken.getId(), TokenHeuristic::NONE);
    assertTrue(earliest(tokensToOrder.back()) == baseTime + (increment * (tokenCount-1)));
    assertTrue(earliest(tokensToOrder[0]) == baseTime,
	       toString(earliest(tokensToOrder.back()))); 

    // NEAR: First should be the same bounds. Latest should be either extreme.
    TokenHeuristic::orderTokens(tokensToOrder, referenceToken.getId(), TokenHeuristic::NEAR);
    assertTrue(earliest(tokensToOrder[0]) ==  earliest(referenceToken));
    assertTrue(abs((int) (earliest(tokensToOrder.back()))) == abs(baseTime + increment),
	       toString(earliest(tokensToOrder.back())));

    // FAR: The opposite of near
    TokenHeuristic::orderTokens(tokensToOrder, referenceToken.getId(), TokenHeuristic::FAR);
    assertTrue(earliest(tokensToOrder.back()) == earliest(referenceToken));
    assertTrue(abs((int) (earliest(tokensToOrder[0]))) == abs(baseTime + increment),
	       toString(earliest(tokensToOrder[0])));

    cleanup(tokensToDelete);
    return true;
  }

  static bool testVariableValueOrderingChoices(){
    DEFAULT_SETUP(ce,db,false);      
    // Want a bunch of objects - note the names will not match the keys. 
    Object o3(db.getId(), "Object", "o3");
    Object o2(db.getId(), "Object", "o2");
    Object o4(db.getId(), "Object", "o4");
    Object o5(db.getId(), "Object", "o5");
    Object o1(db.getId(), "Object", "o1");
    Object o6(db.getId(), "Object", "o6");
    db.close();

    // Start with an object variable
    Variable<ObjectDomain> v0(ce.getId(), ObjectDomain("Object"));
    db.makeObjectVariableFromType("Object", v0.getId());
    std::list<double> initialValues;
    v0.baseDomain().getValues(initialValues);

    // Define an enumeration - it prunes. It also orders by an ordering that I would not otherwise expect
    std::list<double> values;
    values.push_back(LabelStr("o5"));
    values.push_back(LabelStr("o1"));
    values.push_back(LabelStr("o3"));
    {
      std::list<double> orderedChoices = initialValues;
      VariableHeuristic::orderChoices(db.getId(), v0.getId(), values, VariableHeuristic::ENUMERATION, orderedChoices);
      assertTrue(orderedChoices.size() == values.size(), toString(orderedChoices.size()));
      assertTrue(orderedChoices.front() == o5.getId());
      assertTrue(orderedChoices.back() == o3.getId());
    }

    // Go for ascending order by key. This will give order of initialization. No pruning
    {
      std::list<double> orderedChoices = initialValues;
      VariableHeuristic::orderChoices(db.getId(), v0.getId(), VariableHeuristic::noValues(), 
				      VariableHeuristic::ASCENDING, orderedChoices);
      assertTrue(orderedChoices.size() == initialValues.size(), toString(orderedChoices.size()));
      assertTrue(orderedChoices.front() == o3.getId(), ((ObjectId) orderedChoices.front())->getName().toString());
      assertTrue(orderedChoices.back() == o6.getId(), ((ObjectId) orderedChoices.back())->getName().toString());
    }

    // Go for descending order by key. This will give reverse order of initialization. No pruning
    {
      std::list<double> orderedChoices = initialValues;
      VariableHeuristic::orderChoices(db.getId(), v0.getId(), VariableHeuristic::noValues(), 
				      VariableHeuristic::DESCENDING, orderedChoices);
      assertTrue(orderedChoices.size() == initialValues.size(), toString(orderedChoices.size()));
      assertTrue(orderedChoices.back() == o3.getId());
      assertTrue(orderedChoices.front() == o6.getId());
    }

    // Now lets go for no intersection - empty choices.
    values.clear();
    values.push_back(LabelStr("o5"));
    values.push_back(LabelStr("o1"));
    values.push_back(LabelStr("o3"));
    v0.specify(o4.getId());

    {
      std::list<double> orderedChoices;
      v0.lastDomain().getValues(orderedChoices);
      VariableHeuristic::orderChoices(db.getId(), v0.getId(), values, VariableHeuristic::ENUMERATION, orderedChoices);
      assertTrue(orderedChoices.empty());
    }

    // Now lets try a BoolDomain
    Variable<BoolDomain> v1(ce.getId(), BoolDomain());
    initialValues.clear();
    v1.baseDomain().getValues(initialValues);

    // Define an enumeration - it prunes. 
    values.clear();
    values.push_back(true);
    {
      std::list<double> orderedChoices = initialValues;
      VariableHeuristic::orderChoices(db.getId(), v1.getId(), values, VariableHeuristic::ENUMERATION, orderedChoices);
      assertTrue(orderedChoices.size() == values.size(), toString(orderedChoices.size()));
      assertTrue(orderedChoices.front() == true);
    }

    // Now lets reverse the order
    {
      std::list<double> orderedChoices = initialValues;
      VariableHeuristic::orderChoices(db.getId(), v1.getId(), VariableHeuristic::noValues(), 
				      VariableHeuristic::DESCENDING, orderedChoices);
      assertTrue(orderedChoices.size() == initialValues.size(), toString(orderedChoices.size()));
      assertTrue(orderedChoices.back() == false);
      assertTrue(orderedChoices.front() == true);
    }

    return true;
  }

};

int main() {
  LockManager::instance().connect();
  LockManager::instance().lock();

  Schema::instance();

  LockManager::instance().unlock();

  for (int i = 0; i < 1; i++) {
    LockManager::instance().lock();
    runTestSuite(HeuristicsEngineTest::test);
    runTestSuite(HeuristicsTest::test);
    LockManager::instance().unlock();
  }
  LockManager::instance().lock();

  std::cout << "Finished" << std::endl;
  ConstraintLibrary::purgeAll();
  exit(0);
}
