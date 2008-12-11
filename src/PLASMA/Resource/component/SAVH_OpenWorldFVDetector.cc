#include "SAVH_OpenWorldFVDetector.hh"

#include <cmath>

namespace EUROPA {
namespace SAVH {

OpenWorldFVDetector::OpenWorldFVDetector(const ResourceId res) : GenericFVDetector(res) {
}

ResourceProblem::Type OpenWorldFVDetector::getResourceLevelViolation(const InstantId inst) const
{
	if (inst->getUpperLevel() + (m_maxCumulativeProduction - inst->getMinCumulativeProduction()) < m_lowerLimit)
	{
		debugMsg("OpenWorldFVDetector:detect", "Lower limit violation.  Limit: " << m_lowerLimit << " Upper level: " << inst->getUpperLevel() <<
				" Maximum remaining production: " << (m_maxCumulativeProduction - inst->getMinCumulativeProduction()));
		return ResourceProblem::LevelTooLow;
	}
	if (inst->getLowerLevel() - (m_maxCumulativeConsumption - inst->getMinCumulativeConsumption()) > m_upperLimit)
	{
		debugMsg("OpenWorldFVDetector:detect", "Upper limit violation.  Limit: " << m_upperLimit << " Lower level: " << inst->getLowerLevel() <<
				" Maxumum remaining consumption: " << (m_maxCumulativeConsumption - inst->getMinCumulativeConsumption()));
    	return ResourceProblem::LevelTooHigh;
	}
	return ResourceProblem::NoProblem;
}

void OpenWorldFVDetector::handleResourceLevelFlaws(const InstantId inst)
{
	if(inst->getLowerLevelMax() < m_lowerLimit) 
	{
		inst->setFlawed(true);
		inst->setLower(true);
		inst->setLowerMagnitude(std::abs(m_lowerLimit - inst->getLowerLevelMax()));
		debugMsg("OpenWorldFVDetector:detect", "Lower limit flaw.");
	}
	if(inst->getUpperLevelMin() > m_upperLimit)
	{
		inst->setFlawed(true);	
		inst->setUpper(true);
		inst->setUpperMagnitude(std::abs(m_upperLimit - inst->getUpperLevelMin()));
		debugMsg("OpenWorldFVDetector:detect", "Upper limit flaw.");	
	}
}

}
}
