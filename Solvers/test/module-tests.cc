#include "Nddl.hh"
#include "StandardAssembly.hh"
#include "Solver.hh"
#include "ComponentFactory.hh"
#include "VariableFlawManager.hh"
//#include "FlawFilter.hh"
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
REGISTER_COMPONENT_FACTORY(SingletonFilter, Singleton)

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

      TestComponent * testComponent = static_cast<TestComponent*>(Allocator::allocate(*child));
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
    return true;
  }

private:
  
  static bool testVariableFiltering(){
    TiXmlElement* root = initXml("FlawFilterTests.xml", "VariableFlawManager");

    StandardAssembly assembly(Schema::instance());
    VariableFlawManager fm(*root);
    fm.initialize(assembly.getPlanDatabase());
    assert(assembly.playTransactions("VariableFiltering.xml"));

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

    /*
    // All Variables of a predicate are excluded
    TokenSet tokens = assembly.getPlanDatabase()->getTokens();
    for(TokenSet::const_iterator it = tokens.begin(); it != tokens.end(); ++it){
      static const LabelStr excludedPredicates(":D.predicateA:D.predicateB:D.predicateC:E.predicateC:");
      TokenId token = *it;
      std::string s = ":" + token->getPredicateName().toString() + ":";
      if(excludedPredicates.contains(s))
	assertTrue(!staticFilter.inScope(token))
      else
	assertTrue(token->isActive() || staticFilter.inScope(token), token->toString());
    }

    ObjectSet objects = assembly.getPlanDatabase()->getObjects();
    for(ObjectSet::const_iterator it = objects.begin(); it != objects.end(); ++it){
      static const LabelStr excludedObjectTypes(":NoPredicates:PredicateRoot:PredicateDerived:");
      ObjectId object = *it;
      std::string s = ":" + object->getType().toString() + ":";
      if(excludedObjectTypes.contains(s))
	assertFalse(staticFilter.inScope(object));
    }

    return true;
    }*/
};


class SolverTests {
public:
  static bool test(){
    runTest(testMinValuesSimpleCSP);
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

  runTestSuite(ComponentFactoryTests::test);
  runTestSuite(FlawFilterTests::test);
  runTestSuite(SolverTests::test);

  return 0;
}
