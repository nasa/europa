#include "solvers-test-module.hh"
#include "Nddl.hh"
#include "Solver.hh"
#include "ComponentFactory.hh"
#include "Constraint.hh"
#include "ConstraintType.hh"
#include "UnboundVariableManager.hh"
#include "OpenConditionManager.hh"
#include "ThreatManager.hh"
#include "Object.hh"
#include "Timeline.hh"
#include "Filters.hh"
#include "IntervalToken.hh"
#include "TokenVariable.hh"
#include "Utils.hh"
#include "Debug.hh"
#include "Variable.hh"
#include "Domains.hh"
#include "MatchingEngine.hh"
#include "HSTSDecisionPoints.hh"
#include "PlanDatabaseWriter.hh"
#include "Rule.hh"
#include "RulesEngine.hh"
#include "NddlDefs.hh"
#include "Context.hh"
#include "STNTemporalAdvisor.hh"
#include "DbClientTransactionPlayer.hh"
#include "TemporalPropagator.hh"

#include "ModuleConstraintEngine.hh"
#include "ModulePlanDatabase.hh"
#include "ModuleRulesEngine.hh"
#include "ModuleTemporalNetwork.hh"
#include "ModuleSolvers.hh"
#include "ModuleNddl.hh"

#include <fstream>

#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif
/**
 * @file Provides module tests for Solver Module.
 * @author Conor McGann
 * @date May, 2005
 */



using namespace EUROPA;
using namespace EUROPA::SOLVERS;
using namespace EUROPA::SOLVERS::HSTS;

void registerTestElements(EngineId& engine);

class SolversTestEngine : public EngineBase
{
  public:
	SolversTestEngine();
	virtual ~SolversTestEngine();

    const SchemaId&           getSchema()           { return ((Schema*)getComponent("Schema"))->getId(); }
    const ConstraintEngineId& getConstraintEngine() { return ((ConstraintEngine*)getComponent("ConstraintEngine"))->getId(); }
    const PlanDatabaseId&     getPlanDatabase()     { return ((PlanDatabase*)getComponent("PlanDatabase"))->getId(); }
    const RulesEngineId&      getRulesEngine()      { return ((RulesEngine*)getComponent("RulesEngine"))->getId(); }

  protected:
    void createModules();
};

SolversTestEngine::SolversTestEngine()
{
    createModules();
    doStart();
    executeScript("nddl-xml","Model.xml",true/*isFile*/);
    registerTestElements(getId());
}

SolversTestEngine::~SolversTestEngine()
{
    doShutdown();
}

void SolversTestEngine::createModules()
{
    addModule((new ModuleConstraintEngine())->getId());
    addModule((new ModuleConstraintLibrary())->getId());
    addModule((new ModulePlanDatabase())->getId());
    addModule((new ModuleRulesEngine())->getId());
    addModule((new ModuleTemporalNetwork())->getId());
    addModule((new ModuleSolvers())->getId());
    addModule((new ModuleNddl())->getId());
}


class TestEngine : public SolversTestEngine
{
public:
  TestEngine()
  {
  }

  bool playTransactions(const char* txSource)
  {
    check_error(txSource != NULL, "NULL transaction source provided.");

    // Obtain the client to play transactions on.
    DbClientId client = getPlanDatabase()->getClient();

    // Construct player
    DbClientTransactionPlayer player(client);

    // Open transaction source and play transactions
    std::ifstream in(txSource);

    check_error(in, "Invalid transaction source '" + std::string(txSource) + "'.");
    player.play(in);

    return getConstraintEngine()->constraintConsistent();
  }
};

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
    EUROPA_runTest(testBasicAllocation);
    return true;
  }

private:
  static bool testBasicAllocation(){
    TestEngine testEngine;

    TiXmlElement* configXml = initXml((getTestLoadLibraryPath() + "/ComponentFactoryTest.xml").c_str());

    for (TiXmlElement * child = configXml->FirstChildElement();
         child != NULL;
         child = child->NextSiblingElement()) {

      ComponentFactoryMgr* cfm = (ComponentFactoryMgr*)testEngine.getComponent("ComponentFactoryMgr");
      TestComponent * testComponent = static_cast<TestComponent*>(cfm->createInstance(*child));
      delete testComponent;
    }

    CPPUNIT_ASSERT(TestComponent::counter() == 5);

    delete configXml;

    return true;
  }
};

class FilterTests {
public:
  static bool test(){
    EUROPA_runTest(testRuleMatching);
    EUROPA_runTest(testVariableFiltering);
    EUROPA_runTest(testTokenFiltering);
    EUROPA_runTest(testThreatFiltering);
    return true;
  }

private:
  static bool testRuleMatching() {
    TestEngine testEngine;

    TiXmlElement* root = initXml( (getTestLoadLibraryPath() + "/RuleMatchingTests.xml").c_str(), "MatchingEngine");
    MatchingEngine me(testEngine.getId(),*root);
    CPPUNIT_ASSERT_MESSAGE(toString(me.ruleCount()), me.ruleCount() == 13);
    CPPUNIT_ASSERT(me.hasRule("[R0]*.*.*.*.*.*"));
    CPPUNIT_ASSERT(me.hasRule("[R1]*.*.start.*.*.*"));
    CPPUNIT_ASSERT(me.hasRule("[R2]*.*.arg3.*.*.*"));
    CPPUNIT_ASSERT(me.hasRule("[R3]*.predicateF.*.*.*.*"));
    CPPUNIT_ASSERT(me.hasRule("[R4]*.predicateC.arg6.*.*.*"));
    CPPUNIT_ASSERT(me.hasRule("[R5]C.predicateC.*.*.*.*"));
    CPPUNIT_ASSERT(me.hasRule("[R6]C.*.*.*.*.*"));
    CPPUNIT_ASSERT(me.hasRule("[R7]*.*.duration.*.Object.*"));
    CPPUNIT_ASSERT(me.hasRule("[R7a]*.*.duration.none.*.*"));
    CPPUNIT_ASSERT(me.hasRule("[R8]*.*.*.*.B.*"));
    CPPUNIT_ASSERT(me.hasRule("[R9]*.*.*.meets.D.predicateG"));
    CPPUNIT_ASSERT(me.hasRule("[R10]*.*.*.before.*.*"));
    CPPUNIT_ASSERT(me.hasRule("[R11]*.*.neverMatched.*.*.*"));

    PlanDatabaseId db = testEngine.getPlanDatabase();
    Object o1(db, "A", "o1");
    Object o2(db, "D", "o2");
    Object o3(db, "C", "o3");
    Object o4(db, "E", "o4");
    db->close();

    // test RO
    {
      Variable<IntervalIntDomain> v0(testEngine.getConstraintEngine(), IntervalIntDomain(0, 10), false, true, "v0");
      std::vector<MatchingRuleId> rules;
      me.getMatches(v0.getId(), rules);
      CPPUNIT_ASSERT_MESSAGE(toString(rules.size()), rules.size() == 1);
      CPPUNIT_ASSERT_MESSAGE(rules[0]->toString(), rules[0]->toString() == "[R0]*.*.*.*.*.*");
    }

    // test R1
    {
      IntervalToken token(db,
                          "A.predicateA",
                          true,
                          false,
                          IntervalIntDomain(0, 1000),
                          IntervalIntDomain(0, 1000),
                          IntervalIntDomain(2, 10),
                          Token::noObject(), true);

      std::vector<MatchingRuleId> rules;
      me.getMatches(ConstrainedVariableId(token.start()), rules);
      CPPUNIT_ASSERT_MESSAGE(toString(rules.size()), rules.size() == 2);
      CPPUNIT_ASSERT_MESSAGE(rules[1]->toString(), rules[1]->toString() == "[R1]*.*.start.*.*.*");
    }

    // test R2
    {
      Variable<IntervalIntDomain> v0(testEngine.getConstraintEngine(), IntervalIntDomain(0, 10), false, true, "arg3");
      std::vector<MatchingRuleId> rules;
      me.getMatches(v0.getId(), rules);
      CPPUNIT_ASSERT_MESSAGE(toString(rules.size()), rules.size() == 2);
      CPPUNIT_ASSERT_MESSAGE(rules[1]->toString(), rules[1]->toString() == "[R2]*.*.arg3.*.*.*");
    }

    // test R3
    {
      TokenId token = db->getClient()->createToken("D.predicateF", false);
      std::vector<MatchingRuleId> rules;
      me.getMatches(token, rules);
      CPPUNIT_ASSERT_MESSAGE(toString(rules.size()), rules.size() == 2);
      CPPUNIT_ASSERT_MESSAGE(rules[1]->toString(), rules[1]->toString() == "[R3]*.predicateF.*.*.*.*");
      token->discard();
    }

    // test R4
    {
      TokenId token = db->getClient()->createToken("D.predicateC", false);
      std::vector<MatchingRuleId> rules;
      me.getMatches(token->getVariable("arg6"), rules);
      CPPUNIT_ASSERT_MESSAGE(toString(rules.size()), rules.size() == 2);
      CPPUNIT_ASSERT_MESSAGE(rules[1]->toString(), rules[1]->toString() == "[R4]*.predicateC.arg6.*.*.*");
      token->discard();
    }

    // test R5 & R6
    {
      TokenId token = db->getClient()->createToken("C.predicateC", false);
      std::vector<MatchingRuleId> rules;
      me.getMatches(token, rules);
      CPPUNIT_ASSERT_MESSAGE(toString(rules.size()) + " for " + token->getUnqualifiedPredicateName().toString(), rules.size() == 3);
      CPPUNIT_ASSERT_MESSAGE(rules[1]->toString(), rules[1]->toString() == "[R5]C.predicateC.*.*.*.*");
      CPPUNIT_ASSERT_MESSAGE(rules[2]->toString(), rules[2]->toString() == "[R6]C.*.*.*.*.*");
      token->discard();
    }

    // test R6
    {
      TokenId token = db->getClient()->createToken("C.predicateA", false);
      std::vector<MatchingRuleId> rules;
      me.getMatches(token, rules);
      CPPUNIT_ASSERT_MESSAGE(toString(rules.size()) + " for " + token->getUnqualifiedPredicateName().toString(), rules.size() == 2);
      CPPUNIT_ASSERT_MESSAGE(rules[1]->toString(), rules[1]->toString() == "[R6]C.*.*.*.*.*");
      token->discard();
    }

    // test R7
    {
      TokenId token = db->getClient()->createToken("D.predicateF", false);
      token->activate();
      TokenId E_predicateC = *(token->slaves().begin());
      std::vector<MatchingRuleId> rules;
      me.getMatches(ConstrainedVariableId(E_predicateC->duration()), rules);
      CPPUNIT_ASSERT_MESSAGE(toString(rules.size()) + " for " + token->getUnqualifiedPredicateName().toString(), rules.size() == 2);
      CPPUNIT_ASSERT_MESSAGE(rules[1]->toString(), rules[1]->toString() == "[R7]*.*.duration.*.Object.*");
      token->discard();
    }

    // test R7a
    {
      TokenId token = db->getClient()->createToken("E.predicateC", false);
      std::vector<MatchingRuleId> rules;
      me.getMatches(ConstrainedVariableId(token->duration()), rules);
      CPPUNIT_ASSERT_MESSAGE(toString(rules.size()) + " for " + token->getPredicateName().toString(), rules.size() == 2);
      CPPUNIT_ASSERT_MESSAGE(rules[1]->toString(), rules[1]->toString() == "[R7a]*.*.duration.none.*.*");
      token->discard();
    }

    // test R8
    {
      TokenId token = db->getClient()->createToken("B.predicateC", false);
      token->activate();
      TokenId E_predicateC = *(token->slaves().begin());
      std::vector<MatchingRuleId> rules;
      me.getMatches(ConstrainedVariableId(E_predicateC->duration()), rules);
      CPPUNIT_ASSERT_MESSAGE(toString(rules.size()) + " for " + token->getPredicateName().toString(), rules.size() == 3);
      CPPUNIT_ASSERT_MESSAGE(rules[1]->toString(), rules[1]->toString() == "[R8]*.*.*.*.B.*");
      CPPUNIT_ASSERT_MESSAGE(rules[2]->toString(), rules[2]->toString() == "[R7]*.*.duration.*.Object.*");
      token->discard();
    }

    // test R*, R9 and R10
    {
      TokenId token = db->getClient()->createToken("D.predicateG", false);
      token->activate();
      TokenId E_predicateC = *(token->slaves().begin());

      // Expect to fire R8, R9 and R10
      std::set<LabelStr> expectedRules;
      expectedRules.insert(LabelStr("[R8]*.*.*.*.B.*"));
      expectedRules.insert(LabelStr("[R9]*.*.*.meets.D.predicateG"));
      expectedRules.insert(LabelStr("[R10]*.*.*.before.*.*"));
      std::vector<MatchingRuleId> rules;
      me.getMatches(ConstrainedVariableId(E_predicateC->duration()), rules);
      CPPUNIT_ASSERT_MESSAGE(toString(rules.size()) + " for " + token->getPredicateName().toString(), rules.size() == 4);
      for(int i=0;i>4; i++)
        CPPUNIT_ASSERT_MESSAGE(rules[i]->toString(), expectedRules.find(LabelStr(rules[i]->toString())) != expectedRules.end());

      token->discard();
    }

    return true;
  }

