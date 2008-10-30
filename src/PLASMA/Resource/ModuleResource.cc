#include "ModuleResource.hh"
#include "ResourceConstraint.hh"
#include "ConstraintFactory.hh"
#include "SAVH_FlowProfile.hh"
#include "SAVH_IncrementalFlowProfile.hh"
#include "SAVH_TimetableProfile.hh"
#include "SAVH_ProfilePropagator.hh"
#include "SAVH_OpenWorldFVDetector.hh"
#include "SAVH_ClosedWorldFVDetector.hh"
#include "ResourcePropagator.hh"
#include "NddlXml.hh"
#include "InterpreterResources.hh"
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

      REGISTER_SYSTEM_CONSTRAINT(ce->getCESchema(),ResourceConstraint, "ResourceTransactionRelation", "Resource");
	  new SAVH::ProfilePropagator(LabelStr("SAVH_Resource"), ce->getId());
	  new ResourcePropagator(LabelStr("Resource"), ce->getId(), pdb->getId());

      schema->addObjectType("Resource");
      schema->addMember("Resource", "float", "initialCapacity");
      schema->addMember("Resource", "float", "levelLimitMin");
      schema->addMember("Resource", "float", "levelLimitMax");
      schema->addMember("Resource", "float", "productionRateMax");
      schema->addMember("Resource", "float", "productionMax");
      schema->addMember("Resource", "float", "consumptionRateMax");
      schema->addMember("Resource", "float", "consumptionMax");
      REGISTER_OBJECT_FACTORY(schema,ResourceObjectFactory, Resource);
      REGISTER_OBJECT_FACTORY(schema,ResourceObjectFactory, Resource:float:float:float);
      REGISTER_OBJECT_FACTORY(schema,ResourceObjectFactory, Resource:float:float:float:float:float);
      REGISTER_OBJECT_FACTORY(schema,ResourceObjectFactory, Resource:float:float:float:float:float:float:float);
      schema->addPredicate("Resource.change");
      schema->addMember("Resource.change", "float", "quantity");
      schema->registerTokenFactory((new ResourceChangeTokenFactory("Resource.change"))->getId());

      schema->addObjectType("Reusable");
      schema->addMember("Reusable", "float", "capacity");
      schema->addMember("Reusable", "float", "levelLimitMin");
      schema->addMember("Reusable", "float", "consumptionMax");
      schema->addMember("Reusable", "float", "consumptionRateMax");
      REGISTER_OBJECT_FACTORY(schema,ReusableObjectFactory, Reusable);
      REGISTER_OBJECT_FACTORY(schema,ReusableObjectFactory, Reusable:float:float);
      REGISTER_OBJECT_FACTORY(schema,ReusableObjectFactory, Reusable:float:float:float);
      REGISTER_OBJECT_FACTORY(schema,ReusableObjectFactory, Reusable:float:float:float:float);
      schema->addPredicate("Reusable.uses");
      schema->addMember("Reusable.uses", "float", "quantity");
      schema->registerTokenFactory((new ReusableUsesTokenFactory("Reusable.uses"))->getId());

      schema->addObjectType("Reservoir");
      schema->addMember("Reservoir", "float", "initialCapacity");
      schema->addMember("Reservoir", "float", "levelLimitMin");
      schema->addMember("Reservoir", "float", "levelLimitMax");
      schema->addMember("Reservoir", "float", "productionRateMax");
      schema->addMember("Reservoir", "float", "productionMax");
      schema->addMember("Reservoir", "float", "consumptionRateMax");
      schema->addMember("Reservoir", "float", "consumptionMax");
      REGISTER_OBJECT_FACTORY(schema,ReservoirObjectFactory, Reservoir);
      REGISTER_OBJECT_FACTORY(schema,ReservoirObjectFactory, Reservoir:float:float:float);
      REGISTER_OBJECT_FACTORY(schema,ReservoirObjectFactory, Reservoir:float:float:float:float:float);
      REGISTER_OBJECT_FACTORY(schema,ReservoirObjectFactory, Reservoir:float:float:float:float:float:float:float);

      schema->addPredicate("Reservoir.produce");
      schema->addMember("Reservoir.produce", "float", "quantity");
      schema->registerTokenFactory((new ReservoirProduceTokenFactory("Reservoir.produce"))->getId());

      schema->addPredicate("Reservoir.consume");
      schema->addMember("Reservoir.consume", "float", "quantity");
      schema->registerTokenFactory((new ReservoirConsumeTokenFactory("Reservoir.consume"))->getId());

      schema->addObjectType("Unary");
      schema->addMember("Unary", "float", "consumptionMax");
      REGISTER_OBJECT_FACTORY(schema,UnaryObjectFactory, Unary);
      REGISTER_OBJECT_FACTORY(schema,UnaryObjectFactory, Unary:float);
      schema->addPredicate("Unary.use");
      schema->registerTokenFactory((new UnaryUseTokenFactory("Unary.use"))->getId());

      FactoryMgr* pfm = new FactoryMgr();
      engine->addComponent("ProfileFactoryMgr",pfm);
      REGISTER_PROFILE(pfm,EUROPA::SAVH::TimetableProfile, TimetableProfile );
      REGISTER_PROFILE(pfm,EUROPA::SAVH::FlowProfile, FlowProfile);
      REGISTER_PROFILE(pfm,EUROPA::SAVH::IncrementalFlowProfile, IncrementalFlowProfile );

      // Solver
      FactoryMgr* fvdfm = new FactoryMgr();
      engine->addComponent("FVDetectorFactoryMgr",fvdfm);
      REGISTER_FVDETECTOR(fvdfm,EUROPA::SAVH::OpenWorldFVDetector,OpenWorldFVDetector);
      REGISTER_FVDETECTOR(fvdfm,EUROPA::SAVH::ClosedWorldFVDetector,ClosedWorldFVDetector);

      EUROPA::SOLVERS::ComponentFactoryMgr* cfm = (EUROPA::SOLVERS::ComponentFactoryMgr*)engine->getComponent("ComponentFactoryMgr");
      REGISTER_FLAW_MANAGER(cfm,SAVH::ThreatManager, SAVHThreatManager);
      REGISTER_FLAW_HANDLER(cfm,SAVH::ThreatDecisionPoint, SAVHThreatHandler);
      REGISTER_FLAW_HANDLER(cfm,EUROPA::SOLVERS::ResourceThreatDecisionPoint, ResourceThreat);

      EUROPA::SOLVERS::MatchFinderMgr* mfm = (EUROPA::SOLVERS::MatchFinderMgr*)engine->getComponent("MatchFinderMgr");
      REGISTER_MATCH_FINDER(mfm,EUROPA::SOLVERS::InstantMatchFinder,SAVH::Instant::entityTypeName());

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
      const char* fmgrs[] = {"ProfileFactoryMgr","FVDetectorFactoryMgr"};
      for (int i=0;i<2;i++) {
          FactoryMgr* fm = (FactoryMgr*)engine->removeComponent(fmgrs[i]);
          delete fm;
      }

      // TODO: clean up other pieces added by this module
  }
}
