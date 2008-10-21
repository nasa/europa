#ifndef H_RESOURCE_MODULE_TESTS
#define H_RESOURCE_MODULE_TESTS

#include <string>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class ResourceModuleTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ResourceModuleTests);
  CPPUNIT_TEST(defaultSetupTests);
  CPPUNIT_TEST(resourceTests);
  CPPUNIT_TEST(profileTests);
  CPPUNIT_TEST(SAVHResourceTests);
  CPPUNIT_TEST(ResourceSolverTests);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    ResourceModuleTests::cppSetup();
  }

  void tearDown()
  {
  }

//  static void runTests(std::string path);
  void cppSetup(void);
  void defaultSetupTests(void);
  void resourceTests(void);
  void profileTests(void);
  void SAVHResourceTests(void);
  void ResourceSolverTests(void);
};

#endif /* H_RESOURCE_MODULE_TESTS */
