
#include "nddl-test-module.hh"
#include "NddlUtils.hh"
#include "NddlTestEngine.hh"
#include "Utils.hh"
#include "ModuleConstraintEngine.hh"
#include "ModulePlanDatabase.hh"
#include "ModuleTemporalNetwork.hh"
#include "ModuleRulesEngine.hh"
#include "ModuleNddl.hh"

using namespace EUROPA;
using namespace NDDL;

void NDDLModuleTests::syntaxTests()
{
    std::string filename="parser.nddl";
    NddlTestEngine engine;
    engine.init();
    std::string result = engine.executeScript("nddl3",filename,true /*isFile*/);
    CPPUNIT_ASSERT_MESSAGE("Nddl3 parser reported problems :\n" + result,result.size() == 0);
}

class UtilitiesTest {
public:
  static bool test() {
    std::string tokenizedString("A$B$C$D$");
    const std::list<double>& tokens = listFromString(tokenizedString, false);
    CPPUNIT_ASSERT(tokens.size() == 4);
    std::string newString;
    for(std::list<double>::const_iterator it = tokens.begin(); it != tokens.end(); ++it){
      LabelStr value(*it);
      newString += value.toString() + "$";
    }
    CPPUNIT_ASSERT(newString == tokenizedString);

    std::string numberStr("1$2.45$3.04$-8.9$");
    const std::list<double>& numbers= listFromString(numberStr, true);
    double sum = 0;

    for(std::list<double>::const_iterator it = numbers.begin(); it != numbers.end(); ++it){
      double number = *it;
      sum += number;
    }
    CPPUNIT_ASSERT(numbers.size() == 4);
    CPPUNIT_ASSERT(sum == (1 + 2.45 + 3.04 - 8.9));
    return true;
  }
};

void NDDLModuleTests::utilitiesTests()
{
  UtilitiesTest::test();
}


NddlTest::NddlTest(const std::string& testName,
                   const std::string& nddlFile,
                   const std::string& result,
                   bool setBaseline)
    : m_name(testName)
    , m_nddlFile(nddlFile)
    , m_result(result)
    , m_setBaseline(setBaseline)
{
}

void NddlTest::setUp()
{
    m_engine = new NddlTestEngine();
    m_engine->init();
}

void NddlTest::tearDown()
{
    delete m_engine;
}

void NddlTest::run()
{
    std::string result = m_engine->executeScript("nddl3",m_nddlFile,true /*isFile*/);

    if (m_setBaseline) {
        // TODO: where to save result?
    }
    else {
        // TODO: save diff
        CPPUNIT_ASSERT_MESSAGE(
                "Expected:"+m_result+"\n"+
                "Obtained:"+result+"\n",
                m_result == result);
    }
}

CppUnit::Test* ErrorCheckingTests::suite(const std::string& testFilename,bool setBaseline)
{
  CppUnit::TestSuite* s = new CppUnit::TestSuite( "ErrorCheckingTests" );

  std::vector<NddlTest*> tests = readTests(testFilename,setBaseline);

  for (unsigned int i=0;i<tests.size();i++)
      s->addTest(
          new CppUnit::TestCaller<NddlTest>(
              tests[i]->name(),
              &NddlTest::run,
              tests[i]
          )
      );

  return s;
}

std::vector<NddlTest*> ErrorCheckingTests::readTests(const std::string& testFilename,bool setBaseline)
{
    std::vector<NddlTest*> retval;

    // TODO: read from file
    retval.push_back(new NddlTest("Test1","parser.nddl","",setBaseline));
    retval.push_back(new NddlTest("Test2","parser.nddl","",setBaseline));

    return retval;
}
