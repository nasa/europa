#include "TestSet.hh"

namespace Prototype {
  TestSet::TestSet(const PlanDatabaseId&) : Test::Test() {}

  TestSet::~TestSet() {
    for(std::vector<TestId>::iterator it = m_tests.begin(); it != m_tests.end(); ++it)
      delete (Test*)(*it);
  }

  Test::Status TestSet::result() {
    Test::Status status = Test::INCOMPLETE;
    unsigned int passed = 0;
    bool incomplete = false;
    for(std::vector<TestId>::iterator it = m_tests.begin();
        it != m_tests.end(); ++it) {
      status = (*it)->result();
      if(status == Test::FAILED)
        return status;
      else if(status == Test::INCOMPLETE)
        incomplete = true;
      else if(status == Test::PASSED)
        passed++;
    }
    if(incomplete)
      return Test::INCOMPLETE;
    if(passed == m_tests.size())
      return Test::PASSED;
    handle_error(false, "Test system in bizarre state",
                TestErr::IndeterminateStateError());
    return Test::FAILED;
  }

  void TestSet::dumpResults() {
    for(std::vector<TestId>::iterator it = m_tests.begin(); it != m_tests.end();
        ++it)
      (*it)->dumpResults();
  }
}
