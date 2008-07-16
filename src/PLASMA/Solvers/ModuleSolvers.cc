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
      
      REGISTER_COMPONENT_FACTORY(SOLVERS::FlawFilter, FlawFilter); 
      
      REGISTER_FLAW_MANAGER(SOLVERS::UnboundVariableManager, UnboundVariableManager); 
      REGISTER_FLAW_HANDLER(SOLVERS::MinValue, StandardVariableHandler); 
      REGISTER_FLAW_HANDLER(SOLVERS::MinValue, Min); 
      REGISTER_FLAW_HANDLER(SOLVERS::MaxValue, Max); 
      REGISTER_FLAW_HANDLER(SOLVERS::HSTS::ValueEnum, ValEnum); 
      
      REGISTER_FLAW_MANAGER(SOLVERS::OpenConditionManager, OpenConditionManager); 
      REGISTER_FLAW_HANDLER(SOLVERS::OpenConditionDecisionPoint, StandardOpenConditionHandler); 
      REGISTER_FLAW_HANDLER(SOLVERS::HSTS::OpenConditionDecisionPoint, HSTSOpenConditionDecisionPoint); 
      
      REGISTER_FLAW_MANAGER(SOLVERS::ThreatManager, ThreatManager); 
      REGISTER_FLAW_HANDLER(SOLVERS::ThreatDecisionPoint, StandardThreatHandler); 
      REGISTER_FLAW_HANDLER(SOLVERS::HSTS::ThreatDecisionPoint, HSTSThreatDecisionPoint); 
      
      REGISTER_FLAW_FILTER(SOLVERS::SingletonFilter, Singleton); 
      REGISTER_FLAW_FILTER(SOLVERS::InfiniteDynamicFilter, InfiniteDynamicFilter); 
      REGISTER_FLAW_FILTER(SOLVERS::HorizonFilter, HorizonFilter); 
      REGISTER_FLAW_FILTER(SOLVERS::HorizonVariableFilter, HorizonVariableFilter); 
      REGISTER_FLAW_FILTER(SOLVERS::MasterMustBeAssignedFilter, MasterMustBeInsertedFilter); 
      REGISTER_FLAW_FILTER(SOLVERS::TokenMustBeAssignedFilter, TokenMustBeAssignedFilter); 
      REGISTER_FLAW_FILTER(SOLVERS::TokenMustBeAssignedFilter, ParentMustBeInsertedFilter); 
      
      SOLVERS::MatchingEngine::addMatchFinder(ConstrainedVariable::entityTypeName(),(new SOLVERS::VariableMatchFinder())->getId()); 
      SOLVERS::MatchingEngine::addMatchFinder(Token::entityTypeName(),(new SOLVERS::TokenMatchFinder())->getId()); 
      
      REGISTER_TOKEN_SORTER(SOLVERS::HSTS::EarlyTokenComparator, early); 
      REGISTER_TOKEN_SORTER(SOLVERS::HSTS::LateTokenComparator, late); 
      REGISTER_TOKEN_SORTER(SOLVERS::HSTS::NearTokenComparator, near); 
      REGISTER_TOKEN_SORTER(SOLVERS::HSTS::FarTokenComparator, far); 
      REGISTER_TOKEN_SORTER(SOLVERS::HSTS::AscendingKeyTokenComparator, ascendingKey);       
  }
  
  void ModuleSolvers::uninitialize(EngineId engine)
  {	 
      PSSolverManager* sm = (PSSolverManager*)engine->removeComponent("PSSolverManager");
      delete sm;
      
      SOLVERS::Component::AbstractFactory::purge();
      SOLVERS::MatchingEngine::purgeAll();
  }  
}
