#include "solvers-test-module.hh"
#include "Nddl.hh"
#include "StandardAssembly.hh"
#include "Solver.hh"
#include "ComponentFactory.hh"
#include "Constraint.hh"
#include "ConstraintLibrary.hh"
#include "UnboundVariableManager.hh"
#include "OpenConditionManager.hh"
#include "ThreatManager.hh"
#include "Object.hh"
#include "Filters.hh"
#include "IntervalToken.hh"
#include "TokenVariable.hh"
#include "TestSupport.hh"
#include "Debug.hh"
#include "Variable.hh"
#include "IntervalDomain.hh"
#include "IntervalIntDomain.hh"
#include "EnumeratedDomain.hh"
#include "MatchingEngine.hh"
#include "PlanDatabaseWriter.hh"

/**
 * @file Provides module tests for Solver Module.
 * @author Conor McGann
 * @date May, 2005
 */



using namespace EUROPA;
using namespace EUROPA::SOLVERS;

/**
 * @brief Test Constraint to only fire when all values are singletons and to then
 * require that all values are different. Deliberately want to force an inefficient search with
 * lots of backtrack.
 */
class LazyAllDiff: public Constraint {
public:
  LazyAllDiff(const LabelStr& name,
	      const LabelStr& propagatorName,
	      const ConstraintEngineId& constraintEngine,
	      const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
  }

  void handleExecute() {
    std::set<double> singletonValues;
    std::vector<ConstrainedVariableId>::const_iterator it_end = getScope().end();
    for(std::vector<ConstrainedVariableId>::const_iterator it = getScope().begin(); it != it_end; ++it){
      ConstrainedVariableId var = *it;
      if(getCurrentDomain(var).isSingleton())
	singletonValues.insert(getCurrentDomain(var).getSingletonValue());
      else
	return;
    }

    if(singletonValues.size() < getScope().size())
      getCurrentDomain(getScope().front()).empty();
  }
};

/**
 * @brief Test Constraint to only fire when all values are singletons and to then always fail. Used to force exhaustive search.
 */
class LazyAlwaysFails: public Constraint {
public:
  LazyAlwaysFails(const LabelStr& name,
	      const LabelStr& propagatorName,
	      const ConstraintEngineId& constraintEngine,
	      const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables) {
  }

  void handleExecute() {
    std::vector<ConstrainedVariableId>::const_iterator it_end = getScope().end();
    for(std::vector<ConstrainedVariableId>::const_iterator it = getScope().begin(); it != it_end; ++it){
      ConstrainedVariableId var = *it;
      if(!getCurrentDomain(var).isSingleton())
	return;
    }

    getCurrentDomain(getScope().front()).empty();
  }
};

class TestComponent: public Component{
public:
  TestComponent(const TiXmlElement& configData): Component(configData){s_counter++;}

  static void reset(){s_counter = 0;}

  static unsigned int counter(){return s_counter;}

private:
  static unsigned int s_counter;
};

unsigned int TestComponent::s_counter(0);

class ComponentFactoryTests{
public:
  static bool test(){
    runTest(testBasicAllocation);
    return true;
  }

private:
  static bool testBasicAllocation(){
    TiXmlElement* configXml = initXml((getTestLoadLibraryPath() + "/ComponentFactoryTest.xml").c_str());

    for (TiXmlElement * child = configXml->FirstChildElement(); 
	 child != NULL; 
	 child = child->NextSiblingElement()) {

      TestComponent * testComponent = static_cast<TestComponent*>(Component::AbstractFactory::allocate(*child));
      delete testComponent;
    }

    assert(TestComponent::counter() == 5);

    delete configXml;

    return true;
  }
};