  static bool testVariableFiltering(){
    TiXmlElement* root = initXml( (getTestLoadLibraryPath() + "/FlawFilterTests.xml").c_str(), "UnboundVariableManager");

    TestEngine testEngine;
    UnboundVariableManager fm(*root);
    CPPUNIT_ASSERT(testEngine.playTransactions( (getTestLoadLibraryPath() + "/UnboundVariableFiltering.xml").c_str() ));

    // Initialize after filling the database since we are not connected to an event source
    fm.initialize(*root,testEngine.getPlanDatabase());

    // Set the horizon
    IntervalIntDomain& horizon = HorizonFilter::getHorizon();
    horizon = IntervalIntDomain(0, 1000);

    // Simple filter on a variable
    ConstrainedVariableSet variables = testEngine.getConstraintEngine()->getVariables();
    for(ConstrainedVariableSet::const_iterator it = variables.begin(); it != variables.end(); ++it){
      ConstrainedVariableId var = *it;

      // Confirm temporal variables have been excluded
      static const LabelStr excludedVariables(":start:end:duration:arg1:arg3:arg4:arg6:arg7:arg8:filterVar:");
      static const LabelStr includedVariables(":arg2:arg5:keepVar:");
      std::string s = ":" + var->getName().toString() + ":";
      if(excludedVariables.contains(s))
        CPPUNIT_ASSERT_MESSAGE(var->toString(), !fm.inScope(var));
      else if(includedVariables.contains(s))
        CPPUNIT_ASSERT_MESSAGE(var->toString(), fm.inScope(var));
    }

    // Confirm that a global variable is first a flaw, but when bound is no longer a flaw, and when bound again,
    // returns as a flaw
    ConstrainedVariableId globalVar1 = testEngine.getPlanDatabase()->getGlobalVariable("globalVariable1");
    ConstrainedVariableId globalVar2 = testEngine.getPlanDatabase()->getGlobalVariable("globalVariable2");
    ConstrainedVariableId globalVar3 = testEngine.getPlanDatabase()->getGlobalVariable("globalVariable3");
    CPPUNIT_ASSERT(!fm.inScope(globalVar1));
    CPPUNIT_ASSERT(fm.inScope(globalVar2));
    globalVar2->specify(globalVar2->lastDomain().getLowerBound());
    testEngine.getConstraintEngine()->propagate();
    CPPUNIT_ASSERT(!fm.inScope(globalVar2));
    CPPUNIT_ASSERT(!fm.inScope(globalVar1)); // By propagation it will be a singleton, so it will be Excluded
    globalVar2->reset();
    testEngine.getConstraintEngine()->propagate();
    CPPUNIT_ASSERT(!fm.inScope(globalVar1));
    CPPUNIT_ASSERT(fm.inScope(globalVar2));

    CPPUNIT_ASSERT(!fm.inScope(globalVar3));

    return true;
  }

  static bool testTokenFiltering(){
    TiXmlElement* root = initXml((getTestLoadLibraryPath() + "/FlawFilterTests.xml").c_str(), "OpenConditionManager");
                check_error(root != NULL, "Error loading xml: " + getTestLoadLibraryPath() + "/FlawFilterTests.xml");

    TestEngine testEngine;
    OpenConditionManager fm(*root);
    IntervalIntDomain& horizon = HorizonFilter::getHorizon();
    horizon = IntervalIntDomain(0, 1000);
    CPPUNIT_ASSERT(testEngine.playTransactions((getTestLoadLibraryPath() + "/OpenConditionFiltering.xml").c_str() ));

    // Initialize with data in the database
    fm.initialize(*root,testEngine.getPlanDatabase());

    TokenSet tokens = testEngine.getPlanDatabase()->getTokens();
    for(TokenSet::const_iterator it = tokens.begin(); it != tokens.end(); ++it){
      static const LabelStr excludedPredicates(":D.predicateA:D.predicateB:D.predicateC:E.predicateC:HorizonFiltered.predicate1:HorizonFiltered.predicate2:HorizonFiltered.predicate5:");
      TokenId token = *it;
      std::string s = ":" + token->getPredicateName().toString() + ":";
      if(excludedPredicates.contains(s))
        CPPUNIT_ASSERT_MESSAGE(token->toString() + " is in scope after all.", !fm.inScope(token));
      else
        CPPUNIT_ASSERT_MESSAGE(token->toString() + " is not in scope and not active.", token->isActive() || fm.inScope(token));
    }

    return true;
  }


  static bool testThreatFiltering(){
    TiXmlElement* root = initXml( (getTestLoadLibraryPath() + "/FlawFilterTests.xml" ).c_str(), "ThreatManager");

    TestEngine testEngine;
    ThreatManager fm(*root);
    IntervalIntDomain& horizon = HorizonFilter::getHorizon();
    horizon = IntervalIntDomain(0, 1000);
    CPPUNIT_ASSERT(testEngine.playTransactions(( getTestLoadLibraryPath() + "/ThreatFiltering.xml").c_str()));

    // Initialize with data in the database
    fm.initialize(*root,testEngine.getPlanDatabase());

    TokenSet tokens = testEngine.getPlanDatabase()->getTokens();
    for(TokenSet::const_iterator it = tokens.begin(); it != tokens.end(); ++it){
      static const LabelStr excludedPredicates(":D.predicateA:D.predicateB:D.predicateC:E.predicateC:HorizonFiltered.predicate1:HorizonFiltered.predicate2:HorizonFiltered.predicate5:");
      TokenId token = *it;
      CPPUNIT_ASSERT_MESSAGE(token->toString() + " is not in scope and not active.", token->isActive() || !fm.inScope(token));
      std::string s = ":" + token->getPredicateName().toString() + ":";
      if(excludedPredicates.contains(s))
        CPPUNIT_ASSERT_MESSAGE(token->toString() + " is in scope after all.", !fm.inScope(token));
    }

    return true;
  }
};


class FlawHandlerTests {
public:
  static bool test(){
    EUROPA_runTest(testPriorities);
    EUROPA_runTest(testGuards);
    EUROPA_runTest(testDynamicFlawManagement);
    EUROPA_runTest(testDefaultVariableOrdering);
    EUROPA_runTest(testHeuristicVariableOrdering);
    EUROPA_runTest(testTokenComparators);
    EUROPA_runTest(testValueEnum);
    EUROPA_runTest(testHSTSOpenConditionDecisionPoint);
    EUROPA_runTest(testHSTSThreatDecisionPoint);
    return true;
  }

private:
  static bool testPriorities(){
    TestEngine testEngine;
    TiXmlElement* root = initXml( (getTestLoadLibraryPath() + "/FlawHandlerTests.xml").c_str(), "TestPriorities");
    MatchingEngine me(testEngine.getId(),*root, "FlawHandler");
    PlanDatabaseId db = testEngine.getPlanDatabase();
    Object o1(db, "A", "o1");
    Object o2(db, "D", "o2");
    Object o3(db, "C", "o3");
    Object o4(db, "E", "o4");
    db->close();

    // test H0
    {
      Variable<IntervalIntDomain> v0(testEngine.getConstraintEngine(), IntervalIntDomain(0, 10), false, true, "v0");
      std::vector<MatchingRuleId> rules;
      me.getMatches(v0.getId(), rules);
      CPPUNIT_ASSERT_MESSAGE(toString(rules.size()), rules.size() == 1);
      FlawHandlerId flawHandler = rules[0];
      CPPUNIT_ASSERT_MESSAGE(toString(flawHandler->getPriority()), flawHandler->getPriority() == 1000);
      CPPUNIT_ASSERT_MESSAGE(toString(flawHandler->getWeight()), flawHandler->getWeight() == 199000);
    }

    // test H1
    {
      Variable<IntervalIntDomain> v0(testEngine.getConstraintEngine(), IntervalIntDomain(0, 10), false, true, "start");
      std::vector<MatchingRuleId> rules;
      me.getMatches(v0.getId(), rules);
      CPPUNIT_ASSERT_MESSAGE(toString(rules.size()), rules.size() == 2);
      FlawHandlerId flawHandler = rules[1];
      CPPUNIT_ASSERT_MESSAGE(toString(flawHandler->getPriority()), flawHandler->getPriority() == 1000);
      CPPUNIT_ASSERT_MESSAGE(toString(flawHandler->getWeight()), flawHandler->getWeight() == 299000);
    }

    // test H2
    {
      Variable<IntervalIntDomain> v0(testEngine.getConstraintEngine(), IntervalIntDomain(0, 10), false, true, "end");
      std::vector<MatchingRuleId> rules;
      me.getMatches(v0.getId(), rules);
      CPPUNIT_ASSERT_MESSAGE(toString(rules.size()), rules.size() == 2);
      FlawHandlerId flawHandler = rules[1];
      CPPUNIT_ASSERT_MESSAGE(toString(flawHandler->getPriority()), flawHandler->getPriority() == 200);
      CPPUNIT_ASSERT_MESSAGE(toString(flawHandler->getWeight()), flawHandler->getWeight() == 299800);
    }

    // test H3
    {
      TokenId token = db->getClient()->createToken("D.predicateG", false);
      token->activate();
      TokenId E_predicateC = *(token->slaves().begin());
      std::vector<MatchingRuleId> rules;
      me.getMatches(ConstrainedVariableId(E_predicateC->end()), rules);
      CPPUNIT_ASSERT_MESSAGE(toString(rules.size()), rules.size() == 3);
      FlawHandlerId flawHandler = rules[2];
      CPPUNIT_ASSERT_MESSAGE(toString(flawHandler->getPriority()), flawHandler->getPriority() == 1);
      CPPUNIT_ASSERT_MESSAGE(toString(flawHandler->getWeight()), flawHandler->getWeight() == 399999);
      token->discard();
    }

    return true;
  }

  static bool testGuards(){
    TestEngine testEngine;
    TiXmlElement* root = initXml( (getTestLoadLibraryPath() + "/FlawHandlerTests.xml").c_str(), "TestGuards");
    MatchingEngine me(testEngine.getId(),*root, "FlawHandler");
    PlanDatabaseId db = testEngine.getPlanDatabase();
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
      me.getMatches(token, rules);
      CPPUNIT_ASSERT_MESSAGE(toString(rules.size()), rules.size() == 1);
      FlawHandlerId flawHandler = rules[0];
      std::vector<ConstrainedVariableId> guards;
      CPPUNIT_ASSERT(flawHandler->makeConstraintScope(token, guards));
      CPPUNIT_ASSERT_MESSAGE(toString(guards.size()), guards.size() == 2);
      CPPUNIT_ASSERT_MESSAGE(guards[0]->toString(), guards[0] == token->start());
      CPPUNIT_ASSERT_MESSAGE(guards[1]->toString(), guards[1] == token->getObject());
      CPPUNIT_ASSERT(!flawHandler->test(guards));
      token->start()->specify(30);
      CPPUNIT_ASSERT(!flawHandler->test(guards));
      token->getObject()->specify(o2.getId());
      CPPUNIT_ASSERT(flawHandler->test(guards));
      token->discard();
    }

    // test H1
    {
      TokenId token = db->getClient()->createToken("C.predicateA", false);
      std::vector<MatchingRuleId> rules;
      me.getMatches(token, rules);
      CPPUNIT_ASSERT_MESSAGE(toString(rules.size()), rules.size() == 1);
      FlawHandlerId flawHandler = rules[0];
      std::vector<ConstrainedVariableId> guards;
      CPPUNIT_ASSERT(!flawHandler->makeConstraintScope(token, guards));
      token->discard();
    }

    // test H2
    {
      TokenId token = db->getClient()->createToken("B.predicateC", false);
      {
        std::vector<MatchingRuleId> rules;
        me.getMatches(token, rules);
        CPPUNIT_ASSERT_MESSAGE(toString(rules.size()), rules.size() == 1);
        FlawHandlerId flawHandler = rules[0];
        std::vector<ConstrainedVariableId> guards;
        CPPUNIT_ASSERT(!flawHandler->makeConstraintScope(token, guards));
      }
      // Now fire on the subgoal. Rule will match as it has a master
      token->activate();
      TokenId B_predicateC = *(token->slaves().begin());
      std::vector<MatchingRuleId> rules;
      me.getMatches(B_predicateC, rules);
      CPPUNIT_ASSERT_MESSAGE(toString(rules.size()), rules.size() == 1);
      FlawHandlerId flawHandler = rules[0];
      std::vector<ConstrainedVariableId> guards;
      CPPUNIT_ASSERT(flawHandler->makeConstraintScope(B_predicateC, guards));
      CPPUNIT_ASSERT(!flawHandler->test(guards));
      // Specify the master guard variable
      token->start()->specify(30);
      CPPUNIT_ASSERT(flawHandler->test(guards));
      token->discard();
    }

    // test H3
    {
      Variable<IntervalIntDomain> v0(testEngine.getConstraintEngine(), IntervalIntDomain(0, 10), false, true, "FreeVariable");
      std::vector<MatchingRuleId> rules;
      me.getMatches(v0.getId(), rules);
      CPPUNIT_ASSERT_MESSAGE(toString(rules.size()), rules.size() == 1);
      FlawHandlerId flawHandler = rules[0];
      std::vector<ConstrainedVariableId> guards;
      CPPUNIT_ASSERT(!flawHandler->makeConstraintScope(v0.getId(), guards));
    }

