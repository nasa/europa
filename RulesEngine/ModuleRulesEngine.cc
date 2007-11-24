#include "ModuleRulesEngine.hh"
#include "Rule.hh"

namespace EUROPA {

  static bool & RulesEngineInitialized() {
    static bool sl_alreadyDone(false);
    return sl_alreadyDone;
  }

  ModuleRulesEngine::ModuleRulesEngine()
      : Module("RulesEngine")
  {
	  
  }

  ModuleRulesEngine::~ModuleRulesEngine()
  {	  
  }  
  
  void ModuleRulesEngine::initialize()
  {
      if(RulesEngineInitialized())
    	  return;
      
      RulesEngineInitialized() = true;
  }  

  void ModuleRulesEngine::uninitialize()
  {
	  Rule::purgeAll();	  	 
	  RulesEngineInitialized() = false;
  }  
  
  void ModuleRulesEngine::initialize(EngineId engine)
  {
  }
  
  void ModuleRulesEngine::uninitialize(EngineId engine)
  {	 
  }  
}
