#include "StandardAssemblyWithResources.hh"

#include "ResourcePropagator.hh"
#include "SAVH_ProfilePropagator.hh"

#include "SAVH_FlowProfile.hh"
#include "SAVH_IncrementalFlowProfile.hh"
#include "SAVH_TimetableProfile.hh"


namespace EUROPA {

  StandardAssemblyWithResources::StandardAssemblyWithResources(const SchemaId& schema)
    : StandardAssembly(schema) {
    new ResourcePropagator(LabelStr("Resource"), m_constraintEngine, m_planDatabase);
    new SAVH::ProfilePropagator(LabelStr("SAVH_Resource"), m_constraintEngine);
 	REGISTER_PROFILE(EUROPA::SAVH::TimetableProfile, TimetableProfile );
    REGISTER_PROFILE(EUROPA::SAVH::FlowProfile, FlowProfile);
 	REGISTER_PROFILE(EUROPA::SAVH::IncrementalFlowProfile, IncrementalFlowProfile );
  }
}
