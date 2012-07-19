#ifndef H_UTILS_MODULE_TESTS
#define H_UTILS_MODULE_TESTS

#include <string>
#include "CommonDefs.hh"

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class UtilModuleTests : public CppUnit::TestFixture {
  
  CPPUNIT_TEST_SUITE(UtilModuleTests);
  CPPUNIT_TEST(errorTests);
  CPPUNIT_TEST(debugTests);
  CPPUNIT_TEST(idTests);
  CPPUNIT_TEST(labelTests);
  CPPUNIT_TEST(entityTests);
  CPPUNIT_TEST(xmlTests);
  CPPUNIT_TEST(numberTests);
//   CPPUNIT_TEST(loggerTests);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    EUROPA::setTestLoadLibraryPath(".");
  }
  
  void errorTests();
  void debugTests();
  void idTests();
  void labelTests();
  void entityTests();
  void xmlTests();
  void numberTests();
//   void loggerTests();
};


#endif /* H_UTILS_MODULE_TESTS */
