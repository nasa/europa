#include "ModuleRulesEngine.hh"
#include "ConstraintType.hh"
#include "PlanDatabase.hh"
#include "ProxyVariableRelation.hh"
#include "Rule.hh"
#include "RulesEngine.hh"
#include "RuleVariableListener.hh"
#include "Constraints.hh"

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
      RuleSchema* rs = new RuleSchema();
      engine->addComponent("RuleSchema",rs);
      RulesEngine* re = new RulesEngine(rs->getId(),pdb->getId());
      engine->addComponent("RulesEngine",re);

      CESchema* ces = (CESchema*)engine->getComponent("CESchema");
      REGISTER_SYSTEM_CONSTRAINT(ces,ProxyVariableRelation, "proxyRelation", "Default");
      REGISTER_SYSTEM_CONSTRAINT(ces,RuleVariableListener, RuleVariableListener::CONSTRAINT_NAME(),"Default");
      REGISTER_SYSTEM_CONSTRAINT(ces,LockConstraint, "filterLock", "RulesEngine");
  }

  void ModuleRulesEngine::uninitialize(EngineId engine)
  {
      RulesEngine* re = (RulesEngine*)engine->removeComponent("RulesEngine");
      delete re;

      RuleSchema* rs = (RuleSchema*)engine->removeComponent("RuleSchema");
      delete rs;
  }
}
