#include "ModuleResource.hh"
#include "SAVH_FlowProfile.hh"
#include "SAVH_IncrementalFlowProfile.hh"
#include "SAVH_TimetableProfile.hh"
#include "SAVH_ProfilePropagator.hh"
#include "SAVH_ReusableFVDetector.hh"
#include "ResourcePropagator.hh"

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

      REGISTER_PROFILE(EUROPA::SAVH::TimetableProfile, TimetableProfile );
      REGISTER_PROFILE(EUROPA::SAVH::FlowProfile, FlowProfile);
      REGISTER_PROFILE(EUROPA::SAVH::IncrementalFlowProfile, IncrementalFlowProfile );
      
      REGISTER_FVDETECTOR(EUROPA::SAVH::ReusableFVDetector, ReusableFVDetector);
      
	  resourceInitialized() = true;
  }  

  void ModuleResource::uninitialize()
  {
	  // TODO: clean up
	  resourceInitialized() = false;
  }  
  
  void ModuleResource::initialize(EngineId engine)
  {
	  PlanDatabaseId& pdb = (PlanDatabaseId&)(engine->getComponent("PlanDatabase"));
	  ConstraintEngineId& ce = (ConstraintEngineId&)(engine->getComponent("ConstraintEngine"));
	  new SAVH::ProfilePropagator(LabelStr("SAVH_Resource"), ce);
	  new ResourcePropagator(LabelStr("Resource"), ce, pdb);	  	  
  }
  
  void ModuleResource::uninitialize(EngineId engine)
  {	  
	  // TODO: clean up
  }  
}