    return true;
  }

  static bool testDynamicFlawManagement(){
    TestEngine testEngine;
    TiXmlElement* root = initXml( (getTestLoadLibraryPath() + "/FlawHandlerTests.xml").c_str(), "TestDynamicFlaws");
    TiXmlElement* child = root->FirstChildElement();
    PlanDatabaseId db = testEngine.getPlanDatabase();
    Object o1(db, "A", "o1");
    Object o2(db, "D", "o2");
    Object o3(db, "C", "o3");
    Object o4(db, "E", "o4");
    Object o5(db, "D", "o5");
    db->close();
    Solver solver(testEngine.getPlanDatabase(), *child);

    // test basic flaw filtering and default handler access
    {
      TokenId token = db->getClient()->createToken("D.predicateG", false);
      db->getConstraintEngine()->propagate();
      // Initially the token is in scope and the variable is not
      CPPUNIT_ASSERT(solver.inScope(token));
      CPPUNIT_ASSERT(!solver.inScope(token->start()));
      CPPUNIT_ASSERT(solver.getFlawHandler(token->start()).isNoId());
      CPPUNIT_ASSERT(solver.getFlawHandler(token).isValid());

      // Activate the token. The variable will still no be in scope since it is not finite.
      token->activate();
      db->getConstraintEngine()->propagate();
      CPPUNIT_ASSERT(solver.getFlawHandler(token->start()).isNoId());

      // The token should not be a flaw since it is nota timeline!
      CPPUNIT_ASSERT(solver.getFlawHandler(token).isNoId());

      // Restrict the base domain to finite bounds for the start variable
      token->start()->restrictBaseDomain(IntervalIntDomain(0, 50));
      db->getConstraintEngine()->propagate();
      CPPUNIT_ASSERT(solver.getFlawHandler(token->start()).isValid());

      // Now insert the token and bind the variable
      token->start()->specify(30);
      db->getConstraintEngine()->propagate();
      CPPUNIT_ASSERT(!solver.inScope(token));
      CPPUNIT_ASSERT(!solver.inScope(token->start()));

      // Reset the variable and it should be back in business
      token->start()->reset();
      db->getConstraintEngine()->propagate();
      CPPUNIT_ASSERT(solver.getFlawHandler(token->start()).isValid());

      // Deactivation of the token will introduce it as a flaw, and nuke the start variable
      token->cancel();
      db->getConstraintEngine()->propagate();
      CPPUNIT_ASSERT(solver.inScope(token));
      CPPUNIT_ASSERT(!solver.inScope(token->start()));

      // Now activate it and the variable should be back
      token->activate();
      db->getConstraintEngine()->propagate();
      CPPUNIT_ASSERT(solver.getFlawHandler(token->start()).isId());

      // Restrict the base domain of the variable to a singleton. It should no longer be a flaw
      token->start()->restrictBaseDomain(IntervalIntDomain(0, 0));
      db->getConstraintEngine()->propagate();
      CPPUNIT_ASSERT(solver.getFlawHandler(token->start()).isNoId());

      solver.reset();
      token->discard();
    }

    // Now handle a case with increasingly restrictive filters
    {
      TokenId master = db->getClient()->createToken("D.predicateF", false);
      db->getConstraintEngine()->propagate();
      master->activate();
      TokenId slave = master->getSlave(1);
      CPPUNIT_ASSERT_MESSAGE(slave->getPredicateName().toString(), slave->getPredicateName() == LabelStr("D.predicateC"));

      // With no guards set, we should just get the default priority
      db->getConstraintEngine()->propagate();
      CPPUNIT_ASSERT(solver.getFlawHandler(slave)->getPriority() == 99999);

      slave->start()->specify(10);
      db->getConstraintEngine()->propagate();
      CPPUNIT_ASSERT(solver.getFlawHandler(slave)->getPriority() == 1);

      slave->end()->specify(20);
      db->getConstraintEngine()->propagate();
      CPPUNIT_ASSERT(solver.getFlawHandler(slave)->getPriority() == 2);

      slave->getObject()->specify(o5.getId());
      db->getConstraintEngine()->propagate();
      CPPUNIT_ASSERT(solver.getFlawHandler(slave)->getPriority() == 3);

      master->start()->specify(10);
      db->getConstraintEngine()->propagate();
      CPPUNIT_ASSERT(solver.getFlawHandler(slave)->getPriority() == 4);

      master->end()->specify(20);
      db->getConstraintEngine()->propagate();
      CPPUNIT_ASSERT(solver.getFlawHandler(slave)->getPriority() == 5);

      slave->start()->reset();
      slave->start()->specify(11);
      db->getConstraintEngine()->propagate();
      CPPUNIT_ASSERT(solver.getFlawHandler(slave)->getPriority() == 99999);

      master->discard();
    }

    return true;
  }

  static bool testDefaultVariableOrdering(){
    TestEngine testEngine;
    TiXmlElement* root = initXml( (getTestLoadLibraryPath() + "/FlawHandlerTests.xml").c_str(), "DefaultVariableOrdering");
    TiXmlElement* child = root->FirstChildElement();
    {
      CPPUNIT_ASSERT(testEngine.playTransactions( (getTestLoadLibraryPath() + "/StaticCSP.xml").c_str()));
      Solver solver(testEngine.getPlanDatabase(), *child);
      CPPUNIT_ASSERT(solver.solve());
      CPPUNIT_ASSERT(solver.getStepCount() == solver.getDepth());
      CPPUNIT_ASSERT_MESSAGE(toString(solver.getStepCount()), solver.getStepCount() == 2);
      ConstrainedVariableId v1 = testEngine.getPlanDatabase()->getGlobalVariable("v1");
      CPPUNIT_ASSERT_MESSAGE(v1->toString(), v1->lastDomain().getSingletonValue() == 1);
      ConstrainedVariableId v2 = testEngine.getPlanDatabase()->getGlobalVariable("v2");
      CPPUNIT_ASSERT_MESSAGE(v2->toString(), v2->lastDomain().getSingletonValue() == 0);
    }

    return true;
  }

  static bool testHeuristicVariableOrdering(){
    TestEngine testEngine;
    TiXmlElement* root = initXml( (getTestLoadLibraryPath() + "/FlawHandlerTests.xml").c_str(), "HeuristicVariableOrdering");
    TiXmlElement* child = root->FirstChildElement();
    {
      CPPUNIT_ASSERT(testEngine.playTransactions( (getTestLoadLibraryPath() + "/StaticCSP.xml").c_str()));
      Solver solver(testEngine.getPlanDatabase(), *child);
      CPPUNIT_ASSERT(solver.solve());
      CPPUNIT_ASSERT(solver.getStepCount() == solver.getDepth());
      CPPUNIT_ASSERT_MESSAGE(toString(solver.getStepCount()), solver.getStepCount() == 3);
      ConstrainedVariableId v1 = testEngine.getPlanDatabase()->getGlobalVariable("v1");
      CPPUNIT_ASSERT_MESSAGE(v1->toString(), v1->getSpecifiedValue() == 9);
      ConstrainedVariableId v2 = testEngine.getPlanDatabase()->getGlobalVariable("v2");
      CPPUNIT_ASSERT_MESSAGE(v2->toString(), v2->getSpecifiedValue() == 10);
    }

    return true;
  }

  static bool testTokenComparators() {
    TestEngine testEngine;
    testEngine.getSchema()->addPredicate("A.Foo");
    PlanDatabaseId db = testEngine.getPlanDatabase();

    Object o1(db, "A", "o1");

    //create the token to which everything is going to get compared.  Midpoint at 10.
    IntervalToken foo(db, "A.Foo", false, false, IntervalIntDomain(5, 10), IntervalIntDomain(6, 20), IntervalIntDomain(1, 10), "o1", true);
    //create a token after foo
    IntervalToken t1(db, "A.Foo", false, false, IntervalIntDomain(7, 10), IntervalIntDomain(8, 20), IntervalIntDomain(1, 10), "o1", true);
    //create a token before foo
    IntervalToken t2(db, "A.Foo", false, false, IntervalIntDomain(4, 10), IntervalIntDomain(5, 20), IntervalIntDomain(1, 10), "o1", true);
    //create a token that starts at the same time as foo
    IntervalToken t3(db, "A.Foo", false, false, IntervalIntDomain(5, 10), IntervalIntDomain(9, 20), IntervalIntDomain(4, 10), "o1", true);

    EarlyTokenComparator early(foo.getId());
    CPPUNIT_ASSERT(early.compare(foo.getId(), t1.getId()));
    CPPUNIT_ASSERT(!early.compare(t1.getId(), foo.getId()));
    CPPUNIT_ASSERT(!early.compare(foo.getId(), t2.getId()));
    CPPUNIT_ASSERT(early.compare(t2.getId(), foo.getId()));
    CPPUNIT_ASSERT(!early.compare(foo.getId(), t3.getId()));
    CPPUNIT_ASSERT(!early.compare(t3.getId(), foo.getId()));

    LateTokenComparator late(foo.getId());
    CPPUNIT_ASSERT(!late.compare(foo.getId(), t1.getId()));
    CPPUNIT_ASSERT(late.compare(t1.getId(), foo.getId()));
    CPPUNIT_ASSERT(late.compare(foo.getId(), t2.getId()));
    CPPUNIT_ASSERT(!late.compare(t2.getId(), foo.getId()));
    CPPUNIT_ASSERT(!late.compare(foo.getId(), t3.getId()));
    CPPUNIT_ASSERT(!late.compare(t3.getId(), foo.getId()));

    //create a token w/ midpoint 1 after foo's midpoint, starting before foo
    IntervalToken t4(db, "A.Foo", false, false, IntervalIntDomain(4, 9), IntervalIntDomain(5, 19), IntervalIntDomain(1, 14), "o1", true);
    //create a token w/ midpoint 1 after foo's midpoint, starting at the same time as foo
    IntervalToken t5(db, "A.Foo", false, false, IntervalIntDomain(5, 10), IntervalIntDomain(6, 22), IntervalIntDomain(1, 12), "o1", true);
    //create a token w/ midpoint 1 after foo's midpoint, starting after foo
    IntervalToken t6(db, "A.Foo", false, false, IntervalIntDomain(6, 11), IntervalIntDomain(7, 21), IntervalIntDomain(1, 10), "o1", true);

    //create a token w/ midpoint 1 before foo's midpoint, starting before foo
    IntervalToken t7(db, "A.Foo", false, false, IntervalIntDomain(4, 9), IntervalIntDomain(5, 19), IntervalIntDomain(1, 10), "o1", true);
    //create a token w/ midpoint 1 before foo's midpoint, starting at the same time as foo
    IntervalToken t8(db, "A.Foo", false, false, IntervalIntDomain(5, 10), IntervalIntDomain(6, 18), IntervalIntDomain(1, 8), "o1", true);
    //create a token w/ midpoint 1 before foo's midpoint, starting after foo
    IntervalToken t9(db, "A.Foo", false, false, IntervalIntDomain(6, 11), IntervalIntDomain(7, 17), IntervalIntDomain(1, 6), "o1", true);

    //create a token w/ midpoint 2 after foo's midpoint, starting before foo
    IntervalToken t10(db, "A.Foo", false, false, IntervalIntDomain(4, 9), IntervalIntDomain(5, 25), IntervalIntDomain(1, 16), "o1", true);
    //create a token w/ midpoint 2 after foo's midpoint, starting at the same time as foo
    IntervalToken t11(db, "A.Foo", false, false, IntervalIntDomain(5, 10), IntervalIntDomain(6, 24), IntervalIntDomain(1, 14), "o1", true);
    //create a token w/ midpoint 2 after foo's midpoint, starting after foo
    IntervalToken t12(db, "A.Foo", false, false, IntervalIntDomain(6, 11), IntervalIntDomain(7, 23), IntervalIntDomain(1, 12), "o1", true);

    //create a token w/ midpoint 2 before foo's midpoint, starting before foo
    IntervalToken t13(db, "A.Foo", false, false, IntervalIntDomain(4, 9), IntervalIntDomain(5, 17), IntervalIntDomain(1, 8), "o1", true);
    //create a token w/ midpoint 2 before foo's midpoint, starting at the same time as foo
    IntervalToken t14(db, "A.Foo", false, false, IntervalIntDomain(5, 10), IntervalIntDomain(6, 16), IntervalIntDomain(1, 6), "o1", true);
    //create a token w/ midpoint 2 before foo's mispoint, starting after foo
    IntervalToken t15(db, "A.Foo", false, false, IntervalIntDomain(6, 11), IntervalIntDomain(7, 15), IntervalIntDomain(1, 4), "o1", true);

    NearTokenComparator near(foo.getId());
    CPPUNIT_ASSERT(!near.compare(foo.getId(), foo.getId()));
    CPPUNIT_ASSERT(!near.compare(t4.getId(), t5.getId()));
    CPPUNIT_ASSERT(!near.compare(t5.getId(), t6.getId()));
    CPPUNIT_ASSERT(!near.compare(t4.getId(), t6.getId()));

    CPPUNIT_ASSERT(!near.compare(t4.getId(), t7.getId()));
    CPPUNIT_ASSERT(!near.compare(t7.getId(), t4.getId()));
    CPPUNIT_ASSERT(!near.compare(t4.getId(), t8.getId()));
    CPPUNIT_ASSERT(!near.compare(t8.getId(), t4.getId()));
    CPPUNIT_ASSERT(!near.compare(t4.getId(), t9.getId()));
    CPPUNIT_ASSERT(!near.compare(t9.getId(), t4.getId()));

    CPPUNIT_ASSERT(near.compare(t4.getId(), t10.getId()));
    CPPUNIT_ASSERT(!near.compare(t10.getId(), t4.getId()));
    CPPUNIT_ASSERT(near.compare(t4.getId(), t11.getId()));
    CPPUNIT_ASSERT(!near.compare(t11.getId(), t4.getId()));
    CPPUNIT_ASSERT(near.compare(t4.getId(), t12.getId()));
    CPPUNIT_ASSERT(!near.compare(t12.getId(), t4.getId()));

    CPPUNIT_ASSERT(near.compare(t4.getId(), t13.getId()));
    CPPUNIT_ASSERT(!near.compare(t13.getId(), t4.getId()));
    CPPUNIT_ASSERT(near.compare(t4.getId(), t14.getId()));
    CPPUNIT_ASSERT(!near.compare(t14.getId(), t4.getId()));
    CPPUNIT_ASSERT(near.compare(t4.getId(), t15.getId()));
    CPPUNIT_ASSERT(!near.compare(t15.getId(), t4.getId()));


    CPPUNIT_ASSERT(!near.compare(t5.getId(), t7.getId()));
    CPPUNIT_ASSERT(!near.compare(t7.getId(), t5.getId()));
    CPPUNIT_ASSERT(!near.compare(t5.getId(), t8.getId()));
    CPPUNIT_ASSERT(!near.compare(t8.getId(), t5.getId()));
    CPPUNIT_ASSERT(!near.compare(t5.getId(), t9.getId()));
    CPPUNIT_ASSERT(!near.compare(t9.getId(), t5.getId()));

    CPPUNIT_ASSERT(near.compare(t5.getId(), t10.getId()));
    CPPUNIT_ASSERT(!near.compare(t10.getId(), t5.getId()));
    CPPUNIT_ASSERT(near.compare(t5.getId(), t11.getId()));
    CPPUNIT_ASSERT(!near.compare(t11.getId(), t5.getId()));
    CPPUNIT_ASSERT(near.compare(t5.getId(), t12.getId()));
    CPPUNIT_ASSERT(!near.compare(t12.getId(), t5.getId()));

    CPPUNIT_ASSERT(near.compare(t5.getId(), t13.getId()));
    CPPUNIT_ASSERT(!near.compare(t13.getId(), t5.getId()));
    CPPUNIT_ASSERT(near.compare(t5.getId(), t14.getId()));
    CPPUNIT_ASSERT(!near.compare(t14.getId(), t5.getId()));
    CPPUNIT_ASSERT(near.compare(t5.getId(), t15.getId()));
    CPPUNIT_ASSERT(!near.compare(t15.getId(), t5.getId()));


    CPPUNIT_ASSERT(!near.compare(t6.getId(), t7.getId()));
    CPPUNIT_ASSERT(!near.compare(t7.getId(), t6.getId()));
    CPPUNIT_ASSERT(!near.compare(t6.getId(), t8.getId()));
    CPPUNIT_ASSERT(!near.compare(t8.getId(), t6.getId()));
    CPPUNIT_ASSERT(!near.compare(t6.getId(), t9.getId()));
    CPPUNIT_ASSERT(!near.compare(t9.getId(), t6.getId()));

    CPPUNIT_ASSERT(near.compare(t6.getId(), t10.getId()));
    CPPUNIT_ASSERT(!near.compare(t10.getId(), t6.getId()));
    CPPUNIT_ASSERT(near.compare(t6.getId(), t11.getId()));
    CPPUNIT_ASSERT(!near.compare(t11.getId(), t6.getId()));
    CPPUNIT_ASSERT(near.compare(t6.getId(), t12.getId()));
    CPPUNIT_ASSERT(!near.compare(t12.getId(), t6.getId()));

    CPPUNIT_ASSERT(near.compare(t6.getId(), t13.getId()));
    CPPUNIT_ASSERT(!near.compare(t13.getId(), t6.getId()));
    CPPUNIT_ASSERT(near.compare(t6.getId(), t14.getId()));
    CPPUNIT_ASSERT(!near.compare(t14.getId(), t6.getId()));
    CPPUNIT_ASSERT(near.compare(t6.getId(), t15.getId()));
    CPPUNIT_ASSERT(!near.compare(t15.getId(), t6.getId()));




    CPPUNIT_ASSERT(!near.compare(t7.getId(), t8.getId()));
    CPPUNIT_ASSERT(!near.compare(t8.getId(), t9.getId()));
    CPPUNIT_ASSERT(!near.compare(t9.getId(), t7.getId()));

    CPPUNIT_ASSERT(near.compare(t7.getId(), t10.getId()));
    CPPUNIT_ASSERT(!near.compare(t10.getId(), t7.getId()));
    CPPUNIT_ASSERT(near.compare(t7.getId(), t11.getId()));
    CPPUNIT_ASSERT(!near.compare(t11.getId(), t7.getId()));
    CPPUNIT_ASSERT(near.compare(t7.getId(), t12.getId()));
    CPPUNIT_ASSERT(!near.compare(t12.getId(), t7.getId()));

    CPPUNIT_ASSERT(near.compare(t7.getId(), t13.getId()));
    CPPUNIT_ASSERT(!near.compare(t13.getId(), t7.getId()));
    CPPUNIT_ASSERT(near.compare(t7.getId(), t14.getId()));
    CPPUNIT_ASSERT(!near.compare(t14.getId(), t7.getId()));
    CPPUNIT_ASSERT(near.compare(t7.getId(), t15.getId()));
    CPPUNIT_ASSERT(!near.compare(t15.getId(), t7.getId()));

    CPPUNIT_ASSERT(near.compare(t8.getId(), t10.getId()));
    CPPUNIT_ASSERT(!near.compare(t10.getId(), t8.getId()));
    CPPUNIT_ASSERT(near.compare(t8.getId(), t11.getId()));
    CPPUNIT_ASSERT(!near.compare(t11.getId(), t8.getId()));
    CPPUNIT_ASSERT(near.compare(t8.getId(), t12.getId()));
    CPPUNIT_ASSERT(!near.compare(t12.getId(), t8.getId()));

    CPPUNIT_ASSERT(near.compare(t8.getId(), t13.getId()));
    CPPUNIT_ASSERT(!near.compare(t13.getId(), t8.getId()));
    CPPUNIT_ASSERT(near.compare(t8.getId(), t14.getId()));
    CPPUNIT_ASSERT(!near.compare(t14.getId(), t8.getId()));
    CPPUNIT_ASSERT(near.compare(t8.getId(), t15.getId()));
    CPPUNIT_ASSERT(!near.compare(t15.getId(), t8.getId()));


    CPPUNIT_ASSERT(near.compare(t9.getId(), t10.getId()));
    CPPUNIT_ASSERT(!near.compare(t10.getId(), t9.getId()));
    CPPUNIT_ASSERT(near.compare(t9.getId(), t11.getId()));
    CPPUNIT_ASSERT(!near.compare(t11.getId(), t9.getId()));
    CPPUNIT_ASSERT(near.compare(t9.getId(), t12.getId()));
    CPPUNIT_ASSERT(!near.compare(t12.getId(), t9.getId()));

    CPPUNIT_ASSERT(near.compare(t9.getId(), t13.getId()));
    CPPUNIT_ASSERT(!near.compare(t13.getId(), t9.getId()));
    CPPUNIT_ASSERT(near.compare(t9.getId(), t14.getId()));
    CPPUNIT_ASSERT(!near.compare(t14.getId(), t9.getId()));
    CPPUNIT_ASSERT(near.compare(t9.getId(), t15.getId()));
    CPPUNIT_ASSERT(!near.compare(t15.getId(), t9.getId()));


    CPPUNIT_ASSERT(!near.compare(t10.getId(), t11.getId()));
    CPPUNIT_ASSERT(!near.compare(t11.getId(), t12.getId()));
    CPPUNIT_ASSERT(!near.compare(t12.getId(), t10.getId()));

    CPPUNIT_ASSERT(!near.compare(t10.getId(), t13.getId()));
    CPPUNIT_ASSERT(!near.compare(t13.getId(), t10.getId()));
    CPPUNIT_ASSERT(!near.compare(t10.getId(), t14.getId()));
    CPPUNIT_ASSERT(!near.compare(t14.getId(), t10.getId()));
    CPPUNIT_ASSERT(!near.compare(t10.getId(), t15.getId()));
    CPPUNIT_ASSERT(!near.compare(t15.getId(), t10.getId()));

    CPPUNIT_ASSERT(!near.compare(t11.getId(), t13.getId()));
    CPPUNIT_ASSERT(!near.compare(t13.getId(), t11.getId()));
    CPPUNIT_ASSERT(!near.compare(t11.getId(), t14.getId()));
    CPPUNIT_ASSERT(!near.compare(t14.getId(), t11.getId()));
    CPPUNIT_ASSERT(!near.compare(t11.getId(), t15.getId()));
    CPPUNIT_ASSERT(!near.compare(t15.getId(), t11.getId()));

    CPPUNIT_ASSERT(!near.compare(t12.getId(), t13.getId()));
    CPPUNIT_ASSERT(!near.compare(t13.getId(), t12.getId()));
    CPPUNIT_ASSERT(!near.compare(t12.getId(), t14.getId()));
    CPPUNIT_ASSERT(!near.compare(t14.getId(), t12.getId()));
    CPPUNIT_ASSERT(!near.compare(t12.getId(), t15.getId()));
    CPPUNIT_ASSERT(!near.compare(t15.getId(), t12.getId()));

    CPPUNIT_ASSERT(!near.compare(t13.getId(), t14.getId()));
    CPPUNIT_ASSERT(!near.compare(t14.getId(), t15.getId()));
    CPPUNIT_ASSERT(!near.compare(t15.getId(), t13.getId()));


    FarTokenComparator far(foo.getId());
    CPPUNIT_ASSERT(!far.compare(foo.getId(), foo.getId()));
    CPPUNIT_ASSERT(!far.compare(t4.getId(), t5.getId()));
    CPPUNIT_ASSERT(!far.compare(t5.getId(), t6.getId()));
    CPPUNIT_ASSERT(!far.compare(t4.getId(), t6.getId()));

    CPPUNIT_ASSERT(!far.compare(t4.getId(), t7.getId()));
    CPPUNIT_ASSERT(!far.compare(t7.getId(), t4.getId()));
    CPPUNIT_ASSERT(!far.compare(t4.getId(), t8.getId()));
    CPPUNIT_ASSERT(!far.compare(t8.getId(), t4.getId()));
    CPPUNIT_ASSERT(!far.compare(t4.getId(), t9.getId()));
    CPPUNIT_ASSERT(!far.compare(t9.getId(), t4.getId()));

    CPPUNIT_ASSERT(!far.compare(t4.getId(), t10.getId()));
    CPPUNIT_ASSERT(far.compare(t10.getId(), t4.getId()));
    CPPUNIT_ASSERT(!far.compare(t4.getId(), t11.getId()));
    CPPUNIT_ASSERT(far.compare(t11.getId(), t4.getId()));
    CPPUNIT_ASSERT(!far.compare(t4.getId(), t12.getId()));
    CPPUNIT_ASSERT(far.compare(t12.getId(), t4.getId()));

    CPPUNIT_ASSERT(!far.compare(t4.getId(), t13.getId()));
    CPPUNIT_ASSERT(far.compare(t13.getId(), t4.getId()));
    CPPUNIT_ASSERT(!far.compare(t4.getId(), t14.getId()));
    CPPUNIT_ASSERT(far.compare(t14.getId(), t4.getId()));
    CPPUNIT_ASSERT(!far.compare(t4.getId(), t15.getId()));
    CPPUNIT_ASSERT(far.compare(t15.getId(), t4.getId()));


    CPPUNIT_ASSERT(!far.compare(t5.getId(), t7.getId()));
    CPPUNIT_ASSERT(!far.compare(t7.getId(), t5.getId()));
    CPPUNIT_ASSERT(!far.compare(t5.getId(), t8.getId()));
    CPPUNIT_ASSERT(!far.compare(t8.getId(), t5.getId()));
    CPPUNIT_ASSERT(!far.compare(t5.getId(), t9.getId()));
    CPPUNIT_ASSERT(!far.compare(t9.getId(), t5.getId()));

    CPPUNIT_ASSERT(!far.compare(t5.getId(), t10.getId()));
    CPPUNIT_ASSERT(far.compare(t10.getId(), t5.getId()));
    CPPUNIT_ASSERT(!far.compare(t5.getId(), t11.getId()));
    CPPUNIT_ASSERT(far.compare(t11.getId(), t5.getId()));
    CPPUNIT_ASSERT(!far.compare(t5.getId(), t12.getId()));
    CPPUNIT_ASSERT(far.compare(t12.getId(), t5.getId()));

    CPPUNIT_ASSERT(!far.compare(t5.getId(), t13.getId()));
    CPPUNIT_ASSERT(far.compare(t13.getId(), t5.getId()));
    CPPUNIT_ASSERT(!far.compare(t5.getId(), t14.getId()));
    CPPUNIT_ASSERT(far.compare(t14.getId(), t5.getId()));
    CPPUNIT_ASSERT(!far.compare(t5.getId(), t15.getId()));
    CPPUNIT_ASSERT(far.compare(t15.getId(), t5.getId()));


    CPPUNIT_ASSERT(!far.compare(t6.getId(), t7.getId()));
    CPPUNIT_ASSERT(!far.compare(t7.getId(), t6.getId()));
    CPPUNIT_ASSERT(!far.compare(t6.getId(), t8.getId()));
    CPPUNIT_ASSERT(!far.compare(t8.getId(), t6.getId()));
    CPPUNIT_ASSERT(!far.compare(t6.getId(), t9.getId()));
    CPPUNIT_ASSERT(!far.compare(t9.getId(), t6.getId()));

    CPPUNIT_ASSERT(!far.compare(t6.getId(), t10.getId()));
    CPPUNIT_ASSERT(far.compare(t10.getId(), t6.getId()));
    CPPUNIT_ASSERT(!far.compare(t6.getId(), t11.getId()));
    CPPUNIT_ASSERT(far.compare(t11.getId(), t6.getId()));
    CPPUNIT_ASSERT(!far.compare(t6.getId(), t12.getId()));
    CPPUNIT_ASSERT(far.compare(t12.getId(), t6.getId()));

    CPPUNIT_ASSERT(!far.compare(t6.getId(), t13.getId()));
    CPPUNIT_ASSERT(far.compare(t13.getId(), t6.getId()));
    CPPUNIT_ASSERT(!far.compare(t6.getId(), t14.getId()));
    CPPUNIT_ASSERT(far.compare(t14.getId(), t6.getId()));
    CPPUNIT_ASSERT(!far.compare(t6.getId(), t15.getId()));
    CPPUNIT_ASSERT(far.compare(t15.getId(), t6.getId()));




    CPPUNIT_ASSERT(!far.compare(t7.getId(), t8.getId()));
    CPPUNIT_ASSERT(!far.compare(t8.getId(), t9.getId()));
    CPPUNIT_ASSERT(!far.compare(t9.getId(), t7.getId()));

    CPPUNIT_ASSERT(!far.compare(t7.getId(), t10.getId()));
    CPPUNIT_ASSERT(far.compare(t10.getId(), t7.getId()));
    CPPUNIT_ASSERT(!far.compare(t7.getId(), t11.getId()));
    CPPUNIT_ASSERT(far.compare(t11.getId(), t7.getId()));
    CPPUNIT_ASSERT(!far.compare(t7.getId(), t12.getId()));
    CPPUNIT_ASSERT(far.compare(t12.getId(), t7.getId()));

    CPPUNIT_ASSERT(!far.compare(t7.getId(), t13.getId()));
    CPPUNIT_ASSERT(far.compare(t13.getId(), t7.getId()));
    CPPUNIT_ASSERT(!far.compare(t7.getId(), t14.getId()));
    CPPUNIT_ASSERT(far.compare(t14.getId(), t7.getId()));
    CPPUNIT_ASSERT(!far.compare(t7.getId(), t15.getId()));
    CPPUNIT_ASSERT(far.compare(t15.getId(), t7.getId()));

    CPPUNIT_ASSERT(!far.compare(t8.getId(), t10.getId()));
    CPPUNIT_ASSERT(far.compare(t10.getId(), t8.getId()));
    CPPUNIT_ASSERT(!far.compare(t8.getId(), t11.getId()));
    CPPUNIT_ASSERT(far.compare(t11.getId(), t8.getId()));
    CPPUNIT_ASSERT(!far.compare(t8.getId(), t12.getId()));
    CPPUNIT_ASSERT(far.compare(t12.getId(), t8.getId()));

    CPPUNIT_ASSERT(!far.compare(t8.getId(), t13.getId()));
    CPPUNIT_ASSERT(far.compare(t13.getId(), t8.getId()));
    CPPUNIT_ASSERT(!far.compare(t8.getId(), t14.getId()));
    CPPUNIT_ASSERT(far.compare(t14.getId(), t8.getId()));
    CPPUNIT_ASSERT(!far.compare(t8.getId(), t15.getId()));
    CPPUNIT_ASSERT(far.compare(t15.getId(), t8.getId()));


    CPPUNIT_ASSERT(!far.compare(t9.getId(), t10.getId()));
    CPPUNIT_ASSERT(far.compare(t10.getId(), t9.getId()));
    CPPUNIT_ASSERT(!far.compare(t9.getId(), t11.getId()));
    CPPUNIT_ASSERT(far.compare(t11.getId(), t9.getId()));
    CPPUNIT_ASSERT(!far.compare(t9.getId(), t12.getId()));
    CPPUNIT_ASSERT(far.compare(t12.getId(), t9.getId()));

    CPPUNIT_ASSERT(!far.compare(t9.getId(), t13.getId()));
    CPPUNIT_ASSERT(far.compare(t13.getId(), t9.getId()));
    CPPUNIT_ASSERT(!far.compare(t9.getId(), t14.getId()));
    CPPUNIT_ASSERT(far.compare(t14.getId(), t9.getId()));
    CPPUNIT_ASSERT(!far.compare(t9.getId(), t15.getId()));
    CPPUNIT_ASSERT(far.compare(t15.getId(), t9.getId()));


    CPPUNIT_ASSERT(!far.compare(t10.getId(), t11.getId()));
    CPPUNIT_ASSERT(!far.compare(t11.getId(), t12.getId()));
    CPPUNIT_ASSERT(!far.compare(t12.getId(), t10.getId()));

    CPPUNIT_ASSERT(!far.compare(t10.getId(), t13.getId()));
    CPPUNIT_ASSERT(!far.compare(t13.getId(), t10.getId()));
    CPPUNIT_ASSERT(!far.compare(t10.getId(), t14.getId()));
    CPPUNIT_ASSERT(!far.compare(t14.getId(), t10.getId()));
    CPPUNIT_ASSERT(!far.compare(t10.getId(), t15.getId()));
    CPPUNIT_ASSERT(!far.compare(t15.getId(), t10.getId()));

    CPPUNIT_ASSERT(!far.compare(t11.getId(), t13.getId()));
    CPPUNIT_ASSERT(!far.compare(t13.getId(), t11.getId()));
    CPPUNIT_ASSERT(!far.compare(t11.getId(), t14.getId()));
    CPPUNIT_ASSERT(!far.compare(t14.getId(), t11.getId()));
    CPPUNIT_ASSERT(!far.compare(t11.getId(), t15.getId()));
    CPPUNIT_ASSERT(!far.compare(t15.getId(), t11.getId()));

    CPPUNIT_ASSERT(!far.compare(t12.getId(), t13.getId()));
    CPPUNIT_ASSERT(!far.compare(t13.getId(), t12.getId()));
    CPPUNIT_ASSERT(!far.compare(t12.getId(), t14.getId()));
    CPPUNIT_ASSERT(!far.compare(t14.getId(), t12.getId()));
    CPPUNIT_ASSERT(!far.compare(t12.getId(), t15.getId()));
    CPPUNIT_ASSERT(!far.compare(t15.getId(), t12.getId()));

    CPPUNIT_ASSERT(!far.compare(t13.getId(), t14.getId()));
    CPPUNIT_ASSERT(!far.compare(t14.getId(), t15.getId()));
    CPPUNIT_ASSERT(!far.compare(t15.getId(), t13.getId()));

    return true;
  }

  static bool testValueEnum() {
    TestEngine testEngine;
    PlanDatabaseId db = testEngine.getPlanDatabase();
    ConstraintEngineId ce = testEngine.getConstraintEngine();

    Variable<IntervalIntDomain> intIntVar(ce, IntervalIntDomain(1, 5), false, true);

    std::list<double> ints;
    ints.push_back(1);

    EnumeratedDomain singletonEnumIntDom(IntDT::instance(),ints);
    Variable<EnumeratedDomain> singletonEnumIntVar(ce, singletonEnumIntDom, false, true);

    ints.push_back(2);
    ints.push_back(3);
    ints.push_back(4);
    ints.push_back(5);
    EnumeratedDomain enumIntDom(IntDT::instance(),ints);
    Variable<EnumeratedDomain> enumIntVar(ce, enumIntDom, false, true);

    std::list<double> strings;
    strings.push_back(LabelStr("foo"));
    strings.push_back(LabelStr("bar"));
    strings.push_back(LabelStr("baz"));
    strings.push_back(LabelStr("quux"));
    EnumeratedDomain enumStrDom(StringDT::instance(),strings);
    Variable<EnumeratedDomain> enumStrVar(ce, enumStrDom, false, true);


    EnumeratedDomain enumObjDom(db->getSchema()->getObjectType("A")->getVarType());
    Variable<EnumeratedDomain> enumObjVar(ce, enumObjDom, false, true);
    Object o1(db, "A", "o1");
    Object o2(db, "A", "o2");
    Object o3(db, "A", "o3");
    Object o4(db, "A", "o4");
    db->makeObjectVariableFromType("A", enumObjVar.getId());

    std::string intHeur("<FlawHandler component=\"ValueEnum\">\
                           <Value val=\"4\"/>\
                           <Value val=\"1\"/>\
                           <Value val=\"5\"/>\
                           <Value val=\"2\"/>\
                           <Value val=\"3\"/>\
                         </FlawHandler>");
    TiXmlElement* intHeurXml = initXml(intHeur);
    CPPUNIT_ASSERT(intHeurXml != NULL);

    std::string intTrimHeur("<FlawHandler component=\"ValueEnum\">\
                               <Value val=\"3\"/>\
                               <Value val=\"2\"/>\
                               <Value val=\"4\"/>\
                             </FlawHandler>");
    TiXmlElement* intTrimHeurXml = initXml(intTrimHeur);
    CPPUNIT_ASSERT(intTrimHeurXml != NULL);

    std::string strHeur("<FlawHandler component=\"ValueEnum\">\
                           <Value val=\"baz\"/>\
                           <Value val=\"quux\"/>\
                           <Value val=\"bar\"/>\
                           <Value val=\"foo\"/>\
                         </FlawHandler>");
    TiXmlElement* strHeurXml = initXml(strHeur);
    CPPUNIT_ASSERT(strHeurXml != NULL);

    std::string strTrimHeur("<FlawHandler component=\"ValueEnum\">\
                               <Value val=\"quux\"/>\
                               <Value val=\"foo\"/>\
                             </FlawHandler>");
    TiXmlElement* strTrimHeurXml = initXml(strTrimHeur);
    CPPUNIT_ASSERT(strTrimHeurXml != NULL);

    std::string objHeur("<FlawHandler component=\"ValueEnum\">\
                           <Value val=\"o3\"/>\
                           <Value val=\"o2\"/>\
                           <Value val=\"o4\"/>\
                           <Value val=\"o1\"/>\
                         </FlawHandler>");
    TiXmlElement* objHeurXml = initXml(objHeur);
    CPPUNIT_ASSERT(objHeurXml != NULL);

    std::string objTrimHeur("<FlawHandler component=\"ValueEnum\">\
                               <Value val=\"o1\"/>\
                               <Value val=\"o4\"/>\
                               <Value val=\"o2\"/>\
                             </FlawHandler>");
    TiXmlElement* objTrimHeurXml = initXml(objTrimHeur);
    CPPUNIT_ASSERT(objTrimHeurXml != NULL);

    ValueEnum intIntDP(db->getClient(), intIntVar.getId(), *intHeurXml);
    ValueEnum enumIntDP(db->getClient(), enumIntVar.getId(), *intHeurXml);

    CPPUNIT_ASSERT(intIntDP.getNext() == 4);
    CPPUNIT_ASSERT(enumIntDP.getNext() == 4);
    CPPUNIT_ASSERT(intIntDP.getNext() == 1);
    CPPUNIT_ASSERT(enumIntDP.getNext() == 1);
    CPPUNIT_ASSERT(intIntDP.getNext() == 5);
    CPPUNIT_ASSERT(enumIntDP.getNext() == 5);
    CPPUNIT_ASSERT(intIntDP.getNext() == 2);
    CPPUNIT_ASSERT(enumIntDP.getNext() == 2);
    CPPUNIT_ASSERT(intIntDP.getNext() == 3);
    CPPUNIT_ASSERT(enumIntDP.getNext() == 3);

    ValueEnum trimIntIntDP(db->getClient(), intIntVar.getId(), *intTrimHeurXml);
    ValueEnum trimEnumIntDP(db->getClient(), enumIntVar.getId(), *intTrimHeurXml);

    CPPUNIT_ASSERT(trimIntIntDP.getNext() == 3);
    CPPUNIT_ASSERT(trimEnumIntDP.getNext() == 3);
    CPPUNIT_ASSERT(trimIntIntDP.getNext() == 2);
    CPPUNIT_ASSERT(trimEnumIntDP.getNext() == 2);
    CPPUNIT_ASSERT(trimIntIntDP.getNext() == 4);
    CPPUNIT_ASSERT(trimEnumIntDP.getNext() == 4);

    ValueEnum enumStrDP(db->getClient(), enumStrVar.getId(), *strHeurXml);

    CPPUNIT_ASSERT(enumStrDP.getNext() == LabelStr("baz"));
    CPPUNIT_ASSERT(enumStrDP.getNext() == LabelStr("quux"));
    CPPUNIT_ASSERT(enumStrDP.getNext() == LabelStr("bar"));
    CPPUNIT_ASSERT(enumStrDP.getNext() == LabelStr("foo"));

    ValueEnum trimEnumStrDP(db->getClient(), enumStrVar.getId(), *strTrimHeurXml);

    CPPUNIT_ASSERT(trimEnumStrDP.getNext() == LabelStr("quux"));
    CPPUNIT_ASSERT(trimEnumStrDP.getNext() == LabelStr("foo"));

    ValueEnum enumObjDP(db->getClient(), enumObjVar.getId(), *objHeurXml);

    CPPUNIT_ASSERT(enumObjDP.getNext() == o3.getId());
    CPPUNIT_ASSERT(enumObjDP.getNext() == o2.getId());
    CPPUNIT_ASSERT(enumObjDP.getNext() == o4.getId());
    CPPUNIT_ASSERT(enumObjDP.getNext() == o1.getId());

    ValueEnum trimEnumObjDP(db->getClient(), enumObjVar.getId(), *objTrimHeurXml);

    CPPUNIT_ASSERT(trimEnumObjDP.getNext() == o1.getId());
    CPPUNIT_ASSERT(trimEnumObjDP.getNext() == o4.getId());
    CPPUNIT_ASSERT(trimEnumObjDP.getNext() == o2.getId());

    //the value enumeration should be ignored if the domain is a singleton
    ValueEnum ignoreHeurDP(db->getClient(), singletonEnumIntVar.getId(), *intTrimHeurXml);

    CPPUNIT_ASSERT(ignoreHeurDP.getNext() == 1);

    delete objTrimHeurXml;
    delete objHeurXml;
    delete strTrimHeurXml;
    delete strHeurXml;
    delete intTrimHeurXml;
    delete intHeurXml;

    return true;
  }
  static bool testHSTSOpenConditionDecisionPoint() {
    TestEngine testEngine;
    testEngine.getSchema()->addPredicate("A.Foo");
    PlanDatabaseId db = testEngine.getPlanDatabase();
    DbClientId client = db->getClient();
    Object o1(db, "A", "o1");

    //flawed token that should be compatible with everything
    IntervalToken flawedToken(db, "A.Foo", false, false);

    CPPUNIT_ASSERT(client->propagate());
    //test activate only
    std::string aOnlyHeur("<FlawHandler component=\"HSTSOpenConditionDecisionPoint\" choice=\"activateOnly\"/>");
    TiXmlElement* aOnlyHeurXml = initXml(aOnlyHeur);
    HSTS::OpenConditionDecisionPoint aOnly(client, flawedToken.getId(), *aOnlyHeurXml);
    aOnly.initialize();
    CPPUNIT_ASSERT(aOnly.getStateChoices().size() == 1);
    CPPUNIT_ASSERT(aOnly.getStateChoices()[0] == Token::ACTIVE);
    CPPUNIT_ASSERT(aOnly.getCompatibleTokens().empty());

    //test merge only
    IntervalToken tok1(db, "A.Foo", false, false, IntervalIntDomain(1, 10), IntervalIntDomain(6, 20), IntervalIntDomain(5, 10), "o1");
    client->activate(tok1.getId());
    std::string mOnlyHeur("<FlawHandler component=\"HSTSOpenConditionDecisionPoint\" choice=\"mergeOnly\"/>");
    TiXmlElement* mOnlyHeurXml = initXml(mOnlyHeur);
    HSTS::OpenConditionDecisionPoint mOnly(client, flawedToken.getId(), *mOnlyHeurXml);
    CPPUNIT_ASSERT(client->propagate());
    mOnly.initialize();
    CPPUNIT_ASSERT(mOnly.getStateChoices().size() == 1);
    CPPUNIT_ASSERT(mOnly.getStateChoices()[0] == Token::MERGED);
    CPPUNIT_ASSERT(mOnly.getCompatibleTokens().size() == 1);
    CPPUNIT_ASSERT(mOnly.getCompatibleTokens()[0] == tok1.getId());

    //test activate/merge
    std::string aFirstHeur("<FlawHandler component=\"HSTSOpenConditionDecisionPoint\" choice=\"activateFirst\"/>");
    TiXmlElement* aFirstHeurXml = initXml(aFirstHeur);
    HSTS::OpenConditionDecisionPoint aFirst(client, flawedToken.getId(), *aFirstHeurXml);
    CPPUNIT_ASSERT(client->propagate());
    aFirst.initialize();
    CPPUNIT_ASSERT(aFirst.getStateChoices().size() == 2);
    CPPUNIT_ASSERT(aFirst.getStateChoices()[0] == Token::ACTIVE);
    CPPUNIT_ASSERT(aFirst.getStateChoices()[1] == Token::MERGED);
    CPPUNIT_ASSERT(aFirst.getCompatibleTokens().size() == 1);
    CPPUNIT_ASSERT(aFirst.getCompatibleTokens()[0] == tok1.getId());

    //test merge/activate
    std::string mFirstHeur("<FlawHandler component=\"HSTSOpenConditionDecisionPoint\" choice=\"mergeFirst\"/>");
    TiXmlElement* mFirstHeurXml = initXml(mFirstHeur);
    HSTS::OpenConditionDecisionPoint mFirst(client, flawedToken.getId(), *mFirstHeurXml);
    CPPUNIT_ASSERT(client->propagate());
    mFirst.initialize();
    CPPUNIT_ASSERT(mFirst.getStateChoices().size() == 2);
    CPPUNIT_ASSERT(mFirst.getStateChoices()[0] == Token::MERGED);
    CPPUNIT_ASSERT(mFirst.getStateChoices()[1] == Token::ACTIVE);
    CPPUNIT_ASSERT(mFirst.getCompatibleTokens().size() == 1);
    CPPUNIT_ASSERT(mFirst.getCompatibleTokens()[0] == tok1.getId());

    //test early
    IntervalToken tok2(db, "A.Foo", false, false, IntervalIntDomain(3, 10), IntervalIntDomain(8, 20), IntervalIntDomain(5, 10), "o1");
    client->activate(tok2.getId());
    std::string mEarlyHeur("<FlawHandler component=\"HSTSOpenConditionDecisionPoint\" choice=\"mergeOnly\" order=\"early\"/>");
    TiXmlElement* mEarlyHeurXml = initXml(mEarlyHeur);
    HSTS::OpenConditionDecisionPoint mEarly(client, flawedToken.getId(), *mEarlyHeurXml);
    CPPUNIT_ASSERT(client->propagate());
    mEarly.initialize();
    CPPUNIT_ASSERT(mEarly.getStateChoices().size() == 1);
    CPPUNIT_ASSERT(mEarly.getStateChoices()[0] == Token::MERGED);
    CPPUNIT_ASSERT(mEarly.getCompatibleTokens().size() == 2);
    CPPUNIT_ASSERT(mEarly.getCompatibleTokens()[0] == tok1.getId());
    CPPUNIT_ASSERT(mEarly.getCompatibleTokens()[1] == tok2.getId());

    //test late
    std::string mLateHeur("<FlawHandler component=\"HSTSOpenConditionDecisionPoint\" choice=\"mergeOnly\" order=\"late\"/>");
    TiXmlElement* mLateHeurXml = initXml(mLateHeur);
    HSTS::OpenConditionDecisionPoint mLate(client, flawedToken.getId(), *mLateHeurXml);
    CPPUNIT_ASSERT(client->propagate());
    mLate.initialize();
    CPPUNIT_ASSERT(mLate.getStateChoices().size() == 1);
    CPPUNIT_ASSERT(mLate.getStateChoices()[0] == Token::MERGED);
    CPPUNIT_ASSERT(mLate.getCompatibleTokens().size() == 2);
    CPPUNIT_ASSERT(mLate.getCompatibleTokens()[0] == tok2.getId());
    CPPUNIT_ASSERT(mLate.getCompatibleTokens()[1] == tok1.getId());

    //test near
    //tok1 has midpoint at 11, tok2 has midpoint at 13, so put flawedToken's midpoint at 10
    const_cast<AbstractDomain&>(flawedToken.start()->lastDomain()).intersect(4, 10);
    const_cast<AbstractDomain&>(flawedToken.end()->lastDomain()).intersect(5, 12);
    CPPUNIT_ASSERT(testEngine.getConstraintEngine()->propagate());
    std::string mNearHeur("<FlawHandler component=\"HSTSOpenConditionDecisionPoint\" choice=\"mergeOnly\" order=\"near\"/>");
    TiXmlElement* mNearHeurXml = initXml(mNearHeur);
    HSTS::OpenConditionDecisionPoint mNear(client, flawedToken.getId(), *mNearHeurXml);
    CPPUNIT_ASSERT(client->propagate());
    mNear.initialize();
    CPPUNIT_ASSERT(mNear.getStateChoices().size() == 1);
    CPPUNIT_ASSERT(mNear.getStateChoices()[0] == Token::MERGED);
    CPPUNIT_ASSERT(mNear.getCompatibleTokens().size() == 2);
    CPPUNIT_ASSERT(mNear.getCompatibleTokens()[0] == tok1.getId());
    CPPUNIT_ASSERT(mNear.getCompatibleTokens()[1] == tok2.getId());

    //test far
    std::string mFarHeur("<FlawHandler component=\"HSTSOpenConditionDecisionPoint\" choice=\"mergeOnly\" order=\"far\"/>");
    TiXmlElement* mFarHeurXml = initXml(mFarHeur);
    HSTS::OpenConditionDecisionPoint mFar(client, flawedToken.getId(), *mFarHeurXml);
    mFar.initialize();
    CPPUNIT_ASSERT(mFar.getStateChoices().size() == 1);
    CPPUNIT_ASSERT(mFar.getStateChoices()[0] == Token::MERGED);
    CPPUNIT_ASSERT(mFar.getCompatibleTokens().size() == 2);
    CPPUNIT_ASSERT(mFar.getCompatibleTokens()[0] == tok2.getId());
    CPPUNIT_ASSERT(mFar.getCompatibleTokens()[1] == tok1.getId());

    delete aOnlyHeurXml;
    delete mOnlyHeurXml;
    delete aFirstHeurXml;
    delete mFirstHeurXml;
    delete mEarlyHeurXml;
    delete mLateHeurXml;
    delete mNearHeurXml;
    delete mFarHeurXml;
    return true;
  }
  static bool testHSTSThreatDecisionPoint() {
    TestEngine testEngine;
    testEngine.getSchema()->addPredicate("A.Foo");
    PlanDatabaseId db = testEngine.getPlanDatabase();
    DbClientId client = db->getClient();
    Timeline o1(db, "A", "o1");

    IntervalToken tok1(db, "A.Foo", false, false, IntervalIntDomain(3, 10), IntervalIntDomain(7, 14),
                       IntervalIntDomain(4, 4), "o1");
    client->activate(tok1.getId());

    IntervalToken tok2(db, "A.Foo", false, false, IntervalIntDomain(7, 16), IntervalIntDomain(8, 20),
                       IntervalIntDomain(1, 4), "o1");
    client->activate(tok2.getId());

    client->constrain(o1.getId(), tok1.getId(), tok1.getId());
    client->constrain(o1.getId(), tok1.getId(), tok2.getId());

    IntervalToken flawedToken(db, "A.Foo", false, false, IntervalIntDomain(1, 10), IntervalIntDomain(4, 20),
                              IntervalIntDomain(3, 10), "o1");
    client->activate(flawedToken.getId());

    std::string earlyHeur("<FlawHandler component=\"HSTSThreatDecisionPoint\" order=\"early\"/>");
    TiXmlElement* earlyHeurXml = initXml(earlyHeur);
    HSTS::ThreatDecisionPoint early(client, flawedToken.getId(), *earlyHeurXml);
    CPPUNIT_ASSERT(client->propagate());
    early.initialize();
    CPPUNIT_ASSERT(early.getOrderingChoices().size() == 3);
    CPPUNIT_ASSERT(early.getOrderingChoices()[0].second.second == tok1.getId());
    CPPUNIT_ASSERT(early.getOrderingChoices()[1].second.second == tok2.getId());
    CPPUNIT_ASSERT(early.getOrderingChoices()[2].second.second == flawedToken.getId());

    std::string lateHeur("<FlawHandler component=\"HSTSThreatDecisionPoint\" order=\"late\"/>");
    TiXmlElement* lateHeurXml = initXml(lateHeur);
    HSTS::ThreatDecisionPoint late(client, flawedToken.getId(), *lateHeurXml);
    CPPUNIT_ASSERT(client->propagate());
    late.initialize();
    CPPUNIT_ASSERT(late.getOrderingChoices().size() == 3);
    CPPUNIT_ASSERT(late.getOrderingChoices()[0].second.second == flawedToken.getId());
    CPPUNIT_ASSERT(late.getOrderingChoices()[1].second.second == tok2.getId());
    CPPUNIT_ASSERT(late.getOrderingChoices()[2].second.second == tok1.getId());

    std::string nearHeur("<FlawHandler component=\"HSTSThreatDecisionPoint\" order=\"near\"/>");
    TiXmlElement* nearHeurXml = initXml(nearHeur);
    HSTS::ThreatDecisionPoint near(client, flawedToken.getId(), *nearHeurXml);
    CPPUNIT_ASSERT(client->propagate());
    near.initialize();
    CPPUNIT_ASSERT(near.getOrderingChoices().size() == 3);
    CPPUNIT_ASSERT(near.getOrderingChoices()[0].second.second == tok1.getId());
    CPPUNIT_ASSERT(near.getOrderingChoices()[1].second.second == tok2.getId() ||
               near.getOrderingChoices()[1].second.second == flawedToken.getId());
    CPPUNIT_ASSERT(near.getOrderingChoices()[2].second.second == flawedToken.getId() ||
               near.getOrderingChoices()[2].second.second == tok2.getId());


    std::string farHeur("<FlawHandler component=\"HSTSThreatDecisionPoint\" order=\"far\"/>");
    TiXmlElement* farHeurXml = initXml(farHeur);
    HSTS::ThreatDecisionPoint far(client, flawedToken.getId(), *farHeurXml);
    CPPUNIT_ASSERT(client->propagate());
    far.initialize();
    CPPUNIT_ASSERT(far.getOrderingChoices().size() == 3);
    CPPUNIT_ASSERT(far.getOrderingChoices()[0].second.second == tok2.getId() ||
               far.getOrderingChoices()[0].second.second == flawedToken.getId());
    CPPUNIT_ASSERT(far.getOrderingChoices()[1].second.second == flawedToken.getId() ||
               far.getOrderingChoices()[1].second.second == tok2.getId());
    CPPUNIT_ASSERT(far.getOrderingChoices()[2].second.second == tok1.getId());

    delete earlyHeurXml;
    delete lateHeurXml;
    delete nearHeurXml;
    delete farHeurXml;
    return true;
  }
};

