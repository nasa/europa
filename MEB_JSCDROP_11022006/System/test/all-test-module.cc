#include "all-test-module.hh"
#include "../../NDDL/test/nddl-test-module.hh"
#include "../../HSTS/test/hsts-test-module.hh"
#include "../../Utils/test/util-test-module.hh"
#include "../../Solvers/test/solvers-test-module.hh"
#include "../../Aver/test/aver-test-module.hh"
#include "../../CBPlanner/test/cbp-test-module.hh"
#include "../../ConstraintEngine/test/ce-test-module.hh"
#include "../../HeuristicsEngine/test/he-test-module.hh"
#include "../../PlanDatabase/test/db-test-module.hh"
#include "../../Resource/test/rs-test-module.hh"
#include "../../RulesEngine/test/re-test-module.hh"
#include "../../TemporalNetwork/test/tn-test-module.hh"


void AllModuleTests::runTests() {

  /** O R D E R  I S  I M P O R T A N T ! **/

  // Keep ConstraintEngine test first as it demands an absolutly clean system to operate. 
  ConstraintEngineModuleTests::runTests("../../ConstraintEngine/test");
 
  // The following cluster of tests can be placed in any order
  AverModuleTests::runTests("../../Aver/test");
  CBPlannerModuleTests::runTests("../../CBPlanner/test");
  HSTSModuleTests::runTests("../../HSTS/test");  
  NDDLModuleTests::runTests("../../NDDL/test"); 
  ResourceModuleTests::runTests("../../Resource/test");
  RulesEngineModuleTests::runTests("../../RulesEngine/test");
  HeuristicsEngineModuleTests::runTests("../../HeuristicsEngine/test");
  TemporalNetworkModuleTests::runTests("../../TemporalNetwork/test");

  // Keep SolversModuleTests and PlanDatabaseModuleTests here. They
  // leave the system in a dirty state that will upset the earlier tests.
  SolversModuleTests::runTests("../../Solvers/test");  
  PlanDatabaseModuleTests::runTests("../../PlanDatabase/test"); // domain comparater interacting from HSTS test.
 
  // Keep UtilModuleTests last as it leaves some debug messages set as a side effect. 
  // The "fix" would be to detect the set of debug messages registered by test
  // then unregister them iff they were not previously registered.
  UtilModuleTests::runTests("../../Utils/test");

 }
