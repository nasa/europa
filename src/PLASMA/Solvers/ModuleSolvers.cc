#include "ModuleSolvers.hh"
#include "SolverDefs.hh"
#include "ConstrainedVariable.hh"
#include "ConstraintFactory.hh"
#include "ComponentFactory.hh"
#include "Filters.hh"
#include "FlawFilter.hh"
#include "FlawHandler.hh"
#include "HSTSDecisionPoints.hh"
#include "MatchingEngine.hh"
#include "OpenConditionManager.hh"
#include "ThreatManager.hh"
#include "Token.hh"
#include "UnboundVariableManager.hh"
#include "RulesEngine.hh"
#include "PSSolversImpl.hh"

// TODO: is this necessary?
#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif

namespace EUROPA {

  ModuleSolvers::ModuleSolvers()
      : Module("Solvers")
  {	 
  }

  ModuleSolvers::~ModuleSolvers()
  {	  
  }  
  
  void ModuleSolvers::initialize()
  {
  }  

  void ModuleSolvers::uninitialize()
  {
  }  
  
  void ModuleSolvers::initialize(EngineId engine)
  {
      ConstraintEngine* ce = (ConstraintEngine*)engine->getComponent("ConstraintEngine");
      PlanDatabase* pdb = (PlanDatabase*)engine->getComponent("PlanDatabase");
      RulesEngine* re = (RulesEngine*)engine->getComponent("RulesEngine");
      PSSolverManager* sm = new PSSolverManagerImpl(ce->getId(),pdb->getId(),re->getId());
      engine->addComponent("PSSolverManager",sm);
      
      REGISTER_SYSTEM_CONSTRAINT(ce->getCESchema(),SOLVERS::FlawHandler::VariableListener,SOLVERS::FlawHandler::VariableListener::CONSTRAINT_NAME(),SOLVERS::FlawHandler::VariableListener::PROPAGATOR_NAME());
      
      EUROPA::SOLVERS::ComponentFactoryMgr* cfm = new EUROPA::SOLVERS::ComponentFactoryMgr();
      engine->addComponent("ComponentFactoryMgr",cfm);
      REGISTER_COMPONENT_FACTORY(cfm,SOLVERS::FlawFilter, FlawFilter); 
      
      REGISTER_FLAW_MANAGER(cfm,SOLVERS::UnboundVariableManager, UnboundVariableManager); 
      REGISTER_FLAW_HANDLER(cfm,SOLVERS::MinValue, StandardVariableHandler); 
      REGISTER_FLAW_HANDLER(cfm,SOLVERS::MinValue, Min); 
      REGISTER_FLAW_HANDLER(cfm,SOLVERS::MaxValue, Max); 
      REGISTER_FLAW_HANDLER(cfm,SOLVERS::HSTS::ValueEnum, ValEnum); 
      
      REGISTER_FLAW_MANAGER(cfm,SOLVERS::OpenConditionManager, OpenConditionManager); 
      REGISTER_FLAW_HANDLER(cfm,SOLVERS::OpenConditionDecisionPoint, StandardOpenConditionHandler); 
      REGISTER_FLAW_HANDLER(cfm,SOLVERS::HSTS::OpenConditionDecisionPoint, HSTSOpenConditionDecisionPoint); 
      
      REGISTER_FLAW_MANAGER(cfm,SOLVERS::ThreatManager, ThreatManager); 
      REGISTER_FLAW_HANDLER(cfm,SOLVERS::ThreatDecisionPoint, StandardThreatHandler); 
      REGISTER_FLAW_HANDLER(cfm,SOLVERS::HSTS::ThreatDecisionPoint, HSTSThreatDecisionPoint); 
      
      REGISTER_FLAW_FILTER(cfm,SOLVERS::SingletonFilter, Singleton); 
      REGISTER_FLAW_FILTER(cfm,SOLVERS::InfiniteDynamicFilter, InfiniteDynamicFilter); 
      REGISTER_FLAW_FILTER(cfm,SOLVERS::HorizonFilter, HorizonFilter); 
      REGISTER_FLAW_FILTER(cfm,SOLVERS::HorizonVariableFilter, HorizonVariableFilter); 
      REGISTER_FLAW_FILTER(cfm,SOLVERS::MasterMustBeAssignedFilter, MasterMustBeInsertedFilter); 
      REGISTER_FLAW_FILTER(cfm,SOLVERS::TokenMustBeAssignedFilter, TokenMustBeAssignedFilter); 
      REGISTER_FLAW_FILTER(cfm,SOLVERS::TokenMustBeAssignedFilter, ParentMustBeInsertedFilter); 
      
      REGISTER_TOKEN_SORTER(cfm,SOLVERS::HSTS::EarlyTokenComparator, early); 
      REGISTER_TOKEN_SORTER(cfm,SOLVERS::HSTS::LateTokenComparator, late); 
      REGISTER_TOKEN_SORTER(cfm,SOLVERS::HSTS::NearTokenComparator, near); 
      REGISTER_TOKEN_SORTER(cfm,SOLVERS::HSTS::FarTokenComparator, far); 
      REGISTER_TOKEN_SORTER(cfm,SOLVERS::HSTS::AscendingKeyTokenComparator, ascendingKey);       
      
      EUROPA::SOLVERS::MatchFinderMgr* mfm = new EUROPA::SOLVERS::MatchFinderMgr();
      engine->addComponent("MatchFinderMgr",mfm);
      REGISTER_MATCH_FINDER(mfm,SOLVERS::VariableMatchFinder,ConstrainedVariable::entityTypeName());
      REGISTER_MATCH_FINDER(mfm,SOLVERS::TokenMatchFinder,Token::entityTypeName());
  }
  
  void ModuleSolvers::uninitialize(EngineId engine)
  {	 
      EUROPA::SOLVERS::MatchFinderMgr* mfm = (EUROPA::SOLVERS::MatchFinderMgr*)engine->removeComponent("MatchFinderMgr");
      delete mfm;
      
      EUROPA::SOLVERS::ComponentFactoryMgr* cfm = (EUROPA::SOLVERS::ComponentFactoryMgr*)engine->removeComponent("ComponentFactoryMgr");
      delete cfm;
      
      PSSolverManager* sm = (PSSolverManager*)engine->removeComponent("PSSolverManager");
      delete sm;
  }  
}
