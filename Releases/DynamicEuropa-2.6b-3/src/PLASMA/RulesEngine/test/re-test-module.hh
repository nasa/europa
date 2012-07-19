#ifndef H_RE_MODULE_TESTS
#define H_RE_MODULE_TESTS

#include <string>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class RulesEngineModuleTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(RulesEngineModuleTests);
  CPPUNIT_TEST(rulesEngineTests);
  CPPUNIT_TEST_SUITE_END();
  
public:
  inline void setUp()
  {
    cppTest();
  }

  inline void tearDown() 
  { 
  }
  
  void cppTest();
  void rulesEngineTests();
};

#endif /* H_RE_MODULE_TESTS */
