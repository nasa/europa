#include "../../NDDL/test/nddl-test-module.hh"
#include "../../HSTS/test/hsts-test-module.hh"
#include "../../utils/test/util-test-module.hh"
#include "../../Solvers/test/solvers-test-module.hh"
#include "../../Aver/test/aver-test-module.hh"
#include "../../CBPlanner/test/cbp-test-module.hh"
#include "../../ConstraintEngine/test/ce-test-module.hh"
#include "../../HeuristicsEngine/test/he-test-module.hh"
#include "../../PlanDatabase/test/db-test-module.hh"
#include "../../Resource/test/rs-test-module.hh"
#include "../../RulesEngine/test/re-test-module.hh"
#include "../../TemporalNetwork/test/tn-test-module.hh"

/**
 * @brief Test harness for running all module tests from a single binary to enable code coverage
 *        analysis tools to examine the codebase.
 */

// stubbed out - just provide a defintion of this symbol.
extern "C" void loadSchema() {}


int main(int argc, const char** argv) {

  // c++ path finding problems etc.

  HSTSModuleTests::runTests("../../hsts/test"); // not able to find some hsts includes/
  //UtilModuleTests::runTests("../../Utils/test");
  //NDDLModuleTests::runTests("../../NDDL/test");

  // PLASMA problems

  //ConstraintEngineModuleTests::runTests("../../ConstraintEngine/test"); // assertion failure in domian-tests.cc a is b   instance of a can be compared to instance of b
  //SolversModuleTests::runTests("../../Solvers/test");  // Factory "A" is not registered.

  AverModuleTests::runTests("../../Aver/test");
  CBPlannerModuleTests::runTests("../../CBPlanner/test");
  PlanDatabaseModuleTests::runTests("../../PlanDatabase/test");
  ResourceModuleTests::runTests("../../Resource/test");
  RulesEngineModuleTests::runTests("../../RulesEngine/test");
  HeuristicsEngineModuleTests::runTests("../../HeuristicsEngine/test");
  TemporalNetworkModuleTests::runTests("../../TemporalNetwork/test"); 

  return 0;
}