class SolverTests {
public:
  static bool test(){
    EUROPA_runTest(testMinValuesSimpleCSP);
    EUROPA_runTest(testSuccessfulSearch);
    EUROPA_runTest(testExhaustiveSearch);
    EUROPA_runTest(testSimpleActivation);
    EUROPA_runTest(testSimpleRejection);
    EUROPA_runTest(testMultipleSearch);
    EUROPA_runTest(testOversearch);
    EUROPA_runTest(testBacktrackFirstDecisionPoint);
    EUROPA_runTest(testMultipleSolutionsSearch);
    EUROPA_runTest(testGNATS_3196);
    EUROPA_runTest(testContext);
    EUROPA_runTest(testDeletedFlaw);
    EUROPA_runTest(testDeleteAfterCommit);
    EUROPA_runTest(testSingleonGuardLoop);
    return true;
  }

private:

  /**
   * @brief Tests for an infinite loop when binding a singleton guard.
   */
  static bool testSingleonGuardLoop() {
    TestEngine testEngine;
    TiXmlElement* root = initXml( (getTestLoadLibraryPath() + "/SolverTests.xml").c_str(), "SingletonLoop");
    TiXmlElement* child = root->FirstChildElement();
    {
      CPPUNIT_ASSERT(testEngine.playTransactions( (getTestLoadLibraryPath() + "/SingletonGuardLoopTest.xml").c_str()));
      Solver solver(testEngine.getPlanDatabase(), *child);
      CPPUNIT_ASSERT(solver.solve(50, 50));
      CPPUNIT_ASSERT(solver.getStepCount() == solver.getDepth());
    }
    return true;
  }

