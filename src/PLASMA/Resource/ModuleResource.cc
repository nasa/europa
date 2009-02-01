#include "ModuleResource.hh"
#include "ConstraintFactory.hh"
#include "Schema.hh"
#include "FlawHandler.hh"
#include "NddlXml.hh"
#include "InterpreterResources.hh"
#include "ResourceConstraint.hh"
#include "ResourcePropagator.hh"
#include "ResourceMatching.hh"
#include "ResourceThreatDecisionPoint.hh"
#include "SAVH_ProfilePropagator.hh"
#include "SAVH_FlowProfile.hh"
#include "SAVH_IncrementalFlowProfile.hh"
#include "SAVH_TimetableProfile.hh"
#include "SAVH_GroundedReusableProfile.hh"
#include "SAVH_OpenWorldFVDetector.hh"
#include "SAVH_ClosedWorldFVDetector.hh"
#include "SAVH_Instant.hh"
#include "SAVH_ThreatDecisionPoint.hh"
#include "SAVH_ThreatManager.hh"
#include "SAVH_Reusable.hh"

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

	  ObjectType* ot;

      ot = new ObjectType("Resource","Object",true /*isNative*/);
      ot->addMember("float", "initialCapacity");
      ot->addMember("float", "levelLimitMin");
      ot->addMember("float", "levelLimitMax");
      ot->addMember("float", "productionRateMax");
      ot->addMember("float", "productionMax");
      ot->addMember("float", "consumptionRateMax");
      ot->addMember("float", "consumptionMax");
      ot->addObjectFactory((new ResourceObjectFactory("Resource"))->getId());
      ot->addObjectFactory((new ResourceObjectFactory("Resource:float:float:float"))->getId());
      ot->addObjectFactory((new ResourceObjectFactory("Resource:float:float:float:float:float"))->getId());
      ot->addObjectFactory((new ResourceObjectFactory("Resource:float:float:float:float:float:float:float"))->getId());
      ot->addTokenFactory((new ResourceChangeTokenFactory("Resource.change"))->getId());
      schema->registerObjectType(ot);

      // TODO: preserve class hierarchy, all Resource types should extend Resource, not Object
      ot = new ObjectType("Reusable","Object",true /*isNative*/);
      ot->addMember("float", "capacity");
      ot->addMember("float", "levelLimitMin");
      ot->addMember("float", "consumptionMax");
      ot->addMember("float", "consumptionRateMax");
      ot->addObjectFactory((new ReusableObjectFactory("Reusable"))->getId());
      ot->addObjectFactory((new ReusableObjectFactory("Reusable:float:float"))->getId());
      ot->addObjectFactory((new ReusableObjectFactory("Reusable:float:float:float"))->getId());
      ot->addObjectFactory((new ReusableObjectFactory("Reusable:float:float:float:float"))->getId());
      ot->addTokenFactory((new ReusableUsesTokenFactory("Reusable.uses"))->getId());
      schema->registerObjectType(ot);

      ot = new ObjectType("CBReusable","Object",true /*isNative*/);
      ot->addMember("float", "capacity");
      ot->addMember("float", "levelLimitMin");
      ot->addMember("float", "consumptionMax");
      ot->addMember("float", "consumptionRateMax");
      ot->addObjectFactory((new CBReusableObjectFactory("CBReusable"))->getId());
      ot->addObjectFactory((new CBReusableObjectFactory("CBReusable:float:float"))->getId());
      ot->addObjectFactory((new CBReusableObjectFactory("CBReusable:float:float:float"))->getId());
      ot->addObjectFactory((new CBReusableObjectFactory("CBReusable:float:float:float:float"))->getId());
      schema->registerObjectType(ot);
      REGISTER_CONSTRAINT(
        ce->getCESchema(),
        SAVH::Uses,
        SAVH::Uses::CONSTRAINT_NAME(),
        SAVH::Uses::PROPAGATOR_NAME()
      );

      ot = new ObjectType("Reservoir","Object",true /*isNative*/);
      ot->addMember("float", "initialCapacity");
      ot->addMember("float", "levelLimitMin");
      ot->addMember("float", "levelLimitMax");
      ot->addMember("float", "productionRateMax");
      ot->addMember("float", "productionMax");
      ot->addMember("float", "consumptionRateMax");
      ot->addMember("float", "consumptionMax");
      ot->addObjectFactory((new ReservoirObjectFactory("Reservoir"))->getId());
      ot->addObjectFactory((new ReservoirObjectFactory("Reservoir:float:float:float"))->getId());
      ot->addObjectFactory((new ReservoirObjectFactory("Reservoir:float:float:float:float:float"))->getId());
      ot->addObjectFactory((new ReservoirObjectFactory("Reservoir:float:float:float:float:float:float:float"))->getId());
      ot->addTokenFactory((new ReservoirProduceTokenFactory("Reservoir.produce"))->getId());
      ot->addTokenFactory((new ReservoirConsumeTokenFactory("Reservoir.consume"))->getId());
      schema->registerObjectType(ot);

      ot = new ObjectType("Unary","Object",true /*isNative*/);
      ot->addMember("float","consumptionMax");
      ot->addObjectFactory((new UnaryObjectFactory("Unary"))->getId());
      ot->addObjectFactory((new UnaryObjectFactory("Unary:float"))->getId());
      ot->addTokenFactory((new UnaryUseTokenFactory("Unary.use"))->getId());
      schema->registerObjectType(ot);

      FactoryMgr* pfm = new FactoryMgr();
      engine->addComponent("ProfileFactoryMgr",pfm);
      REGISTER_PROFILE(pfm,EUROPA::SAVH::TimetableProfile, TimetableProfile );
      REGISTER_PROFILE(pfm,EUROPA::SAVH::FlowProfile, FlowProfile);
      REGISTER_PROFILE(pfm,EUROPA::SAVH::IncrementalFlowProfile, IncrementalFlowProfile );
      REGISTER_PROFILE(pfm,EUROPA::SAVH::GroundedReusableProfile, GroundedReusableProfile );

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

      // TODO: this can be removed when code generation is gone
      NddlXmlInterpreter* nddlXml = (NddlXmlInterpreter*)engine->getLanguageInterpreter("nddl-xml");
      if (nddlXml != NULL) {
          std::vector<std::string> nativeTokens;
          nativeTokens.push_back("Resource.change");
          nddlXml->addNativeClass("Resource",nativeTokens);

          nativeTokens.clear();
          nativeTokens.push_back("Reusable.uses");
          nddlXml->addNativeClass("Reusable",nativeTokens);

          nativeTokens.clear();
          nddlXml->addNativeClass("CBReusable",nativeTokens);

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
