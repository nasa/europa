#ifndef H_CE_MODULE_TESTS
#define H_CE_MODULE_TESTS

#include <string>

#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

class ConstraintEngineModuleTests : public CppUnit::TestFixture {

  CPPUNIT_TEST_SUITE(ConstraintEngineModuleTests);
  CPPUNIT_TEST(domainTests);
  CPPUNIT_TEST(typeFactoryTests);
  CPPUNIT_TEST(entityTests);
  CPPUNIT_TEST(constraintEngineTests);
  CPPUNIT_TEST(variableTests);
  CPPUNIT_TEST(constraintTests);
  CPPUNIT_TEST(constraintFactoryTests);
  CPPUNIT_TEST(equivalenceClassTests);
  CPPUNIT_TEST_SUITE_END();

public:
  void setUp()
  {
    ConstraintEngineModuleTests::cppSetup();
  }

  void tearDown()
  {
//    std::cout << "Finished" << std::endl;
  }

  void cppSetup(void);
  void domainTests();
  void typeFactoryTests();
  void entityTests();
  void constraintEngineTests();
  void variableTests();
  void constraintTests();
  void constraintFactoryTests();
  void equivalenceClassTests();
};

#endif /* H_CE_MODULE_TESTS */
