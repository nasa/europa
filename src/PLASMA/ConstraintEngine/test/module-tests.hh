#ifndef _H_ModuleTests
#define _H_ModuleTests

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class DomainTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(DomainTests);
  CPPUNIT_TEST(cppTests);
  CPPUNIT_TEST_SUITE_END();
  
public:
  inline void setUp()
  {
  }

  inline void tearDown() 
  { 
  }
  
  void cppTests();

  static bool test();
};

#endif