class FilterTests {
public:
  static bool test(){
    runTest(testRuleMatching);
    runTest(testVariableFiltering);
    runTest(testTokenFiltering);
    runTest(testThreatFiltering);
    return true;
  }

private:
  static bool testRuleMatching() {
    TiXmlElement* root = initXml( (getTestLoadLibraryPath() + "/RuleMatchingTests.xml").c_str(), "MatchingEngine");
    MatchingEngine me(*root);
    assertTrue(me.ruleCount() == 13, toString(me.ruleCount()));
    assertTrue(me.hasRule("[R0]*.*.*.*.*.*"));
    assertTrue(me.hasRule("[R1]*.*.start.*.*.*"));
    assertTrue(me.hasRule("[R2]*.*.arg3.*.*.*"));
    assertTrue(me.hasRule("[R3]*.predicateF.*.*.*.*"));
    assertTrue(me.hasRule("[R4]*.predicateC.arg6.*.*.*"));
    assertTrue(me.hasRule("[R5]C.predicateC.*.*.*.*"));
    assertTrue(me.hasRule("[R6]C.*.*.*.*.*"));
    assertTrue(me.hasRule("[R7]*.*.duration.*.Object.*"));
    assertTrue(me.hasRule("[R7a]*.*.duration.none.*.*"));
    assertTrue(me.hasRule("[R8]*.*.*.*.B.*"));
    assertTrue(me.hasRule("[R9]*.*.*.meets.D.predicateG"));
    assertTrue(me.hasRule("[R10]*.*.*.before.*.*"));
    assertTrue(me.hasRule("[R11]*.*.neverMatched.*.*.*"));

    StandardAssembly assembly(Schema::instance());
    PlanDatabaseId db = assembly.getPlanDatabase();
    Object o1(db, "A", "o1");
    Object o2(db, "D", "o2");
    Object o3(db, "C", "o3");
    Object o4(db, "E", "o4");
    db->close();

    // test RO
    {
      Variable<IntervalIntDomain> v0(assembly.getConstraintEngine(), IntervalIntDomain(0, 10), true, "v0");
      std::vector<MatchingRuleId> rules;
      me.getVariableMatches(v0.getId(), rules);
      assertTrue(rules.size() == 1, toString(rules.size()));
      assertTrue(rules[0]->toString() == "[R0]*.*.*.*.*.*", rules[0]->toString());
    }

    // test R1 
    {
      IntervalToken token(db, 
			  "A.predicateA", 
			  true, 
			  IntervalIntDomain(0, 1000),
			  IntervalIntDomain(0, 1000),
			  IntervalIntDomain(2, 10),
			  Token::noObject(), true);

      std::vector<MatchingRuleId> rules;
      me.getVariableMatches(token.getStart(), rules);
      assertTrue(rules.size() == 2, toString(rules.size()));
      assertTrue(rules[1]->toString() == "[R1]*.*.start.*.*.*", rules[1]->toString());
    }

    // test R2 
    {
      Variable<IntervalIntDomain> v0(assembly.getConstraintEngine(), IntervalIntDomain(0, 10), true, "arg3");
      std::vector<MatchingRuleId> rules;
      me.getVariableMatches(v0.getId(), rules);
      assertTrue(rules.size() == 2, toString(rules.size()));
      assertTrue(rules[1]->toString() == "[R2]*.*.arg3.*.*.*", rules[1]->toString());
    }

    // test R3 
    {
      TokenId token = db->getClient()->createToken("D.predicateF", false);
      std::vector<MatchingRuleId> rules;
      me.getTokenMatches(token, rules);
      assertTrue(rules.size() == 2, toString(rules.size()));
      assertTrue(rules[1]->toString() == "[R3]*.predicateF.*.*.*.*", rules[1]->toString());
      token->discard();
    }

    // test R4
    {
      TokenId token = db->getClient()->createToken("D.predicateC", false);
      std::vector<MatchingRuleId> rules;
      me.getVariableMatches(token->getVariable("arg6"), rules);
      assertTrue(rules.size() == 2, toString(rules.size()));
      assertTrue(rules[1]->toString() == "[R4]*.predicateC.arg6.*.*.*", rules[1]->toString());
      token->discard();
    }

    // test R5 & R6
    {
      TokenId token = db->getClient()->createToken("C.predicateC", false);
      std::vector<MatchingRuleId> rules;
      me.getTokenMatches(token, rules);
      assertTrue(rules.size() == 3, toString(rules.size()) + " for " + token->getUnqualifiedPredicateName().toString());
      assertTrue(rules[1]->toString() == "[R5]C.predicateC.*.*.*.*", rules[1]->toString());
      assertTrue(rules[2]->toString() == "[R6]C.*.*.*.*.*", rules[2]->toString());
      token->discard();
    }

    // test R6
    {
      TokenId token = db->getClient()->createToken("C.predicateA", false);
      std::vector<MatchingRuleId> rules;
      me.getTokenMatches(token, rules);
      assertTrue(rules.size() == 2, toString(rules.size()) + " for " + token->getUnqualifiedPredicateName().toString());
      assertTrue(rules[1]->toString() == "[R6]C.*.*.*.*.*", rules[1]->toString());
      token->discard();
    }

    // test R7
    {
      TokenId token = db->getClient()->createToken("D.predicateF", false);
      token->activate();
      TokenId E_predicateC = *(token->getSlaves().begin());
      std::vector<MatchingRuleId> rules;
      me.getVariableMatches(E_predicateC->getDuration(), rules);
      assertTrue(rules.size() == 2, toString(rules.size()) + " for " + token->getUnqualifiedPredicateName().toString());
      assertTrue(rules[1]->toString() == "[R7]*.*.duration.*.Object.*", rules[1]->toString());
      token->discard();
    }

    // test R7a
    {
      TokenId token = db->getClient()->createToken("E.predicateC", false);
      std::vector<MatchingRuleId> rules;
      me.getVariableMatches(token->getDuration(), rules);
      assertTrue(rules.size() == 2, toString(rules.size()) + " for " + token->getPredicateName().toString());
      assertTrue(rules[1]->toString() == "[R7a]*.*.duration.none.*.*", rules[1]->toString());
      token->discard();
    }

    // test R8
    {
      TokenId token = db->getClient()->createToken("B.predicateC", false);
      token->activate();
      TokenId E_predicateC = *(token->getSlaves().begin());
      std::vector<MatchingRuleId> rules;
      me.getVariableMatches(E_predicateC->getDuration(), rules);
      assertTrue(rules.size() == 3, toString(rules.size()) + " for " + token->getPredicateName().toString());
      assertTrue(rules[1]->toString() == "[R8]*.*.*.*.B.*", rules[1]->toString());
      assertTrue(rules[2]->toString() == "[R7]*.*.duration.*.Object.*", rules[2]->toString());
      token->discard();
    }

    // test R*, R9 and R10
    {
      TokenId token = db->getClient()->createToken("D.predicateG", false);
      token->activate();
      TokenId E_predicateC = *(token->getSlaves().begin());

      // Expect to fire R8, R9 and R10
      std::set<LabelStr> expectedRules;
      expectedRules.insert(LabelStr("[R8]*.*.*.*.B.*"));
      expectedRules.insert(LabelStr("[R9]*.*.*.meets.D.predicateG"));
      expectedRules.insert(LabelStr("[R10]*.*.*.before.*.*"));
      std::vector<MatchingRuleId> rules;
      me.getVariableMatches(E_predicateC->getDuration(), rules);
      assertTrue(rules.size() == 4, toString(rules.size()) + " for " + token->getPredicateName().toString());
      for(int i=0;i>4; i++)
	assertTrue(expectedRules.find(LabelStr(rules[i]->toString())) != expectedRules.end(), rules[i]->toString());

      token->discard();
    }

    return true;
  }

