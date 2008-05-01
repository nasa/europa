#include "ModuleResource.hh"
#include "ResourceConstraint.hh"
#include "ConstraintLibrary.hh"
#include "SAVH_FlowProfile.hh"
#include "SAVH_IncrementalFlowProfile.hh"
#include "SAVH_TimetableProfile.hh"
#include "SAVH_ProfilePropagator.hh"
#include "SAVH_ReusableFVDetector.hh"
#include "SAVH_TimetableFVDetector.hh"
#include "SAVH_OpenWorldFVDetector.hh"
#include "SAVH_ClosedWorldFVDetector.hh"
#include "ResourcePropagator.hh"
#include "PSPlanDatabase.hh"
#include "PSResource.hh"
#include "PSResourceImpl.hh"
#include "TransactionInterpreter.hh"
#include "TransactionInterpreterResources.hh"
#include "Schema.hh"
#include "ResourceMatching.hh"
#include "ResourceThreatDecisionPoint.hh"
#include "SAVH_Instant.hh"
#include "SAVH_ThreatDecisionPoint.hh"
#include "SAVH_ThreatManager.hh"
#include "FlawHandler.hh"


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
  	  REGISTER_FVDETECTOR(EUROPA::SAVH::OpenWorldFVDetector, OpenWorldFVDetector);
      REGISTER_FVDETECTOR(EUROPA::SAVH::ClosedWorldFVDetector, ClosedWorldFVDetector);               

      REGISTER_OBJECT_FACTORY(ResourceObjectFactory, Resource);                   
      REGISTER_OBJECT_FACTORY(ResourceObjectFactory, Resource:float:float:float);                     
      REGISTER_OBJECT_FACTORY(ResourceObjectFactory, Resource:float:float:float:float:float);                     
      REGISTER_OBJECT_FACTORY(ResourceObjectFactory, Resource:float:float:float:float:float:float:float);                     
      new ResourceChangeTokenFactory("Resource.change");

      REGISTER_OBJECT_FACTORY(ReusableObjectFactory, Reusable);                   
      REGISTER_OBJECT_FACTORY(ReusableObjectFactory, Reusable:float:float);                   
      REGISTER_OBJECT_FACTORY(ReusableObjectFactory, Reusable:float:float:float);                     
      REGISTER_OBJECT_FACTORY(ReusableObjectFactory, Reusable:float:float:float:float);                   
      new ReusableUsesTokenFactory("Reusable.uses");

      REGISTER_OBJECT_FACTORY(ReservoirObjectFactory, Reservoir);                     
      REGISTER_OBJECT_FACTORY(ReservoirObjectFactory, Reservoir:float:float:float);                   
      REGISTER_OBJECT_FACTORY(ReservoirObjectFactory, Reservoir:float:float:float:float:float);                   
      REGISTER_OBJECT_FACTORY(ReservoirObjectFactory, Reservoir:float:float:float:float:float:float:float);                   
      new ReservoirProduceTokenFactory("Reservoir.produce");      
      new ReservoirConsumeTokenFactory("Reservoir.consume");      

      // Solver
      REGISTER_FLAW_MANAGER(SAVH::ThreatManager, SAVHThreatManager); 
      REGISTER_FLAW_HANDLER(SAVH::ThreatDecisionPoint, SAVHThreatHandler); 
      REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::ResourceThreatDecisionPoint, ResourceThreat); 
      EUROPA::SOLVERS::MatchingEngine::addMatchFinder(SAVH::Instant::entityTypeName(),(new EUROPA::SOLVERS::InstantMatchFinder())->getId()); 
      
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
      SchemaId schema = (SchemaId&)(engine->getComponent("Schema"));
      schema->declareObjectType("Resource");
      schema->declareObjectType("Reusable");
      schema->declareObjectType("Reservoir");
      
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
