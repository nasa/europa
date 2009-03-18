#include "rs-test-module.hh"
#include "rs-flow-test-module.hh"
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>

CPPUNIT_TEST_SUITE_REGISTRATION( ResourceModuleTests );
CPPUNIT_TEST_SUITE_REGISTRATION( FlowProfileModuleTests );

int main(int argc, const char** argv) {
  CppUnit::TextUi::TestRunner runner;
  CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
  runner.addTest( registry.makeTest() );
  return !runner.run("", false);
}
