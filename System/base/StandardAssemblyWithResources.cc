#include "StandardAssemblyWithResources.hh"

// #include "Resource.hh"
// #include "ResourceConstraint.hh"
// #include "ResourceDefs.hh"
#include "ResourcePropagator.hh"
// #include "Transaction.hh"

// #include "SAVH_Profile.hh"
// #include "SAVH_FVDetector.hh"
// #include "SAVH_TimetableProfile.hh"
// #include "SAVH_TimetableFVDetector.hh"
#include "SAVH_ProfilePropagator.hh"
// #include "SAVH_FlowProfile.hh"
// #include "SAVH_IncrementalFlowProfile.hh"


namespace EUROPA {

  StandardAssemblyWithResources::StandardAssemblyWithResources(const SchemaId& schema)
    : StandardAssembly(schema) {
    new ResourcePropagator(LabelStr("Resource"), m_constraintEngine, m_planDatabase);
    new SAVH::ProfilePropagator(LabelStr("SAVH_Resource"), m_constraintEngine);
  }
}
