#include <iostream>

#ifdef VOID
#undef VOID
#endif

#include "TestSupport.hh"

#include "anml-test-module.hh"
// Support for default setup
#include "ConstraintEngine.hh"
#include "PlanDatabase.hh"
#include "RulesEngine.hh"
#include "Schema.hh"
#include "DefaultPropagator.hh"
#include "Object.hh"
#include "Constraints.hh"
#include "ANMLParser.hpp"

using namespace EUROPA;

void ANMLModuleTests::runTests(std::string path) {
  initConstraintLibrary();
  setTestLoadLibraryPath(path);

  REGISTER_CONSTRAINT(EqualConstraint, "eq", "Default");

  std::cout << "Finished" << std::endl;
  ConstraintLibrary::purgeAll();
  uninitConstraintLibrary();
  }


