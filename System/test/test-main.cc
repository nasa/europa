//#include "../../Solvers/test/test-module.cc"
//#include "../../NDDL/test/nddl-test-module.cc"
#include "../Aver/test/aver-test-module.hh"
#include "../CBPlanner/test/cbp-test-module.hh"
#include "../ConstraintEngine/test/ce-test-module.hh"
#include "../../HeuristicsEngine/test/he-test-module.cc"
#include "../PlanDatabase/test/db-test-module.hh"
#include "../Resource/test/rs-test-module.hh"
#include "../RulesEngine/test/re-test-module.hh"
#include "../TemporalNetwork/test/tn-test-module.hh"
#include "../HSTS/test/hsts-test-module.hh"
//#include "../../Utils/test/util-test-module.cc" // duplicate symbol defintion problems.

/**
 * @brief Test harness for running all module tests from a single binary to enable code coverage
 *        analysis tools to examine the codebase.
 */

int main(int argc, const char** argv) {
 
  //SolverModuleTests::runTests();  // cannot include .hh file
  //NDDLModuleTets::runTests(); // cannot include .hh file

  //AverModuleTests::runTests();  // load file path
  //UtilModuleTests::runTests(); // lot of duplicate symbol problems
  //HSTSModuleTests::runTests();  // load file path
  CBPlannerModuleTests::runTests();
  //ConstraintEngineModuleTests::runTests(); // load file path
  PlanDatabaseModuleTests::runTests();
  ResourceModuleTests::runTests();
  RulesEngineModuleTests::runTests();
  //HeuristicsEngineModuleTests::runTests();  // load file path
  TemporalNetworkModuleTests::runTests(); 

  return 0;
}