  /**
   * @brief Will load an intial state and solve a csp with only variables.
   */
  static bool testMinValuesSimpleCSP(){
    TestEngine testEngine;
    TiXmlElement* root = initXml( (getTestLoadLibraryPath() + "/SolverTests.xml").c_str(), "SimpleCSPSolver");
    TiXmlElement* child = root->FirstChildElement();
    {
      CPPUNIT_ASSERT(testEngine.playTransactions( (getTestLoadLibraryPath() + "/StaticCSP.xml").c_str()));
      Solver solver(testEngine.getPlanDatabase(), *child);
      CPPUNIT_ASSERT(solver.solve());
      CPPUNIT_ASSERT(solver.getStepCount() == solver.getDepth());
      const ConstrainedVariableSet& allVars = testEngine.getPlanDatabase()->getGlobalVariables();
      for(ConstrainedVariableSet::const_iterator it = allVars.begin(); it != allVars.end(); ++it){
        ConstrainedVariableId var = *it;
        CPPUNIT_ASSERT(var->lastDomain().isSingleton());
      }

      // Run the solver again.
      CPPUNIT_ASSERT(solver.solve());
      CPPUNIT_ASSERT(solver.getStepCount() == 2);
      CPPUNIT_ASSERT(solver.getDepth() == 2);

      // Now clear it and run it again
      solver.reset();
      CPPUNIT_ASSERT(solver.solve());
      CPPUNIT_ASSERT(solver.getStepCount() == 2);
      CPPUNIT_ASSERT(solver.getDepth() == 2);

      // Now partially reset it, and run again
      solver.reset(1);
      CPPUNIT_ASSERT(solver.solve());
      CPPUNIT_ASSERT_MESSAGE(toString(solver.getStepCount()), solver.getStepCount() == 1);
      CPPUNIT_ASSERT_MESSAGE(toString(solver.getDepth()), solver.getDepth() == 2);

      // Now we reset one decision, then clear it. Expect the solution and depth to be 1.
      solver.reset(1);
      solver.clear();
      CPPUNIT_ASSERT(solver.solve());
      CPPUNIT_ASSERT_MESSAGE(toString(solver.getStepCount()), solver.getStepCount() == 1);
      CPPUNIT_ASSERT_MESSAGE(toString(solver.getDepth()), solver.getDepth() == 1);
    }

    return true;
  }


