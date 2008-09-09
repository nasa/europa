#ifndef H_FLOW_PROFILE_MODULE_TESTS
#define H_FLOW_PROFILE_MODULE_TESTS

#include <string>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class FlowProfileModuleTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(FlowProfileModuleTests);
  CPPUNIT_TEST(defaultSetupTests);
  CPPUNIT_TEST(flowProfileTests);
  CPPUNIT_TEST(FVDetectorTests);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    FlowProfileModuleTests::cppSetup();
  }

  void tearDown() 
  {
    std::cout << "Finished" << std::endl;
  }

  static void runTests(std::string path);
  void cppSetup(void);
  void defaultSetupTests(void);
  void flowProfileTests(void);
  void FVDetectorTests(void);
};

#endif //H_FLOW_PROFILE_MODULE_TESTS