  static bool testVariableFiltering(){
    TiXmlElement* root = initXml( (getTestLoadLibraryPath() + "/FlawFilterTests.xml").c_str(), "UnboundVariableManager");

    StandardAssembly assembly(Schema::instance());
    UnboundVariableManager fm(*root);
    assert(assembly.playTransactions( (getTestLoadLibraryPath() + "/UnboundVariableFiltering.xml").c_str() ));

    // Initialize after filling the database since we are not connected to an event source
    fm.initialize(assembly.getPlanDatabase());

    // Set the horizon
    IntervalIntDomain& horizon = HorizonFilter::getHorizon();
    horizon = IntervalIntDomain(0, 1000);

    // Simple filter on a variable
    ConstrainedVariableSet variables = assembly.getConstraintEngine()->getVariables();
    for(ConstrainedVariableSet::const_iterator it = variables.begin(); it != variables.end(); ++it){
      ConstrainedVariableId var = *it;

      // Confirm temporal variables have been excluded
      static const LabelStr excludedVariables(":start:end:duration:arg1:arg3:arg4:arg6:arg7:arg8:filterVar:");
      static const LabelStr includedVariables(":arg2:arg5:keepVar:");
      std::string s = ":" + var->getName().toString() + ":";
      if(excludedVariables.contains(s))
	assertTrue(!fm.inScope(var), var->toString())
      else if(includedVariables.contains(s))
	assertTrue(fm.inScope(var), var->toString());
    }

    // Confirm that a global variable is first a flaw, but when bound is no longer a flaw, and when bound again,
    // returns as a flaw
    ConstrainedVariableId globalVar1 = assembly.getPlanDatabase()->getGlobalVariable("globalVariable1");
    ConstrainedVariableId globalVar2 = assembly.getPlanDatabase()->getGlobalVariable("globalVariable2");
    ConstrainedVariableId globalVar3 = assembly.getPlanDatabase()->getGlobalVariable("globalVariable3");
    assertTrue(!fm.inScope(globalVar1));
    assertTrue(fm.inScope(globalVar2));
    globalVar2->specify(globalVar2->lastDomain().getLowerBound());
    assembly.getConstraintEngine()->propagate();
    assertTrue(!fm.inScope(globalVar2));
    assertFalse(fm.inScope(globalVar1)); // By propagation it will be a singleton, so it will be Excluded
    globalVar2->reset();
    assembly.getConstraintEngine()->propagate();
    assertTrue(!fm.inScope(globalVar1));
    assertTrue(fm.inScope(globalVar2));

    assertTrue(!fm.inScope(globalVar3));

    return true;
  }

  static bool testTokenFiltering(){
    TiXmlElement* root = initXml((getTestLoadLibraryPath() + "/FlawFilterTests.xml").c_str(), "OpenConditionManager");

    StandardAssembly assembly(Schema::instance());
    OpenConditionManager fm(*root);
    IntervalIntDomain& horizon = HorizonFilter::getHorizon();
    horizon = IntervalIntDomain(0, 1000);
    assert(assembly.playTransactions((getTestLoadLibraryPath() + "/OpenConditionFiltering.xml").c_str() ));

    // Initialize with data in the database
    fm.initialize(assembly.getPlanDatabase());

    TokenSet tokens = assembly.getPlanDatabase()->getTokens();
    for(TokenSet::const_iterator it = tokens.begin(); it != tokens.end(); ++it){
      static const LabelStr excludedPredicates(":D.predicateA:D.predicateB:D.predicateC:E.predicateC:HorizonFiltered.predicate1:HorizonFiltered.predicate2:HorizonFiltered.predicate5:");
      TokenId token = *it;
      std::string s = ":" + token->getPredicateName().toString() + ":";
      if(excludedPredicates.contains(s))
	assertTrue(!fm.inScope(token), token->toString() + " is in scope after all.")
      else
	assertTrue(token->isActive() || fm.inScope(token), token->toString() + " is not in scope and not active.");
    }

    return true;
  }


  static bool testThreatFiltering(){
    TiXmlElement* root = initXml( (getTestLoadLibraryPath() + "/FlawFilterTests.xml" ).c_str(), "ThreatManager");

    StandardAssembly assembly(Schema::instance());
    ThreatManager fm(*root);
    IntervalIntDomain& horizon = HorizonFilter::getHorizon();
    horizon = IntervalIntDomain(0, 1000);
    assert(assembly.playTransactions(( getTestLoadLibraryPath() + "/ThreatFiltering.xml").c_str()));

    // Initialize with data in the database
    fm.initialize(assembly.getPlanDatabase());

    TokenSet tokens = assembly.getPlanDatabase()->getTokens();
    for(TokenSet::const_iterator it = tokens.begin(); it != tokens.end(); ++it){
      static const LabelStr excludedPredicates(":D.predicateA:D.predicateB:D.predicateC:E.predicateC:HorizonFiltered.predicate1:HorizonFiltered.predicate2:HorizonFiltered.predicate5:");
      TokenId token = *it;
      assertTrue(token->isActive() || !fm.inScope(token), token->toString() + " is not in scope and not active.");
      std::string s = ":" + token->getPredicateName().toString() + ":";
      if(excludedPredicates.contains(s))
	assertTrue(!fm.inScope(token), token->toString() + " is in scope after all.")
    }

    return true;
  }
};

