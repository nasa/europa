#include "NddlTestEngine.hh"

/**
 * @file Provides main execution program to run a test which loads a model and dumps the resulting database
 */
int main(int argc, const char** argv)
{
  NddlTestEngine engine;
  return engine.run(argc,argv);
}
