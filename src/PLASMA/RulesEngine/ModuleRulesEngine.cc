#include "ModuleRulesEngine.hh"
#include "ConstraintFactory.hh"
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
      
      CESchema* ces = (CESchema*)engine->getComponent("CESchema");
      REGISTER_SYSTEM_CONSTRAINT(ces,ProxyVariableRelation, "proxyRelation", "Default");
      REGISTER_SYSTEM_CONSTRAINT(ces,RuleVariableListener, RuleVariableListener::CONSTRAINT_NAME(),RuleVariableListener::PROPAGATOR_NAME());      
  }
  
  void ModuleRulesEngine::uninitialize(EngineId engine)
  {	 
      RulesEngine* re = (RulesEngine*)engine->removeComponent("RulesEngine");      
      delete re;
      
      Rule::purgeAll();      
  }  
}
