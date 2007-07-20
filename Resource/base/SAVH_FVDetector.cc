#include "SAVH_FVDetector.hh"
#include "PlanDatabase.hh"

namespace EUROPA {
  namespace SAVH {

    bool FVDetector::allowViolations() const 
    { 
    	return m_res->getPlanDatabase()->getConstraintEngine()->getAllowViolations(); 
    }
  
    FVDetectorId FVDetectorFactory::createInstance(const LabelStr& name, const ResourceId res) {
      std::map<double, FVDetectorFactory*>::const_iterator it = factoryMap().find(name);
      checkError(it != factoryMap().end(), "No factory registered for '" << name.toString() << "'");
      
      FVDetectorFactory* factory = it->second;
      return factory->create(res);
    }

    void FVDetectorFactory::registerFactory(const LabelStr& name, FVDetectorFactory* factory) {
      //checkError(factoryMap().find(name) == factoryMap().end(), "Tried to reigster factory for '" << name.toString() << "' twice.");
      std::map<double, FVDetectorFactory*>::iterator it = factoryMap().find(name);
      if(it != factoryMap().end()) {
	delete it->second;
	factoryMap().erase(it);
      }
      factoryMap().insert(std::pair<double, FVDetectorFactory*>(name, factory));
    }

    std::map<double, FVDetectorFactory*>& FVDetectorFactory::factoryMap() {
      static std::map<double, FVDetectorFactory*> sl_factoryMap;
      return sl_factoryMap;
    }
        
  }
}
