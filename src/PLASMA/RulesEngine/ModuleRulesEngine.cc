#include "ModuleRulesEngine.hh"
#include "ConstraintLibrary.hh"
#include "PlanDatabase.hh"
#include "ProxyVariableRelation.hh"
#include "Rule.hh"
#include "RulesEngine.hh"
#include "RuleVariableListener.hh"

namespace EUROPA {

  ModuleRulesEngine::ModuleRulesEngine()
      : Module("RulesEngine")
  {	  
  }

  ModuleRulesEngine::~ModuleRulesEngine()
  {	  
  }  
  
  void ModuleRulesEngine::initialize()
  {
  }  

  void ModuleRulesEngine::uninitialize()
  {
  }  
  
  void ModuleRulesEngine::initialize(EngineId engine)
  {
      PlanDatabase* pdb = (PlanDatabase*)engine->getComponent("PlanDatabase");
      RulesEngine* re = new RulesEngine(pdb->getId());      
      engine->addComponent("RulesEngine",re);        
      
      REGISTER_SYSTEM_CONSTRAINT(ProxyVariableRelation, "proxyRelation", "Default");
      REGISTER_SYSTEM_CONSTRAINT(RuleVariableListener, RuleVariableListener::CONSTRAINT_NAME(),RuleVariableListener::PROPAGATOR_NAME());      
  }
  
  void ModuleRulesEngine::uninitialize(EngineId engine)
  {	 
      RulesEngine* re = (RulesEngine*)engine->removeComponent("RulesEngine");      
      delete re;
      
      Rule::purgeAll();      
  }  
}
