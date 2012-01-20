#include "rs-test-module.hh"
#include "rs-flow-test-module.hh"
#include "CppUnitUtils.hh"
#include "DataTypes.hh"
using namespace EUROPA;

CPPUNIT_TEST_SUITE_REGISTRATION( ResourceModuleTests );
CPPUNIT_TEST_SUITE_REGISTRATION( FlowProfileModuleTests );

int main(int argc, const char** argv)
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
