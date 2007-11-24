#include "ModuleResource.hh"

namespace EUROPA {

  static bool & resourceInitialized() {
    static bool sl_alreadyDone(false);
    return sl_alreadyDone;
  }

  ModuleResource::ModuleResource()
      : Module("Resource")
  {
	  
  }

  ModuleResource::~ModuleResource()
  {	  
  }  
  
  void ModuleResource::initialize()
  {
      if(resourceInitialized())
    	  return;
      
	  resourceInitialized() = true;
  }  

  void ModuleResource::uninitialize()
  {
	  resourceInitialized() = false;
  }  
  
  void ModuleResource::initialize(EngineId engine)
  {
  }
  
  void ModuleResource::uninitialize(EngineId engine)
  {	  
  }  
}
