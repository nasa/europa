#include "ModuleTemporalNetwork.hh"
#include "ConstraintEngine.hh"
#include "ConstraintLibrary.hh"
#include "Constraints.hh"
#include "PlanDatabase.hh"
#include "TemporalPropagator.hh"
#include "STNTemporalAdvisor.hh"

namespace EUROPA {

  static bool & TemporalNetworkInitialized() {
    static bool sl_alreadyDone(false);
    return sl_alreadyDone;
  }

  ModuleTemporalNetwork::ModuleTemporalNetwork()
      : Module("TemporalNetwork")
  {
	  
  }

  ModuleTemporalNetwork::~ModuleTemporalNetwork()
  {	  
  }  
  
  void ModuleTemporalNetwork::initialize()
  {
      if(TemporalNetworkInitialized())
    	  return;
      
      REGISTER_SYSTEM_CONSTRAINT(EqualConstraint, "concurrent", "Temporal");
      REGISTER_SYSTEM_CONSTRAINT(LessThanEqualConstraint, "precedes", "Temporal"); 
      REGISTER_SYSTEM_CONSTRAINT(AddEqualConstraint, "temporaldistance", "Temporal");
      REGISTER_SYSTEM_CONSTRAINT(AddEqualConstraint, "temporalDistance", "Temporal");

      TemporalNetworkInitialized() = true;
  }  

  void ModuleTemporalNetwork::uninitialize()
  {
	  TemporalNetworkInitialized() = false;
  }  
  
  void ModuleTemporalNetwork::initialize(EngineId engine)
  {
	  PlanDatabaseId& pdb = (PlanDatabaseId&)(engine->getComponent("PlanDatabase"));
	  ConstraintEngineId& ce = (ConstraintEngineId&)(engine->getComponent("ConstraintEngine"));

	  new TemporalPropagator(LabelStr("Temporal"), ce);
	  PropagatorId temporalPropagator = ce->getPropagatorByName(LabelStr("Temporal"));
	  pdb->setTemporalAdvisor((new STNTemporalAdvisor(temporalPropagator))->getId());
  }
  
  void ModuleTemporalNetwork::uninitialize(EngineId engine)
  {	 
  }  
}
