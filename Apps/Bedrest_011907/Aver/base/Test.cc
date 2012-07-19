#include "Test.hh"
#include "Assertion.hh"

namespace EUROPA {
  Test::Test(const std::string& name) : m_name(name), m_id(this) { }
  
  Test::~Test() {
    m_id.remove();
  }

  void Test::dumpResults(std::ostream& os) const {
    os << "Results for '" << m_name << "': " << std::endl;
    for(std::list<AssertionId>::const_iterator it = m_assertions.begin();
        it != m_assertions.end(); ++it) {
      AssertionId assn = *it;
      assn->dumpResults(os);
    }
    for(std::list<TestId>::const_iterator it = m_tests.begin();
        it != m_tests.end(); ++it) {
      TestId test = *it;
      test->dumpResults(os);
    }
  }
}
