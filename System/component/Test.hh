#ifndef _Test_HH
#define _Test_HH

#include "Error.hh"
#include "Id.hh"

#define ADD_TEST(test) m_tests.push_back((new test(planDb))->getId())
#define ADD_ASSERTION(assertion) m_tests.push_back((new assertion(planDb))->getId())

namespace Prototype {
  class Test;
  typedef Id<Test> TestId;
  
  
  class TestErr {
  public:
    DECLARE_ERROR(IndeterminateStateError);
  };
  
  class Test {
  public:
    enum Status {
      INCOMPLETE = -1,
      FAILED,
      PASSED,
    };
    Test() : m_id(this) {};
    virtual ~Test() {m_id.remove();}
    virtual Status result(){return INCOMPLETE;};
    virtual void dumpResults(){};
    TestId getId(){return m_id;}
  protected:
  private:
    TestId m_id;
  };
}

#endif
