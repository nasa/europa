#ifndef _H_GroundedFVDetector
#define _H_GroundedFVDetector

#include "GenericFVDetector.hh"
namespace EUROPA {

	class GroundedFVDetector : public GenericFVDetector {
    public:
    	GroundedFVDetector(const ResourceId res);

    protected:
    	Resource::ProblemType getResourceLevelViolation(const InstantId inst) const;
    	void handleResourceLevelFlaws(const InstantId inst);
    	void getGroundedLevelBounds(const InstantId& inst, edouble& lb, edouble& ub) const;
    };
}

#endif
