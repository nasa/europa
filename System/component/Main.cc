/**
 * @file Main.cc
 *
 * @brief Provides an executable for your project which will use a
 * standard Chronological backtracking planner and a Test Assembly of
 * EUROPA
 */

#include "Nddl.hh" /*!< Includes protypes required to load a model */
#include "TestAssembly.hh" /*!< For using a test EUROPA Assembly */
#include "ComponentFactory.hh"
#include "OpenConditionDecisionPoint.hh"
#include "OpenConditionManager.hh"
#include "ThreatDecisionPoint.hh"
#include "ThreatManager.hh"
#include "UnboundVariableDecisionPoint.hh"
#include "UnboundVariableManager.hh"
#include "DecisionPoint.hh"
#include "MatchingRule.hh"
#include "Filters.hh"

using namespace EUROPA;

/**
 * @brief Uses the planner to solve a planning problem
 */
int main(int argc, const char ** argv){
  if (argc != 3) {
    std::cerr << "Must provide initial transactions file." << std::endl;
    return -1;
  }

  const char* txSource = argv[1];

  const char* plannerConfig = argv[2];

  REGISTER_VARIABLE_DECISION_FACTORY(EUROPA::SOLVERS::MinValue, MinValue);
  REGISTER_COMPONENT_FACTORY(EUROPA::SOLVERS::UnboundVariableManager, UnboundVariableManager);
  
  REGISTER_OPENCONDITION_DECISION_FACTORY(EUROPA::SOLVERS::OpenConditionDecisionPoint, StandardOpenConditionHandler);
  REGISTER_COMPONENT_FACTORY(EUROPA::SOLVERS::OpenConditionManager, OpenConditionManager);
  
  REGISTER_THREAT_DECISION_FACTORY(EUROPA::SOLVERS::ThreatDecisionPoint, StandardThreatHandler);
  REGISTER_COMPONENT_FACTORY(EUROPA::SOLVERS::ThreatManager, ThreatManager);
  
  REGISTER_COMPONENT_FACTORY(EUROPA::SOLVERS::InfiniteDynamicFilter, InfiniteDynamicFilter);
  REGISTER_COMPONENT_FACTORY(EUROPA::SOLVERS::HorizonFilter, HorizonFilter);
  REGISTER_COMPONENT_FACTORY(EUROPA::SOLVERS::HorizonVariableFilter, HorizonVariableFilter);

  // Initialize Library  
  TestAssembly::initialize();

  // Allocate the schema with a call to the linked in model function - eventually
  // make this called via dlopen
  SchemaId schema = NDDL::loadSchema();

  // Enacpsualte allocation so that they go out of scope before calling terminate
  {  
    // Allocate the test assembly.
    TestAssembly assembly(schema);

    // Run the planner
    assembly.plan(txSource, plannerConfig);

    // Dump the results
    assembly.write(std::cout);
  }

  // Terminate the library
  TestAssembly::terminate();

  std::cout << "Finished\n";
}
