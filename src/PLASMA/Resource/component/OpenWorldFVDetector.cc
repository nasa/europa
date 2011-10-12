#include "OpenWorldFVDetector.hh"

#include <cmath>

namespace EUROPA {

OpenWorldFVDetector::OpenWorldFVDetector(const ResourceId res) : GenericFVDetector(res) {
}

Resource::ProblemType OpenWorldFVDetector::getResourceLevelViolation(const InstantId inst) const
{
	if (inst->getUpperLevel() + (m_maxCumulativeProduction - inst->getMinCumulativeProduction()) < getLowerLimit(inst))
	{
		debugMsg("OpenWorldFVDetector:detect", "Lower limit violation.  Limit: " << getLowerLimit(inst) << " Upper level: " << inst->getUpperLevel() <<
				" Maximum remaining production: " << (m_maxCumulativeProduction - inst->getMinCumulativeProduction()));
		return Resource::LevelTooLow;
	}
	if (inst->getLowerLevel() - (m_maxCumulativeConsumption - inst->getMinCumulativeConsumption()) > getUpperLimit(inst))
	{
		debugMsg("OpenWorldFVDetector:detect", "Upper limit violation.  Limit: " << getUpperLimit(inst) << " Lower level: " << inst->getLowerLevel() <<
				" Maxumum remaining consumption: " << (m_maxCumulativeConsumption - inst->getMinCumulativeConsumption()));
    	return Resource::LevelTooHigh;
	}
	return Resource::NoProblem;
}

void OpenWorldFVDetector::handleResourceLevelFlaws(const InstantId inst)
{
	if(inst->getLowerLevelMax() < getLowerLimit(inst))
	{
		inst->setFlawed(true);
		inst->setLower(true);
		inst->setLowerMagnitude(std::abs(getLowerLimit(inst) - inst->getLowerLevelMax()));
		debugMsg("OpenWorldFVDetector:detect", "Lower limit flaw.");
	}
	if(inst->getUpperLevelMin() > getUpperLimit(inst))
	{
		inst->setFlawed(true);
		inst->setUpper(true);
		inst->setUpperMagnitude(std::abs(getUpperLimit(inst) - inst->getUpperLevelMin()));
		debugMsg("OpenWorldFVDetector:detect", "Upper limit flaw.");	
	}
}

}
