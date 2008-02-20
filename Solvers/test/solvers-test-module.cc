#include "solvers-test-module.hh"
#include "Nddl.hh"
#include "Solver.hh"
#include "ComponentFactory.hh"
#include "Constraint.hh"
#include "ConstraintLibrary.hh"
#include "UnboundVariableManager.hh"
#include "OpenConditionManager.hh"
#include "ThreatManager.hh"
#include "Object.hh"
#include "Timeline.hh"
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
#include "HSTSDecisionPoints.hh"
#include "PlanDatabaseWriter.hh"
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

class SolversTestEngine : public Engine 
{
  public:  
	SolversTestEngine();
	virtual ~SolversTestEngine();
	
	static void initialize();
	static void terminate();

	virtual EngineComponentId& getComponent(const std::string& name);
    
    virtual void addLanguageInterpreter(const std::string& language, LanguageInterpreter* interpreter) {}
    virtual void removeLanguageInterpreter(const std::string& language) {}       

    const PlanDatabaseId& getPlanDatabase() const          { return m_planDatabase; }
	const ConstraintEngineId& getConstraintEngine() const  {return m_constraintEngine;} 
	const RulesEngineId& getRulesEngine() const            {return m_rulesEngine;}
	  
  protected: 
	ConstraintEngineId m_constraintEngine;
	PlanDatabaseId m_planDatabase;
	RulesEngineId m_rulesEngine;
	    
	void allocateComponents();
	void deallocateComponents();
	
	static void createModules();
	static void initializeModules();
	static void uninitializeModules();
	static std::vector<ModuleId> m_modules;	    
};

std::vector<ModuleId> SolversTestEngine::m_modules;

SolversTestEngine::SolversTestEngine()
{
   allocateComponents();	
}

SolversTestEngine::~SolversTestEngine()
{
   deallocateComponents();	
}

void SolversTestEngine::initialize()
{
	initializeModules();    	
}

void SolversTestEngine::terminate()
{
	uninitializeModules();
}

void SolversTestEngine::createModules()
{
    m_modules.push_back(new ModuleConstraintEngine()); 
    m_modules.push_back(new ModuleConstraintLibrary());
    m_modules.push_back(new ModulePlanDatabase());
    m_modules.push_back(new ModuleRulesEngine());
    m_modules.push_back(new ModuleTemporalNetwork());
    m_modules.push_back(new ModuleSolvers());
    m_modules.push_back(new ModuleNddl());
}

void SolversTestEngine::initializeModules()
{
    createModules();
  
    for (unsigned int i=0;i<m_modules.size();i++) {
    	m_modules[i]->initialize();
    }	  
}

void SolversTestEngine::uninitializeModules()
{
    Entity::purgeStarted();      
    for (unsigned int i=m_modules.size();i>0;i--) {
    	unsigned int idx = i-1;
    	m_modules[idx]->uninitialize();
    	m_modules[idx].release();
    }	  
    Entity::purgeEnded();	  

    m_modules.clear();	  
}


void SolversTestEngine::allocateComponents()
{
    m_constraintEngine = (new ConstraintEngine())->getId();	  
    m_planDatabase = (new PlanDatabase(m_constraintEngine, Schema::instance()))->getId();	
    m_rulesEngine = (new RulesEngine(m_planDatabase))->getId();	  

    for (unsigned int i=0;i<m_modules.size();i++) {
    	m_modules[i]->initialize(getId());
    }	  	  
}	  

void SolversTestEngine::deallocateComponents()
{
	for (unsigned int i=m_modules.size();i>0;i--) {
	  unsigned int idx = i-1;
	  m_modules[idx]->uninitialize(getId());
    }	  

	Entity::purgeStarted();
    
    if(m_rulesEngine.isValid()) delete (RulesEngine*) m_rulesEngine;
	if(m_planDatabase.isValid()) delete (PlanDatabase*) m_planDatabase; 
	if(m_constraintEngine.isValid()) delete (ConstraintEngine*) m_constraintEngine;

	Entity::purgeEnded();	  
}    

EngineComponentId& SolversTestEngine::getComponent(const std::string& name)
{
	  static EngineComponentId noId = EngineComponentId::noId();
	  
	  if (name == "ConstraintEngine")
		  return (EngineComponentId&)m_constraintEngine;
	  if (name == "PlanDatabase")
		  return (EngineComponentId&)m_planDatabase;
	  if (name == "RulesEngine")
		  return (EngineComponentId&)m_constraintEngine;

	  return noId;
}          

class TestAssembly : public SolversTestEngine 
{
public:
  bool playTransactions(const char* txSource){
    check_error(txSource != NULL, "NULL transaction source provided.");
    
    // Obtain the client to play transactions on.
    DbClientId client = m_planDatabase->getClient();
    
    // Construct player
    DbClientTransactionPlayer player(client);
    
    // Open transaction source and play transactions
    std::ifstream in(txSource);

    check_error(in, "Invalid transaction source '" + std::string(txSource) + "'.");
    player.play(in);
    
    return m_constraintEngine->constraintConsistent();
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

    TestAssembly assembly;
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
      me.getMatches(v0.getId(), rules);
      assertTrue(rules.size() == 1, toString(rules.size()));
      assertTrue(rules[0]->toString() == "[R0]*.*.*.*.*.*", rules[0]->toString());
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
      me.getMatches(ConstrainedVariableId(token.getStart()), rules);
      assertTrue(rules.size() == 2, toString(rules.size()));
      assertTrue(rules[1]->toString() == "[R1]*.*.start.*.*.*", rules[1]->toString());
    }

    // test R2 
    {
      Variable<IntervalIntDomain> v0(assembly.getConstraintEngine(), IntervalIntDomain(0, 10), true, "arg3");
      std::vector<MatchingRuleId> rules;
      me.getMatches(v0.getId(), rules);
      assertTrue(rules.size() == 2, toString(rules.size()));
      assertTrue(rules[1]->toString() == "[R2]*.*.arg3.*.*.*", rules[1]->toString());
    }

    // test R3 
    {
      TokenId token = db->getClient()->createToken("D.predicateF", false);
      std::vector<MatchingRuleId> rules;
      me.getMatches(token, rules);
      assertTrue(rules.size() == 2, toString(rules.size()));
      assertTrue(rules[1]->toString() == "[R3]*.predicateF.*.*.*.*", rules[1]->toString());
      token->discard();
    }

    // test R4
    {
      TokenId token = db->getClient()->createToken("D.predicateC", false);
      std::vector<MatchingRuleId> rules;
      me.getMatches(token->getVariable("arg6"), rules);
      assertTrue(rules.size() == 2, toString(rules.size()));
      assertTrue(rules[1]->toString() == "[R4]*.predicateC.arg6.*.*.*", rules[1]->toString());
      token->discard();
    }

    // test R5 & R6
    {
      TokenId token = db->getClient()->createToken("C.predicateC", false);
      std::vector<MatchingRuleId> rules;
      me.getMatches(token, rules);
      assertTrue(rules.size() == 3, toString(rules.size()) + " for " + token->getUnqualifiedPredicateName().toString());
      assertTrue(rules[1]->toString() == "[R5]C.predicateC.*.*.*.*", rules[1]->toString());
      assertTrue(rules[2]->toString() == "[R6]C.*.*.*.*.*", rules[2]->toString());
      token->discard();
    }

