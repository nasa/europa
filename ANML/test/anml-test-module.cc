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
  static bool testParse() {
    // do tests with assertTrue!
		assertTrue(ANMLParser::parse(getTestLoadLibraryPath(),"CrewPlanning-problem-instance.anml"));
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


