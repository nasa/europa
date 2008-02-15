#include "ModuleResource.hh"
#include "ResourceConstraint.hh"
#include "ConstraintLibrary.hh"
#include "SAVH_FlowProfile.hh"
#include "SAVH_IncrementalFlowProfile.hh"
#include "SAVH_TimetableProfile.hh"
#include "SAVH_ProfilePropagator.hh"
#include "SAVH_ReusableFVDetector.hh"
#include "SAVH_TimetableFVDetector.hh"
#include "ResourcePropagator.hh"
#include "PSPlanDatabase.hh"
#include "PSResource.hh"
#include "PSResourceImpl.hh"

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

  	  REGISTER_SYSTEM_CONSTRAINT(ResourceConstraint, "ResourceTransactionRelation", "Resource");

  	  REGISTER_PROFILE(EUROPA::SAVH::TimetableProfile, TimetableProfile );
      REGISTER_PROFILE(EUROPA::SAVH::FlowProfile, FlowProfile);
      REGISTER_PROFILE(EUROPA::SAVH::IncrementalFlowProfile, IncrementalFlowProfile );
      
   	  REGISTER_FVDETECTOR(EUROPA::SAVH::TimetableFVDetector, TimetableFVDetector);
      REGISTER_FVDETECTOR(EUROPA::SAVH::ReusableFVDetector, ReusableFVDetector);
      
	  resourceInitialized() = true;
  }  

  void ModuleResource::uninitialize()
  {
	  // TODO: clean up
	  SAVH::ProfileFactory::purgeAll();
	  resourceInitialized() = false;
  }  
  
  class ResourceWrapperGenerator : public ObjectWrapperGenerator 
  {
  public:
    PSObject* wrap(const EntityId& obj) {
      checkRuntimeError(SAVH::ResourceId::convertable(obj),
			"Object " << obj->toString() << " is not a resource.");
      return new PSResourceImpl(SAVH::ResourceId(obj));
    }
  }; 
  
  void ModuleResource::initialize(EngineId engine)
  {
	  PlanDatabaseId& pdb = (PlanDatabaseId&)(engine->getComponent("PlanDatabase"));
	  ConstraintEngineId& ce = (ConstraintEngineId&)(engine->getComponent("ConstraintEngine"));

	  new SAVH::ProfilePropagator(LabelStr("SAVH_Resource"), ce);
	  new ResourcePropagator(LabelStr("Resource"), ce, pdb);	  	  
	  
      pdb->addObjectWrapperGenerator("Reservoir", new ResourceWrapperGenerator());
      pdb->addObjectWrapperGenerator("Reusable", new ResourceWrapperGenerator());
      pdb->addObjectWrapperGenerator("Unary", new ResourceWrapperGenerator());
  }
  
  void ModuleResource::uninitialize(EngineId engine)
  {	  
	  // TODO: clean up
  }  
}
