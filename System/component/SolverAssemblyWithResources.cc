#include "SolverAssemblyWithResources.hh"

#include "ResourceThreatDecisionPoint.hh"
#include "SAVH_ThreatDecisionPoint.hh"
#include "FlawHandler.hh"
#include "Token.hh"


namespace EUROPA { 

  bool SolverAssemblyWithResources::s_initialized = false;

  SolverAssemblyWithResources::SolverAssemblyWithResources(const SchemaId& schema)
    : StandardAssembly(schema), StandardAssemblyWithResources(schema), SolverAssembly(schema) {
    if(!s_initialized) {
      REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::ResourceThreatDecisionPoint, ResourceThreat);
    }
  }
}
