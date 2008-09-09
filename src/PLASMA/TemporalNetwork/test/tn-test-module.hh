#ifndef H_TEMPORAL_NETWORK_MODULE_TESTS
#define H_TEMPORAL_NETWORK_MODULE_TESTS

#include <string>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class TemporalNetworkModuleTests : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(TemporalNetworkModuleTests);
  CPPUNIT_TEST(temporalNetworkTests);
  CPPUNIT_TEST(temporalNetworkConstraintEngineOnlyTests);
  CPPUNIT_TEST(temporalPropagatorTests);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    TemporalNetworkModuleTests::cppSetup();
  }

  void tearDown() 
  {
  }

  static void runTests(std::string path);
  void cppSetup();
  void temporalNetworkTests();
  void temporalNetworkConstraintEngineOnlyTests();
  void temporalPropagatorTests();
};

#endif /* H_PLAN_DATABAE_MODULE_TESTS */
