/**
 * @file Main.cc
 *
 * @brief Provides an executable for your project which will use a
 * standard Chronological backtracking planner and a Standard Assembly of
 * PLASMA
 */

#include "Nddl.hh" /*!< Includes protypes required to load a model */
#include "StandardAssembly.hh" /*!< For using a standard PLASMA Assembly */

using namespace PLASMA;

/**
 * @brief Uses the planner to solve a planning problem
 */
int main(int argc, const char ** argv){
  if (argc != 2) {
    std::cerr << "Must provide initial transactions file." << std::endl;
    return -1;
  }

  const char* txSource = argv[1];

  // Initialize Library  
  StandardAssembly::initialize();

  // Allocate the schema with a call to the linked in model function - eventually
  // make this called via dlopen
  SchemaId schema = NDDL::loadSchema();

  // Enacpsualte allocation so that they go out of scope before calling terminate
  {  
    // Allocate the standard assembly.
    StandardAssembly assembly(schema);

    // Run the planner
    assembly.plan(txSource);

    // Dump the results
    assembly.write(std::cout);
  }

  // Terminate the library
  StandardAssembly::terminate();

  std::cout << "Finished\n";
}
