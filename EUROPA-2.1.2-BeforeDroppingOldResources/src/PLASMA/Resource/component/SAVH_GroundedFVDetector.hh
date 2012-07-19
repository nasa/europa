#ifndef _H_SAVH_GroundedFVDetector
#define _H_SAVH_GroundedFVDetector

#include "SAVH_GenericFVDetector.hh"
namespace EUROPA {
  namespace SAVH {

    class GroundedFVDetector : public GenericFVDetector {
  public:
   GroundedFVDetector(const ResourceId res);

  protected:
   ResourceProblem::Type getResourceLevelViolation(const InstantId inst) const;
    void handleResourceLevelFlaws(const InstantId inst);
    };
  }
}

#endif
