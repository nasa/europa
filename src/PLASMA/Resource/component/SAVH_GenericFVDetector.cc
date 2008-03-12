#include "SAVH_GenericFVDetector.hh"
#include "SAVH_Instant.hh"
#include "SAVH_FVDetector.hh"

namespace EUROPA {
  namespace SAVH {

    GenericFVDetector::GenericFVDetector(const ResourceId res) : FVDetector(res) {
      m_lowerLimit = res->getLowerLimit();
      m_upperLimit = res->getUpperLimit();
      m_maxInstConsumption = res->getMaxInstConsumption();
      m_maxInstProduction = res->getMaxInstProduction();
      m_maxCumulativeConsumption = res->getMaxConsumption();
      m_maxCumulativeProduction = res->getMaxProduction();
      debugMsg("GenericFVDetector:GenericFVDetector", "Created FVDetector for " << res->toString());
      debugMsg("GenericFVDetector:GenericFVDetector", "Got values: lower limit(" << m_lowerLimit << ") upper limit(" << 
	       m_upperLimit << ") max instantaneous consumption(" << m_maxInstConsumption << ") max instantaneous production(" <<
	       m_maxInstProduction << ") max consumption(" << m_maxCumulativeConsumption << ") max production(" << 
	       m_maxCumulativeProduction << ")");
    }

    ResourceProblem::Type GenericFVDetector::getResourceViolation(const InstantId inst) const
    {
    	if (inst->getMinCumulativeConsumption() > m_maxCumulativeConsumption)
    	{
    		debugMsg("GenericFVDetector:detect", "Cumulative consumption violation.");
    		return ResourceProblem::ConsumptionSumExceeded;
    	}
    	
    	if (inst->getMinCumulativeProduction() > m_maxCumulativeProduction)
    	{
    		debugMsg("GenericFVDetector:detect", "Cumulative production violation.");
    		return ResourceProblem::ProductionSumExceeded;
    	}
    	         
    	if (inst->getMinInstantConsumption() > m_maxInstConsumption)
    	{
    		debugMsg("GenericFVDetector:detect", "Instantaneous consumption violation.");
    		return ResourceProblem::ConsumptionRateExceeded;
    	}
    	
    	if (inst->getMinInstantProduction() > m_maxInstProduction)
    	{
    		debugMsg("GenericFVDetector:detect", "Instantaneous production violation.");
    		return ResourceProblem::ProductionRateExceeded;
    	}
    	return getResourceLevelViolation(inst);
    }
    /* Reporting these flaws is new, and is not fully supported.  To get full functionality, we need:
     * 
     * 1) To implement the ?? methods as is done for ??={Lower,Upper}and used in the handleResourceLevelFlaws subclass methods.
     *    The variables set in that case are used by the threat manager so that one can decide to prefer one type of flaw 
     *    over another, or to order flaws by magnitude.
     *
     * 2) To handle these flaws differently.  A flaw could be resolved by either bounding consumption/production variables OR
     *    by adding an action (I think) and current flaw handling will simply try rescheduling things, which could easily not 
     *    help.  We probably need a specialized DecisionPoint.
     * 
     * Alternatives to full functionality:
     * a) Ignore these flaws (currently we cannot filter flaws based on type, but we may in the future) 
     * b) Be conservative and report flaws as violations.
     * 
     */
    void GenericFVDetector::handleResourceFlaws(const InstantId inst)
    {
    	if (inst->getMaxCumulativeConsumption() > m_maxCumulativeConsumption)
    	{
    		inst->setFlawed(true);
//       		inst->set??(true);
//       		inst->set??Magnitude(fabs(m_maxCumulativeConsumption - inst->getMaxCumulativeConsumption()));
       		debugMsg("GenericFVDetector:detect", "Cumulative consumption flaw.");
    	}	
    	if (inst->getMaxCumulativeProduction() > m_maxCumulativeProduction)
    	{
    		inst->setFlawed(true);
//       		inst->set??(true);
//       		inst->set??Magnitude(fabs(m_maxCumulativeProduction - inst->getMaxCumulativeProduction()));
       		debugMsg("GenericFVDetector:detect", "Cumulative production flaw.");
    	}     
    	if (inst->getMaxInstantConsumption() > m_maxInstConsumption)
    	{
    		inst->setFlawed(true);
//       		inst->set??(true);
//       		inst->set??Magnitude(fabs(m_maxInstConsumption - inst->getMaxInstantConsumption()));
       		debugMsg("GenericFVDetector:detect", "Instantaneous consumption flaw.");
    	} 
    	if (inst->getMaxInstantProduction() > m_maxInstProduction)
    	{
    		inst->setFlawed(true);
//       		inst->set??(true);
//       		inst->set??Magnitude(fabs(m_maxInstProduction - inst->getMaxInstantProduction()));
       		debugMsg("GenericFVDetector:detect", "Instantaneous production flaw.");
    	}
    	handleResourceLevelFlaws(inst);
    }
    
    bool GenericFVDetector::detect(const InstantId inst) {
        debugMsg("GenericFVDetector:detect", "Detecting flaws and violations at time " << inst->getTime());
     	debugMsg("GenericFVDetector:detect", "Max cumulative consumption: " << m_maxCumulativeConsumption << 
    			".  At instant: " << inst->getMaxCumulativeConsumption() << ".");
    	debugMsg("GenericFVDetector:detect", "Max cumulative production: " << m_maxCumulativeConsumption << 
    			".  At instant: " << inst->getMaxCumulativeProduction() << ".");
    	debugMsg("GenericFVDetector:detect", "Max instantaneous consumption: " << m_maxInstConsumption << 
    			".  At instant: " << inst->getMaxInstantConsumption() << ".");
    	debugMsg("GenericFVDetector:detect", "Max instantaneous production: " << m_maxInstConsumption << 
    			".  At instant: " << inst->getMaxInstantProduction() << ".");
    	
    	// A: Look for violations (note that only the first one found is returned)
   	 	bool wasViolated = inst->isViolated();
    	inst->setViolated(false);
     	
    	ResourceProblem::Type violation = getResourceViolation(inst);
    	
    	if(violation != ResourceProblem::NoProblem) {
    		debugMsg("GenericFVDetector:detect", "Flagging violation at instant " << inst->getTime() << " : ");
    		inst->setViolated(true);
    		notifyOfViolation(inst, violation);
    	}
    	else if(wasViolated)
    	{
    		notifyNoLongerViolated(inst);
    		debugMsg("GenericFVDetector:detect", "Clearing Violation");	
    	}

    	// B:  Look for flaws (multiple flaws might be found)
    	bool wasFlawed = inst->isFlawed();
    	inst->setFlawed(false); // new
    	
    	handleResourceFlaws(inst);
    	
    	if(inst->isFlawed())
    	{
    		debugMsg("GenericFVDetector:detect", "Flagging (at least one) flaw:");
    		notifyOfFlaw(inst);
    	}
    	else if(wasFlawed)
    	{
    		debugMsg("GenericFVDetector:detect", "Clearing flaw.");
    		notifyNoLongerFlawed(inst);    	
    	}
    	
    	// Return true if there was a violation and we're not allowing violations:
    	return(inst->isViolated() && !allowViolations());
    }
  }
}
