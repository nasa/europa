#include "ClosedWorldFVDetector.hh"

#include <cmath>

namespace EUROPA {

ClosedWorldFVDetector::ClosedWorldFVDetector(const ResourceId res) : GenericFVDetector(res) {
}

Resource::ProblemType ClosedWorldFVDetector::getResourceLevelViolation(const InstantId inst) const
{
	if (inst->getUpperLevel() < getLowerLimit(inst))
	{
		debugMsg("ClosedWorldFVDetector:detect",
				"Lower limit violation.  Limit: " << getLowerLimit(inst) << " Upper level: " << inst->getUpperLevel());
		return Resource::LevelTooLow;
	}
	if (inst->getLowerLevel() > getUpperLimit(inst))
	{
    	debugMsg("ClosedWorldFVDetector:detect",
    			"Upper limit violation.  Limit: " << getUpperLimit(inst) << " Lower level: " << inst->getLowerLevel());
    	return Resource::LevelTooHigh;
	}
	return Resource::NoProblem;
}

void ClosedWorldFVDetector::handleResourceLevelFlaws(const InstantId inst)
{
	if(inst->getLowerLevel() < getLowerLimit(inst))
	{
		inst->setFlawed(true);
		inst->setLower(true);
		inst->setLowerMagnitude(std::abs(getLowerLimit(inst) - inst->getLowerLevel()));
		debugMsg("ClosedWorldFVDetector:detect", "Lower limit flaw.");
	}

	if(inst->getUpperLevel() > getUpperLimit(inst))
	{
		inst->setFlawed(true);
		inst->setUpper(true);
		inst->setUpperMagnitude(std::abs(getUpperLimit(inst) - inst->getUpperLevel()));
		debugMsg("ClosedWorldFVDetector:detect", "Upper limit flaw.");
	}
}

}
