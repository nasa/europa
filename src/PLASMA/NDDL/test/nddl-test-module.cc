
#include "nddl-test-module.hh"
#include "NddlUtils.hh"
#include "TestSupport.hh"

using namespace EUROPA;
using namespace NDDL;

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

/*void NDDLModuleTests::runTests(std::string path) 
{
  runTestSuite(UtilitiesTest::test);
}*/

void NDDLModuleTests::utilitiesTests()
{
  UtilitiesTest::test();
}

