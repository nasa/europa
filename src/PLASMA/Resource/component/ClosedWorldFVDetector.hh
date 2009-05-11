#ifndef _H_ClosedWorldFVDetector
#define _H_ClosedWorldFVDetector

#include "GenericFVDetector.hh"
namespace EUROPA {

    class ClosedWorldFVDetector : public GenericFVDetector {
  public:
   ClosedWorldFVDetector(const ResourceId res);

  protected:
   Resource::ProblemType getResourceLevelViolation(const InstantId inst) const;
    void handleResourceLevelFlaws(const InstantId inst);
    };
}

#endif
