#include "db-test-module.hh"
#include <string>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

CPPUNIT_TEST_SUITE_REGISTRATION( PlanDatabaseModuleTests );

int main( int argc, char **argv)
{
  CppUnit::TextUi::TestRunner runner;
  CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
  runner.addTest( registry.makeTest() );
  return !runner.run("", false);
}
