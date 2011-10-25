#ifndef _H_OpenWorldFVDetector
#define _H_OpenWorldFVDetector

#include "GenericFVDetector.hh"
namespace EUROPA {

	class OpenWorldFVDetector : public GenericFVDetector {
	public:
		OpenWorldFVDetector(const ResourceId res);
	protected:
		virtual Resource::ProblemType getResourceLevelViolation(const InstantId inst) const;
		virtual void getFDLevelBounds(const InstantId& inst, edouble& lb, edouble& ub) const; // Level Bounds for FlawDetection
		virtual void getVDLevelBounds(const InstantId& inst, edouble& lb, edouble& ub) const; // Level Bounds for ViolationDetection
	};
}

#endif
