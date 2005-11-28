#include "SolverDefs.hh"
#include "UnboundVariableManager.hh"
#include "OpenConditionManager.hh"
#include "ThreatManager.hh"
#include "MatchingRule.hh"
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
	REGISTER_COMPONENT_FACTORY(MatchingRule, MatchingRule);
	REGISTER_COMPONENT_FACTORY(FlawFilter, FlawFilter);   
	REGISTER_VARIABLE_DECISION_FACTORY(EUROPA::SOLVERS::MinValue, StandardVariableHandler);
	REGISTER_COMPONENT_FACTORY(EUROPA::SOLVERS::UnboundVariableManager, UnboundVariableManager);
	REGISTER_OPENCONDITION_DECISION_FACTORY(EUROPA::SOLVERS::OpenConditionDecisionPoint, 
						StandardOpenConditionHandler);
	REGISTER_COMPONENT_FACTORY(EUROPA::SOLVERS::OpenConditionManager, OpenConditionManager);
	REGISTER_THREAT_DECISION_FACTORY(EUROPA::SOLVERS::ThreatDecisionPoint, StandardThreatHandler);
	REGISTER_COMPONENT_FACTORY(EUROPA::SOLVERS::ThreatManager, ThreatManager);
	sl_registerComponents = true;
      }
    }
  }
}
