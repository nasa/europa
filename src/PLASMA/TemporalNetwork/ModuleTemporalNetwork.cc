#include "ModuleTemporalNetwork.hh"
#include "ConstraintEngine.hh"
#include "ConstraintType.hh"
#include "Constraints.hh"
#include "PlanDatabase.hh"
#include "Propagators.hh"
#include "TemporalPropagator.hh"
#include "STNTemporalAdvisor.hh"
#include "CESchema.hh"

#include <boost/cast.hpp>

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

void ModuleTemporalNetwork::initialize(EngineId engine) {
  ConstraintEngine* ce =
      boost::polymorphic_cast<ConstraintEngine*>(engine->getComponent("ConstraintEngine"));
  CESchema* ces = ce->getCESchema();

  REGISTER_SYSTEM_CONSTRAINT(ces,EqualConstraint, "concurrent", "Temporal");
  REGISTER_SYSTEM_CONSTRAINT(ces,LessThanEqualConstraint, "precedes", "Temporal");
  REGISTER_SYSTEM_CONSTRAINT(ces, LessThanConstraint, "strictlyPrecedes", "Temporal");
  REGISTER_SYSTEM_CONSTRAINT(ces,AddEqualConstraint, "temporalDistance", "Temporal");

  PlanDatabase* pdb =
      boost::polymorphic_cast<PlanDatabase*>(engine->getComponent("PlanDatabase"));

  PropagatorId temporalPropagator;
  if (engine->getConfig()->getProperty("TemporalNetwork.useTemporalPropagator") != "N") {
    temporalPropagator = (new TemporalPropagator("Temporal", ce->getId()))->getId();
    pdb->setTemporalAdvisor((new STNTemporalAdvisor(temporalPropagator))->getId());
  }
  else {
    temporalPropagator = (new DefaultPropagator("Temporal", ce->getId()))->getId();
    pdb->setTemporalAdvisor((new DefaultTemporalAdvisor(ce->getId()))->getId());
  }
}

void ModuleTemporalNetwork::uninitialize(EngineId) {
  // TODO: cleanup
}
}
