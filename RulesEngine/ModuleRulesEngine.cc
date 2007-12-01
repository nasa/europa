#include "ModuleRulesEngine.hh"
#include "ConstraintLibrary.hh"
#include "ProxyVariableRelation.hh"
#include "Rule.hh"
#include "RuleVariableListener.hh"

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
      
      REGISTER_SYSTEM_CONSTRAINT(ProxyVariableRelation, "proxyRelation", "Default");
      REGISTER_SYSTEM_CONSTRAINT(RuleVariableListener, RuleVariableListener::CONSTRAINT_NAME(),RuleVariableListener::PROPAGATOR_NAME());
      
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
