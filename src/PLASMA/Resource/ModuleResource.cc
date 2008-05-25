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

  ModuleResource::ModuleResource()
      : Module("Resource")
  {	  
  }

  ModuleResource::~ModuleResource()
  {	  
  }  
  
  void ModuleResource::initialize()
  {
  }  

  void ModuleResource::uninitialize()
  {
  }  
  
  
  void ModuleResource::initialize(EngineId engine)
  {
      REGISTER_SYSTEM_CONSTRAINT(ResourceConstraint, "ResourceTransactionRelation", "Resource");

      REGISTER_PROFILE(EUROPA::SAVH::TimetableProfile, TimetableProfile );
      REGISTER_PROFILE(EUROPA::SAVH::FlowProfile, FlowProfile);
      REGISTER_PROFILE(EUROPA::SAVH::IncrementalFlowProfile, IncrementalFlowProfile );
      
      REGISTER_FVDETECTOR(EUROPA::SAVH::TimetableFVDetector, TimetableFVDetector);
      REGISTER_FVDETECTOR(EUROPA::SAVH::ReusableFVDetector, ReusableFVDetector);
      REGISTER_FVDETECTOR(EUROPA::SAVH::OpenWorldFVDetector, OpenWorldFVDetector);
      REGISTER_FVDETECTOR(EUROPA::SAVH::ClosedWorldFVDetector, ClosedWorldFVDetector);               

      // Solver
      REGISTER_FLAW_MANAGER(SAVH::ThreatManager, SAVHThreatManager); 
      REGISTER_FLAW_HANDLER(SAVH::ThreatDecisionPoint, SAVHThreatHandler); 
      REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::ResourceThreatDecisionPoint, ResourceThreat); 
      EUROPA::SOLVERS::MatchingEngine::addMatchFinder(SAVH::Instant::entityTypeName(),(new EUROPA::SOLVERS::InstantMatchFinder())->getId()); 
      
      Schema* schema = (Schema*)(engine->getComponent("Schema"));
      schema->declareObjectType("Resource");
      schema->declareObjectType("Reusable");
      schema->declareObjectType("Reservoir");
      
      PlanDatabase* pdb = (PlanDatabase*)(engine->getComponent("PlanDatabase"));
	  ConstraintEngine* ce = (ConstraintEngine*)(engine->getComponent("ConstraintEngine"));

	  new SAVH::ProfilePropagator(LabelStr("SAVH_Resource"), ce->getId());
	  new ResourcePropagator(LabelStr("Resource"), ce->getId(), pdb->getId());	  	  
	  
      // TODO: check if NDDL module is available?
      NddlXmlInterpreter* nddlXml = (NddlXmlInterpreter*)engine->getLanguageInterpreter("nddl-xml");
      if (nddlXml != NULL) {
          std::vector<std::string> nativeTokens;
          nativeTokens.push_back("Resource.change");
          nddlXml->addNativeClass("Resource",nativeTokens); 

          nativeTokens.clear();
          nativeTokens.push_back("Reusable.uses");
          nddlXml->addNativeClass("Reusable",nativeTokens);

          nativeTokens.clear();
          nativeTokens.push_back("Reservoir.produce");
          nativeTokens.push_back("Reservoir.consume");
          nddlXml->addNativeClass("Reservoir",nativeTokens);
      }
      
      // TODO: expose Unary           
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
  }
  
  void ModuleResource::uninitialize(EngineId engine)
  {	  
      SAVH::ProfileFactory::purgeAll();
      // TODO: clean up
  }  
}
