#ifndef _H_GroundedFVDetector
#define _H_GroundedFVDetector

#include "GenericFVDetector.hh"
namespace EUROPA {

	class GroundedFVDetector : public GenericFVDetector {
    public:
    	GroundedFVDetector(const ResourceId res);

    protected:
		virtual void getFDLevelBounds(const InstantId& inst, edouble& lb, edouble& ub) const; // Level Bounds for FlawDetection
		virtual void getVDLevelBounds(const InstantId& inst, edouble& lb, edouble& ub) const; // Level Bounds for ViolationDetection
    };
}

#endif
