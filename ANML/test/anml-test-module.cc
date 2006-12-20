#include <iostream>
#include "TestSupport.hh"

#include "anml-test-module.hh"
// Support for default setup
#include "ConstraintEngine.hh"
#include "PlanDatabase.hh"
#include "RulesEngine.hh"
#include "Schema.hh"
#include "DefaultPropagator.hh"
#include "Object.hh"
#include "Constraints.hh"
#include "ANMLParser.hpp"

#include "LockManager.hh"

using namespace EUROPA;

class CrewTest {
public:
  static bool test() {
    runTest(testParse);
    return true;
  }
private:
  static void outputAST2Dot(const antlr::RefAST ast, const int parent, int& node) {
    std::cout << "  node_" << node;
    std::cout <<  " [label=\"" << ast->toString() << "\"];\n";
    if(parent != -1)
      std::cout << "  node_" << parent << " -> node_" << node << ";\n";

    if(ast->getFirstChild()) {
			int self = node;
      outputAST2Dot(ast->getFirstChild(), self, ++node);
		}
    
    if(ast->getNextSibling())
      outputAST2Dot(ast->getNextSibling(), parent, ++node);
  }

  static bool testParse() {
    // do tests with assertTrue!
    ANTLR_USE_NAMESPACE(antlr)RefAST ast =
      ANMLParser::parse(getTestLoadLibraryPath(),"CrewPlanning-problem-instance.anml");
    assertTrue(ast != ANTLR_USE_NAMESPACE(antlr)nullAST);
		int node = 0;

		std::cout << "\n\ndigraph CrewPlanningModel {\n";
    outputAST2Dot(ast, -1, node);
		std::cout << "}\n\n";
		//std::cout << ast->toStringList();
    return true;
  }
};

void ANMLModuleTests::runTests(std::string path) {
  LockManager::instance().connect();
  LockManager::instance().lock();
  setTestLoadLibraryPath(path);

  REGISTER_CONSTRAINT(EqualConstraint, "eq", "Default");

  // Pre-allocate a schema
  //SCHEMA;

  //initANML();
  runTestSuite(CrewTest::test); 

  std::cout << "Finished" << std::endl;
  ConstraintLibrary::purgeAll();
  uninitConstraintLibrary();
  }


