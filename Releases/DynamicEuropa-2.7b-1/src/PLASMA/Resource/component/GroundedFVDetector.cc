#include "GroundedFVDetector.hh"

#include <cmath>
#include "Profile.hh"

namespace EUROPA {

GroundedFVDetector::GroundedFVDetector(const ResourceId res)
	: GenericFVDetector(res)
{
}

// Here we handle the LowerLevelMax and UpperLevelMin which, in GroundedProfile are hacked to represent
// the grounded min/max instead of the traditional meaning implied by their names.
void GroundedFVDetector::getFDLevelBounds(const InstantId& inst, edouble& lb, edouble& ub) const
{
	const std::pair<edouble,edouble>& capacityBounds = m_res->getCapacityProfile()->getValue(inst->getTime());

	edouble usageLb = inst->getLowerLevelMax(); // LowerLevelMax is really GroundedMin
	edouble usageUb = inst->getUpperLevelMin(); // UpperLevelMin is really GroundedMax

	lb = capacityBounds.first + usageLb;
	ub = capacityBounds.second + usageUb;
}

void GroundedFVDetector::getVDLevelBounds(const InstantId& inst, edouble& lb, edouble& ub) const
{
	getDefaultLevelBounds(inst,lb,ub);
}

}
