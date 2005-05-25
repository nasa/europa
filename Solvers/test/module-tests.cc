#include "Nddl.hh"
#include "StandardAssembly.hh"
#include "Solver.hh"
#include "ComponentFactory.hh"
#include "Constraint.hh"
#include "ConstraintLibrary.hh"
#include "UnboundVariableManager.hh"
#include "OpenConditionManager.hh"
#include "ThreatManager.hh"
#include "Filters.hh"
#include "Token.hh"
#include "TestSupport.hh"
#include "Debug.hh"
#include "../../PlanDatabase/test/PlanDatabaseTestSupport.hh"
#include <fstream>

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

REGISTER_COMPONENT_FACTORY(TestComponent, A);
REGISTER_COMPONENT_FACTORY(TestComponent, B);
REGISTER_COMPONENT_FACTORY(TestComponent, C);
REGISTER_COMPONENT_FACTORY(TestComponent, D);
REGISTER_COMPONENT_FACTORY(TestComponent, E);

// Register filter components
REGISTER_COMPONENT_FACTORY(SingletonFilter, Singleton);
REGISTER_COMPONENT_FACTORY(HorizonFilter, HorizonFilter);

/**
 * @brief Helper method to get the first xml element in the file
 */
TiXmlElement* initXml(const char* sourceFile, const char* element = NULL){
  std::ifstream is(sourceFile);
  checkError(is.good(), "Invalid input stream '" << sourceFile << "'");

  
  while(!is.eof()){
    while(!is.eof() && is.peek() != '<')
      is.get();

    TiXmlElement * xmlElement = new TiXmlElement("");
    is >> (*xmlElement);
    debugMsg("Tests", "Loading element " << *xmlElement);
    if(element == NULL || strcmp(xmlElement->Value(), element) == 0)
      return xmlElement;
    else
      delete xmlElement;
  }

  return NULL;
}

