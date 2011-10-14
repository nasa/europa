#include "GroundedFVDetector.hh"

#include <cmath>

namespace EUROPA {

GroundedFVDetector::GroundedFVDetector(const ResourceId res)
	: GenericFVDetector(res)
{
}

// Here we handle the LowerLevelMax and UpperLevelMin which, in GroundedProfile are hacked to represent
// the grounded min/max instead of the traditional meaning implied by their names.
void GroundedFVDetector::getFDLevelBounds(const InstantId& inst, edouble& lb, edouble& ub) const
{
	// LowerLevelMax is really GroundedMin
	lb = inst->getLowerLevelMax();
	// UpperLevelMin is really GroundedMax
	ub = inst->getUpperLevelMin();
}

void GroundedFVDetector::getVDLevelBounds(const InstantId& inst, edouble& lb, edouble& ub) const
{
	getDefaultLevelBounds(inst,lb,ub);
}

}
