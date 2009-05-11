#include "FVDetector.hh"
#include "PlanDatabase.hh"

namespace EUROPA {
    bool FVDetector::allowViolations() const
    {
    	return m_res->getPlanDatabase()->getConstraintEngine()->getAllowViolations();
    }
}
