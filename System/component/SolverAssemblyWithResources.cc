#include "SolverAssemblyWithResources.hh"

#include "FlawHandler.hh"
#include "ResourceThreatDecisionPoint.hh"
#include "SAVH_ThreatDecisionPoint.hh"

#include "SAVH_ReusableFVDetector.hh"
#include "SAVH_TimetableFVDetector.hh"

#include "Token.hh"


namespace EUROPA { 

  bool SolverAssemblyWithResources::s_initialized = false;

  SolverAssemblyWithResources::SolverAssemblyWithResources(const SchemaId& schema)
    : StandardAssembly(schema), StandardAssemblyWithResources(schema), SolverAssembly(schema) {
    if(!s_initialized) {
   	  REGISTER_FVDETECTOR(EUROPA::SAVH::ReusableFVDetector, ReusableFVDetector );
   	  REGISTER_FVDETECTOR(EUROPA::SAVH::TimetableFVDetector, TimetableFVDetector );
   	  REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::ResourceThreatDecisionPoint, ResourceThreat);    	  
   	  s_initialized = true;
    }
  }
}
