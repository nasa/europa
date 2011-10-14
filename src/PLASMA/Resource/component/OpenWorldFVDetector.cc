#include "OpenWorldFVDetector.hh"

#include <cmath>

namespace EUROPA {

OpenWorldFVDetector::OpenWorldFVDetector(const ResourceId res)
	: GenericFVDetector(res)
{
}

Resource::ProblemType OpenWorldFVDetector::getResourceLevelViolation(const InstantId inst) const
{
	Resource::ProblemType retval = GenericFVDetector::getResourceLevelViolation(inst);

	if (retval == Resource::LevelTooLow) {
		debugMsg("OpenWorldFVDetector:detect",
				" Lower limit violation. Maximum remaining production: " <<
				(m_maxCumulativeProduction - inst->getMinCumulativeProduction()));
	}
	else if (retval == Resource::LevelTooHigh) {
		debugMsg("OpenWorldFVDetector:detect",
				" Upper limit violation. Maximum remaining consumption: " <<
				(m_maxCumulativeConsumption - inst->getMinCumulativeConsumption()));
	}

	return retval;
}

void OpenWorldFVDetector::getFDLevelBounds(const InstantId& inst, edouble& lb, edouble& ub) const
{
	lb = inst->getLowerLevelMax();
	ub = inst->getUpperLevelMin();
}

void OpenWorldFVDetector::getVDLevelBounds(const InstantId& inst, edouble& lb, edouble& ub) const
{
	getDefaultLevelBounds(inst,lb,ub);
	ub += (m_maxCumulativeProduction - inst->getMinCumulativeProduction());
	lb -= (m_maxCumulativeConsumption - inst->getMinCumulativeConsumption());
}

}
