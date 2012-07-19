#include "solvers-test-module.hh"
#include "DataTypes.hh"
#include "CppUnitUtils.hh"
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

    RUN_CPP_UNIT_MODULE(true);
}