class ComponentFactoryTests{
public:
  static bool test(){
    runTest(testBasicAllocation);
    return true;
  }

private:
  static bool testBasicAllocation(){
    TiXmlElement* configXml = initXml("ComponentFactoryTest.xml");

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



class FlawFilterTests {
public:
  static bool test(){
    runTest(testVariableFiltering);
    runTest(testTokenFiltering);
    runTest(testThreatFiltering);
    return true;
  }

private:
  
  static bool testVariableFiltering(){
    TiXmlElement* root = initXml("FlawFilterTests.xml", "UnboundVariableManager");

    StandardAssembly assembly(Schema::instance());
    UnboundVariableManager fm(*root);
    fm.initialize(assembly.getPlanDatabase());
    assert(assembly.playTransactions("UnboundVariableFiltering.xml"));

    // Simple filter on a variable
    ConstrainedVariableSet variables = assembly.getConstraintEngine()->getVariables();
    for(ConstrainedVariableSet::const_iterator it = variables.begin(); it != variables.end(); ++it){
      ConstrainedVariableId var = *it;

      // Confirm temporal variables have been excluded
      static const LabelStr excludedVariables(":start:end:duration:arg1:arg3:arg4:arg6:arg7:arg8");
      static const LabelStr includedVariables(":arg2:arg5:");
      std::string s = ":" + var->getName().toString() + ":";
      if(excludedVariables.contains(s))
	assertTrue(!fm.inScope(var))
      else if(includedVariables.contains(s))
	assertTrue(fm.inScope(var), var->toString());
    }

    // Confirm that a global variable is first a flaw, but when bound is no longer a flaw, and when bound again,
    // returns as a flaw
    ConstrainedVariableId globalVar1 = assembly.getPlanDatabase()->getGlobalVariable("globalVariable1");
    ConstrainedVariableId globalVar2 = assembly.getPlanDatabase()->getGlobalVariable("globalVariable2");
    assertTrue(!fm.inScope(globalVar1));
    assertTrue(fm.inScope(globalVar2));
    globalVar2->specify(globalVar2->lastDomain().getLowerBound());
    assembly.getConstraintEngine()->propagate();
    assertTrue(!fm.inScope(globalVar2));
    assertTrue(fm.inScope(globalVar1)); // By propagation it will be a singleton, so it will be included
    globalVar2->reset();
    assembly.getConstraintEngine()->propagate();
    assertTrue(!fm.inScope(globalVar1));
    assertTrue(fm.inScope(globalVar2));

    return true;
  }

  static bool testTokenFiltering(){
    TiXmlElement* root = initXml("FlawFilterTests.xml", "OpenConditionManager");

    StandardAssembly assembly(Schema::instance());
    OpenConditionManager fm(*root);
    IntervalIntDomain& horizon = HorizonFilter::getHorizon();
    horizon = IntervalIntDomain(0, 1000);
    fm.initialize(assembly.getPlanDatabase());
    assert(assembly.playTransactions("OpenConditionFiltering.xml"));

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
    TiXmlElement* root = initXml("FlawFilterTests.xml", "ThreatManager");

    StandardAssembly assembly(Schema::instance());
    ThreatManager fm(*root);
    IntervalIntDomain& horizon = HorizonFilter::getHorizon();
    horizon = IntervalIntDomain(0, 1000);
    fm.initialize(assembly.getPlanDatabase());
    assert(assembly.playTransactions("ThreatFiltering.xml"));

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


class SolverTests {
public:
  static bool test(){
    runTest(testMinValuesSimpleCSP);
    runTest(testSuccessfulSearch);
    runTest(testExhaustiveSearch);
    runTest(testSimpleActivation);
    runTest(testSimpleRejection);
    return true;
  }

private:
  /**
   * @brief Will load an intial state and solve a csp with only variables.
   */
  static bool testMinValuesSimpleCSP(){
    StandardAssembly assembly(Schema::instance());
    TiXmlElement* root = initXml("SolverTests.xml", "SimpleCSPSolver");
    TiXmlElement* child = root->FirstChildElement();
    {
      assert(assembly.playTransactions("StaticCSP.xml"));
      Solver solver(assembly.getPlanDatabase(), *child);
      assertTrue(solver.solve());
      const ConstrainedVariableSet& allVars = assembly.getPlanDatabase()->getGlobalVariables();
      assertTrue(solver.getStepCount() == allVars.size());
      assertTrue(solver.getDepth() == allVars.size());
      for(ConstrainedVariableSet::const_iterator it = allVars.begin(); it != allVars.end(); ++it){
	ConstrainedVariableId var = *it;
	assertTrue(var->specifiedDomain().isSingleton());
      }

      // Run the solver again.
      assertTrue(solver.solve());
      assertTrue(solver.getStepCount() == allVars.size());
      assertTrue(solver.getDepth() == allVars.size());

      // Now clear it and run it again
      solver.reset();
      assertTrue(solver.solve());
      assertTrue(solver.getStepCount() == allVars.size());
      assertTrue(solver.getDepth() == allVars.size());

      // Now partially reset it, and run again
      solver.reset(2);
      assertTrue(solver.solve());
      assertTrue(solver.getStepCount() == 2);
      assertTrue(solver.getDepth() == allVars.size());
 
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
    TiXmlElement* root = initXml("SolverTests.xml", "SimpleCSPSolver");
    TiXmlElement* child = root->FirstChildElement();
    {
      assert(assembly.playTransactions("SuccessfulSearch.xml"));
      Solver solver(assembly.getPlanDatabase(), *child);
      assertTrue(solver.solve());
    }
    return true;
  }

  static bool testExhaustiveSearch(){
    StandardAssembly assembly(Schema::instance());
    TiXmlElement* root = initXml("SolverTests.xml", "SimpleCSPSolver");
    TiXmlElement* child = root->FirstChildElement();
    {
      assert(assembly.playTransactions("ExhaustiveSearch.xml"));
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
    TiXmlElement* root = initXml("SolverTests.xml", "SimpleActivationSolver");
    TiXmlElement* child = root->FirstChildElement();
    {
      IntervalIntDomain& horizon = HorizonFilter::getHorizon();
      horizon = IntervalIntDomain(0, 1000);
      assert(assembly.playTransactions("SimpleActivation.xml"));
      Solver solver(assembly.getPlanDatabase(), *child);
      assertTrue(solver.solve());
    }

    return true;
  }

  static bool testSimpleRejection() {
    StandardAssembly assembly(Schema::instance());
    TiXmlElement* root = initXml("SolverTests.xml", "SimpleRejectionSolver");
    TiXmlElement* child = root->FirstChildElement();
    {
      IntervalIntDomain& horizon = HorizonFilter::getHorizon();
      horizon = IntervalIntDomain(0, 1000);
      assert(assembly.playTransactions("SimpleRejection.xml"));
      Solver solver(assembly.getPlanDatabase(), *child);
      assertTrue(solver.solve());
      assertTrue(assembly.getPlanDatabase()->getTokens().size() == 1);
    }


    return true;
  }
};

void initSolverModuleTests() {
  StandardAssembly::initialize();

  // Allocate the schema with a call to the linked in model function - eventually
  // make this called via dlopen
  NDDL::loadSchema();
}

int main(){
  // Initialization of various id's and other required elements
  initSolverModuleTests();

  // Set up the required components. Shoudl eventually go into an assembly. Note they are allocated on the stack, not the heap
  REGISTER_VARIABLE_DECISION_FACTORY(EUROPA::SOLVERS::MinValue, StandardVariableHandler);
  REGISTER_VARIABLE_DECISION_FACTORY(EUROPA::SOLVERS::MinValue, Min);
  REGISTER_VARIABLE_DECISION_FACTORY(EUROPA::SOLVERS::MaxValue, Max);
  REGISTER_VARIABLE_DECISION_FACTORY(EUROPA::SOLVERS::RandomValue, Random);
  REGISTER_COMPONENT_FACTORY(EUROPA::SOLVERS::UnboundVariableManager, UnboundVariableManager);
  REGISTER_OPENCONDITION_DECISION_FACTORY(EUROPA::SOLVERS::OpenConditionDecisionPoint, StandardOpenConditionHandler);
  REGISTER_COMPONENT_FACTORY(EUROPA::SOLVERS::OpenConditionManager, OpenConditionManager);
  REGISTER_THREAT_DECISION_FACTORY(EUROPA::SOLVERS::ThreatDecisionPoint, StandardThreatHandler);
  REGISTER_COMPONENT_FACTORY(EUROPA::SOLVERS::ThreatManager, ThreatManager);


  // Constraints used for testing
  REGISTER_CONSTRAINT(LazyAllDiff, "lazyAllDiff",  "Default");
  REGISTER_CONSTRAINT(LazyAlwaysFails, "lazyAlwaysFails",  "Default");

  runTestSuite(ComponentFactoryTests::test);
  runTestSuite(FlawFilterTests::test);
  runTestSuite(SolverTests::test);

  return 0;
}
