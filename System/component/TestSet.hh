#ifndef _H_TestSet
#define _H_TestSet

#include "Test.hh"
#include "PlanDatabaseDefs.hh"
#include <vector>

namespace PLASMA {
  class TestSet : public Test {
  public:
    TestSet(const PlanDatabaseId&); //no name because it isn't used here
    virtual ~TestSet();
    Test::Status result();
    void dumpResults();
  protected:
    std::vector<TestId> m_tests;
  private:
  };
}

#endif
