#include "OpenWorldFVDetector.hh"

#include <cmath>

namespace EUROPA {

OpenWorldFVDetector::OpenWorldFVDetector(const ResourceId res) : GenericFVDetector(res) {
}

Resource::ProblemType OpenWorldFVDetector::getResourceLevelViolation(const InstantId inst) const
{
	edouble limitLb, limitUb;
	getLimitBounds(inst,limitLb,limitUb);
	edouble levelLb, levelUb;
	getLevelBounds(inst,levelLb,levelUb);

	if (levelUb + (m_maxCumulativeProduction - inst->getMinCumulativeProduction()) < limitLb)
	{
		debugMsg("OpenWorldFVDetector:detect", "Lower limit violation.  Limit: " << limitLb << " Upper level: " << levelUb <<
				" Maximum remaining production: " << (m_maxCumulativeProduction - inst->getMinCumulativeProduction()));
		return Resource::LevelTooLow;
	}
	if (levelLb - (m_maxCumulativeConsumption - inst->getMinCumulativeConsumption()) > limitUb)
	{
		debugMsg("OpenWorldFVDetector:detect", "Upper limit violation.  Limit: " << limitUb << " Lower level: " << levelLb <<
				" Maxumum remaining consumption: " << (m_maxCumulativeConsumption - inst->getMinCumulativeConsumption()));
    	return Resource::LevelTooHigh;
	}
	return Resource::NoProblem;
}

void OpenWorldFVDetector::handleResourceLevelFlaws(const InstantId inst)
{
	edouble limitLb, limitUb;
	getLimitBounds(inst,limitLb,limitUb);
	edouble levelLb, levelUb;
	getOpenLevelBounds(inst,levelLb,levelUb);

	if(levelLb < limitLb)
	{
		inst->setFlawed(true);
		inst->setLower(true);
		inst->setLowerMagnitude(std::abs(limitLb - levelLb));
		debugMsg("OpenWorldFVDetector:detect", "Lower limit flaw.");
	}
	if(levelUb > limitUb)
	{
		inst->setFlawed(true);
		inst->setUpper(true);
		inst->setUpperMagnitude(std::abs(limitUb - levelUb));
		debugMsg("OpenWorldFVDetector:detect", "Upper limit flaw.");	
	}
}

void OpenWorldFVDetector::getOpenLevelBounds(const InstantId& inst, edouble& lb, edouble& ub) const
{
	lb = inst->getLowerLevelMax();
	ub = inst->getUpperLevelMin();
}

}