  static bool testSuccessfulSearch(){
    TestEngine testEngine;
    TiXmlElement* root = initXml( (getTestLoadLibraryPath() + "/SolverTests.xml").c_str(), "SimpleCSPSolver");
    TiXmlElement* child = root->FirstChildElement();
    {
      CPPUNIT_ASSERT(testEngine.playTransactions((getTestLoadLibraryPath() + "/SuccessfulSearch.xml").c_str()));
      Solver solver(testEngine.getPlanDatabase(), *child);
      CPPUNIT_ASSERT(solver.solve());
    }
    return true;
  }

  static bool testExhaustiveSearch(){
    TestEngine testEngine;
    TiXmlElement* root = initXml((getTestLoadLibraryPath() + "/SolverTests.xml").c_str(), "SimpleCSPSolver");
    TiXmlElement* child = root->FirstChildElement();
    {
      CPPUNIT_ASSERT(testEngine.playTransactions((getTestLoadLibraryPath() + "/ExhaustiveSearch.xml").c_str()));
      Solver solver(testEngine.getPlanDatabase(), *child);
      CPPUNIT_ASSERT(!solver.solve());

      debugMsg("SolverTests:testExhaustinveSearch", "Step count == " << solver.getStepCount());

      const ConstrainedVariableSet& allVars = testEngine.getPlanDatabase()->getGlobalVariables();
      unsigned int stepCount = 0;
      unsigned int product = 1;
      for(ConstrainedVariableSet::const_iterator it = allVars.begin(); it != allVars.end(); ++it){
        static const unsigned int baseDomainSize = (*it)->baseDomain().getSize();
        stepCount = stepCount + (product*baseDomainSize);
        product = product*baseDomainSize;
      }

      CPPUNIT_ASSERT(solver.getStepCount() == stepCount);
    }
    return true;
  }

