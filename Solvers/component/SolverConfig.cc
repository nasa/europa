#include "SolverDefs.hh"
#include "UnboundVariableManager.hh"
#include "OpenConditionManager.hh"
#include "ThreatManager.hh"
#include "FlawFilter.hh"
#include "Filters.hh"
#include "Token.hh"
#include "ConstrainedVariable.hh"

namespace EUROPA {
  namespace SOLVERS {

    /**
     * @brief Register default components
     */
    SolverConfig::SolverConfig(){
      static bool sl_registerComponents = false;
      check_error(sl_registerComponents == false, "Should only be called once.");
      if(sl_registerComponents == false){
	REGISTER_COMPONENT_FACTORY(FlawFilter, FlawFilter);

	REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::MinValue, StandardVariableHandler);
	REGISTER_FLAW_MANAGER(EUROPA::SOLVERS::UnboundVariableManager, UnboundVariableManager);

	REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::OpenConditionDecisionPoint, StandardOpenConditionHandler);
	REGISTER_FLAW_MANAGER(EUROPA::SOLVERS::OpenConditionManager, OpenConditionManager);

	REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::ThreatDecisionPoint, StandardThreatHandler);
	REGISTER_FLAW_MANAGER(EUROPA::SOLVERS::ThreatManager, ThreatManager);

	REGISTER_FLAW_FILTER(EUROPA::SOLVERS::SingletonFilter, Singleton);
	REGISTER_FLAW_FILTER(EUROPA::SOLVERS::HorizonFilter, HorizonFilter);
	REGISTER_FLAW_FILTER(EUROPA::SOLVERS::InfiniteDynamicFilter, InfiniteDynamicFilter);
	REGISTER_FLAW_FILTER(EUROPA::SOLVERS::HorizonVariableFilter, HorizonVariableFilter);
	sl_registerComponents = true;
      }
    }
  }
}
