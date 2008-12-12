#include "NddlTestEngine.hh"

/**
 * @file Provides main execution program to run a test which plays transactions
 * on a database for a given model. The model must currently be linked to the executable
 * statically, but we will eventually get to dynamically linking to a model as an argument.
 */
int main(int argc, const char ** argv)
{
  if (argc != 3) {
    std::cerr << "Must provide initial transactions file and flag to indicate whether to use interpreter" << std::endl;
    return -1;
  }

  NddlTestEngine engine;

  engine.init();

  const char* txSource = argv[1];
  std::string useInterpreter = argv[2];
  engine.run(txSource,useInterpreter == "1");

  return 0;
}