    // test R6
    {
      TokenId token = db->getClient()->createToken("C.predicateA", false);
      std::vector<MatchingRuleId> rules;
      me.getMatches(token, rules);
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
      me.getMatches(ConstrainedVariableId(E_predicateC->getDuration()), rules);
      assertTrue(rules.size() == 2, toString(rules.size()) + " for " + token->getUnqualifiedPredicateName().toString());
      assertTrue(rules[1]->toString() == "[R7]*.*.duration.*.Object.*", rules[1]->toString());
      token->discard();
    }

    // test R7a
    {
      TokenId token = db->getClient()->createToken("E.predicateC", false);
      std::vector<MatchingRuleId> rules;
      me.getMatches(ConstrainedVariableId(token->getDuration()), rules);
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
      me.getMatches(ConstrainedVariableId(E_predicateC->getDuration()), rules);
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
      me.getMatches(ConstrainedVariableId(E_predicateC->getDuration()), rules);
      assertTrue(rules.size() == 4, toString(rules.size()) + " for " + token->getPredicateName().toString());
      for(int i=0;i>4; i++)
        assertTrue(expectedRules.find(LabelStr(rules[i]->toString())) != expectedRules.end(), rules[i]->toString());

      token->discard();
    }

    return true;
  }

  static bool testVariableFiltering(){
    TiXmlElement* root = initXml( (getTestLoadLibraryPath() + "/FlawFilterTests.xml").c_str(), "UnboundVariableManager");

    TestAssembly assembly;
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
                check_error(root != NULL, "Error loading xml: " + getTestLoadLibraryPath() + "/FlawFilterTests.xml");

    TestAssembly assembly;
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

    TestAssembly assembly;
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

class ConstraintNameListener : public ConstrainedVariableListener {
public:
  ConstraintNameListener(const ConstrainedVariableId& var) : ConstrainedVariableListener(var), m_constraintName("NO_CONSTRAINT") {}
  void notifyConstraintAdded(const ConstraintId& constr, int argIndex) {
    m_constraintName = constr->getName();
  }
  const LabelStr& getName() {return m_constraintName;}
private:
  LabelStr m_constraintName;
};

class FlawHandlerTests {
public:
  static bool test(){
    runTest(testPriorities);
    runTest(testGuards);
    runTest(testDynamicFlawManagement);
    runTest(testDefaultVariableOrdering);
    runTest(testHeuristicVariableOrdering);
    runTest(testTokenComparators);
    runTest(testValueEnum);
    runTest(testHSTSOpenConditionDecisionPoint);
    runTest(testHSTSThreatDecisionPoint);
    return true;
  }

private:
  static bool testPriorities(){
    TiXmlElement* root = initXml( (getTestLoadLibraryPath() + "/FlawHandlerTests.xml").c_str(), "TestPriorities");
    MatchingEngine me(*root, "FlawHandler");
    TestAssembly assembly;
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
      me.getMatches(v0.getId(), rules);
      assertTrue(rules.size() == 1, toString(rules.size()));
      FlawHandlerId flawHandler = rules[0];
      assertTrue(flawHandler->getPriority() == 1000, toString(flawHandler->getPriority()));
      assertTrue(flawHandler->getWeight() == 199000, toString(flawHandler->getWeight()));
    }

    // test H1
    {
      Variable<IntervalIntDomain> v0(assembly.getConstraintEngine(), IntervalIntDomain(0, 10), true, "start");
      std::vector<MatchingRuleId> rules;
      me.getMatches(v0.getId(), rules);
      assertTrue(rules.size() == 2, toString(rules.size()));
      FlawHandlerId flawHandler = rules[1];
      assertTrue(flawHandler->getPriority() == 1000, toString(flawHandler->getPriority()));
      assertTrue(flawHandler->getWeight() == 299000, toString(flawHandler->getWeight()));
    }

    // test H2
    {
      Variable<IntervalIntDomain> v0(assembly.getConstraintEngine(), IntervalIntDomain(0, 10), true, "end");
      std::vector<MatchingRuleId> rules;
      me.getMatches(v0.getId(), rules);
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
      me.getMatches(ConstrainedVariableId(E_predicateC->getEnd()), rules);
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
    TestAssembly assembly;
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
      me.getMatches(token, rules);
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
      me.getMatches(token, rules);
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
        me.getMatches(token, rules);
        assertTrue(rules.size() == 1, toString(rules.size()));
        FlawHandlerId flawHandler = rules[0];
        std::vector<ConstrainedVariableId> guards;
        assertFalse(flawHandler->makeConstraintScope(token, guards));
      }
      // Now fire on the subgoal. Rule will match as it has a master
      token->activate();
      TokenId B_predicateC = *(token->getSlaves().begin());
      std::vector<MatchingRuleId> rules;
      me.getMatches(B_predicateC, rules);
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
      me.getMatches(v0.getId(), rules);
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
    TestAssembly assembly;
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

      solver.reset();
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
    TestAssembly assembly;
    TiXmlElement* root = initXml( (getTestLoadLibraryPath() + "/FlawHandlerTests.xml").c_str(), "DefaultVariableOrdering");
    TiXmlElement* child = root->FirstChildElement();
    {
      assert(assembly.playTransactions( (getTestLoadLibraryPath() + "/StaticCSP.xml").c_str()));
      Solver solver(assembly.getPlanDatabase(), *child);
      assertTrue(solver.solve());
      assertTrue(solver.getStepCount() == solver.getDepth());
      assertTrue(solver.getStepCount() == 2, toString(solver.getStepCount()));
      ConstrainedVariableId v1 = assembly.getPlanDatabase()->getGlobalVariable("v1");
      assertTrue(v1->lastDomain().getSingletonValue() == 1, v1->toString());
      ConstrainedVariableId v2 = assembly.getPlanDatabase()->getGlobalVariable("v2");
      assertTrue(v2->lastDomain().getSingletonValue() == 0, v2->toString());
    }

    return true;
  }

  static bool testHeuristicVariableOrdering(){
    TestAssembly assembly;
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

  static bool testTokenComparators() {
    TestAssembly assembly;
    Schema::instance()->addPredicate("A.Foo");
    PlanDatabaseId db = assembly.getPlanDatabase();

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
    assertTrue(early.compare(foo.getId(), t1.getId()));
    assertTrue(!early.compare(t1.getId(), foo.getId()));
    assertTrue(!early.compare(foo.getId(), t2.getId()));
    assertTrue(early.compare(t2.getId(), foo.getId()));
    assertTrue(!early.compare(foo.getId(), t3.getId()));
    assertTrue(!early.compare(t3.getId(), foo.getId()));

    LateTokenComparator late(foo.getId());
    assertTrue(!late.compare(foo.getId(), t1.getId()));
    assertTrue(late.compare(t1.getId(), foo.getId()));
    assertTrue(late.compare(foo.getId(), t2.getId()));
    assertTrue(!late.compare(t2.getId(), foo.getId()));
    assertTrue(!late.compare(foo.getId(), t3.getId()));
    assertTrue(!late.compare(t3.getId(), foo.getId()));

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
    assertTrue(!near.compare(foo.getId(), foo.getId()));
    assertTrue(!near.compare(t4.getId(), t5.getId()));
    assertTrue(!near.compare(t5.getId(), t6.getId()));
    assertTrue(!near.compare(t4.getId(), t6.getId()));

    assertTrue(!near.compare(t4.getId(), t7.getId()));
    assertTrue(!near.compare(t7.getId(), t4.getId()));
    assertTrue(!near.compare(t4.getId(), t8.getId()));
    assertTrue(!near.compare(t8.getId(), t4.getId()));
    assertTrue(!near.compare(t4.getId(), t9.getId()));
    assertTrue(!near.compare(t9.getId(), t4.getId()));

    assertTrue(near.compare(t4.getId(), t10.getId()));
    assertTrue(!near.compare(t10.getId(), t4.getId()));
    assertTrue(near.compare(t4.getId(), t11.getId()));
    assertTrue(!near.compare(t11.getId(), t4.getId()));
    assertTrue(near.compare(t4.getId(), t12.getId()));
    assertTrue(!near.compare(t12.getId(), t4.getId()));
    
    assertTrue(near.compare(t4.getId(), t13.getId()));
    assertTrue(!near.compare(t13.getId(), t4.getId()));
    assertTrue(near.compare(t4.getId(), t14.getId()));
    assertTrue(!near.compare(t14.getId(), t4.getId()));
    assertTrue(near.compare(t4.getId(), t15.getId()));
    assertTrue(!near.compare(t15.getId(), t4.getId()));


    assertTrue(!near.compare(t5.getId(), t7.getId()));
    assertTrue(!near.compare(t7.getId(), t5.getId()));
    assertTrue(!near.compare(t5.getId(), t8.getId()));
    assertTrue(!near.compare(t8.getId(), t5.getId()));
    assertTrue(!near.compare(t5.getId(), t9.getId()));
    assertTrue(!near.compare(t9.getId(), t5.getId()));

    assertTrue(near.compare(t5.getId(), t10.getId()));
    assertTrue(!near.compare(t10.getId(), t5.getId()));
    assertTrue(near.compare(t5.getId(), t11.getId()));
    assertTrue(!near.compare(t11.getId(), t5.getId()));
    assertTrue(near.compare(t5.getId(), t12.getId()));
    assertTrue(!near.compare(t12.getId(), t5.getId()));
    
    assertTrue(near.compare(t5.getId(), t13.getId()));
    assertTrue(!near.compare(t13.getId(), t5.getId()));
    assertTrue(near.compare(t5.getId(), t14.getId()));
    assertTrue(!near.compare(t14.getId(), t5.getId()));
    assertTrue(near.compare(t5.getId(), t15.getId()));
    assertTrue(!near.compare(t15.getId(), t5.getId()));


    assertTrue(!near.compare(t6.getId(), t7.getId()));
    assertTrue(!near.compare(t7.getId(), t6.getId()));
    assertTrue(!near.compare(t6.getId(), t8.getId()));
    assertTrue(!near.compare(t8.getId(), t6.getId()));
    assertTrue(!near.compare(t6.getId(), t9.getId()));
    assertTrue(!near.compare(t9.getId(), t6.getId()));

    assertTrue(near.compare(t6.getId(), t10.getId()));
    assertTrue(!near.compare(t10.getId(), t6.getId()));
    assertTrue(near.compare(t6.getId(), t11.getId()));
    assertTrue(!near.compare(t11.getId(), t6.getId()));
    assertTrue(near.compare(t6.getId(), t12.getId()));
    assertTrue(!near.compare(t12.getId(), t6.getId()));
    
    assertTrue(near.compare(t6.getId(), t13.getId()));
    assertTrue(!near.compare(t13.getId(), t6.getId()));
    assertTrue(near.compare(t6.getId(), t14.getId()));
    assertTrue(!near.compare(t14.getId(), t6.getId()));
    assertTrue(near.compare(t6.getId(), t15.getId()));
    assertTrue(!near.compare(t15.getId(), t6.getId()));




    assertTrue(!near.compare(t7.getId(), t8.getId()));
    assertTrue(!near.compare(t8.getId(), t9.getId()));
    assertTrue(!near.compare(t9.getId(), t7.getId()));

    assertTrue(near.compare(t7.getId(), t10.getId()));
    assertTrue(!near.compare(t10.getId(), t7.getId()));
    assertTrue(near.compare(t7.getId(), t11.getId()));
    assertTrue(!near.compare(t11.getId(), t7.getId()));
    assertTrue(near.compare(t7.getId(), t12.getId()));
    assertTrue(!near.compare(t12.getId(), t7.getId()));

    assertTrue(near.compare(t7.getId(), t13.getId()));
    assertTrue(!near.compare(t13.getId(), t7.getId()));
    assertTrue(near.compare(t7.getId(), t14.getId()));
    assertTrue(!near.compare(t14.getId(), t7.getId()));
    assertTrue(near.compare(t7.getId(), t15.getId()));
    assertTrue(!near.compare(t15.getId(), t7.getId()));

    assertTrue(near.compare(t8.getId(), t10.getId()));
    assertTrue(!near.compare(t10.getId(), t8.getId()));
    assertTrue(near.compare(t8.getId(), t11.getId()));
    assertTrue(!near.compare(t11.getId(), t8.getId()));
    assertTrue(near.compare(t8.getId(), t12.getId()));
    assertTrue(!near.compare(t12.getId(), t8.getId()));

    assertTrue(near.compare(t8.getId(), t13.getId()));
    assertTrue(!near.compare(t13.getId(), t8.getId()));
    assertTrue(near.compare(t8.getId(), t14.getId()));
    assertTrue(!near.compare(t14.getId(), t8.getId()));
    assertTrue(near.compare(t8.getId(), t15.getId()));
    assertTrue(!near.compare(t15.getId(), t8.getId()));
    

    assertTrue(near.compare(t9.getId(), t10.getId()));
    assertTrue(!near.compare(t10.getId(), t9.getId()));
    assertTrue(near.compare(t9.getId(), t11.getId()));
    assertTrue(!near.compare(t11.getId(), t9.getId()));
    assertTrue(near.compare(t9.getId(), t12.getId()));
    assertTrue(!near.compare(t12.getId(), t9.getId()));

    assertTrue(near.compare(t9.getId(), t13.getId()));
    assertTrue(!near.compare(t13.getId(), t9.getId()));
    assertTrue(near.compare(t9.getId(), t14.getId()));
    assertTrue(!near.compare(t14.getId(), t9.getId()));
    assertTrue(near.compare(t9.getId(), t15.getId()));
    assertTrue(!near.compare(t15.getId(), t9.getId()));


    assertTrue(!near.compare(t10.getId(), t11.getId()));
    assertTrue(!near.compare(t11.getId(), t12.getId()));
    assertTrue(!near.compare(t12.getId(), t10.getId()));

    assertTrue(!near.compare(t10.getId(), t13.getId()));
    assertTrue(!near.compare(t13.getId(), t10.getId()));
    assertTrue(!near.compare(t10.getId(), t14.getId()));
    assertTrue(!near.compare(t14.getId(), t10.getId()));
    assertTrue(!near.compare(t10.getId(), t15.getId()));
    assertTrue(!near.compare(t15.getId(), t10.getId()));

    assertTrue(!near.compare(t11.getId(), t13.getId()));
    assertTrue(!near.compare(t13.getId(), t11.getId()));
    assertTrue(!near.compare(t11.getId(), t14.getId()));
    assertTrue(!near.compare(t14.getId(), t11.getId()));
    assertTrue(!near.compare(t11.getId(), t15.getId()));
    assertTrue(!near.compare(t15.getId(), t11.getId()));

    assertTrue(!near.compare(t12.getId(), t13.getId()));
    assertTrue(!near.compare(t13.getId(), t12.getId()));
    assertTrue(!near.compare(t12.getId(), t14.getId()));
    assertTrue(!near.compare(t14.getId(), t12.getId()));
    assertTrue(!near.compare(t12.getId(), t15.getId()));
    assertTrue(!near.compare(t15.getId(), t12.getId()));

    assertTrue(!near.compare(t13.getId(), t14.getId()));
    assertTrue(!near.compare(t14.getId(), t15.getId()));
    assertTrue(!near.compare(t15.getId(), t13.getId()));


    FarTokenComparator far(foo.getId());
    assertTrue(!far.compare(foo.getId(), foo.getId()));
    assertTrue(!far.compare(t4.getId(), t5.getId()));
    assertTrue(!far.compare(t5.getId(), t6.getId()));
    assertTrue(!far.compare(t4.getId(), t6.getId()));

    assertTrue(!far.compare(t4.getId(), t7.getId()));
    assertTrue(!far.compare(t7.getId(), t4.getId()));
    assertTrue(!far.compare(t4.getId(), t8.getId()));
    assertTrue(!far.compare(t8.getId(), t4.getId()));
    assertTrue(!far.compare(t4.getId(), t9.getId()));
    assertTrue(!far.compare(t9.getId(), t4.getId()));

    assertTrue(!far.compare(t4.getId(), t10.getId()));
    assertTrue(far.compare(t10.getId(), t4.getId()));
    assertTrue(!far.compare(t4.getId(), t11.getId()));
    assertTrue(far.compare(t11.getId(), t4.getId()));
    assertTrue(!far.compare(t4.getId(), t12.getId()));
    assertTrue(far.compare(t12.getId(), t4.getId()));
    
    assertTrue(!far.compare(t4.getId(), t13.getId()));
    assertTrue(far.compare(t13.getId(), t4.getId()));
    assertTrue(!far.compare(t4.getId(), t14.getId()));
    assertTrue(far.compare(t14.getId(), t4.getId()));
    assertTrue(!far.compare(t4.getId(), t15.getId()));
    assertTrue(far.compare(t15.getId(), t4.getId()));


    assertTrue(!far.compare(t5.getId(), t7.getId()));
    assertTrue(!far.compare(t7.getId(), t5.getId()));
    assertTrue(!far.compare(t5.getId(), t8.getId()));
    assertTrue(!far.compare(t8.getId(), t5.getId()));
    assertTrue(!far.compare(t5.getId(), t9.getId()));
    assertTrue(!far.compare(t9.getId(), t5.getId()));

    assertTrue(!far.compare(t5.getId(), t10.getId()));
    assertTrue(far.compare(t10.getId(), t5.getId()));
    assertTrue(!far.compare(t5.getId(), t11.getId()));
    assertTrue(far.compare(t11.getId(), t5.getId()));
    assertTrue(!far.compare(t5.getId(), t12.getId()));
    assertTrue(far.compare(t12.getId(), t5.getId()));
    
    assertTrue(!far.compare(t5.getId(), t13.getId()));
    assertTrue(far.compare(t13.getId(), t5.getId()));
    assertTrue(!far.compare(t5.getId(), t14.getId()));
    assertTrue(far.compare(t14.getId(), t5.getId()));
    assertTrue(!far.compare(t5.getId(), t15.getId()));
    assertTrue(far.compare(t15.getId(), t5.getId()));


    assertTrue(!far.compare(t6.getId(), t7.getId()));
    assertTrue(!far.compare(t7.getId(), t6.getId()));
    assertTrue(!far.compare(t6.getId(), t8.getId()));
    assertTrue(!far.compare(t8.getId(), t6.getId()));
    assertTrue(!far.compare(t6.getId(), t9.getId()));
    assertTrue(!far.compare(t9.getId(), t6.getId()));

    assertTrue(!far.compare(t6.getId(), t10.getId()));
    assertTrue(far.compare(t10.getId(), t6.getId()));
    assertTrue(!far.compare(t6.getId(), t11.getId()));
    assertTrue(far.compare(t11.getId(), t6.getId()));
    assertTrue(!far.compare(t6.getId(), t12.getId()));
    assertTrue(far.compare(t12.getId(), t6.getId()));
    
    assertTrue(!far.compare(t6.getId(), t13.getId()));
    assertTrue(far.compare(t13.getId(), t6.getId()));
    assertTrue(!far.compare(t6.getId(), t14.getId()));
    assertTrue(far.compare(t14.getId(), t6.getId()));
    assertTrue(!far.compare(t6.getId(), t15.getId()));
    assertTrue(far.compare(t15.getId(), t6.getId()));




    assertTrue(!far.compare(t7.getId(), t8.getId()));
    assertTrue(!far.compare(t8.getId(), t9.getId()));
    assertTrue(!far.compare(t9.getId(), t7.getId()));

    assertTrue(!far.compare(t7.getId(), t10.getId()));
    assertTrue(far.compare(t10.getId(), t7.getId()));
    assertTrue(!far.compare(t7.getId(), t11.getId()));
    assertTrue(far.compare(t11.getId(), t7.getId()));
    assertTrue(!far.compare(t7.getId(), t12.getId()));
    assertTrue(far.compare(t12.getId(), t7.getId()));

    assertTrue(!far.compare(t7.getId(), t13.getId()));
    assertTrue(far.compare(t13.getId(), t7.getId()));
    assertTrue(!far.compare(t7.getId(), t14.getId()));
    assertTrue(far.compare(t14.getId(), t7.getId()));
    assertTrue(!far.compare(t7.getId(), t15.getId()));
    assertTrue(far.compare(t15.getId(), t7.getId()));

    assertTrue(!far.compare(t8.getId(), t10.getId()));
    assertTrue(far.compare(t10.getId(), t8.getId()));
    assertTrue(!far.compare(t8.getId(), t11.getId()));
    assertTrue(far.compare(t11.getId(), t8.getId()));
    assertTrue(!far.compare(t8.getId(), t12.getId()));
    assertTrue(far.compare(t12.getId(), t8.getId()));

    assertTrue(!far.compare(t8.getId(), t13.getId()));
    assertTrue(far.compare(t13.getId(), t8.getId()));
    assertTrue(!far.compare(t8.getId(), t14.getId()));
    assertTrue(far.compare(t14.getId(), t8.getId()));
    assertTrue(!far.compare(t8.getId(), t15.getId()));
    assertTrue(far.compare(t15.getId(), t8.getId()));
    

    assertTrue(!far.compare(t9.getId(), t10.getId()));
    assertTrue(far.compare(t10.getId(), t9.getId()));
    assertTrue(!far.compare(t9.getId(), t11.getId()));
    assertTrue(far.compare(t11.getId(), t9.getId()));
    assertTrue(!far.compare(t9.getId(), t12.getId()));
    assertTrue(far.compare(t12.getId(), t9.getId()));

    assertTrue(!far.compare(t9.getId(), t13.getId()));
    assertTrue(far.compare(t13.getId(), t9.getId()));
    assertTrue(!far.compare(t9.getId(), t14.getId()));
    assertTrue(far.compare(t14.getId(), t9.getId()));
    assertTrue(!far.compare(t9.getId(), t15.getId()));
    assertTrue(far.compare(t15.getId(), t9.getId()));


    assertTrue(!far.compare(t10.getId(), t11.getId()));
    assertTrue(!far.compare(t11.getId(), t12.getId()));
    assertTrue(!far.compare(t12.getId(), t10.getId()));

    assertTrue(!far.compare(t10.getId(), t13.getId()));
    assertTrue(!far.compare(t13.getId(), t10.getId()));
    assertTrue(!far.compare(t10.getId(), t14.getId()));
    assertTrue(!far.compare(t14.getId(), t10.getId()));
    assertTrue(!far.compare(t10.getId(), t15.getId()));
    assertTrue(!far.compare(t15.getId(), t10.getId()));

    assertTrue(!far.compare(t11.getId(), t13.getId()));
    assertTrue(!far.compare(t13.getId(), t11.getId()));
    assertTrue(!far.compare(t11.getId(), t14.getId()));
    assertTrue(!far.compare(t14.getId(), t11.getId()));
    assertTrue(!far.compare(t11.getId(), t15.getId()));
    assertTrue(!far.compare(t15.getId(), t11.getId()));

    assertTrue(!far.compare(t12.getId(), t13.getId()));
    assertTrue(!far.compare(t13.getId(), t12.getId()));
    assertTrue(!far.compare(t12.getId(), t14.getId()));
    assertTrue(!far.compare(t14.getId(), t12.getId()));
    assertTrue(!far.compare(t12.getId(), t15.getId()));
    assertTrue(!far.compare(t15.getId(), t12.getId()));

    assertTrue(!far.compare(t13.getId(), t14.getId()));
    assertTrue(!far.compare(t14.getId(), t15.getId()));
    assertTrue(!far.compare(t15.getId(), t13.getId()));
    
    return true;
  }

  static bool testValueEnum() {
    TestAssembly assembly;
    PlanDatabaseId db = assembly.getPlanDatabase();
    ConstraintEngineId ce = assembly.getConstraintEngine();

    Variable<IntervalIntDomain> intIntVar(ce, IntervalIntDomain(1, 5), true);

    std::list<double> ints;
    ints.push_back(1);

    EnumeratedDomain singletonEnumIntDom(ints, true, "INTEGER_ENUMERATION");
    Variable<EnumeratedDomain> singletonEnumIntVar(ce, singletonEnumIntDom, true);

    ints.push_back(2);
    ints.push_back(3);
    ints.push_back(4);
    ints.push_back(5);
    EnumeratedDomain enumIntDom(ints, true, "INTEGER_ENUMERATION");
    Variable<EnumeratedDomain> enumIntVar(ce, enumIntDom, true);

    std::list<double> strings;
    strings.push_back(LabelStr("foo"));
    strings.push_back(LabelStr("bar"));
    strings.push_back(LabelStr("baz"));
    strings.push_back(LabelStr("quux"));
    EnumeratedDomain enumStrDom(strings, false, "SYMBOL_ENUMERATION");
    Variable<EnumeratedDomain> enumStrVar(ce, enumStrDom, true);


    EnumeratedDomain enumObjDom(false, "A");
    Variable<EnumeratedDomain> enumObjVar(ce, enumObjDom, true);
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
    assertTrue(intHeurXml != NULL);

    std::string intTrimHeur("<FlawHandler component=\"ValueEnum\">\
                               <Value val=\"3\"/>\
                               <Value val=\"2\"/>\
                               <Value val=\"4\"/>\
                             </FlawHandler>");
    TiXmlElement* intTrimHeurXml = initXml(intTrimHeur);
    assertTrue(intTrimHeurXml != NULL);

    std::string strHeur("<FlawHandler component=\"ValueEnum\">\
                           <Value val=\"baz\"/>\
                           <Value val=\"quux\"/>\
                           <Value val=\"bar\"/>\
                           <Value val=\"foo\"/>\
                         </FlawHandler>");
    TiXmlElement* strHeurXml = initXml(strHeur);
    assertTrue(strHeurXml != NULL);

    std::string strTrimHeur("<FlawHandler component=\"ValueEnum\">\
                               <Value val=\"quux\"/>\
                               <Value val=\"foo\"/>\
                             </FlawHandler>");
    TiXmlElement* strTrimHeurXml = initXml(strTrimHeur);
    assertTrue(strTrimHeurXml != NULL);

    std::string objHeur("<FlawHandler component=\"ValueEnum\">\
                           <Value val=\"o3\"/>\
                           <Value val=\"o2\"/>\
                           <Value val=\"o4\"/>\
                           <Value val=\"o1\"/>\
                         </FlawHandler>");
    TiXmlElement* objHeurXml = initXml(objHeur);
    assertTrue(objHeurXml != NULL);

    std::string objTrimHeur("<FlawHandler component=\"ValueEnum\">\
                               <Value val=\"o1\"/>\
                               <Value val=\"o4\"/>\
                               <Value val=\"o2\"/>\
                             </FlawHandler>");
    TiXmlElement* objTrimHeurXml = initXml(objTrimHeur);
    assertTrue(objTrimHeurXml != NULL);

    ValueEnum intIntDP(db->getClient(), intIntVar.getId(), *intHeurXml);
    ValueEnum enumIntDP(db->getClient(), enumIntVar.getId(), *intHeurXml);

    assertTrue(intIntDP.getNext() == 4);
    assertTrue(enumIntDP.getNext() == 4);
    assertTrue(intIntDP.getNext() == 1);
    assertTrue(enumIntDP.getNext() == 1);
    assertTrue(intIntDP.getNext() == 5);
    assertTrue(enumIntDP.getNext() == 5);
    assertTrue(intIntDP.getNext() == 2);
    assertTrue(enumIntDP.getNext() == 2);
    assertTrue(intIntDP.getNext() == 3);
    assertTrue(enumIntDP.getNext() == 3);

    ValueEnum trimIntIntDP(db->getClient(), intIntVar.getId(), *intTrimHeurXml);
    ValueEnum trimEnumIntDP(db->getClient(), enumIntVar.getId(), *intTrimHeurXml);

    assertTrue(trimIntIntDP.getNext() == 3);
    assertTrue(trimEnumIntDP.getNext() == 3); 
    assertTrue(trimIntIntDP.getNext() == 2);
    assertTrue(trimEnumIntDP.getNext() == 2); 
    assertTrue(trimIntIntDP.getNext() == 4);
    assertTrue(trimEnumIntDP.getNext() == 4); 

    ValueEnum enumStrDP(db->getClient(), enumStrVar.getId(), *strHeurXml);

    assertTrue(enumStrDP.getNext() == LabelStr("baz"));
    assertTrue(enumStrDP.getNext() == LabelStr("quux"));
    assertTrue(enumStrDP.getNext() == LabelStr("bar"));
    assertTrue(enumStrDP.getNext() == LabelStr("foo"));

    ValueEnum trimEnumStrDP(db->getClient(), enumStrVar.getId(), *strTrimHeurXml);
    
    assertTrue(trimEnumStrDP.getNext() == LabelStr("quux"));
    assertTrue(trimEnumStrDP.getNext() == LabelStr("foo"));

    ValueEnum enumObjDP(db->getClient(), enumObjVar.getId(), *objHeurXml);

    assertTrue(enumObjDP.getNext() == o3.getId());
    assertTrue(enumObjDP.getNext() == o2.getId());
    assertTrue(enumObjDP.getNext() == o4.getId());
    assertTrue(enumObjDP.getNext() == o1.getId());

    ValueEnum trimEnumObjDP(db->getClient(), enumObjVar.getId(), *objTrimHeurXml);

    assertTrue(trimEnumObjDP.getNext() == o1.getId());
    assertTrue(trimEnumObjDP.getNext() == o4.getId());
    assertTrue(trimEnumObjDP.getNext() == o2.getId());

    //the value enumeration should be ignored if the domain is a singleton
    ValueEnum ignoreHeurDP(db->getClient(), singletonEnumIntVar.getId(), *intTrimHeurXml);

    assertTrue(ignoreHeurDP.getNext() == 1);

    delete objTrimHeurXml;
    delete objHeurXml;
    delete strTrimHeurXml;
    delete strHeurXml;
    delete intTrimHeurXml;
    delete intHeurXml;

    return true;
  }
  static bool testHSTSOpenConditionDecisionPoint() {
    TestAssembly assembly;
    PlanDatabaseId db = assembly.getPlanDatabase();
    DbClientId client = db->getClient();
    Object o1(db, "A", "o1");    

    //flawed token that should be compatible with everything
    IntervalToken flawedToken(db, "A.Foo", false, false);

    assertTrue(client->propagate());
    //test activate only
    std::string aOnlyHeur("<FlawHandler component=\"HSTSOpenConditionDecisionPoint\" choice=\"activateOnly\"/>");
    TiXmlElement* aOnlyHeurXml = initXml(aOnlyHeur);
    HSTS::OpenConditionDecisionPoint aOnly(client, flawedToken.getId(), *aOnlyHeurXml);
    aOnly.initialize();
    assertTrue(aOnly.getStateChoices().size() == 1);
    assertTrue(aOnly.getStateChoices()[0] == Token::ACTIVE);
    assertTrue(aOnly.getCompatibleTokens().empty());

    //test merge only
    IntervalToken tok1(db, "A.Foo", false, false, IntervalIntDomain(1, 10), IntervalIntDomain(6, 20), IntervalIntDomain(5, 10), "o1");
    client->activate(tok1.getId());
    std::string mOnlyHeur("<FlawHandler component=\"HSTSOpenConditionDecisionPoint\" choice=\"mergeOnly\"/>");
    TiXmlElement* mOnlyHeurXml = initXml(mOnlyHeur);
    HSTS::OpenConditionDecisionPoint mOnly(client, flawedToken.getId(), *mOnlyHeurXml);
    assertTrue(client->propagate());
    mOnly.initialize();
    assertTrue(mOnly.getStateChoices().size() == 1);
    assertTrue(mOnly.getStateChoices()[0] == Token::MERGED);
    assertTrue(mOnly.getCompatibleTokens().size() == 1);
    assertTrue(mOnly.getCompatibleTokens()[0] == tok1.getId());

    //test activate/merge
    std::string aFirstHeur("<FlawHandler component=\"HSTSOpenConditionDecisionPoint\" choice=\"activateFirst\"/>");
    TiXmlElement* aFirstHeurXml = initXml(aFirstHeur);
    HSTS::OpenConditionDecisionPoint aFirst(client, flawedToken.getId(), *aFirstHeurXml);
    assertTrue(client->propagate());
    aFirst.initialize();
    assertTrue(aFirst.getStateChoices().size() == 2);
    assertTrue(aFirst.getStateChoices()[0] == Token::ACTIVE);
    assertTrue(aFirst.getStateChoices()[1] == Token::MERGED);
    assertTrue(aFirst.getCompatibleTokens().size() == 1);
    assertTrue(aFirst.getCompatibleTokens()[0] == tok1.getId());

    //test merge/activate
    std::string mFirstHeur("<FlawHandler component=\"HSTSOpenConditionDecisionPoint\" choice=\"mergeFirst\"/>");
    TiXmlElement* mFirstHeurXml = initXml(mFirstHeur);
    HSTS::OpenConditionDecisionPoint mFirst(client, flawedToken.getId(), *mFirstHeurXml);
    assertTrue(client->propagate());
    mFirst.initialize();
    assertTrue(mFirst.getStateChoices().size() == 2);
    assertTrue(mFirst.getStateChoices()[0] == Token::MERGED);
    assertTrue(mFirst.getStateChoices()[1] == Token::ACTIVE);
    assertTrue(mFirst.getCompatibleTokens().size() == 1);
    assertTrue(mFirst.getCompatibleTokens()[0] == tok1.getId());

    //test early
    IntervalToken tok2(db, "A.Foo", false, false, IntervalIntDomain(3, 10), IntervalIntDomain(8, 20), IntervalIntDomain(5, 10), "o1");
    client->activate(tok2.getId());
    std::string mEarlyHeur("<FlawHandler component=\"HSTSOpenConditionDecisionPoint\" choice=\"mergeOnly\" order=\"early\"/>");
    TiXmlElement* mEarlyHeurXml = initXml(mEarlyHeur);
    HSTS::OpenConditionDecisionPoint mEarly(client, flawedToken.getId(), *mEarlyHeurXml);
    assertTrue(client->propagate());
    mEarly.initialize();
    assertTrue(mEarly.getStateChoices().size() == 1);
    assertTrue(mEarly.getStateChoices()[0] == Token::MERGED);
    assertTrue(mEarly.getCompatibleTokens().size() == 2);
    assertTrue(mEarly.getCompatibleTokens()[0] == tok1.getId());
    assertTrue(mEarly.getCompatibleTokens()[1] == tok2.getId());

    //test late
    std::string mLateHeur("<FlawHandler component=\"HSTSOpenConditionDecisionPoint\" choice=\"mergeOnly\" order=\"late\"/>");
    TiXmlElement* mLateHeurXml = initXml(mLateHeur);
    HSTS::OpenConditionDecisionPoint mLate(client, flawedToken.getId(), *mLateHeurXml);
    assertTrue(client->propagate());
    mLate.initialize();
    assertTrue(mLate.getStateChoices().size() == 1);
    assertTrue(mLate.getStateChoices()[0] == Token::MERGED);
    assertTrue(mLate.getCompatibleTokens().size() == 2);
    assertTrue(mLate.getCompatibleTokens()[0] == tok2.getId());
    assertTrue(mLate.getCompatibleTokens()[1] == tok1.getId());

    //test near
    //tok1 has midpoint at 11, tok2 has midpoint at 13, so put flawedToken's midpoint at 10
    const_cast<AbstractDomain&>(flawedToken.getStart()->lastDomain()).intersect(4, 10);
    const_cast<AbstractDomain&>(flawedToken.getEnd()->lastDomain()).intersect(5, 12);
    assertTrue(assembly.getConstraintEngine()->propagate());
    std::string mNearHeur("<FlawHandler component=\"HSTSOpenConditionDecisionPoint\" choice=\"mergeOnly\" order=\"near\"/>");
    TiXmlElement* mNearHeurXml = initXml(mNearHeur);
    HSTS::OpenConditionDecisionPoint mNear(client, flawedToken.getId(), *mNearHeurXml);
    assertTrue(client->propagate());
    mNear.initialize();
    assertTrue(mNear.getStateChoices().size() == 1);
    assertTrue(mNear.getStateChoices()[0] == Token::MERGED);
    assertTrue(mNear.getCompatibleTokens().size() == 2);
    assertTrue(mNear.getCompatibleTokens()[0] == tok1.getId());
    assertTrue(mNear.getCompatibleTokens()[1] == tok2.getId());

    //test far
    std::string mFarHeur("<FlawHandler component=\"HSTSOpenConditionDecisionPoint\" choice=\"mergeOnly\" order=\"far\"/>");
    TiXmlElement* mFarHeurXml = initXml(mFarHeur);
    HSTS::OpenConditionDecisionPoint mFar(client, flawedToken.getId(), *mFarHeurXml);
    mFar.initialize();
    assertTrue(mFar.getStateChoices().size() == 1);
    assertTrue(mFar.getStateChoices()[0] == Token::MERGED);
    assertTrue(mFar.getCompatibleTokens().size() == 2);
    assertTrue(mFar.getCompatibleTokens()[0] == tok2.getId());
    assertTrue(mFar.getCompatibleTokens()[1] == tok1.getId());

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
    TestAssembly assembly;
    PlanDatabaseId db = assembly.getPlanDatabase();
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
    assertTrue(client->propagate());
    early.initialize();
    assertTrue(early.getOrderingChoices().size() == 3);
    assertTrue(early.getOrderingChoices()[0].second.second == tok1.getId());
    assertTrue(early.getOrderingChoices()[1].second.second == tok2.getId());
    assertTrue(early.getOrderingChoices()[2].second.second == flawedToken.getId());

    std::string lateHeur("<FlawHandler component=\"HSTSThreatDecisionPoint\" order=\"late\"/>");
    TiXmlElement* lateHeurXml = initXml(lateHeur);
    HSTS::ThreatDecisionPoint late(client, flawedToken.getId(), *lateHeurXml);
    assertTrue(client->propagate());
    late.initialize();
    assertTrue(late.getOrderingChoices().size() == 3);
    assertTrue(late.getOrderingChoices()[0].second.second == flawedToken.getId());
    assertTrue(late.getOrderingChoices()[1].second.second == tok2.getId());
    assertTrue(late.getOrderingChoices()[2].second.second == tok1.getId());

    std::string nearHeur("<FlawHandler component=\"HSTSThreatDecisionPoint\" order=\"near\"/>");
    TiXmlElement* nearHeurXml = initXml(nearHeur);
    HSTS::ThreatDecisionPoint near(client, flawedToken.getId(), *nearHeurXml);
    assertTrue(client->propagate());
    near.initialize();
    assertTrue(near.getOrderingChoices().size() == 3);
    assertTrue(near.getOrderingChoices()[0].second.second == tok1.getId());
    assertTrue(near.getOrderingChoices()[1].second.second == tok2.getId() ||
               near.getOrderingChoices()[1].second.second == flawedToken.getId());
    assertTrue(near.getOrderingChoices()[2].second.second == flawedToken.getId() ||
               near.getOrderingChoices()[2].second.second == tok2.getId());


    std::string farHeur("<FlawHandler component=\"HSTSThreatDecisionPoint\" order=\"far\"/>");
    TiXmlElement* farHeurXml = initXml(farHeur);
    HSTS::ThreatDecisionPoint far(client, flawedToken.getId(), *farHeurXml);
    assertTrue(client->propagate());
    far.initialize();
    assertTrue(far.getOrderingChoices().size() == 3);
    assertTrue(far.getOrderingChoices()[0].second.second == tok2.getId() ||
               far.getOrderingChoices()[0].second.second == flawedToken.getId());
    assertTrue(far.getOrderingChoices()[1].second.second == flawedToken.getId() ||
               far.getOrderingChoices()[1].second.second == tok2.getId());
    assertTrue(far.getOrderingChoices()[2].second.second == tok1.getId());

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
    runTest(testMinValuesSimpleCSP);
    runTest(testSuccessfulSearch);
    runTest(testExhaustiveSearch);
    runTest(testSimpleActivation);
    runTest(testSimpleRejection);
    runTest(testMultipleSearch);
    runTest(testOversearch);
    runTest(testBacktrackFirstDecisionPoint);
    runTest(testMultipleSolutionsSearch);
    runTest(testGNATS_3196);
    runTest(testContext);
    runTest(testDeletedFlaw);
    runTest(testDeleteAfterCommit);
    return true;
  }

private:
  /**
   * @brief Will load an intial state and solve a csp with only variables.
   */
  static bool testMinValuesSimpleCSP(){
    TestAssembly assembly;
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
      assertTrue(solver.getStepCount() == 2);
      assertTrue(solver.getDepth() == 2);

      // Now clear it and run it again
      solver.reset();
      assertTrue(solver.solve());
      assertTrue(solver.getStepCount() == 2);
      assertTrue(solver.getDepth() == 2);

      // Now partially reset it, and run again
      solver.reset(1);
      assertTrue(solver.solve());
      assertTrue(solver.getStepCount() == 1, toString(solver.getStepCount()));
      assertTrue(solver.getDepth() == 2, toString(solver.getDepth()));
 
      // Now we reset one decision, then clear it. Expect the solution and depth to be 1.
      solver.reset(1);
      solver.clear();
      assertTrue(solver.solve());
      assertTrue(solver.getStepCount() == 1, toString(solver.getStepCount()));
      assertTrue(solver.getDepth() == 1, toString(solver.getDepth()));
    }

    return true;
  }


  static bool testSuccessfulSearch(){
    TestAssembly assembly;
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
    TestAssembly assembly;
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
    TestAssembly assembly;
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
    TestAssembly assembly;
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
    TestAssembly assembly;
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
    TestAssembly assembly;
    TiXmlElement* root = initXml((getTestLoadLibraryPath() + "/SolverTests.xml").c_str(), "SimpleCSPSolver");
    TiXmlElement* child = root->FirstChildElement();

    assert(assembly.playTransactions((getTestLoadLibraryPath() +"/SuccessfulSearch.xml").c_str()));
    Solver solver(assembly.getPlanDatabase(), *child);
    solver.setMaxSteps(5); //arbitrary number of maximum steps
    assert(solver.solve(20)); //arbitrary number of steps < max
    
    return true;
  }

  static bool testBacktrackFirstDecisionPoint(){
    TestAssembly assembly;
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
    TestAssembly assembly;
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

  static bool testGNATS_3196(){
    TestAssembly assembly;
    TiXmlElement* root = initXml( (getTestLoadLibraryPath() + "/SolverTests.xml").c_str(), "GNATS_3196");
    TiXmlElement* child = root->FirstChildElement();
    assert(assembly.playTransactions( (getTestLoadLibraryPath() + "/GNATS_3196.xml").c_str()));
    Solver solver(assembly.getPlanDatabase(), *child);
    assertFalse(solver.solve(1));
    solver.clear();
    TokenId onlyToken = *(assembly.getPlanDatabase()->getTokens().begin());
    onlyToken->discard();
    assertTrue(solver.solve(1));
    return true;
  }

  static bool testContext() {
    TestAssembly assembly;
    std::stringstream data;
    data << "<Solver name=\"TestSolver\">" << std::endl;
    data << "</Solver>" << std::endl;
    std::string xml = data.str();
    TiXmlElement* root = initXml(xml);
    Solver solver(assembly.getPlanDatabase(), *root);
    ContextId ctx = solver.getContext();
    assertTrue(ctx->getName() == LabelStr(solver.getName().toString() + "Context"));
    ctx->put("foo", 1);
    assertTrue(ctx->get("foo") == 1);
    assertTrue(solver.getContext()->get("foo") == 1);
    ctx->remove("foo");
    return true;
  }

  static bool testDeletedFlaw() {
    TiXmlElement* root = initXml((getTestLoadLibraryPath() + "/FlawHandlerTests.xml").c_str(), "TestDynamicFlaws");
    TestAssembly assembly;
    PlanDatabaseId db = assembly.getPlanDatabase();
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
    Solver solver(assembly.getPlanDatabase(), *(root->FirstChildElement()));
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
    TestAssembly assembly;
    PlanDatabaseId db = assembly.getPlanDatabase();
    Object o1(db, "CommitTest", "o1");
    db->close();
    Solver solver(assembly.getPlanDatabase(), *child);

    // iterate over the possibilties with commiting and deleting with four tokens
    for(int i=1;i<255;i++)
    {
   		IntervalIntDomain& horizon = HorizonFilter::getHorizon();
   		horizon = IntervalIntDomain(0, 40);
      TokenId first = db->getClient()->createToken("CommitTest.chaina", false);
      first->getStart()->specify(0);
      solver.solve(100,100);

      TokenId second = first->getSlave(0);
      assertTrue(second->getPredicateName() == LabelStr("CommitTest.chainb"), second->getPredicateName().toString());
      TokenId third = second->getSlave(0);
      assertTrue(third->getPredicateName() == LabelStr("CommitTest.chaina"), third->getPredicateName().toString());
      TokenId fourth = third->getSlave(0);
      assertTrue(fourth->getPredicateName() == LabelStr("CommitTest.chainb"), fourth->getPredicateName().toString());

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

      assertTrue(solver.isValid(), "Solver must be valid after discards.");

   		horizon = IntervalIntDomain(0, 40);
      solver.solve(100,100);
      assertTrue(solver.isValid(), "Solver must be valid after continuing solving after discards.");

      solver.reset();
			// anything which was not deleted must now be deleted
    	TokenSet tokens = assembly.getPlanDatabase()->getTokens();
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
    runTest(testUnboundVariableFlawIteration);
    runTest(testThreatFlawIteration);
    runTest(testOpenConditionFlawIteration);
    //runTest(testSolverIteration);
    return true;
  }
private:

  static bool testUnboundVariableFlawIteration() {
    TiXmlElement* root = initXml((getTestLoadLibraryPath() + "/FlawFilterTests.xml" ).c_str(), "UnboundVariableManager");
    
    TestAssembly assembly;
    UnboundVariableManager fm(*root);
    fm.initialize(assembly.getPlanDatabase());
    assert(assembly.playTransactions((getTestLoadLibraryPath() + "/UnboundVariableFiltering.xml").c_str()));

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
    TiXmlElement* root = initXml((getTestLoadLibraryPath() + "/FlawFilterTests.xml" ).c_str(), "OpenConditionManager");
    
    TestAssembly assembly;
    OpenConditionManager fm(*root);
    IntervalIntDomain& horizon = HorizonFilter::getHorizon();
    horizon = IntervalIntDomain(0, 1000);
    fm.initialize(assembly.getPlanDatabase());
    assert(assembly.playTransactions((getTestLoadLibraryPath() + "/OpenConditionFiltering.xml").c_str()));

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
    TiXmlElement* root = initXml((getTestLoadLibraryPath() + "/FlawFilterTests.xml" ).c_str(), "ThreatManager");

    TestAssembly assembly;
    ThreatManager fm(*root);
    IntervalIntDomain& horizon = HorizonFilter::getHorizon();
    horizon = IntervalIntDomain(0, 1000);
    fm.initialize(assembly.getPlanDatabase());
    assert(assembly.playTransactions((getTestLoadLibraryPath() + "/ThreatFiltering.xml").c_str()));

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
    TiXmlElement* root = initXml((getTestLoadLibraryPath() + "/IterationTests.xml" ).c_str(), "Solver");
    TestAssembly assembly;
    ThreatManager tm(*(root->FirstChildElement("ThreatManager")));
    OpenConditionManager ocm(*(root->FirstChildElement("OpenConditionManager")));
    UnboundVariableManager uvm(*(root->FirstChildElement("UnboundVariableManager")));
    Solver solver(assembly.getPlanDatabase(), *root);

    IntervalIntDomain& horizon = HorizonFilter::getHorizon();
    horizon = IntervalIntDomain(0, 1000);

    assert(assembly.playTransactions((getTestLoadLibraryPath() + "/ThreatFiltering.xml").c_str()));
    //assert(assembly.playTransactions((getTestLoadLibraryPath() + "/OpenConditionFiltering.xml").c_str()));
    //assert(assembly.playTransactions((getTestLoadLibraryPath() + "/UnboundVariableFiltering.xml").c_str()));

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

class FlawManagerTests {
public:
  static bool test() {
    return true;
  }
};

void initSolverModuleTests() {
 
  // Allocate the schema with a call to the linked in model function - eventually
  // make this called via dlopen
  EUROPA::NDDL::loadSchema(); 
}

void SolversModuleTests::runTests(std::string path) {
   SolversTestEngine::initialize();
   setTestLoadLibraryPath(path);

   // For tests on the matching engine
   REGISTER_COMPONENT_FACTORY(MatchingRule, MatchingRule);

   // Register components under program execution so that static allocation can have occurred
   // safely. This was required due to problems on the MAC.
   REGISTER_COMPONENT_FACTORY(TestComponent, A);
   REGISTER_COMPONENT_FACTORY(TestComponent, B);
   REGISTER_COMPONENT_FACTORY(TestComponent, C);
   REGISTER_COMPONENT_FACTORY(TestComponent, D);
   REGISTER_COMPONENT_FACTORY(TestComponent, E);

   // Initialization of various ids and other required elements
   initSolverModuleTests();

   // Set up the required components. Should eventually go into an assembly. Note they are allocated on the stack, not the heap
   REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::RandomValue, Random);

   // Constraints used for testing
   REGISTER_CONSTRAINT(LazyAllDiff, "lazyAllDiff",  "Default");
   REGISTER_CONSTRAINT(LazyAlwaysFails, "lazyAlwaysFails",  "Default");

   runTestSuite(ComponentFactoryTests::test);
   runTestSuite(FilterTests::test);
   runTestSuite(FlawIteratorTests::test);
   runTestSuite(FlawManagerTests::test);
   runTestSuite(FlawHandlerTests::test);
   runTestSuite(SolverTests::test);

   SolversTestEngine::terminate();
}