class FlawHandlerTests {
public:
  static bool test(){
    runTest(testPriorities);
    runTest(testGuards);
    runTest(testDynamicFlawManagement);
    runTest(testDefaultVariableOrdering);
    runTest(testHeuristicVariableOrdering);
    return true;
  }

private:
  static bool testPriorities(){
    TiXmlElement* root = initXml( (getTestLoadLibraryPath() + "/FlawHandlerTests.xml").c_str(), "TestPriorities");
    MatchingEngine me(*root, "FlawHandler");
    StandardAssembly assembly(Schema::instance());
    PlanDatabaseId db = assembly.getPlanDatabase();
    Object o1(db, "A", "o1");
    Object o2(db, "D", "o2");
    Object o3(db, "C", "o3");
    Object o4(db, "E", "o4");
    db->close();

    // test H0
    {
      Variable<IntervalIntDomain> v0(assembly.getConstraintEngine(), IntervalIntDomain(0, 10), true, "v0");
      std::vector<MatchingRuleId> rules;
      me.getVariableMatches(v0.getId(), rules);
      assertTrue(rules.size() == 1, toString(rules.size()));
      FlawHandlerId flawHandler = rules[0];
      assertTrue(flawHandler->getPriority() == 1000, toString(flawHandler->getPriority()));
      assertTrue(flawHandler->getWeight() == 199000, toString(flawHandler->getWeight()));
    }

    // test H1
    {
      Variable<IntervalIntDomain> v0(assembly.getConstraintEngine(), IntervalIntDomain(0, 10), true, "start");
      std::vector<MatchingRuleId> rules;
      me.getVariableMatches(v0.getId(), rules);
      assertTrue(rules.size() == 2, toString(rules.size()));
      FlawHandlerId flawHandler = rules[1];
      assertTrue(flawHandler->getPriority() == 1000, toString(flawHandler->getPriority()));
      assertTrue(flawHandler->getWeight() == 299000, toString(flawHandler->getWeight()));
    }

    // test H2
    {
      Variable<IntervalIntDomain> v0(assembly.getConstraintEngine(), IntervalIntDomain(0, 10), true, "end");
      std::vector<MatchingRuleId> rules;
      me.getVariableMatches(v0.getId(), rules);
      assertTrue(rules.size() == 2, toString(rules.size()));
      FlawHandlerId flawHandler = rules[1];
      assertTrue(flawHandler->getPriority() == 200, toString(flawHandler->getPriority()));
      assertTrue(flawHandler->getWeight() == 299800, toString(flawHandler->getWeight()));
    }

    // test H3
    {
      TokenId token = db->getClient()->createToken("D.predicateG", false);
      token->activate();
      TokenId E_predicateC = *(token->getSlaves().begin());
      std::vector<MatchingRuleId> rules;
      me.getVariableMatches(E_predicateC->getEnd(), rules);
      assertTrue(rules.size() == 3, toString(rules.size()));
      FlawHandlerId flawHandler = rules[2];
      assertTrue(flawHandler->getPriority() == 1, toString(flawHandler->getPriority()));
      assertTrue(flawHandler->getWeight() == 399999, toString(flawHandler->getWeight()));
      token->discard();
    }

    return true;
  }

  static bool testGuards(){
    TiXmlElement* root = initXml( (getTestLoadLibraryPath() + "/FlawHandlerTests.xml").c_str(), "TestGuards");
    MatchingEngine me(*root, "FlawHandler");
    StandardAssembly assembly(Schema::instance());
    PlanDatabaseId db = assembly.getPlanDatabase();
    Object o1(db, "A", "o1");
    Object o2(db, "D", "o2");
    Object o3(db, "C", "o3");
    Object o4(db, "E", "o4");
    Object o5(db, "D", "o5");
    db->close();

    // test H0
    {
      TokenId token = db->getClient()->createToken("D.predicateG", false);
      std::vector<MatchingRuleId> rules;
      me.getTokenMatches(token, rules);
      assertTrue(rules.size() == 1, toString(rules.size()));
      FlawHandlerId flawHandler = rules[0];
      std::vector<ConstrainedVariableId> guards;
      assertTrue(flawHandler->makeConstraintScope(token, guards));
      assertTrue(guards.size() == 2, toString(guards.size()));
      assertTrue(guards[0] == token->getStart(), guards[0]->toString());
      assertTrue(guards[1] == token->getObject(), guards[1]->toString());
      assertFalse(flawHandler->test(guards));
      token->getStart()->specify(30);
      assertFalse(flawHandler->test(guards));
      token->getObject()->specify(o2.getId());
      assertTrue(flawHandler->test(guards));
      token->discard();
    }

    // test H1
    {
      TokenId token = db->getClient()->createToken("C.predicateA", false);
      std::vector<MatchingRuleId> rules;
      me.getTokenMatches(token, rules);
      assertTrue(rules.size() == 1, toString(rules.size()));
      FlawHandlerId flawHandler = rules[0];
      std::vector<ConstrainedVariableId> guards;
      assertFalse(flawHandler->makeConstraintScope(token, guards));
      token->discard();
    }

    // test H2
    {
      TokenId token = db->getClient()->createToken("B.predicateC", false);
      {
	std::vector<MatchingRuleId> rules;
	me.getTokenMatches(token, rules);
	assertTrue(rules.size() == 1, toString(rules.size()));
	FlawHandlerId flawHandler = rules[0];
	std::vector<ConstrainedVariableId> guards;
	assertFalse(flawHandler->makeConstraintScope(token, guards));
      }
      // Now fire on the subgoal. Rule will match as it has a master
      token->activate();
      TokenId B_predicateC = *(token->getSlaves().begin());
      std::vector<MatchingRuleId> rules;
      me.getTokenMatches(B_predicateC, rules);
      assertTrue(rules.size() == 1, toString(rules.size()));
      FlawHandlerId flawHandler = rules[0];
      std::vector<ConstrainedVariableId> guards;
      assertTrue(flawHandler->makeConstraintScope(B_predicateC, guards));
      assertFalse(flawHandler->test(guards));
      // Specify the master guard variable
      token->getStart()->specify(30);
      assertTrue(flawHandler->test(guards));
      token->discard();
    }

    // test H3
    {
      Variable<IntervalIntDomain> v0(assembly.getConstraintEngine(), IntervalIntDomain(0, 10), true, "FreeVariable");
      std::vector<MatchingRuleId> rules;
      me.getVariableMatches(v0.getId(), rules);
      assertTrue(rules.size() == 1, toString(rules.size()));
      FlawHandlerId flawHandler = rules[0];
      std::vector<ConstrainedVariableId> guards;
      assertFalse(flawHandler->makeConstraintScope(v0.getId(), guards));
    }

    return true;
  }

