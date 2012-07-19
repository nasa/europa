#include "SAVH_FVDetector.hh"
#include "PlanDatabase.hh"

namespace EUROPA {
  namespace SAVH {

    bool FVDetector::allowViolations() const 
    { 
    	return m_res->getPlanDatabase()->getConstraintEngine()->getAllowViolations(); 
    }  
  }
}
