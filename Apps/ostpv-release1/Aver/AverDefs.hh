#ifndef _H_AverDefs
#define _H_AverDefs

#include "Error.hh"
#include "Id.hh"

namespace EUROPA {
  class Assertion;
  typedef Id<Assertion> AssertionId;
  
  class AssertionExecutor;
  typedef Id<AssertionExecutor> AssertionExecutorId;
  
  class Test;
  typedef Id<Test> TestId;

  class AverInterp;
  typedef Id<AverInterp> AverInterpId;

  class AverErr {
  public:
    DECLARE_ERROR(XmlError);
    DECLARE_ERROR(FlowError);
    DECLARE_ERROR(AssertionFailedError);
    DECLARE_ERROR(ExecutionError);
  };

}

#endif
