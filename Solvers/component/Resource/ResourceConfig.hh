#ifndef _H_ResourceConfig
#define _H_ResourceConfig

#include "SolverConfig.hh"
#include "SAVH_ThreatDecisionPoint.hh"
#include "SAVH_ThreatManager.hh"

#define RESOURCE_REGISTRATIONS { \
REGISTER_FLAW_HANDLER(SAVH::ThreatDecisionPoint, SAVHThreatHandler); \
REGISTER_FLAW_MANAGER(SAVH::ThreatManager, SAVHThreatManager); \
}

#endif
