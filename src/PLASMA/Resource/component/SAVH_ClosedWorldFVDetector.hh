#ifndef _H_SAVH_ClosedWorldFVDetector
#define _H_SAVH_ClosedWorldFVDetector

#include "SAVH_GenericFVDetector.hh"
namespace EUROPA {
  namespace SAVH {
  
    class ClosedWorldFVDetector : public GenericFVDetector {
  protected:
   ResourceProblem::Type getResourceLevelViolation(const InstantId inst) const;        
    void handleResourceLevelFlaws(const InstantId inst);
    };
  }
}

#endif