  static bool testDynamicFlawManagement(){
    TiXmlElement* root = initXml( (getTestLoadLibraryPath() + "/FlawHandlerTests.xml").c_str(), "TestDynamicFlaws");
    TiXmlElement* child = root->FirstChildElement();
    StandardAssembly assembly(Schema::instance());
    PlanDatabaseId db = assembly.getPlanDatabase();
    Object o1(db, "A", "o1");
    Object o2(db, "D", "o2");
    Object o3(db, "C", "o3");
    Object o4(db, "E", "o4");
    Object o5(db, "D", "o5");
    db->close();
    Solver solver(assembly.getPlanDatabase(), *child);

    // test basic flaw filtering and default handler access
    {
      TokenId token = db->getClient()->createToken("D.predicateG", false);
      db->getConstraintEngine()->propagate();
      // Initially the token is in scope and the variable is not
      assertTrue(solver.inScope(token));
      assertTrue(!solver.inScope(token->getStart()));
      assertTrue(solver.getFlawHandler(token->getStart()).isNoId());
      assertTrue(solver.getFlawHandler(token).isValid());

      // Activate the token. The variable will still no be in scope since it is not finite.
      token->activate();
      db->getConstraintEngine()->propagate();
      assertTrue(solver.getFlawHandler(token->getStart()).isNoId());

      // The token should not be a flaw since it is nota timeline!
      assertTrue(solver.getFlawHandler(token).isNoId());

      // Restrict the base domain to finite bounds for the start variable
      token->getStart()->restrictBaseDomain(IntervalIntDomain(0, 50));
      db->getConstraintEngine()->propagate();
      assertTrue(solver.getFlawHandler(token->getStart()).isValid());

      // Now insert the token and bind the variable
      token->getStart()->specify(30);
      db->getConstraintEngine()->propagate();
      assertTrue(!solver.inScope(token));
      assertTrue(!solver.inScope(token->getStart()));

      // Reset the variable and it should be back in business
      token->getStart()->reset();
      db->getConstraintEngine()->propagate();
      assertTrue(solver.getFlawHandler(token->getStart()).isValid());

      // Deactivation of the token will introduce it as a flaw, and nuke the start variable
      token->cancel();
      db->getConstraintEngine()->propagate();
      assertTrue(solver.inScope(token));
      assertTrue(!solver.inScope(token->getStart()));

      // Now activate it and the variable should be back
      token->activate();
      db->getConstraintEngine()->propagate();
      assertTrue(solver.getFlawHandler(token->getStart()).isId());

      // Restrict the base domain of the variable to a singleton. It should no longer be a flaw
      token->getStart()->restrictBaseDomain(IntervalIntDomain(0, 0));
      db->getConstraintEngine()->propagate();
      assertTrue(solver.getFlawHandler(token->getStart()).isNoId());

      token->discard();
    }

    // Now handle a case with increasingly restrictive filters
    {
      TokenId master = db->getClient()->createToken("D.predicateF", false);
      db->getConstraintEngine()->propagate();
      master->activate();
      TokenId slave = master->getSlave(1);
      assertTrue(slave->getPredicateName() == LabelStr("D.predicateC"), slave->getPredicateName().toString());

      // With no guards set, we should just get the default priority
      db->getConstraintEngine()->propagate();
      assertTrue(solver.getFlawHandler(slave)->getPriority() == 99999);

      slave->getStart()->specify(10);
      db->getConstraintEngine()->propagate();
      assertTrue(solver.getFlawHandler(slave)->getPriority() == 1);

      slave->getEnd()->specify(20);
      db->getConstraintEngine()->propagate();
      assertTrue(solver.getFlawHandler(slave)->getPriority() == 2);

      slave->getObject()->specify(o5.getId());
      db->getConstraintEngine()->propagate();
      assertTrue(solver.getFlawHandler(slave)->getPriority() == 3);

      master->getStart()->specify(10);
      db->getConstraintEngine()->propagate();
      assertTrue(solver.getFlawHandler(slave)->getPriority() == 4);

      master->getEnd()->specify(20);
      db->getConstraintEngine()->propagate();
      assertTrue(solver.getFlawHandler(slave)->getPriority() == 5);

      slave->getStart()->reset();
      slave->getStart()->specify(11);
      db->getConstraintEngine()->propagate();
      assertTrue(solver.getFlawHandler(slave)->getPriority() == 99999);

      master->discard();
    }

    return true;
  }

  static bool testDefaultVariableOrdering(){
    StandardAssembly assembly(Schema::instance());
    TiXmlElement* root = initXml( (getTestLoadLibraryPath() + "/FlawHandlerTests.xml").c_str(), "DefaultVariableOrdering");
    TiXmlElement* child = root->FirstChildElement();
    {
      assert(assembly.playTransactions( (getTestLoadLibraryPath() + "/StaticCSP.xml").c_str()));
      Solver solver(assembly.getPlanDatabase(), *child);
      assertTrue(solver.solve());
      assertTrue(solver.getStepCount() == solver.getDepth());
      assertTrue(solver.getStepCount() == 3, toString(solver.getStepCount()));
      ConstrainedVariableId v1 = assembly.getPlanDatabase()->getGlobalVariable("v1");
      assertTrue(v1->lastDomain().getSingletonValue() == 1, v1->toString());
      ConstrainedVariableId v2 = assembly.getPlanDatabase()->getGlobalVariable("v2");
      assertTrue(v2->lastDomain().getSingletonValue() == 0, v2->toString());
    }

    return true;
  }

  static bool testHeuristicVariableOrdering(){
    StandardAssembly assembly(Schema::instance());
    TiXmlElement* root = initXml( (getTestLoadLibraryPath() + "/FlawHandlerTests.xml").c_str(), "HeuristicVariableOrdering");
    TiXmlElement* child = root->FirstChildElement();
    {
      assert(assembly.playTransactions( (getTestLoadLibraryPath() + "/StaticCSP.xml").c_str()));
      Solver solver(assembly.getPlanDatabase(), *child);
      assertTrue(solver.solve());
      assertTrue(solver.getStepCount() == solver.getDepth());
      assertTrue(solver.getStepCount() == 3, toString(solver.getStepCount()));
      ConstrainedVariableId v1 = assembly.getPlanDatabase()->getGlobalVariable("v1");
      assertTrue(v1->getSpecifiedValue() == 9, v1->toString());
      ConstrainedVariableId v2 = assembly.getPlanDatabase()->getGlobalVariable("v2");
      assertTrue(v2->getSpecifiedValue() == 10, v2->toString());
    }

    return true;
  }
};

