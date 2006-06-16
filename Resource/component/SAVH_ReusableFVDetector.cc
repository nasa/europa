#include "SAVH_ReusableFVDetector.hh"
#include "SAVH_Instant.hh"

namespace EUROPA {
  namespace SAVH {
    ReusableFVDetector::ReusableFVDetector(const ResourceId res) : FVDetector(res) {
      m_upperLimit = res->getUpperLimit();
      m_lowerLimit = res->getLowerLimit();
      m_maxInstConsumption = res->getMaxInstConsumption();
      m_maxCumulativeConsumption = res->getMaxConsumption();
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
      bool isFlawed = inst->isFlawed();
      inst->setViolated(false);

      if(inst->getMaxCumulativeConsumption() > m_maxCumulativeConsumption ||
	 inst->getMaxCumulativeProduction() > m_maxCumulativeConsumption ||
	 inst->getMaxInstantConsumption() > m_maxInstConsumption ||
	 inst->getMaxInstantProduction() > m_maxInstConsumption ||
	 inst->getUpperLevel() < m_lowerLimit ||
	 inst->getLowerLevel() > m_upperLimit) {

	debugMsg("ReusableFVDetector:detect", "Flagging violation:");
	condDebugMsg(inst->getMaxCumulativeConsumption() > m_maxCumulativeConsumption, "ReusableFVDetector:detect", "Cumulative consumption violation.");
	condDebugMsg(inst->getMaxCumulativeProduction() > m_maxCumulativeConsumption, "ReusableFVDetector:detect", "Cumulative production violation.");
	condDebugMsg(inst->getMaxInstantConsumption() > m_maxInstConsumption, "ReusableFVDetector:detect", "Instantaneous consumption violation.");
	condDebugMsg(inst->getMaxInstantProduction() > m_maxInstConsumption, "ReusableFVDetector:detect", "Instantaneous production violation.");
	condDebugMsg(inst->getUpperLevel() < m_lowerLimit, "ReusableFVDetector:detect", "Upper level below limit violation.");
	condDebugMsg(inst->getLowerLevel() > m_upperLimit, "ReusableFVDetector:detect", "Lower level above limit violation.");

	inst->setViolated(true);
	notifyOfViolation(inst);
      }
      else if(inst->getLowerLevel() < m_lowerLimit || inst->getUpperLevel() > m_upperLimit) {
	inst->setFlawed(true);
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
      return false;
    }
  }
}
