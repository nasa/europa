#include "ClosedWorldFVDetector.hh"

namespace EUROPA {

ClosedWorldFVDetector::ClosedWorldFVDetector(const ResourceId res)
	: GenericFVDetector(res)
{
}

void ClosedWorldFVDetector::getFDLevelBounds(const InstantId& inst, edouble& lb, edouble& ub) const
{
	getDefaultLevelBounds(inst,lb,ub);
}

void ClosedWorldFVDetector::getVDLevelBounds(const InstantId& inst, edouble& lb, edouble& ub) const
{
	getDefaultLevelBounds(inst,lb,ub);
}

}
