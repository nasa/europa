#include "SAVH_ReusableFVDetector.hh"
#include "SAVH_Instant.hh"

#include <cmath>

namespace EUROPA {
  namespace SAVH {
    ReusableFVDetector::ReusableFVDetector(const ResourceId res) : FVDetector(res) {
      m_upperLimit = res->getUpperLimit();
      m_lowerLimit = res->getLowerLimit();
      m_maxInstConsumption = res->getMaxInstConsumption();
      m_maxCumulativeConsumption = res->getMaxConsumption();
    }

    ResourceProblem::Type ReusableFVDetector::getResourceProblem(const InstantId inst) const
    {
    	if (inst->getMaxCumulativeConsumption() > m_maxCumulativeConsumption)
    		return ResourceProblem::ConsumptionSumExceeded;
    	
    	if (inst->getMaxCumulativeProduction() > m_maxCumulativeConsumption)
    		return ResourceProblem::ProductionSumExceeded;
    	         
    	if (inst->getMaxInstantConsumption() > m_maxInstConsumption)
    		return ResourceProblem::ConsumptionRateExceeded;
    	
    	if (inst->getMaxInstantProduction() > m_maxInstConsumption)
    		return ResourceProblem::ProductionRateExceeded;
    	
    	if (inst->getUpperLevel() < m_lowerLimit)
    		return ResourceProblem::LevelTooLow;
    	
    	if (inst->getLowerLevel() > m_upperLimit)
    	    return ResourceProblem::LevelTooHigh;
    		
    	return ResourceProblem::NoProblem;
    }
    
    bool ReusableFVDetector::detect(const InstantId inst) {
      debugMsg("ReusableFVDetector:detect", "Detecting flaws and violations at time " << inst->getTime());
      debugMsg("ReusableFVDetector:detect", "Max cumulative consumption: " << m_maxCumulativeConsumption << 
               ".  At instant: " << inst->getMaxCumulativeConsumption() << ".");
      debugMsg("ReusableFVDetector:detect", "Max cumulative production: " << m_maxCumulativeConsumption << 
               ".  At instant: " << inst->getMaxCumulativeProduction() << ".");
      debugMsg("ReusableFVDetector:detect", "Max instantaneous consumption: " << m_maxInstConsumption << 
               ".  At instant: " << inst->getMaxInstantConsumption() << ".");
      debugMsg("ReusableFVDetector:detect", "Max instantaneous production: " << m_maxInstConsumption << 
               ".  At instant: " << inst->getMaxInstantProduction() << ".");
      debugMsg("ReusableFVDetector:detect", "Lower limit: " << m_lowerLimit << ".  Level: [" << inst->getLowerLevel() << " " << inst->getUpperLevel() << "]");
      debugMsg("ReusableFVDetector:detect", "Upper limit: " << m_upperLimit << ".  Level: [" << inst->getLowerLevel() << " " << inst->getUpperLevel() << "]");
      bool isFlawed = inst->isFlawed();
      bool wasViolated = inst->isViolated();
      inst->setViolated(false);

      ResourceProblem::Type problem = getResourceProblem(inst);
      
      if(problem != ResourceProblem::NoProblem) {
        debugMsg("ReusableFVDetector:detect", "Flagging violation at instant " << inst->getTime() << " : ");
        condDebugMsg(problem == ResourceProblem::ConsumptionSumExceeded, "ReusableFVDetector:detect", "Cumulative consumption violation.");
        condDebugMsg(problem == ResourceProblem::ProductionSumExceeded,  "ReusableFVDetector:detect", "Cumulative production violation.");
        condDebugMsg(problem == ResourceProblem::ConsumptionRateExceeded, "ReusableFVDetector:detect", "Instantaneous consumption violation.");
        condDebugMsg(problem == ResourceProblem::ProductionRateExceeded, "ReusableFVDetector:detect", "Instantaneous production violation.");
        condDebugMsg(problem == ResourceProblem::LevelTooLow, "ReusableFVDetector:detect", "Upper level below limit violation.");
        condDebugMsg(problem == ResourceProblem::LevelTooHigh, "ReusableFVDetector:detect", "Lower level above limit violation.");

        inst->setViolated(true);
        notifyOfViolation(inst,problem);
        
        if (allowViolations()) {
            debugMsg("ReusableFVDetector:detect", "Violations allowed, continuing detection");        	
        	return false;
        }
        else {
            debugMsg("ReusableFVDetector:detect", "Violations not allowed, stopping detection");        	
            return true;
        }
      }
      else if(inst->getLowerLevel() < m_lowerLimit || inst->getUpperLevel() > m_upperLimit) {
        inst->setFlawed(true);
        inst->setLower(inst->getLowerLevel() < m_lowerLimit);
        inst->setUpper(inst->getUpperLevel() > m_upperLimit);
        if(inst->hasLowerLevelFlaw())
          inst->setLowerMagnitude(fabs(m_lowerLimit - inst->getLowerLevel()));
        if(inst->hasUpperLevelFlaw())
          inst->setUpperMagnitude(fabs(m_upperLimit - inst->getUpperLevel()));
        debugMsg("ReusableFVDetector:detect", "Flagging flaw:");
        condDebugMsg(inst->getLowerLevel() < m_lowerLimit, "ReusableFVDetector:detect", "Lower limit flaw.");
        condDebugMsg(inst->getUpperLevel() > m_upperLimit, "ReusableFVDetector:detect", "Upper limit flaw.");
        notifyOfFlaw(inst);
      }
      else {
        inst->setFlawed(false);
        if(isFlawed) {
          debugMsg("ReusableFVDetector:detect", "Clearing flaw.");
          notifyNoLongerFlawed(inst);
        }
      }
      
      if (wasViolated) {
    	  notifyNoLongerViolated(inst);
          debugMsg("ReusableFVDetector:detect", "Clearing Violation");
      }
    	  
      return false;
    }
  }
}
