#include "ModuleTemporalNetwork.hh"
#include "ConstraintEngine.hh"
#include "ConstraintFactory.hh"
#include "Constraints.hh"
#include "PlanDatabase.hh"
#include "DefaultPropagator.hh"
#include "TemporalPropagator.hh"
#include "STNTemporalAdvisor.hh"

namespace EUROPA {

  ModuleTemporalNetwork::ModuleTemporalNetwork()
      : Module("TemporalNetwork")
  {
  }

  ModuleTemporalNetwork::~ModuleTemporalNetwork()
  {
  }

  void ModuleTemporalNetwork::initialize()
  {
  }

  void ModuleTemporalNetwork::uninitialize()
  {
  }

  void ModuleTemporalNetwork::initialize(EngineId engine)
  {
      ConstraintEngine* ce = (ConstraintEngine*)engine->getComponent("ConstraintEngine");
      CESchema* ces = ce->getCESchema();

      REGISTER_SYSTEM_CONSTRAINT(ces,EqualConstraint, "concurrent", "Temporal");
      REGISTER_SYSTEM_CONSTRAINT(ces,LessThanEqualConstraint, "precedes", "Temporal");
      REGISTER_SYSTEM_CONSTRAINT(ces,AddEqualConstraint, "temporalDistance", "Temporal");

      PlanDatabase* pdb = (PlanDatabase*)engine->getComponent("PlanDatabase");

	  PropagatorId temporalPropagator;
	  if (engine->getConfig()->getProperty("TemporalNetwork.useTemporalPropagator") != "N") {
	      temporalPropagator = (new TemporalPropagator(LabelStr("Temporal"), ce->getId()))->getId();
	      pdb->setTemporalAdvisor((new STNTemporalAdvisor(temporalPropagator))->getId());
	  }
	  else {
          temporalPropagator = (new DefaultPropagator(LabelStr("Temporal"), ce->getId()))->getId();
          pdb->setTemporalAdvisor((new DefaultTemporalAdvisor(ce->getId()))->getId());
	  }
  }

  void ModuleTemporalNetwork::uninitialize(EngineId engine)
  {
      // TODO: cleanup
  }
}
