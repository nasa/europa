#include "SAVH_ReusableFVDetector.hh"
#include "SAVH_Instant.hh"

namespace EUROPA {
  namespace SAVH {
    ReusableFVDetector::ReusableFVDetector(const ResourceId res) : FVDetector(res) {
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
      inst->setViolated(false);
      inst->setFlawed(false);
      if(inst->getMaxCumulativeConsumption() > m_maxCumulativeConsumption ||
	 inst->getMaxCumulativeProduction() > m_maxCumulativeConsumption ||
	 inst->getMaxInstantConsumption() > m_maxInstConsumption ||
	 inst->getMaxInstantProduction() > m_maxInstConsumption ||
	 inst->getUpperLevel() < m_lowerLimit) {

	debugMsg("ReusableFVDetector:detect", "Flagging violation:");
	condDebugMsg(inst->getMaxCumulativeConsumption() > m_maxCumulativeConsumption, "ReusableFVDetector:detect", "Cumulative consumption violation.");
	condDebugMsg(inst->getMaxCumulativeProduction() > m_maxCumulativeConsumption, "ReusableFVDetector:detect", "Cumulative production violation.");
	condDebugMsg(inst->getMaxInstantConsumption() > m_maxInstConsumption, "ReusableFVDetector:detect", "Instantaneous consumption violation.");
	condDebugMsg(inst->getMaxInstantProduction() > m_maxInstConsumption, "ReusableFVDetector:detect", "Instantaneous production violation.");
	condDebugMsg(inst->getUpperLevel() < m_lowerLimit, "ReusableFVDetector:detect", "Upper level below limit violation.");

	inst->setViolated(true);
	notifyOfViolation(inst);
      }
      else if(inst->getLowerLevel() < m_lowerLimit) {
	inst->setFlawed(true);
	debugMsg("ReusableFVDetector:detect", "Flagging flaw:");
	debugMsg("ReusableFVDetector:detect", "Lower limit flaw.");
	notifyOfFlaw(inst);
      }
      return false;
    }
  }
}
