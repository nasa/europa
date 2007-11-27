#ifndef _H_SolverConfig
#define _H_SolverConfig

#include "SolverDefs.hh"
#include "ConstrainedVariable.hh"
#include "ConstraintLibrary.hh"
#include "ComponentFactory.hh"
#include "Filters.hh"
#include "FlawFilter.hh"
#include "FlawHandler.hh"
#include "HSTSDecisionPoints.hh"
#include "OpenConditionManager.hh"
#include "ThreatManager.hh"
#include "Token.hh"
#include "UnboundVariableManager.hh"

#define BASE_REGISTRATIONS { \
\
REGISTER_SYSTEM_CONSTRAINT(FlawHandler::VariableListener,FlawHandler::VariableListener::CONSTRAINT_NAME(),FlawHandler::VariableListener::PROPAGATOR_NAME());\
\
REGISTER_COMPONENT_FACTORY(EUROPA::SOLVERS::FlawFilter, FlawFilter); \
\
REGISTER_FLAW_MANAGER(EUROPA::SOLVERS::UnboundVariableManager, UnboundVariableManager); \
REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::MinValue, StandardVariableHandler); \
\
REGISTER_FLAW_MANAGER(EUROPA::SOLVERS::OpenConditionManager, OpenConditionManager); \
REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::OpenConditionDecisionPoint, StandardOpenConditionHandler); \
\
REGISTER_FLAW_MANAGER(EUROPA::SOLVERS::ThreatManager, ThreatManager); \
REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::ThreatDecisionPoint, StandardThreatHandler); \
\
REGISTER_FLAW_FILTER(EUROPA::SOLVERS::SingletonFilter, Singleton); \
REGISTER_FLAW_FILTER(EUROPA::SOLVERS::HorizonFilter, HorizonFilter); \
REGISTER_FLAW_FILTER(EUROPA::SOLVERS::InfiniteDynamicFilter, InfiniteDynamicFilter); \
REGISTER_FLAW_FILTER(EUROPA::SOLVERS::HorizonVariableFilter, HorizonVariableFilter); \
\
MatchingEngine::addMatchFinder(ConstrainedVariable::entityTypeName(),(new VariableMatchFinder())->getId()); \
MatchingEngine::addMatchFinder(Token::entityTypeName(),(new TokenMatchFinder())->getId()); \
\
REGISTER_TOKEN_SORTER(HSTS::EarlyTokenComparator, early); \
REGISTER_TOKEN_SORTER(HSTS::LateTokenComparator, late); \
REGISTER_TOKEN_SORTER(HSTS::NearTokenComparator, near); \
REGISTER_TOKEN_SORTER(HSTS::FarTokenComparator, far); \
REGISTER_TOKEN_SORTER(HSTS::AscendingKeyTokenComparator, ascendingKey); \
}

#endif
