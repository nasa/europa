#include "ClosedWorldFVDetector.hh"

#include <cmath>

namespace EUROPA {

ClosedWorldFVDetector::ClosedWorldFVDetector(const ResourceId res) : GenericFVDetector(res) {
}

Resource::ProblemType ClosedWorldFVDetector::getResourceLevelViolation(const InstantId inst) const
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

void ClosedWorldFVDetector::handleResourceLevelFlaws(const InstantId inst)
{
	edouble limitLb, limitUb;
	getLimitBounds(inst,limitLb,limitUb);
	edouble levelLb, levelUb;
	getLevelBounds(inst,levelLb,levelUb);

	if(levelLb < limitLb)
	{
		inst->setFlawed(true);
		inst->setLower(true);
		inst->setLowerMagnitude(std::abs(limitLb - levelLb));
		debugMsg("ClosedWorldFVDetector:detect", "Lower limit flaw.");
	}

	if(levelUb > limitUb)
	{
		inst->setFlawed(true);
		inst->setUpper(true);
		inst->setUpperMagnitude(std::abs(limitUb - levelUb));
		debugMsg("ClosedWorldFVDetector:detect", "Upper limit flaw.");
	}
}

}
