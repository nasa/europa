#include "nddl-test-module.hh"

#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

CPPUNIT_TEST_SUITE_REGISTRATION( NDDLModuleTests );

int main( int argc, char **argv)
{
  CppUnit::TextUi::TestRunner runner;
  CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
  runner.addTest( registry.makeTest() );

  bool setBaseline = false; // TODO: get from argv
  runner.addTest( ErrorCheckingTests::suite("ErrorCheckingTests.txt",setBaseline) );
  return !runner.run("", false);
}

