#include "db-test-module.hh"
#include <string>
#include "DataTypes.hh"
#include "CppUnitUtils.hh"

CPPUNIT_TEST_SUITE_REGISTRATION( PlanDatabaseModuleTests );

int main( int argc, char **argv)
{
    // Init data types so that id counts don't fail
    VoidDT::instance();
    BoolDT::instance();
    IntDT::instance();
    FloatDT::instance();
    StringDT::instance();
    SymbolDT::instance();
    LocationsBaseDomain();

    RUN_CPP_UNIT_MODULE(true);
}
