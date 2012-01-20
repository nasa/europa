#include "util-test-module.hh"
#include "CppUnitUtils.hh"

CPPUNIT_TEST_SUITE_REGISTRATION( UtilModuleTests );

int main( int argc, char **argv)
{
    RUN_CPP_UNIT_MODULE(true);
}
