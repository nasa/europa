#include "SAVH_FVDetector.hh"
#include "PlanDatabase.hh"

namespace EUROPA {
  namespace SAVH {

    bool FVDetector::allowViolations() const 
    { 
    	return m_res->getPlanDatabase()->getConstraintEngine()->getAllowViolations(); 
    }
  
    FVDetectorId FVDetectorFactoryMgr::createInstance(const LabelStr& name, const ResourceId res) {
      FVDetectorFactory* factory = dynamic_cast<FVDetectorFactory*>(getFactory(name));
      return factory->create(res);
    }
  }
}
