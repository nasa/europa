// Include prototypes required to integrate to the NDDL generated model
#include "Nddl.hh"
#include "StandardAssembly.hh"

using namespace Prototype;

/**
 * @file Provides main execution program to run a test which plays transactions
 * on a database for a given model. The model must currently be linked to the executable
 * statically, but we will eventually get to dynamically linking to a model as an argumnet.
 * @author Conor McGann
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
  SchemaId schema = NDDL::schema();
  {  
    // Allocate the assembly
    StandardAssembly assembly(schema);
    assert(assembly.playTransactions(txSource));
  }

  // Terminate the library
  StandardAssembly::terminate();

  std::cout << "Finished\n";
}
