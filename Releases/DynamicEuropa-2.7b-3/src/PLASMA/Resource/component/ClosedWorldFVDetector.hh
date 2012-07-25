#ifndef _H_ClosedWorldFVDetector
#define _H_ClosedWorldFVDetector

#include "GenericFVDetector.hh"
namespace EUROPA {

	class ClosedWorldFVDetector : public GenericFVDetector {
	public:
		ClosedWorldFVDetector(const ResourceId res);

	protected:
		virtual void getFDLevelBounds(const InstantId& inst, edouble& lb, edouble& ub) const; // Level Bounds for FlawDetection
		virtual void getVDLevelBounds(const InstantId& inst, edouble& lb, edouble& ub) const; // Level Bounds for ViolationDetection
	};
}

#endif
