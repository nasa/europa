/**
 * @file Main.cc
 *
 * @brief Provides an executable for your project which will use a
 * standard Chronological backtracking planner and a Test Assembly of
 * EUROPA
 */

#include "Nddl.hh" /*!< Includes protypes required to load a model */
#include "SolverAssembly.hh" /*!< For using a test EUROPA Assembly */

//HACK to include constraint
#include "ConstraintLibrary.hh"
#include "Constraints.hh"


//for PLEXILPlanWriter
#include "PLEXILCLARATyPlanDatabaseWriter.hh"

using namespace EUROPA;

/**
 * @brief Uses the planner to solve a planning problem
 */
int main(int argc, const char ** argv){
  if (argc != 3) {
    std::cerr << "Must provide initial transactions file." << std::endl;
    return -1;
  }

  //HACK until constraint registration bug fixed
  REGISTER_CONSTRAINT (EUROPA::AbsoluteValue, "absVal", "Default");

  const char* txSource = argv[1];

  const char* plannerConfig = argv[2];

  //for PLEXILPlanWriter

  PLEXILCLARATyPlanDatabaseWriter p;

  // Initialize Library  
  SolverAssembly::initialize();

  // Allocate the schema with a call to the linked in model function - eventually
  // make this called via dlopen
  SchemaId schema = NDDL::loadSchema();

  // Enacpsualte allocation so that they go out of scope before calling terminate
  {  
    // Allocate the test assembly.
    SolverAssembly assembly(schema);

    // Run the planner
    assembly.plan(txSource, plannerConfig);

    // Dump the results
    //assembly.write(std::cout);
    PlanDatabaseId db;
    db = assembly.getPlanDatabase();
    TemporalAdvisorId t;
    t=db->getTemporalAdvisor(); 

    //assembly.write(std::cout);

    p.writePLEXILCLARATyPlan(db,std::cout);
  }

  // Terminate the library
  SolverAssembly::terminate();

  //std::cout << "Finished\n";
}