  static bool testSimpleActivation() {
    TestEngine testEngine;
    TiXmlElement* root = initXml((getTestLoadLibraryPath() + "/SolverTests.xml").c_str(), "SimpleActivationSolver");
    TiXmlElement* child = root->FirstChildElement();
    {
      IntervalIntDomain& horizon = HorizonFilter::getHorizon();
      horizon = IntervalIntDomain(0, 1000);
      CPPUNIT_ASSERT(testEngine.playTransactions((getTestLoadLibraryPath() + "/SimpleActivation.xml").c_str()));
      Solver solver(testEngine.getPlanDatabase(), *child);
      CPPUNIT_ASSERT(solver.solve());
    }

    return true;
  }

  static bool testSimpleRejection() {
    TestEngine testEngine;
    TiXmlElement* root = initXml((getTestLoadLibraryPath() + "/SolverTests.xml").c_str(), "SimpleRejectionSolver");
    TiXmlElement* child = root->FirstChildElement();
    {
      IntervalIntDomain& horizon = HorizonFilter::getHorizon();
      horizon = IntervalIntDomain(0, 1000);

      CPPUNIT_ASSERT(testEngine.playTransactions((getTestLoadLibraryPath() + "/SimpleRejection.xml").c_str()));
      Solver solver(testEngine.getPlanDatabase(), *child);

      CPPUNIT_ASSERT(solver.solve(100, 100));
      CPPUNIT_ASSERT_MESSAGE(toString(testEngine.getPlanDatabase()->getTokens().size()), testEngine.getPlanDatabase()->getTokens().size() == 1);
    }

    return true;
  }


  static bool testMultipleSearch(){
    TestEngine testEngine;
    TiXmlElement* root = initXml((getTestLoadLibraryPath() + "/SolverTests.xml").c_str(), "SimpleCSPSolver");
    TiXmlElement* child = root->FirstChildElement();

    // Call the solver
    Solver solver(testEngine.getPlanDatabase(), *child);
    CPPUNIT_ASSERT(solver.solve());

    // Now modify the database and invoke the solver again. Ensure that it does work
    CPPUNIT_ASSERT(testEngine.playTransactions((getTestLoadLibraryPath() + "/SuccessfulSearch.xml").c_str()));
    CPPUNIT_ASSERT(solver.solve());
    CPPUNIT_ASSERT(solver.getDepth() > 0);

    return true;
  }

  //to test GNATS 3068
  static bool testOversearch() {
    TestEngine testEngine;
    TiXmlElement* root = initXml((getTestLoadLibraryPath() + "/SolverTests.xml").c_str(), "SimpleCSPSolver");
    TiXmlElement* child = root->FirstChildElement();

    CPPUNIT_ASSERT(testEngine.playTransactions((getTestLoadLibraryPath() +"/SuccessfulSearch.xml").c_str()));
    Solver solver(testEngine.getPlanDatabase(), *child);
    solver.setMaxSteps(5); //arbitrary number of maximum steps
    CPPUNIT_ASSERT(solver.solve(20)); //arbitrary number of steps < max

    return true;
  }

  static bool testBacktrackFirstDecisionPoint(){
    TestEngine testEngine;
    TiXmlElement* root = initXml((getTestLoadLibraryPath() + "/SolverTests.xml").c_str(), "BacktrackSolver");
    TiXmlElement* child = root->FirstChildElement();
    CPPUNIT_ASSERT(testEngine.playTransactions((getTestLoadLibraryPath() +"/BacktrackFirstDecision.xml").c_str()));
    Solver solver(testEngine.getPlanDatabase(), *child);
    solver.setMaxSteps(5); //arbitrary number of maximum steps
    CPPUNIT_ASSERT(solver.solve(20)); //arbitrary number of steps < max
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
    TestEngine testEngine;
    TiXmlElement* root = initXml( (getTestLoadLibraryPath() + "/SolverTests.xml").c_str(), "SimpleCSPSolver");
    TiXmlElement* child = root->FirstChildElement();
    CPPUNIT_ASSERT(testEngine.playTransactions( (getTestLoadLibraryPath() + "/StaticCSP.xml").c_str()));
    Solver solver(testEngine.getPlanDatabase(), *child);
    CPPUNIT_ASSERT(solver.solve(10));
    CPPUNIT_ASSERT(solver.getStepCount() == solver.getDepth());

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
                 "Solution " << solutionCount << " found." << PlanDatabaseWriter::toString(testEngine.getPlanDatabase()));

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

  static bool testGNATS_3196(){
    TestEngine testEngine;
    TiXmlElement* root = initXml( (getTestLoadLibraryPath() + "/SolverTests.xml").c_str(), "GNATS_3196");
    TiXmlElement* child = root->FirstChildElement();
    CPPUNIT_ASSERT(testEngine.playTransactions( (getTestLoadLibraryPath() + "/GNATS_3196.xml").c_str()));
    Solver solver(testEngine.getPlanDatabase(), *child);
    CPPUNIT_ASSERT(!solver.solve(1));
    solver.clear();
    TokenId onlyToken = *(testEngine.getPlanDatabase()->getTokens().begin());
    onlyToken->discard();
    CPPUNIT_ASSERT(solver.solve(1));
    return true;
  }

