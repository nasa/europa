#ifndef _H_SAVH_ReusableFVDetector
#define _H_SAVH_ReusableFVDetector

#include "SAVH_FVDetector.hh"

namespace EUROPA {
  namespace SAVH {
    class ReusableFVDetector : public FVDetector {
    public:
      ReusableFVDetector(const ResourceId res);
      void initialize(const InstantId inst) {}
      void initialize() {}
      bool detect(const InstantId inst);
    protected:
    private:
      double m_upperLimit, m_lowerLimit, m_maxInstConsumption, m_maxCumulativeConsumption;
    };
  }
}

#endif
