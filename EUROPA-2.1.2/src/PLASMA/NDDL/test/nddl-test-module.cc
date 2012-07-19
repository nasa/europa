
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
    assertTrue(tokens.size() == 4);
    std::string newString;
    for(std::list<double>::const_iterator it = tokens.begin(); it != tokens.end(); ++it){
      LabelStr value(*it);
      newString += value.toString() + "$";
    }
    assertTrue(newString == tokenizedString);

    std::string numberStr("1$2.45$3.04$-8.9$");
    const std::list<double>& numbers= listFromString(numberStr, true);
    double sum = 0; 

    for(std::list<double>::const_iterator it = numbers.begin(); it != numbers.end(); ++it){
      double number = *it;
      sum += number;
    }
    assertTrue(numbers.size() == 4);
    assertTrue(sum == (1 + 2.45 + 3.04 - 8.9));    
    return true;
  }
};

void NDDLModuleTests::runTests(std::string path) 
{
  runTestSuite(UtilitiesTest::test);
}


