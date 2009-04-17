#include "solvers-test-module.hh"

#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/extensions/TestFactoryRegistry.h>
#include "DataTypes.hh"
using namespace EUROPA;

CPPUNIT_TEST_SUITE_REGISTRATION( SolversModuleTests );

int main( int argc, char **argv)
{
    // Init data types so that id counts don't fail
    VoidDT::instance();
    BoolDT::instance();
    IntDT::instance();
    FloatDT::instance();
    StringDT::instance();
    SymbolDT::instance();

    CppUnit::TextUi::TestRunner runner;
    CppUnit::TestFactoryRegistry &registry = CppUnit::TestFactoryRegistry::getRegistry();
    runner.addTest( registry.makeTest() );
    return !runner.run("", false);
}

