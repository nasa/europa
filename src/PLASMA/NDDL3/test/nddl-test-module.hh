#ifndef H_NDDL_MODULE_TESTS
#define H_NDDL_MODULE_TESTS

#include <string>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class NDDLModuleTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(NDDLModuleTests);
  CPPUNIT_TEST(syntaxTests);
  CPPUNIT_TEST_SUITE_END();

public:
  inline void setUp()
  {
  }

  inline void tearDown()
  {
  }

  void syntaxTests();
};

#endif /* H_NDDL_MODULE_TESTS */
