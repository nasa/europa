#include "ModuleSolvers.hh"
#include "SolverDefs.hh"
#include "ConstrainedVariable.hh"
#include "ConstraintLibrary.hh"
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

// TODO: is this necessary?
#ifdef near
#undef near
#endif
#ifdef far
#undef far
#endif

namespace EUROPA {

  static bool & SolversInitialized() {
    static bool sl_alreadyDone(false);
    return sl_alreadyDone;
  }

  ModuleSolvers::ModuleSolvers()
      : Module("Solvers")
  {
	  
  }

  ModuleSolvers::~ModuleSolvers()
  {	  
  }  
  
  void ModuleSolvers::initialize()
  {
      if(SolversInitialized())
    	  return;

      REGISTER_SYSTEM_CONSTRAINT(EUROPA::SOLVERS::FlawHandler::VariableListener,EUROPA::SOLVERS::FlawHandler::VariableListener::CONSTRAINT_NAME(),EUROPA::SOLVERS::FlawHandler::VariableListener::PROPAGATOR_NAME());
      
      REGISTER_COMPONENT_FACTORY(EUROPA::SOLVERS::FlawFilter, FlawFilter); 
      
      REGISTER_FLAW_MANAGER(EUROPA::SOLVERS::UnboundVariableManager, UnboundVariableManager); 
      REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::MinValue, StandardVariableHandler); 
      REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::MinValue, Min); 
      REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::MaxValue, Max); 
      REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::HSTS::ValueEnum, ValEnum); 
      
      REGISTER_FLAW_MANAGER(EUROPA::SOLVERS::OpenConditionManager, OpenConditionManager); 
      REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::OpenConditionDecisionPoint, StandardOpenConditionHandler); 
      REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::HSTS::OpenConditionDecisionPoint, HSTSOpenConditionDecisionPoint); 
      
      REGISTER_FLAW_MANAGER(EUROPA::SOLVERS::ThreatManager, ThreatManager); 
      REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::ThreatDecisionPoint, StandardThreatHandler); 
      REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::HSTS::ThreatDecisionPoint, HSTSThreatDecisionPoint); 
      
      REGISTER_FLAW_FILTER(EUROPA::SOLVERS::SingletonFilter, Singleton); 
      REGISTER_FLAW_FILTER(EUROPA::SOLVERS::InfiniteDynamicFilter, InfiniteDynamicFilter); 
      REGISTER_FLAW_FILTER(EUROPA::SOLVERS::HorizonFilter, HorizonFilter); 
      REGISTER_FLAW_FILTER(EUROPA::SOLVERS::HorizonVariableFilter, HorizonVariableFilter); 
      REGISTER_FLAW_FILTER(EUROPA::SOLVERS::MasterMustBeAssignedFilter, MasterMustBeInsertedFilter); 
      REGISTER_FLAW_FILTER(EUROPA::SOLVERS::TokenMustBeAssignedFilter, TokenMustBeAssignedFilter); 
      REGISTER_FLAW_FILTER(EUROPA::SOLVERS::TokenMustBeAssignedFilter, ParentMustBeInsertedFilter); 
      
      EUROPA::SOLVERS::MatchingEngine::addMatchFinder(ConstrainedVariable::entityTypeName(),(new EUROPA::SOLVERS::VariableMatchFinder())->getId()); 
      EUROPA::SOLVERS::MatchingEngine::addMatchFinder(Token::entityTypeName(),(new EUROPA::SOLVERS::TokenMatchFinder())->getId()); 
      
      REGISTER_TOKEN_SORTER(EUROPA::SOLVERS::HSTS::EarlyTokenComparator, early); 
      REGISTER_TOKEN_SORTER(EUROPA::SOLVERS::HSTS::LateTokenComparator, late); 
      REGISTER_TOKEN_SORTER(EUROPA::SOLVERS::HSTS::NearTokenComparator, near); 
      REGISTER_TOKEN_SORTER(EUROPA::SOLVERS::HSTS::FarTokenComparator, far); 
      REGISTER_TOKEN_SORTER(EUROPA::SOLVERS::HSTS::AscendingKeyTokenComparator, ascendingKey); 
      
      SolversInitialized() = true;
  }  

  void ModuleSolvers::uninitialize()
  {
      if(!SolversInitialized())
    	  return;
      
	  // TODO: cleanup  
	  SolversInitialized() = false;
  }  
  
  void ModuleSolvers::initialize(EngineId engine)
  {
  }
  
  void ModuleSolvers::uninitialize(EngineId engine)
  {	 
  }  
}
