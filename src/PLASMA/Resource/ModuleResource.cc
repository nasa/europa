#include "ModuleResource.hh"
#include "ConstraintType.hh"
#include "Schema.hh"
#include "FlawHandler.hh"
#include "NddlXml.hh"
#include "InterpreterResources.hh"
#include "ResourceMatching.hh"
#include "ResourceThreatDecisionPoint.hh"
#include "ProfilePropagator.hh"
#include "FlowProfile.hh"
#include "IncrementalFlowProfile.hh"
#include "TimetableProfile.hh"
#include "GroundedProfile.hh"
#include "OpenWorldFVDetector.hh"
#include "ClosedWorldFVDetector.hh"
#include "GroundedFVDetector.hh"
#include "Instant.hh"
#include "ThreatDecisionPoint.hh"
#include "ResourceThreatManager.hh"
#include "Reusable.hh"

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
	  new ProfilePropagator(LabelStr("Resource"), ce->getId());

	  ObjectTypeId objectOT = schema->getObjectType(Schema::rootObject());
	  ObjectType* ot;

      // TODO: preserve class hierarchy, all Resource types should extend Resource, not Object
      ot = new ObjectType("Reusable",objectOT,true /*isNative*/);
      ot->addMember(FloatDT::instance(), "capacity");
      ot->addMember(FloatDT::instance(), "levelLimitMin");
      ot->addMember(FloatDT::instance(), "consumptionMax");
      ot->addMember(FloatDT::instance(), "consumptionRateMax");
      ot->addObjectFactory((new ReusableObjectFactory(ot->getId(),"Reusable"))->getId());
      ot->addObjectFactory((new ReusableObjectFactory(ot->getId(),"Reusable:float:float"))->getId());
      ot->addObjectFactory((new ReusableObjectFactory(ot->getId(),"Reusable:float:float:float"))->getId());
      ot->addObjectFactory((new ReusableObjectFactory(ot->getId(),"Reusable:float:float:float:float"))->getId());
      ot->addTokenFactory((new ReusableUsesTokenFactory("Reusable.uses"))->getId());
      schema->registerObjectType(ot->getId());

      ot = new ObjectType("CBReusable",objectOT,true /*isNative*/);
      ot->addMember(FloatDT::instance(), "capacity");
      ot->addMember(FloatDT::instance(), "levelLimitMin");
      ot->addMember(FloatDT::instance(), "consumptionMax");
      ot->addMember(FloatDT::instance(), "consumptionRateMax");
      ot->addObjectFactory((new CBReusableObjectFactory(ot->getId(),"CBReusable"))->getId());
      ot->addObjectFactory((new CBReusableObjectFactory(ot->getId(),"CBReusable:float:float"))->getId());
      ot->addObjectFactory((new CBReusableObjectFactory(ot->getId(),"CBReusable:float:float:float"))->getId());
      ot->addObjectFactory((new CBReusableObjectFactory(ot->getId(),"CBReusable:float:float:float:float"))->getId());
      schema->registerObjectType(ot->getId());
      REGISTER_CONSTRAINT(
        ce->getCESchema(),
        Uses,
        Uses::CONSTRAINT_NAME(),
        Uses::PROPAGATOR_NAME()
      );

      ot = new ObjectType("Reservoir",objectOT,true /*isNative*/);
      ot->addMember(FloatDT::instance(), "initialCapacity");
      ot->addMember(FloatDT::instance(), "levelLimitMin");
      ot->addMember(FloatDT::instance(), "levelLimitMax");
      ot->addMember(FloatDT::instance(), "productionRateMax");
      ot->addMember(FloatDT::instance(), "productionMax");
      ot->addMember(FloatDT::instance(), "consumptionRateMax");
      ot->addMember(FloatDT::instance(), "consumptionMax");
      ot->addObjectFactory((new ReservoirObjectFactory(ot->getId(),"Reservoir"))->getId());
      ot->addObjectFactory((new ReservoirObjectFactory(ot->getId(),"Reservoir:float:float:float"))->getId());
      ot->addObjectFactory((new ReservoirObjectFactory(ot->getId(),"Reservoir:float:float:float:float:float"))->getId());
      ot->addObjectFactory((new ReservoirObjectFactory(ot->getId(),"Reservoir:float:float:float:float:float:float:float"))->getId());
      ot->addTokenFactory((new ReservoirProduceTokenFactory("Reservoir.produce"))->getId());
      ot->addTokenFactory((new ReservoirConsumeTokenFactory("Reservoir.consume"))->getId());
      schema->registerObjectType(ot->getId());

      ot = new ObjectType("Unary",objectOT,true /*isNative*/);
      ot->addMember(FloatDT::instance(),"consumptionMax");
      ot->addObjectFactory((new UnaryObjectFactory(ot->getId(),"Unary"))->getId());
      ot->addObjectFactory((new UnaryObjectFactory(ot->getId(),"Unary:float"))->getId());
      ot->addTokenFactory((new UnaryUseTokenFactory("Unary.use"))->getId());
      schema->registerObjectType(ot->getId());

      FactoryMgr* pfm = new FactoryMgr();
      engine->addComponent("ProfileFactoryMgr",pfm);
      REGISTER_PROFILE(pfm,TimetableProfile, TimetableProfile );
      REGISTER_PROFILE(pfm,FlowProfile, FlowProfile);
      REGISTER_PROFILE(pfm,IncrementalFlowProfile, IncrementalFlowProfile );
      REGISTER_PROFILE(pfm,GroundedProfile, GroundedProfile );

      // Solver
      FactoryMgr* fvdfm = new FactoryMgr();
      engine->addComponent("FVDetectorFactoryMgr",fvdfm);
      REGISTER_FVDETECTOR(fvdfm,OpenWorldFVDetector,OpenWorldFVDetector);
      REGISTER_FVDETECTOR(fvdfm,ClosedWorldFVDetector,ClosedWorldFVDetector);
      REGISTER_FVDETECTOR(fvdfm,GroundedFVDetector,GroundedFVDetector);

      SOLVERS::ComponentFactoryMgr* cfm = (SOLVERS::ComponentFactoryMgr*)engine->getComponent("ComponentFactoryMgr");
      REGISTER_FLAW_MANAGER(cfm,ResourceThreatManager, ResourceThreatManager);
      REGISTER_FLAW_HANDLER(cfm,ResourceThreatDecisionPoint, ResourceThreatHandler);
//      REGISTER_FLAW_HANDLER(cfm,SOLVERS::ResourceThreatDecisionPoint, ResourceThreat);

      SOLVERS::MatchFinderMgr* mfm = (EUROPA::SOLVERS::MatchFinderMgr*)engine->getComponent("MatchFinderMgr");
      REGISTER_MATCH_FINDER(mfm,EUROPA::SOLVERS::InstantMatchFinder,Instant::entityTypeName());
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
