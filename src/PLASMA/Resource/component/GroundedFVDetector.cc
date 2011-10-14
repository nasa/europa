#include "GroundedFVDetector.hh"

#include <cmath>

namespace EUROPA {

GroundedFVDetector::GroundedFVDetector(const ResourceId res) : GenericFVDetector(res) {
}

// NOTE: This is the same as ClosedWorldFVDetector, but we don't subclass that as we're expecting a
// reorganization where the violation and flaw detection pieces can be configured separately
Resource::ProblemType GroundedFVDetector::getResourceLevelViolation(const InstantId inst) const
{
	edouble limitLb, limitUb;
	getLimitBounds(inst,limitLb,limitUb);
	edouble levelLb, levelUb;
	getLevelBounds(inst,levelLb,levelUb);

	if (levelUb < limitLb)
	{
		debugMsg("ClosedWorldFVDetector:detect",
				"Lower limit violation.  Limit: " << limitLb << " Upper level: " << levelUb);
		return Resource::LevelTooLow;
	}
	if (levelLb > limitUb)
	{
    	debugMsg("ClosedWorldFVDetector:detect",
    			"Upper limit violation.  Limit: " << limitUb << " Lower level: " << levelLb);
    	return Resource::LevelTooHigh;
	}
	return Resource::NoProblem;
}

// Here we handle the LowerLevelMax and UpperLevelMin which, in GroundedProfile are hacked to represent
// the grounded min/max instead of the traditional meaning implied by their names.
void GroundedFVDetector::handleResourceLevelFlaws(const InstantId inst)
{
	edouble limitLb, limitUb;
	getLimitBounds(inst,limitLb,limitUb);
	edouble levelLb, levelUb;
	getGroundedLevelBounds(inst,levelLb,levelUb);

	if(levelLb < limitLb)
	{
		inst->setFlawed(true);
		inst->setLower(true);
		inst->setLowerMagnitude(std::abs(limitLb - levelLb));
		debugMsg("GroundedFVDetector:detect", "Lower limit flaw.");
	}

	if(levelUb > limitUb)
	{
		inst->setFlawed(true);
		inst->setUpper(true);
		inst->setUpperMagnitude(std::abs(limitUb - levelUb));
		debugMsg("GroundedFVDetector:detect", "Upper limit flaw.");
	}
}

void GroundedFVDetector::getGroundedLevelBounds(const InstantId& inst, edouble& lb, edouble& ub) const
{
	// LowerLevelMax is really GroundedMin
	lb = inst->getLowerLevelMax();
	// UpperLevelMin is really GroundedMax
	ub = inst->getUpperLevelMin();
}

}
