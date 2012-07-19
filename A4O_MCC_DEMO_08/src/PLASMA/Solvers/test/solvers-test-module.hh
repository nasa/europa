#ifndef H_SOLVERS_MODULE_TESTS
#define H_SOLVERS_MODULE_TESTS

#include <string>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class SolversModuleTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(SolversModuleTests);
  CPPUNIT_TEST(componentFactoryTests);
  CPPUNIT_TEST(filterTests);
  CPPUNIT_TEST(flawIteratorTests);
  CPPUNIT_TEST(flawManagerTests);
  CPPUNIT_TEST(flawHandlerTests);
  CPPUNIT_TEST(solverTests);
  CPPUNIT_TEST_SUITE_END();
  
public:
  inline void setUp()
  {
    cppTests();
  }

  inline void tearDown() 
  { 
  }
  
  void cppTests();
  void componentFactoryTests();
  void filterTests();
  void flawIteratorTests();
  void flawManagerTests();
  void flawHandlerTests();
  void solverTests();
  
};


#endif /* H_SOLVERS_MODULE_TESTS */
