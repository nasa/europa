#include "NddlTestEngine.hh"

/**
 * @file Provides main execution program to run a test which loads a model and dumps the resulting database
 */
int main(int argc, const char ** argv)
{
  if (argc != 3) {
    std::cerr << "Must provide initial transactions file and language to interpret" << std::endl;
    return -1;
  }

  NddlTestEngine engine;

  engine.init();

  const char* txSource = argv[1];
  std::string language = argv[2];
  engine.run(txSource,language);

  return 0;
}
