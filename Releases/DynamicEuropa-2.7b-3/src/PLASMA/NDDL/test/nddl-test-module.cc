
#include "nddl-test-module.hh"
#include <fstream>
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
    std::string result = engine.executeScript("nddl",filename,true /*isFile*/);
    CPPUNIT_ASSERT_MESSAGE("Nddl3 parser reported problems :\n" + result,result.size() == 0);
}

class UtilitiesTest {
public:
  static bool test() {
    std::string tokenizedString("A$B$C$D$");
    const std::list<edouble>& tokens = listFromString(tokenizedString, false);
    CPPUNIT_ASSERT(tokens.size() == 4);
    std::string newString;
    for(std::list<edouble>::const_iterator it = tokens.begin(); it != tokens.end(); ++it){
      LabelStr value(*it);
      newString += value.toString() + "$";
    }
    CPPUNIT_ASSERT(newString == tokenizedString);

    std::string numberStr("1$2.45$3.04$-8.9$");
    const std::list<edouble>& numbers= listFromString(numberStr, true);
    edouble sum = 0;

    for(std::list<edouble>::const_iterator it = numbers.begin(); it != numbers.end(); ++it){
      edouble number = *it;
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
    std::string result = m_engine->executeScript("nddl",m_nddlFile,true /*isFile*/);

    if (m_setBaseline) {
        m_result = result;
    }
    else {
        // TODO: save diff to file?
        CPPUNIT_ASSERT_MESSAGE(
                "Expected:\n"+m_result+"\n"+
                "Obtained:\n"+result+"\n",
                m_result == result);
    }
}

std::string NddlTest::toString() const
{
    std::ostringstream os;

    os << m_name << std::endl
       << m_nddlFile << std::endl
       << "{{{" << std::endl
       << m_result << std::endl
       << "}}}" << std::endl;

    return os.str();
}


std::string readLine(std::ifstream& inFile)
{
    std::string retval;
    while(!inFile.eof()) {
        getline(inFile,retval);
        if (retval.size()>0)
            return retval;
    }

    return retval;
}

void match(std::ifstream& inFile, const std::string& element)
{
    std::string result = readLine(inFile);
    if (result != element)
        throw std::string("Expected '"+element+"' read '"+result+"'");
}

std::string readTestName(std::ifstream& inFile)
{
    return readLine(inFile);
}

std::string readTestFilename(std::ifstream& inFile)
{
    std::string retval = readLine(inFile);

    if (retval.size()==0)
        throw std::string("Expected test filename");

    return retval;
}

std::string readTestResult(std::ifstream& inFile)
{
    std::string retval;
    match(inFile,"{{{");
    std::string line;
    while (!inFile.eof() && (line=readLine(inFile))!="}}}")
        retval += line + "\n";

    if (line != "}}}")
        throw std::string("Expected }}}");

    return retval;
}

ErrorCheckingTests::ErrorCheckingTests(const std::string& testFilename,bool setBaseline)
    : CppUnit::TestSuite()
    , m_testFilename(testFilename)
    , m_setBaseline(setBaseline)
{
    readTests();

    for (unsigned int i=0;i<m_tests.size();i++)
        addTest(
            new CppUnit::TestCaller<NddlTest>(
                m_tests[i]->name(),
                &NddlTest::run,
                m_tests[i]
            )
        );
}

void ErrorCheckingTests::run(CppUnit::TestResult *result)
{
    CppUnit::TestSuite::run(result);

    if (m_setBaseline) {
        // Write results back
        std::ofstream outFile(m_testFilename.c_str());
        if (!outFile)
            throw std::string("Unable to open "+m_testFilename);

        for (unsigned int i=0;i<m_tests.size();i++)
            outFile << m_tests[i]->toString() << std::endl;

        outFile.close();
    }
}

void ErrorCheckingTests::readTests()
{
    std::ifstream inFile(m_testFilename.c_str());

    if (!inFile)
        throw std::string("Unable to open "+m_testFilename);

    std::string line;
    while (!inFile.eof()) {
        std::string name = readTestName(inFile);
        if (name.size()==0)
            break;
        std::string filename = readTestFilename(inFile);
        std::string result = readTestResult(inFile);
        m_tests.push_back(new NddlTest(name,filename,result,m_setBaseline));
    }
    inFile.close();
}
