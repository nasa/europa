#ifndef _H_SolverConfig
#define _H_SolverConfig

#include "SolverDefs.hh"
#include "UnboundVariableManager.hh"
#include "OpenConditionManager.hh"
#include "ThreatManager.hh"
#include "FlawFilter.hh"
#include "Filters.hh"
#include "Token.hh"
#include "ConstrainedVariable.hh"
#include "ComponentFactory.hh"

#define BASE_REGISTRATIONS { \
    REGISTER_COMPONENT_FACTORY(EUROPA::SOLVERS::FlawFilter, FlawFilter); \
\
REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::MinValue, StandardVariableHandler); \
REGISTER_FLAW_MANAGER(EUROPA::SOLVERS::UnboundVariableManager, UnboundVariableManager); \
 \
REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::OpenConditionDecisionPoint, StandardOpenConditionHandler); \
REGISTER_FLAW_MANAGER(EUROPA::SOLVERS::OpenConditionManager, OpenConditionManager); \
 \
REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::ThreatDecisionPoint, StandardThreatHandler); \
REGISTER_FLAW_MANAGER(EUROPA::SOLVERS::ThreatManager, ThreatManager); \
 \
REGISTER_FLAW_FILTER(EUROPA::SOLVERS::SingletonFilter, Singleton); \
REGISTER_FLAW_FILTER(EUROPA::SOLVERS::HorizonFilter, HorizonFilter); \
REGISTER_FLAW_FILTER(EUROPA::SOLVERS::InfiniteDynamicFilter, InfiniteDynamicFilter); \
REGISTER_FLAW_FILTER(EUROPA::SOLVERS::HorizonVariableFilter, HorizonVariableFilter); \
}

#endif