class SolverTests {
public:
  static bool test(){
    runTest(testMinValuesSimpleCSP);
    runTest(testSuccessfulSearch);
    runTest(testExhaustiveSearch);
    runTest(testSimpleActivation);
    runTest(testSimpleRejection);
    runTest(testMultipleSearch);
    runTest(testOversearch);
    runTest(testBacktrackFirstDecisionPoint);
    runTest(testMultipleSolutionsSearch);
    return true;
  }

private:
  /**
   * @brief Will load an intial state and solve a csp with only variables.
   */
  static bool testMinValuesSimpleCSP(){
    StandardAssembly assembly(Schema::instance());
    TiXmlElement* root = initXml( (getTestLoadLibraryPath() + "/SolverTests.xml").c_str(), "SimpleCSPSolver");
    TiXmlElement* child = root->FirstChildElement();
    {
      assert(assembly.playTransactions( (getTestLoadLibraryPath() + "/StaticCSP.xml").c_str()));
      Solver solver(assembly.getPlanDatabase(), *child);
      assertTrue(solver.solve());
      assertTrue(solver.getStepCount() == solver.getDepth());
      const ConstrainedVariableSet& allVars = assembly.getPlanDatabase()->getGlobalVariables();
      for(ConstrainedVariableSet::const_iterator it = allVars.begin(); it != allVars.end(); ++it){
	ConstrainedVariableId var = *it;
	assertTrue(var->lastDomain().isSingleton());
      }

      // Run the solver again.
      assertTrue(solver.solve());
      assertTrue(solver.getStepCount() == 3);
      assertTrue(solver.getDepth() == 3);

      // Now clear it and run it again
      solver.reset();
      assertTrue(solver.solve());
      assertTrue(solver.getStepCount() == 3);
      assertTrue(solver.getDepth() == 3);

      // Now partially reset it, and run again
      solver.reset(1);
      assertTrue(solver.solve());
      assertTrue(solver.getStepCount() == 1);
      assertTrue(solver.getDepth() == 3);
 
      // Now we reset one decision, then clear it. Expect the solution and depth to be 1.
      solver.reset(1);
      solver.clear();
      assertTrue(solver.solve());
      assertTrue(solver.getStepCount() == 1);
      assertTrue(solver.getDepth() == 1);
    }

    return true;
  }


  static bool testSuccessfulSearch(){
    StandardAssembly assembly(Schema::instance());
    TiXmlElement* root = initXml( (getTestLoadLibraryPath() + "/SolverTests.xml").c_str(), "SimpleCSPSolver");
    TiXmlElement* child = root->FirstChildElement();
    {
      assert(assembly.playTransactions((getTestLoadLibraryPath() + "/SuccessfulSearch.xml").c_str()));
      Solver solver(assembly.getPlanDatabase(), *child);
      assertTrue(solver.solve());
    }
    return true;
  }

  static bool testExhaustiveSearch(){
    StandardAssembly assembly(Schema::instance());
    TiXmlElement* root = initXml((getTestLoadLibraryPath() + "/SolverTests.xml").c_str(), "SimpleCSPSolver");
    TiXmlElement* child = root->FirstChildElement();
    {
      assert(assembly.playTransactions((getTestLoadLibraryPath() + "/ExhaustiveSearch.xml").c_str()));
      Solver solver(assembly.getPlanDatabase(), *child);
      assertFalse(solver.solve());

      debugMsg("SolverTests:testExhaustinveSearch", "Step count == " << solver.getStepCount());

      const ConstrainedVariableSet& allVars = assembly.getPlanDatabase()->getGlobalVariables();
      unsigned int stepCount = 0;
      unsigned int product = 1;
      for(ConstrainedVariableSet::const_iterator it = allVars.begin(); it != allVars.end(); ++it){
	static const unsigned int baseDomainSize = (*it)->baseDomain().getSize();
	stepCount = stepCount + (product*baseDomainSize);
	product = product*baseDomainSize;
      }

      assertTrue(solver.getStepCount() == stepCount);
    }
    return true;
  }

  static bool testSimpleActivation() {
    StandardAssembly assembly(Schema::instance());
    TiXmlElement* root = initXml((getTestLoadLibraryPath() + "/SolverTests.xml").c_str(), "SimpleActivationSolver");
    TiXmlElement* child = root->FirstChildElement();
    {
      IntervalIntDomain& horizon = HorizonFilter::getHorizon();
      horizon = IntervalIntDomain(0, 1000);
      assert(assembly.playTransactions((getTestLoadLibraryPath() + "/SimpleActivation.xml").c_str()));
      Solver solver(assembly.getPlanDatabase(), *child);
      assertTrue(solver.solve());
    }

    return true;
  }

  static bool testSimpleRejection() {
    StandardAssembly assembly(Schema::instance());
    TiXmlElement* root = initXml((getTestLoadLibraryPath() + "/SolverTests.xml").c_str(), "SimpleRejectionSolver");
    TiXmlElement* child = root->FirstChildElement();
    {
      IntervalIntDomain& horizon = HorizonFilter::getHorizon();
      horizon = IntervalIntDomain(0, 1000);
      assert(assembly.playTransactions((getTestLoadLibraryPath() + "/SimpleRejection.xml").c_str()));
      Solver solver(assembly.getPlanDatabase(), *child);
      assertTrue(solver.solve(100, 100));
      assertTrue(assembly.getPlanDatabase()->getTokens().size() == 1, 
		 toString(assembly.getPlanDatabase()->getTokens().size()));
    }


    return true;
  }


  static bool testMultipleSearch(){
    StandardAssembly assembly(Schema::instance());
    TiXmlElement* root = initXml((getTestLoadLibraryPath() + "/SolverTests.xml").c_str(), "SimpleCSPSolver");
    TiXmlElement* child = root->FirstChildElement();

    // Call the solver
    Solver solver(assembly.getPlanDatabase(), *child);
    assertTrue(solver.solve());

    // Now modify the database and invoke the solver again. Ensure that it does work
    assert(assembly.playTransactions((getTestLoadLibraryPath() + "/SuccessfulSearch.xml").c_str()));
    assertTrue(solver.solve());
    assertTrue(solver.getDepth() > 0);

    return true;
  }

  //to test GNATS 3068
  static bool testOversearch() {
    StandardAssembly assembly(Schema::instance());
    TiXmlElement* root = initXml((getTestLoadLibraryPath() + "/SolverTests.xml").c_str(), "SimpleCSPSolver");
    TiXmlElement* child = root->FirstChildElement();

    assert(assembly.playTransactions((getTestLoadLibraryPath() +"/SuccessfulSearch.xml").c_str()));
    Solver solver(assembly.getPlanDatabase(), *child);
    solver.setMaxSteps(5); //arbitrary number of maximum steps
    assert(solver.solve(20)); //arbitrary number of steps < max
    
    return true;
  }

