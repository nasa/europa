#ifndef _H_ResourceConfig
#define _H_ResourceConfig

#include "SolverConfig.hh"
#include "ResourceMatching.hh"
#include "ResourceThreatDecisionPoint.hh"
#include "SAVH_Instant.hh"
#include "SAVH_ThreatDecisionPoint.hh"
#include "SAVH_ThreatManager.hh"

#define RESOURCE_REGISTRATIONS { \
REGISTER_FLAW_MANAGER(SAVH::ThreatManager, SAVHThreatManager); \
REGISTER_FLAW_HANDLER(SAVH::ThreatDecisionPoint, SAVHThreatHandler); \
REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::ResourceThreatDecisionPoint, ResourceThreat); \
MatchingEngine::addMatchFinder(SAVH::Instant::entityTypeName(),(new InstantMatchFinder())->getId()); \
}

#endif
