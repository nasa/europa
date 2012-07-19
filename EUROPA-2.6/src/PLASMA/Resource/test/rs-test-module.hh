#ifndef H_RESOURCE_MODULE_TESTS
#define H_RESOURCE_MODULE_TESTS

#include <string>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class ResourceModuleTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(ResourceModuleTests);
  CPPUNIT_TEST(defaultSetupTests);
  CPPUNIT_TEST(profileTests);
  CPPUNIT_TEST(ResourceTests);
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

  void cppSetup(void);
  void defaultSetupTests(void);
  void profileTests(void);
  void ResourceTests(void);
  void ResourceSolverTests(void);
};

#endif /* H_RESOURCE_MODULE_TESTS */
