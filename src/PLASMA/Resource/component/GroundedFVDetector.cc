#include "GroundedFVDetector.hh"

#include <cmath>

namespace EUROPA {

GroundedFVDetector::GroundedFVDetector(const ResourceId res) : GenericFVDetector(res) {
}

// NOTE: This is the same as ClosedWorldFVDetector, but we don't subclass that as we're expecting a
// reorganization where the violation and flaw detection pieces can be configured separately
Resource::ProblemType GroundedFVDetector::getResourceLevelViolation(const InstantId inst) const
{
	if (inst->getUpperLevel() < getLowerLimit(inst))
	{
		debugMsg("GroundedFVDetector:detect",
				"Lower limit violation.  Limit: " << getLowerLimit(inst) << " Upper level: " << inst->getUpperLevel());
		return Resource::LevelTooLow;
	}
	if (inst->getLowerLevel() > getUpperLimit(inst))
	{
    	debugMsg("GroundedFVDetector:detect",
    			"Upper limit violation.  Limit: " << getUpperLimit(inst) << " Lower level: " << inst->getLowerLevel());
    	return Resource::LevelTooHigh;
	}
	return Resource::NoProblem;
}

// Here we handle the LowerLevelMax and UpperLevelMin which, in GroundedProfile are hacked to represent
// the grounded min/max instead of the traditional meaning implied by their names.
void GroundedFVDetector::handleResourceLevelFlaws(const InstantId inst)
{
	// LowerLevelMax is really GroundedMin
	if(inst->getLowerLevelMax() < getLowerLimit(inst))
	{
		inst->setFlawed(true);
		inst->setLower(true);
		inst->setLowerMagnitude(std::abs(getLowerLimit(inst) - inst->getLowerLevelMax()));
		debugMsg("GroundedFVDetector:detect", "Lower limit flaw.");
	}

	// UpperLevelMin is really GroundedMax
	if(inst->getUpperLevelMin() > getUpperLimit(inst))
	{
		inst->setFlawed(true);
		inst->setUpper(true);
		inst->setUpperMagnitude(std::abs(getUpperLimit(inst) - inst->getUpperLevelMin()));
		debugMsg("GroundedFVDetector:detect", "Upper limit flaw.");
	}
}

}
