#ifndef H_PLAN_DATABASE_MODULE_TESTS
#define H_PLAN_DATABASE_MODULE_TESTS

#include <string>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "SymbolDomain.hh"

using namespace EUROPA;

class PlanDatabaseModuleTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(PlanDatabaseModuleTests);
  CPPUNIT_TEST(schemaTest);
  CPPUNIT_TEST(objectTest);
  CPPUNIT_TEST(tokenTest);
  CPPUNIT_TEST(timelineTest);
  CPPUNIT_TEST(DBClientTest);
  CPPUNIT_TEST(DBTransPlayerTest);
  CPPUNIT_TEST_SUITE_END();

  public:
  void setUp()
  {
    cppSetup();
  }

  void tearDown()
  {
  }

  static void cppSetup(void);
  void schemaTest(void);
  void objectTest(void);
  void tokenTest(void);
  void timelineTest(void);
  void DBClientTest(void);
  void DBTransPlayerTest(void);
};

typedef SymbolDomain Locations;

const Locations& LocationsBaseDomain();


#endif /* H_PLAN_DATABAE_MODULE_TESTS */
