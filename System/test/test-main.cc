//#include "../../Aver/test/test-module.cc"
//#include "../../CBPlanner/test/test-module.cc"
//#include "../../ConstraintEngine/test/test-module.cc"
//#include "../../HeuristicsEngine/test/test-module.cc"
//#include "../../HSTS/test/test-module.cc"
//#include "../../NDDL/test/test-module.cc"
#include "../../PlanDatabase/test/test-module.cc"
#include "../../Resource/test/test-module.cc"
//#include "../../RulesEngine/test/test-module.cc"
//#include "../../Solvers/test/test-module.cc"
//#include "../../TemporalNetwork/test/test-module.cc"
//#include "../../Utils/test/test-module.cc"

/**
 * @brief Test harness for running all module tests from a single binary to enable code coverage
 *        analysis tools to examine the codebase.
 */

int main(int argc, const char** argv) {
  // setup singleton entities for all tests.
  SchemaId schema = Schema::instance();

  //AverModuleTests::runTests();  // looks like a path problem in the test itself when called from outside standard dir.
  //CBPlannerModuleTests::runTests(); // crashes.
  //ConstraintEngineModuleTests::runTests();  // load problem
  //HeuristicsEngineModuleTests::runTests(); // xmutils error
  //HSTSModuleTests::runTests(); // cannot find hstsnobranch...
  // NDDLModuleTets::runTests(); // conot find domain.hh

  PlanDatabaseModuleTests::runTests();
  ResourceModuleTests::runTests();

  // RulesEngineModuleTests::runTests(); // test rule not found
  //SolverModuleTests::runTests(); // won't ru n
  //TemporalNetworkModuleTests::runTests(); // cannot find testSubgoalRule
  //UtilModuleTests::runTests(); // test appears to fail
  return 0;
}
