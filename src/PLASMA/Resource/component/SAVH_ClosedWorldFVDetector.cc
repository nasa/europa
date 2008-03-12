#include "SAVH_ClosedWorldFVDetector.hh"

#include <cmath>

namespace EUROPA {
namespace SAVH {

ClosedWorldFVDetector::ClosedWorldFVDetector(const ResourceId res) : GenericFVDetector(res) {
}

ResourceProblem::Type ClosedWorldFVDetector::getResourceLevelViolation(const InstantId inst) const
{
	if (inst->getUpperLevel() < m_lowerLimit)
	{
		debugMsg("ClosedWorldFVDetector:detect", 
				"Lower limit violation.  Limit: " << m_lowerLimit << " Upper level: " << inst->getUpperLevel());
		return ResourceProblem::LevelTooLow;
	}
	if (inst->getLowerLevel() > m_upperLimit)
	{
    	debugMsg("ClosedWorldFVDetector:detect", 
    			"Upper limit violation.  Limit: " << m_upperLimit << " Lower level: " << inst->getLowerLevel());
    	return ResourceProblem::LevelTooHigh;
	}
	return ResourceProblem::NoProblem;
}

void ClosedWorldFVDetector::handleResourceLevelFlaws(const InstantId inst)
{
	if(inst->getLowerLevel() < m_lowerLimit)
	{
		inst->setFlawed(true);
		inst->setLower(true);
		inst->setLowerMagnitude(fabs(m_lowerLimit - inst->getLowerLevel()));
		debugMsg("ClosedWorldFVDetector:detect", "Lower limit flaw.");
	}

	if(inst->getUpperLevel() > m_upperLimit) 
	{
		inst->setFlawed(true);	
		inst->setUpper(true);
		inst->setUpperMagnitude(fabs(m_upperLimit - inst->getUpperLevel()));
		debugMsg("ClosedWorldFVDetector:detect", "Upper limit flaw.");
	}
}

}
}
