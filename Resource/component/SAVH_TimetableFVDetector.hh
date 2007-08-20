#ifndef _H_SAVH_TimetableFVDetector
#define _H_SAVH_TimetableFVDetector

#include "SAVH_FVDetector.hh"
namespace EUROPA {
  namespace SAVH {
    class TimetableFVDetector : public FVDetector {
    public:
      TimetableFVDetector(const ResourceId res);
      bool detect(const InstantId inst);
    protected:
      virtual ResourceProblem::Type getResourceProblem(const InstantId inst) const;    	
    	
    private:
      double m_lowerLimit, m_upperLimit, m_maxInstConsumption, m_maxInstProduction;
      double m_maxCumulativeConsumption, m_maxCumulativeProduction;
    };
  }
}

#endif
