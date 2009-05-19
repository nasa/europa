#ifndef H_NDDL_MODULE_TESTS
#define H_NDDL_MODULE_TESTS

#include <string>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>
#include "NddlTestEngine.hh"

class NDDLModuleTests : public CppUnit::TestFixture {
  CPPUNIT_TEST_SUITE(NDDLModuleTests);
  CPPUNIT_TEST(syntaxTests);
  CPPUNIT_TEST(utilitiesTests);
  CPPUNIT_TEST_SUITE_END();

public:
  inline void setUp()
  {
  }

  inline void tearDown()
  {
  }

  void syntaxTests();
  void utilitiesTests();
};

class NddlTest : public CppUnit::TestFixture
{
public:
    NddlTest(const std::string& testName,
             const std::string& nddlFile,
             const std::string& result,
             bool setBaseline);

    std::string& name() { return m_name; }

    void setUp();
    void tearDown();
    void run();

protected:
    std::string m_name;
    std::string m_nddlFile;
    std::string m_result;
    bool m_setBaseline;
    NddlTestEngine* m_engine;
};

class ErrorCheckingTests : public CppUnit::TestSuite
{
public:
    ErrorCheckingTests();

    static CppUnit::Test *suite(const std::string& testFilename,bool setBaseline);
    static std::vector<NddlTest*> readTests(const std::string& testFilename,bool setBaseline);
};

#endif /* H_NDDL_MODULE_TESTS */