  static bool testContext() {
    TestEngine testEngine;
    std::stringstream data;
    data << "<Solver name=\"TestSolver\">" << std::endl;
    data << "</Solver>" << std::endl;
    std::string xml = data.str();
    TiXmlElement* root = initXml(xml);
    Solver solver(testEngine.getPlanDatabase(), *root);
    ContextId ctx = solver.getContext();
    CPPUNIT_ASSERT(ctx->getName() == LabelStr(solver.getName().toString() + "Context"));
    ctx->put("foo", 1);
    CPPUNIT_ASSERT(ctx->get("foo") == 1);
    CPPUNIT_ASSERT(solver.getContext()->get("foo") == 1);
    ctx->remove("foo");
    return true;
  }

  static bool testDeletedFlaw() {
    TiXmlElement* root = initXml((getTestLoadLibraryPath() + "/FlawHandlerTests.xml").c_str(), "TestDynamicFlaws");
    TestEngine testEngine;
    PlanDatabaseId db = testEngine.getPlanDatabase();
    Object o1(db, "GuardTest", "o1");
    db->close();
    TokenId t1 = db->getClient()->createToken("GuardTest.pred");
    TokenId t2 = db->getClient()->createToken("GuardTest.pred");
    TokenId t3 = db->getClient()->createToken("GuardTest.pred");
    db->getConstraintEngine()->propagate();
    t1->activate();
    t2->activate();
    t3->activate();
    db->getConstraintEngine()->propagate();
    Solver solver(testEngine.getPlanDatabase(), *(root->FirstChildElement()));
    solver.step(); //decide first 'a'
    solver.reset();
    t1->discard();
    t2->discard();
    solver.step();
    solver.step();
    solver.step();
    solver.reset();
    t3->discard();
    //t2->discard();
    //t1->discard();
//     solver.backjump(1);
//     solver.step();
    //t1->discard();
    return true;
  }

  static bool testDeleteAfterCommit() {
    TiXmlElement* root = initXml((getTestLoadLibraryPath() + "/FlawHandlerTests.xml").c_str(), "TestCommit");
    TiXmlElement* child = root->FirstChildElement();
    TestEngine testEngine;
    PlanDatabaseId db = testEngine.getPlanDatabase();
    Object o1(db, "CommitTest", "o1");
    db->close();
    Solver solver(testEngine.getPlanDatabase(), *child);

    // iterate over the possibilties with commiting and deleting with four tokens
    for(int i=1;i<255;i++)
    {
      IntervalIntDomain& horizon = HorizonFilter::getHorizon();
      horizon = IntervalIntDomain(0, 40);
      TokenId first = db->getClient()->createToken("CommitTest.chaina", false);
      first->start()->specify(0);
      solver.solve(100,100);

      TokenId second = first->getSlave(0);
      CPPUNIT_ASSERT_MESSAGE(second->getPredicateName().toString(), second->getPredicateName() == LabelStr("CommitTest.chainb"));
      TokenId third = second->getSlave(0);
      CPPUNIT_ASSERT_MESSAGE(third->getPredicateName().toString(), third->getPredicateName() == LabelStr("CommitTest.chaina"));
      TokenId fourth = third->getSlave(0);
      CPPUNIT_ASSERT_MESSAGE(fourth->getPredicateName().toString(), fourth->getPredicateName() == LabelStr("CommitTest.chainb"));

      if(i & (1 << 0))
				first->commit();
      if(i & (1 << 1))
				second->commit();
      if(i & (1 << 2))
				third->commit();
      if(i & (1 << 3))
				fourth->commit();

      solver.reset();

      if(i & (16 << 0))
				first->discard();
      if(i & (16 << 1))
				second->discard();
      if(i & (16 << 2))
				third->discard();
      if(i & (16 << 3))
				fourth->discard();

      CPPUNIT_ASSERT_MESSAGE("Solver must be valid after discards.", solver.isValid());

      horizon = IntervalIntDomain(0, 40);
      solver.solve(100,100);
      // TODO: this is failing, why? re-enable after understanding causes
      //CPPUNIT_ASSERT_MESSAGE("Solver must be valid after continuing solving after discards.", solver.isValid());

      solver.reset();

      // anything which was not deleted must now be deleted
      TokenSet tokens = testEngine.getPlanDatabase()->getTokens();
      for(TokenSet::const_iterator it = tokens.begin(); it != tokens.end(); ++it) {
          if(!(*it)->isDiscarded()) {
              (*it)->discard();
          }
      }
    }
    return true;
  }
};

class FlawIteratorTests {
public:
  static bool test() {
    EUROPA_runTest(testUnboundVariableFlawIteration);
    EUROPA_runTest(testThreatFlawIteration);
    EUROPA_runTest(testOpenConditionFlawIteration);
    //EUROPA_runTest(testSolverIteration);
    return true;
  }
private:

  static bool testUnboundVariableFlawIteration() {
    TiXmlElement* root = initXml((getTestLoadLibraryPath() + "/FlawFilterTests.xml" ).c_str(), "UnboundVariableManager");

    TestEngine testEngine;
    UnboundVariableManager fm(*root);
    fm.initialize(*root,testEngine.getPlanDatabase());
    CPPUNIT_ASSERT(testEngine.playTransactions((getTestLoadLibraryPath() + "/UnboundVariableFiltering.xml").c_str()));

    IntervalIntDomain& horizon = HorizonFilter::getHorizon();
    horizon = IntervalIntDomain(0, 1000);

    ConstrainedVariableSet variables = testEngine.getConstraintEngine()->getVariables();
    IteratorId flawIterator = fm.createIterator();
    while(!flawIterator->done()) {
      const ConstrainedVariableId var = (const ConstrainedVariableId) flawIterator->next();
      if(var.isNoId())
        continue;
      CPPUNIT_ASSERT(fm.inScope(var));
      CPPUNIT_ASSERT(variables.find(var) != variables.end());
      variables.erase(var);
    }

    CPPUNIT_ASSERT(flawIterator->done());

    for(ConstrainedVariableSet::const_iterator it = variables.begin(); it != variables.end(); ++it)
      CPPUNIT_ASSERT(!fm.inScope(*it));

    delete (Iterator*) flawIterator;
    delete root;
    return true;
  }

  static bool testOpenConditionFlawIteration() {
    TiXmlElement* root = initXml((getTestLoadLibraryPath() + "/FlawFilterTests.xml" ).c_str(), "OpenConditionManager");

    TestEngine testEngine;
    OpenConditionManager fm(*root);
    IntervalIntDomain& horizon = HorizonFilter::getHorizon();
    horizon = IntervalIntDomain(0, 1000);
    fm.initialize(*root,testEngine.getPlanDatabase());
    CPPUNIT_ASSERT(testEngine.playTransactions((getTestLoadLibraryPath() + "/OpenConditionFiltering.xml").c_str()));

    TokenSet tokens = testEngine.getPlanDatabase()->getTokens();
    IteratorId flawIterator = fm.createIterator();

    while(!flawIterator->done()) {
      const TokenId token = (const TokenId) flawIterator->next();
      if(token.isNoId())
        continue;
      CPPUNIT_ASSERT(fm.inScope(token));
      CPPUNIT_ASSERT(tokens.find(token) != tokens.end());
      tokens.erase(token);
    }

    CPPUNIT_ASSERT(flawIterator->done());

    for(TokenSet::const_iterator it = tokens.begin(); it != tokens.end(); ++it)
      CPPUNIT_ASSERT(!fm.inScope(*it));

    delete (Iterator*) flawIterator;
    delete root;

    return true;
  }

  static bool testThreatFlawIteration() {
    TiXmlElement* root = initXml((getTestLoadLibraryPath() + "/FlawFilterTests.xml" ).c_str(), "ThreatManager");

    TestEngine testEngine;
    ThreatManager fm(*root);
    IntervalIntDomain& horizon = HorizonFilter::getHorizon();
    horizon = IntervalIntDomain(0, 1000);
    fm.initialize(*root,testEngine.getPlanDatabase());
    CPPUNIT_ASSERT(testEngine.playTransactions((getTestLoadLibraryPath() + "/ThreatFiltering.xml").c_str()));

    TokenSet tokens = testEngine.getPlanDatabase()->getTokens();
    IteratorId flawIterator = fm.createIterator();

    while(!flawIterator->done()) {
      const TokenId token = (const TokenId) flawIterator->next();
      if(token.isNoId())
        continue;
      CPPUNIT_ASSERT(fm.inScope(token));
      CPPUNIT_ASSERT(tokens.find(token) != tokens.end());
      tokens.erase(token);
    }

    CPPUNIT_ASSERT(flawIterator->done());

    for(TokenSet::const_iterator it = tokens.begin(); it != tokens.end(); ++it)
      CPPUNIT_ASSERT(!fm.inScope(*it));

    delete (Iterator*) flawIterator;
    delete root;
    return true;
  }

  static bool testSolverIteration() {
    TiXmlElement* root = initXml((getTestLoadLibraryPath() + "/IterationTests.xml" ).c_str(), "Solver");
    TestEngine testEngine;
    ThreatManager tm(*(root->FirstChildElement("ThreatManager")));
    OpenConditionManager ocm(*(root->FirstChildElement("OpenConditionManager")));
    UnboundVariableManager uvm(*(root->FirstChildElement("UnboundVariableManager")));
    Solver solver(testEngine.getPlanDatabase(), *root);

    IntervalIntDomain& horizon = HorizonFilter::getHorizon();
    horizon = IntervalIntDomain(0, 1000);

    CPPUNIT_ASSERT(testEngine.playTransactions((getTestLoadLibraryPath() + "/ThreatFiltering.xml").c_str()));

    tm.initialize(*(root->FirstChildElement("ThreatManager")),testEngine.getPlanDatabase());
    ocm.initialize(*(root->FirstChildElement("OpenConditionManager")),testEngine.getPlanDatabase());
    uvm.initialize(*(root->FirstChildElement("UnboundVariableManager")),testEngine.getPlanDatabase());

    TokenSet tokens = testEngine.getPlanDatabase()->getTokens();
    ConstrainedVariableSet vars = testEngine.getConstraintEngine()->getVariables();

    IteratorId flawIterator = solver.createIterator();
    while(!flawIterator->done()) {
      const EntityId entity = flawIterator->next();
      if(entity.isNoId())
        continue;
      if(TokenId::convertable(entity)) {
        const TokenId tok = (const TokenId) entity;
        CPPUNIT_ASSERT(tm.inScope(tok) || ocm.inScope(tok));
        CPPUNIT_ASSERT(tokens.find(tok) != tokens.end());
        tokens.erase(tok);
      }
      else if(ConstrainedVariableId::convertable(entity)) {
        const ConstrainedVariableId var = (const ConstrainedVariableId) entity;
        CPPUNIT_ASSERT(uvm.inScope(var));
        CPPUNIT_ASSERT(vars.find(var) != vars.end());
        std::cerr << var->toString() << std::endl;
        vars.erase(var);
      }
      else
        CPPUNIT_ASSERT(false);
    }

    for(TokenSet::const_iterator it = tokens.begin(); it != tokens.end(); ++it)
      CPPUNIT_ASSERT(!tm.inScope(*it) && !ocm.inScope(*it));

    for(ConstrainedVariableSet::const_iterator it = vars.begin(); it != vars.end(); ++it) {
      std::cerr << (*it)->toString() << std::endl;
      CPPUNIT_ASSERT(!uvm.inScope(*it));
    }

    delete (Iterator*) flawIterator;
    delete root;

    return true;
  }
};

class FlawManagerTests {
public:
  static bool test() {
    return true;
  }
};

void registerTestElements(EngineId& engine)
{
   CESchema* ces = (CESchema*)engine->getComponent("CESchema");
   EUROPA::SOLVERS::ComponentFactoryMgr* cfm = (EUROPA::SOLVERS::ComponentFactoryMgr*)engine->getComponent("ComponentFactoryMgr");

   // For tests on the matching engine
   REGISTER_COMPONENT_FACTORY(cfm,MatchingRule, MatchingRule);

   REGISTER_COMPONENT_FACTORY(cfm,TestComponent, A);
   REGISTER_COMPONENT_FACTORY(cfm,TestComponent, B);
   REGISTER_COMPONENT_FACTORY(cfm,TestComponent, C);
   REGISTER_COMPONENT_FACTORY(cfm,TestComponent, D);
   REGISTER_COMPONENT_FACTORY(cfm,TestComponent, E);

   REGISTER_FLAW_HANDLER(cfm,EUROPA::SOLVERS::RandomValue, Random);

   REGISTER_CONSTRAINT(ces,LazyAllDiff, "lazyAllDiff",  "Default");
   REGISTER_CONSTRAINT(ces,LazyAlwaysFails, "lazyAlwaysFails",  "Default");
}

void SolversModuleTests::cppTests()
{
   setTestLoadLibraryPath(".");
}

void SolversModuleTests::componentFactoryTests()
{
   ComponentFactoryTests::test();
}

void SolversModuleTests::filterTests()
{
   FilterTests::test();
}

void SolversModuleTests::flawIteratorTests()
{
   FlawIteratorTests::test();
}

void SolversModuleTests::flawManagerTests()
{
   FlawManagerTests::test();
}

void SolversModuleTests::flawHandlerTests()
{
   FlawHandlerTests::test();
}

void SolversModuleTests::solverTests()
{
   SolverTests::test();
}