  static bool testBacktrackFirstDecisionPoint(){
    StandardAssembly assembly(Schema::instance());
    TiXmlElement* root = initXml((getTestLoadLibraryPath() + "/SolverTests.xml").c_str(), "BacktrackSolver");
    TiXmlElement* child = root->FirstChildElement();
    assert(assembly.playTransactions((getTestLoadLibraryPath() +"/BacktrackFirstDecision.xml").c_str()));
    Solver solver(assembly.getPlanDatabase(), *child);
    solver.setMaxSteps(5); //arbitrary number of maximum steps
    assert(solver.solve(20)); //arbitrary number of steps < max
    return true;
  }

  /**
   * Tests the ability to use backjumping and an outer loop to find multiple solutions. The loop used will
   * search for a given number of solutions. It will be bounded by a maximum number of iterations, and it will
   * have a cut-off for exploration within each ieration. The idea of backjumping:
   * 1. If I have a solution, invoking backjump(1) and then calling solve will yield the next alternative.
   * 2. If I call solve, and it returns that it has exhausted all poossibilities, then we have explored all
   * possible solutions. This is because a call to backjump will invoke backtrack, which may cause multiple decisions
   * to be popped and undone if they are exhausted.
   * 3. If we timeout, then we wish to explore an alternate path. This may or may not be a good idea. It assumes
   * we are better off trying anoher branch with the time we have rather than working the current one.
   */
  static bool testMultipleSolutionsSearch(){
    StandardAssembly assembly(Schema::instance());
    TiXmlElement* root = initXml( (getTestLoadLibraryPath() + "/SolverTests.xml").c_str(), "SimpleCSPSolver");
    TiXmlElement* child = root->FirstChildElement();
    assert(assembly.playTransactions( (getTestLoadLibraryPath() + "/StaticCSP.xml").c_str()));
    Solver solver(assembly.getPlanDatabase(), *child);
    assertTrue(solver.solve(10));
    assertTrue(solver.getStepCount() == solver.getDepth());

    unsigned int solutionLimit = 100;
    unsigned int solutionCount = 1;
    unsigned int timeoutCount = 0;
    unsigned int iterationCount = 0;
    unsigned int backjumpDistance = 1;
    while(iterationCount < solutionLimit && !solver.isExhausted()){
      debugMsg("SolverTests:testMultipleSolutionsSearch", "Solving for iteration " << iterationCount);

      unsigned int priorDepth = solver.getDepth();
      iterationCount++;
      solver.backjump(backjumpDistance);
      solver.solve(10);
      if(solver.noMoreFlaws()){
	solutionCount++;

	debugMsg("SolverTests:testMultipleSolutionsSearch", 
		 "Solution " << solutionCount << " found." << PlanDatabaseWriter::toString(assembly.getPlanDatabase()));

	backjumpDistance = 1;
      }
      else if(solver.isTimedOut()){
	// In the event of a tmeout, we may have backtracked to a lesser depth in searching, or we may have stopped
	// further down the stack. In the latter case we will have to backjump further.
	backjumpDistance = (solver.getDepth() > priorDepth ? solver.getDepth() - priorDepth : 1 );

	debugMsg("SolverTests:testMultipleSolutionsSearch", 
		 "Timed out on iteration " << iterationCount << " at depth " << solver.getDepth() << 
		 ". Backjump distance = " << backjumpDistance);

	timeoutCount++;
      }
      else {
	debugMsg("SolverTests:testMultipleSolutionsSearch", 
		 "Exhausted after iteration " << iterationCount);
      }
    }

    return true;
  }
};


class FlawIteratorTests {
public:
  static bool test() {
    runTest(testUnboundVariableFlawIteration);
    runTest(testThreatFlawIteration);
    runTest(testOpenConditionFlawIteration);
    runTest(testSolverIteration);
    return true;
  }
private:

  static bool testUnboundVariableFlawIteration() {
    TiXmlElement* root = initXml("FlawFilterTests.xml", "UnboundVariableManager");
    
    StandardAssembly assembly(Schema::instance());
    UnboundVariableManager fm(*root);
    fm.initialize(assembly.getPlanDatabase());
    assert(assembly.playTransactions("UnboundVariableFiltering.xml"));

    IntervalIntDomain& horizon = HorizonFilter::getHorizon();
    horizon = IntervalIntDomain(0, 1000);

    ConstrainedVariableSet variables = assembly.getConstraintEngine()->getVariables();
    IteratorId flawIterator = fm.createIterator();
    while(!flawIterator->done()) {
      const ConstrainedVariableId var = (const ConstrainedVariableId) flawIterator->next();
      if(var.isNoId())
	continue;
      assertTrue(fm.inScope(var));
      assertTrue(variables.find(var) != variables.end());
      variables.erase(var);
    }
    
    assertTrue(flawIterator->done());
    
    for(ConstrainedVariableSet::const_iterator it = variables.begin(); it != variables.end(); ++it)
      assertTrue(!fm.inScope(*it));

    delete (Iterator*) flawIterator;
    delete root;
    return true;
  }
  
  static bool testOpenConditionFlawIteration() {
    TiXmlElement* root = initXml("FlawFilterTests.xml", "OpenConditionManager");
    
    StandardAssembly assembly(Schema::instance());
    OpenConditionManager fm(*root);
    IntervalIntDomain& horizon = HorizonFilter::getHorizon();
    horizon = IntervalIntDomain(0, 1000);
    fm.initialize(assembly.getPlanDatabase());
    assert(assembly.playTransactions("OpenConditionFiltering.xml"));

    TokenSet tokens = assembly.getPlanDatabase()->getTokens();
    IteratorId flawIterator = fm.createIterator();
    
    while(!flawIterator->done()) {
      const TokenId token = (const TokenId) flawIterator->next();
      if(token.isNoId())
	continue;
      assertTrue(fm.inScope(token));
      assertTrue(tokens.find(token) != tokens.end());
      tokens.erase(token);
    }

    assertTrue(flawIterator->done());

    for(TokenSet::const_iterator it = tokens.begin(); it != tokens.end(); ++it)
      assertTrue(!fm.inScope(*it));

    delete (Iterator*) flawIterator;
    delete root;
    
    return true;
  }

