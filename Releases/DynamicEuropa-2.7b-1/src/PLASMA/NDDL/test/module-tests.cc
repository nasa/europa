#include "nddl-test-module.hh"
#include <stdlib.h>
#include "CppUnitUtils.hh"

CPPUNIT_TEST_SUITE_REGISTRATION( NDDLModuleTests );

int main( int argc, char **argv)
{
  CppUnit::TextUi::TestRunner runner;
  CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
  runner.addTest( registry.makeTest() );

  const char* baselineVar=getenv("SET_TEST_BASELINE");
  bool setBaseline = (baselineVar!=NULL && (std::string(baselineVar)=="1"));
  runner.addTest(new ErrorCheckingTests("ErrorCheckingTests.txt",setBaseline));

  // NOTE:  We can't use the standard RUN_CPP_UNIT_MODULE macro because of the above
  //        'addTest' special case.  However, the rest of the functionality is still
  //        available in this macro:
  RUN_CPP_UNIT_RUNNER(runner, true);
}

