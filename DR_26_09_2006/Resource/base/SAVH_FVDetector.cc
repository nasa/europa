#include "SAVH_FVDetector.hh"

namespace EUROPA {
  namespace SAVH {

    FVDetectorId FVDetectorFactory::createInstance(const LabelStr& name, const ResourceId res) {
      std::map<double, FVDetectorFactory*>::const_iterator it = factoryMap().find(name);
      checkError(it != factoryMap().end(), "No factory registered for '" << name.toString() << "'");
      
      FVDetectorFactory* factory = it->second;
      return factory->create(res);
    }

    void FVDetectorFactory::registerFactory(const LabelStr& name, FVDetectorFactory* factory) {
      checkError(factoryMap().find(name) == factoryMap().end(), "Tried to reigster factory for '" << name.toString() << "' twice.");
      factoryMap().insert(std::pair<double, FVDetectorFactory*>(name, factory));
    }

    std::map<double, FVDetectorFactory*>& FVDetectorFactory::factoryMap() {
      static std::map<double, FVDetectorFactory*> sl_factoryMap;
      return sl_factoryMap;
    }
  }
}