  static bool testThreatFlawIteration() {
    TiXmlElement* root = initXml("FlawFilterTests.xml", "ThreatManager");

    StandardAssembly assembly(Schema::instance());
    ThreatManager fm(*root);
    IntervalIntDomain& horizon = HorizonFilter::getHorizon();
    horizon = IntervalIntDomain(0, 1000);
    fm.initialize(assembly.getPlanDatabase());
    assert(assembly.playTransactions("ThreatFiltering.xml"));

    TokenSet tokens = assembly.getPlanDatabase()->getTokens();
    IteratorId flawIterator = fm.createIterator();
    
    while(!flawIterator->done()) {
      const TokenId token = (const TokenId) flawIterator->next();
      if(token.isNoId())
	continue;
      assertTrue(fm.inScope(token));
      assertTrue(tokens.find(token) != tokens.end());
      tokens.erase(token);
    }

    assertTrue(flawIterator->done());

    for(TokenSet::const_iterator it = tokens.begin(); it != tokens.end(); ++it)
      assertTrue(!fm.inScope(*it));

    delete (Iterator*) flawIterator;
    delete root;
    return true;
  }

  static bool testSolverIteration() {
    TiXmlElement* root = initXml("IterationTests.xml", "Solver");
    StandardAssembly assembly(Schema::instance());
    ThreatManager tm(*(root->FirstChildElement("ThreatManager")));
    OpenConditionManager ocm(*(root->FirstChildElement("OpenConditionManager")));
    UnboundVariableManager uvm(*(root->FirstChildElement("UnboundVariableManager")));
    Solver solver(assembly.getPlanDatabase(), *root);

    IntervalIntDomain& horizon = HorizonFilter::getHorizon();
    horizon = IntervalIntDomain(0, 1000);

    assert(assembly.playTransactions("ThreatFiltering.xml"));
    //assert(assembly.playTransactions("OpenConditionFiltering.xml"));
    //assert(assembly.playTransactions("UnboundVariableFiltering.xml"));

    tm.initialize(assembly.getPlanDatabase());
    ocm.initialize(assembly.getPlanDatabase());
    uvm.initialize(assembly.getPlanDatabase());

    TokenSet tokens = assembly.getPlanDatabase()->getTokens();
    ConstrainedVariableSet vars = assembly.getConstraintEngine()->getVariables();

    IteratorId flawIterator = solver.createIterator();
    while(!flawIterator->done()) {
      const EntityId entity = flawIterator->next();
      if(entity.isNoId())
	continue;
      if(TokenId::convertable(entity)) {
	const TokenId tok = (const TokenId) entity;
	assertTrue(tm.inScope(tok) || ocm.inScope(tok));
	assertTrue(tokens.find(tok) != tokens.end());
	tokens.erase(tok);
      }
      else if(ConstrainedVariableId::convertable(entity)) {
	const ConstrainedVariableId var = (const ConstrainedVariableId) entity;
	assertTrue(uvm.inScope(var));
	assertTrue(vars.find(var) != vars.end());
	std::cerr << var->toString() << std::endl;
	vars.erase(var);
      }
      else
	assertTrue(false);
    }

    for(TokenSet::const_iterator it = tokens.begin(); it != tokens.end(); ++it)
      assertTrue(!tm.inScope(*it) && !ocm.inScope(*it));

    for(ConstrainedVariableSet::const_iterator it = vars.begin(); it != vars.end(); ++it) {
      std::cerr << (*it)->toString() << std::endl;
      assertTrue(!uvm.inScope(*it));
    }

    delete (Iterator*) flawIterator;
    delete root;

    return true;
  }
};

void initSolverModuleTests() {
 
  StandardAssembly::initialize();

  // Allocate the schema with a call to the linked in model function - eventually
  // make this called via dlopen
  NDDL::loadSchema();
 
}

void SolversModuleTests::runTests(std::string path) {
   setTestLoadLibraryPath(path);

   // For tests on th ematching engine
   REGISTER_COMPONENT_FACTORY(MatchingRule, MatchingRule);

   // Register components under program execution so that static allocation can have occurred
   // safely. This was required due to problems on the MAC.
   REGISTER_COMPONENT_FACTORY(TestComponent, A);
   REGISTER_COMPONENT_FACTORY(TestComponent, B);
   REGISTER_COMPONENT_FACTORY(TestComponent, C);
   REGISTER_COMPONENT_FACTORY(TestComponent, D);
   REGISTER_COMPONENT_FACTORY(TestComponent, E);

   // Register filter components
   REGISTER_FLAW_FILTER(EUROPA::SOLVERS::SingletonFilter, Singleton);
   REGISTER_FLAW_FILTER(EUROPA::SOLVERS::HorizonFilter, HorizonFilter);
   REGISTER_FLAW_FILTER(EUROPA::SOLVERS::InfiniteDynamicFilter, InfiniteDynamicFilter);
   REGISTER_FLAW_FILTER(EUROPA::SOLVERS::HorizonVariableFilter, HorizonVariableFilter);

   // Initialization of various ids and other required elements
   initSolverModuleTests();

   // Set up the required components. Should eventually go into an assembly. Note they are allocated on the stack, not the heap
   REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::MinValue, Min);
   REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::MaxValue, Max);
   REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::RandomValue, Random);

   // Constraints used for testing
   REGISTER_CONSTRAINT(LazyAllDiff, "lazyAllDiff",  "Default");
   REGISTER_CONSTRAINT(LazyAlwaysFails, "lazyAlwaysFails",  "Default");

   runTestSuite(ComponentFactoryTests::test);
   runTestSuite(FilterTests::test);
   runTestSuite(FlawHandlerTests::test);
   runTestSuite(SolverTests::test);

   uninitConstraintLibrary();
}

