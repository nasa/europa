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
#include "SAVH_GroundedProfile.hh"
#include "SAVH_OpenWorldFVDetector.hh"
#include "SAVH_ClosedWorldFVDetector.hh"
#include "SAVH_GroundedFVDetector.hh"
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
      ot->addObjectFactory((new ResourceObjectFactory(ot->getId(),"Resource"))->getId());
      ot->addObjectFactory((new ResourceObjectFactory(ot->getId(),"Resource:float:float:float"))->getId());
      ot->addObjectFactory((new ResourceObjectFactory(ot->getId(),"Resource:float:float:float:float:float"))->getId());
      ot->addObjectFactory((new ResourceObjectFactory(ot->getId(),"Resource:float:float:float:float:float:float:float"))->getId());
      ot->addTokenFactory((new ResourceChangeTokenFactory("Resource.change"))->getId());
      schema->registerObjectType(ot->getId());

      // TODO: preserve class hierarchy, all Resource types should extend Resource, not Object
      ot = new ObjectType("Reusable","Object",true /*isNative*/);
      ot->addMember("float", "capacity");
      ot->addMember("float", "levelLimitMin");
      ot->addMember("float", "consumptionMax");
      ot->addMember("float", "consumptionRateMax");
      ot->addObjectFactory((new ReusableObjectFactory(ot->getId(),"Reusable"))->getId());
      ot->addObjectFactory((new ReusableObjectFactory(ot->getId(),"Reusable:float:float"))->getId());
      ot->addObjectFactory((new ReusableObjectFactory(ot->getId(),"Reusable:float:float:float"))->getId());
      ot->addObjectFactory((new ReusableObjectFactory(ot->getId(),"Reusable:float:float:float:float"))->getId());
      ot->addTokenFactory((new ReusableUsesTokenFactory("Reusable.uses"))->getId());
      schema->registerObjectType(ot->getId());

      ot = new ObjectType("CBReusable","Object",true /*isNative*/);
      ot->addMember("float", "capacity");
      ot->addMember("float", "levelLimitMin");
      ot->addMember("float", "consumptionMax");
      ot->addMember("float", "consumptionRateMax");
      ot->addObjectFactory((new CBReusableObjectFactory(ot->getId(),"CBReusable"))->getId());
      ot->addObjectFactory((new CBReusableObjectFactory(ot->getId(),"CBReusable:float:float"))->getId());
      ot->addObjectFactory((new CBReusableObjectFactory(ot->getId(),"CBReusable:float:float:float"))->getId());
      ot->addObjectFactory((new CBReusableObjectFactory(ot->getId(),"CBReusable:float:float:float:float"))->getId());
      schema->registerObjectType(ot->getId());
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
      ot->addObjectFactory((new ReservoirObjectFactory(ot->getId(),"Reservoir"))->getId());
      ot->addObjectFactory((new ReservoirObjectFactory(ot->getId(),"Reservoir:float:float:float"))->getId());
      ot->addObjectFactory((new ReservoirObjectFactory(ot->getId(),"Reservoir:float:float:float:float:float"))->getId());
      ot->addObjectFactory((new ReservoirObjectFactory(ot->getId(),"Reservoir:float:float:float:float:float:float:float"))->getId());
      ot->addTokenFactory((new ReservoirProduceTokenFactory("Reservoir.produce"))->getId());
      ot->addTokenFactory((new ReservoirConsumeTokenFactory("Reservoir.consume"))->getId());
      schema->registerObjectType(ot->getId());

      ot = new ObjectType("Unary","Object",true /*isNative*/);
      ot->addMember("float","consumptionMax");
      ot->addObjectFactory((new UnaryObjectFactory(ot->getId(),"Unary"))->getId());
      ot->addObjectFactory((new UnaryObjectFactory(ot->getId(),"Unary:float"))->getId());
      ot->addTokenFactory((new UnaryUseTokenFactory("Unary.use"))->getId());
      schema->registerObjectType(ot->getId());

      FactoryMgr* pfm = new FactoryMgr();
      engine->addComponent("ProfileFactoryMgr",pfm);
      REGISTER_PROFILE(pfm,EUROPA::SAVH::TimetableProfile, TimetableProfile );
      REGISTER_PROFILE(pfm,EUROPA::SAVH::FlowProfile, FlowProfile);
      REGISTER_PROFILE(pfm,EUROPA::SAVH::IncrementalFlowProfile, IncrementalFlowProfile );
      REGISTER_PROFILE(pfm,EUROPA::SAVH::GroundedProfile, GroundedProfile );

      // Solver
      FactoryMgr* fvdfm = new FactoryMgr();
      engine->addComponent("FVDetectorFactoryMgr",fvdfm);
      REGISTER_FVDETECTOR(fvdfm,EUROPA::SAVH::OpenWorldFVDetector,OpenWorldFVDetector);
      REGISTER_FVDETECTOR(fvdfm,EUROPA::SAVH::ClosedWorldFVDetector,ClosedWorldFVDetector);
      REGISTER_FVDETECTOR(fvdfm,EUROPA::SAVH::GroundedFVDetector,GroundedFVDetector);

      EUROPA::SOLVERS::ComponentFactoryMgr* cfm = (EUROPA::SOLVERS::ComponentFactoryMgr*)engine->getComponent("ComponentFactoryMgr");
      REGISTER_FLAW_MANAGER(cfm,SAVH::ThreatManager, SAVHThreatManager);
      REGISTER_FLAW_HANDLER(cfm,SAVH::ThreatDecisionPoint, SAVHThreatHandler);
      REGISTER_FLAW_HANDLER(cfm,EUROPA::SOLVERS::ResourceThreatDecisionPoint, ResourceThreat);

      EUROPA::SOLVERS::MatchFinderMgr* mfm = (EUROPA::SOLVERS::MatchFinderMgr*)engine->getComponent("MatchFinderMgr");
      REGISTER_MATCH_FINDER(mfm,EUROPA::SOLVERS::InstantMatchFinder,SAVH::Instant::entityTypeName());
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
