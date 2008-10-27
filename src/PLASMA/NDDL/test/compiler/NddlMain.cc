#include "NddlTestEngine.hh"

/**
 * @file Provides main execution program to run a test which plays transactions
 * on a database for a given model. The model must currently be linked to the executable
 * statically, but we will eventually get to dynamically linking to a model as an argument.
 */
int main(int argc, const char ** argv)
{
  if (argc != 2) {
    std::cerr << "Must provide initial transactions file." << std::endl;
    return -1;
  }

  NddlTestEngine engine;

  engine.init();

  const char* txSource = argv[1];
  engine.run(txSource);

  return 0;
}
