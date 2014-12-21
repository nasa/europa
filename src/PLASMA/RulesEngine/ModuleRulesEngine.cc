#include "ModuleRulesEngine.hh"
#include "ConstraintType.hh"
#include "PlanDatabase.hh"
#include "ProxyVariableRelation.hh"
#include "Rule.hh"
#include "RulesEngine.hh"
#include "RuleVariableListener.hh"
#include "Constraints.hh"
#include "Propagators.hh"
#include "CESchema.hh"

#include <boost/cast.hpp>

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

void ModuleRulesEngine::initialize(EngineId engine) {
  PlanDatabase* pdb =
      boost::polymorphic_cast<PlanDatabase*>(engine->getComponent("PlanDatabase"));
  RuleSchema* rs = new RuleSchema();
  engine->addComponent("RuleSchema",rs);
  RulesEngine* re = new RulesEngine(rs->getId(),pdb->getId());
  engine->addComponent("RulesEngine",re);

  // Allocate an instance of Default Propagator to handle rule related constraint propagation.
  // Will be cleaned up automatically by the ConstraintEngine
  new DefaultPropagator("RulesEngine", pdb->getConstraintEngine(), SYSTEM_PRIORITY);

  CESchema* ces = boost::polymorphic_cast<CESchema*>(engine->getComponent("CESchema"));
  REGISTER_SYSTEM_CONSTRAINT(ces,ProxyVariableRelation, "proxyRelation", "Default");
  REGISTER_SYSTEM_CONSTRAINT(ces,RuleVariableListener, RuleVariableListener::CONSTRAINT_NAME(),"Default");
  REGISTER_SYSTEM_CONSTRAINT(ces,LockConstraint, "filterLock", "RulesEngine");
}

void ModuleRulesEngine::uninitialize(EngineId engine) {
  RulesEngine* re = boost::polymorphic_cast<RulesEngine*>(engine->removeComponent("RulesEngine"));
  delete re;
  
  RuleSchema* rs = boost::polymorphic_cast<RuleSchema*>(engine->removeComponent("RuleSchema"));
  delete rs;
}
}
