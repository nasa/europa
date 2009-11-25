#include "nddl-test-module.hh"
#include <stdlib.h>

#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>


CPPUNIT_TEST_SUITE_REGISTRATION( NDDLModuleTests );

int main( int argc, char **argv)
{
  CppUnit::TextUi::TestRunner runner;
  CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
  runner.addTest( registry.makeTest() );

  const char* baselineVar=getenv("SET_TEST_BASELINE");
  bool setBaseline = (baselineVar!=NULL && (std::string(baselineVar)=="1"));
  runner.addTest(new ErrorCheckingTests("ErrorCheckingTests.txt",setBaseline));
  return !runner.run("", false);
}

