#ifndef _H_OpenWorldFVDetector
#define _H_OpenWorldFVDetector

#include "GenericFVDetector.hh"
namespace EUROPA {

    class OpenWorldFVDetector : public GenericFVDetector {
  public:
   OpenWorldFVDetector(const ResourceId res);
  protected:
   Resource::ProblemType getResourceLevelViolation(const InstantId inst) const;
    void handleResourceLevelFlaws(const InstantId inst);
    };
}

#endif
