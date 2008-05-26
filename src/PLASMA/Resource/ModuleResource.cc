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
      ConstraintEngine* ce = (ConstraintEngine*)(engine->getComponent("ConstraintEngine"));
      Schema* schema = (Schema*)(engine->getComponent("Schema"));
      PlanDatabase* pdb = (PlanDatabase*)(engine->getComponent("PlanDatabase"));

      REGISTER_SYSTEM_CONSTRAINT(ResourceConstraint, "ResourceTransactionRelation", "Resource");
	  new SAVH::ProfilePropagator(LabelStr("SAVH_Resource"), ce->getId());
	  new ResourcePropagator(LabelStr("Resource"), ce->getId(), pdb->getId());	  	  
	  
      // TODO: this should go away soon
      pdb->addObjectWrapperGenerator("Resource", new ResourceWrapperGenerator());
      pdb->addObjectWrapperGenerator("Reservoir", new ResourceWrapperGenerator());
      pdb->addObjectWrapperGenerator("Reusable", new ResourceWrapperGenerator());
      pdb->addObjectWrapperGenerator("Unary", new ResourceWrapperGenerator());
      
      schema->addObjectType("Resource");      
      schema->addMember("Resource", "float", "initialCapacity");
      schema->addMember("Resource", "float", "levelLimitMin");
      schema->addMember("Resource", "float", "levelLimitMax");
      schema->addMember("Resource", "float", "productionRateMax");
      schema->addMember("Resource", "float", "productionMax");
      schema->addMember("Resource", "float", "consumptionRateMax");
      schema->addMember("Resource", "float", "consumptionMax");
      REGISTER_OBJECT_FACTORY(ResourceObjectFactory, Resource);                   
      REGISTER_OBJECT_FACTORY(ResourceObjectFactory, Resource:float:float:float);                     
      REGISTER_OBJECT_FACTORY(ResourceObjectFactory, Resource:float:float:float:float:float);                     
      REGISTER_OBJECT_FACTORY(ResourceObjectFactory, Resource:float:float:float:float:float:float:float);                     
      schema->addPredicate("Resource.change");      
      schema->addMember("Resource.change", "float", "quantity");
      new ResourceChangeTokenFactory("Resource.change");

      schema->addObjectType("Reusable");
      schema->addMember("Reusable", "float", "capacity");
      schema->addMember("Reusable", "float", "levelLimitMin");
      schema->addMember("Reusable", "float", "consumptionMax");
      schema->addMember("Reusable", "float", "consumptionRateMax");
      REGISTER_OBJECT_FACTORY(ReusableObjectFactory, Reusable);                   
      REGISTER_OBJECT_FACTORY(ReusableObjectFactory, Reusable:float:float);                   
      REGISTER_OBJECT_FACTORY(ReusableObjectFactory, Reusable:float:float:float);                     
      REGISTER_OBJECT_FACTORY(ReusableObjectFactory, Reusable:float:float:float:float);                   
      schema->addPredicate("Reusable.uses");      
      schema->addMember("Reusable.uses", "float", "quantity");
      new ReusableUsesTokenFactory("Reusable.uses");

      schema->addObjectType("Reservoir");
      schema->addMember("Reservoir", "float", "initialCapacity");
      schema->addMember("Reservoir", "float", "levelLimitMin");
      schema->addMember("Reservoir", "float", "levelLimitMax");
      schema->addMember("Reservoir", "float", "productionRateMax");
      schema->addMember("Reservoir", "float", "productionMax");
      schema->addMember("Reservoir", "float", "consumptionRateMax");
      schema->addMember("Reservoir", "float", "consumptionMax");
      REGISTER_OBJECT_FACTORY(ReservoirObjectFactory, Reservoir);                     
      REGISTER_OBJECT_FACTORY(ReservoirObjectFactory, Reservoir:float:float:float);                   
      REGISTER_OBJECT_FACTORY(ReservoirObjectFactory, Reservoir:float:float:float:float:float);                   
      REGISTER_OBJECT_FACTORY(ReservoirObjectFactory, Reservoir:float:float:float:float:float:float:float);                   

      schema->addPredicate("Reservoir.produce");      
      schema->addMember("Reservoir.produce", "float", "quantity");
      new ReservoirProduceTokenFactory("Reservoir.produce");      

      schema->addPredicate("Reservoir.consume");      
      schema->addMember("Reservoir.consume", "float", "quantity");
      new ReservoirConsumeTokenFactory("Reservoir.consume");      
      
      schema->addObjectType("Unary");
      schema->addMember("Unary", "float", "consumptionMax");
      REGISTER_OBJECT_FACTORY(UnaryObjectFactory, Unary);                     
      REGISTER_OBJECT_FACTORY(UnaryObjectFactory, Unary:float);                   
      schema->addPredicate("Unary.use");
      new UnaryUseTokenFactory("Unary.use");             
      
      REGISTER_PROFILE(EUROPA::SAVH::TimetableProfile, TimetableProfile );
      REGISTER_PROFILE(EUROPA::SAVH::FlowProfile, FlowProfile);
      REGISTER_PROFILE(EUROPA::SAVH::IncrementalFlowProfile, IncrementalFlowProfile );
      
      // Solver
      REGISTER_FVDETECTOR(EUROPA::SAVH::TimetableFVDetector, TimetableFVDetector);
      REGISTER_FVDETECTOR(EUROPA::SAVH::ReusableFVDetector, ReusableFVDetector);
      REGISTER_FVDETECTOR(EUROPA::SAVH::OpenWorldFVDetector, OpenWorldFVDetector);
      REGISTER_FVDETECTOR(EUROPA::SAVH::ClosedWorldFVDetector, ClosedWorldFVDetector);               

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
          
          nativeTokens.clear();
          nativeTokens.push_back("Unary.use");
          nddlXml->addNativeClass("Unary",nativeTokens);
      }
      else {
          // TODO: log a warning/info?
      }      
  }
  
  void ModuleResource::uninitialize(EngineId engine)
  {	  
      SAVH::ProfileFactory::purgeAll();
      // TODO: clean up
  }  
}
