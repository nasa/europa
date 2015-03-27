#ifndef H_BOOST_FLOW_PROFILE
#define H_BOOST_FLOW_PROFILE
#include "FlowProfile.hh"
#include "BoostFlowProfileGraph.hh"
namespace EUROPA {

class BoostFlowProfile : public FlowProfile {
 public:
  BoostFlowProfile(const PlanDatabaseId db, const FVDetectorId flawDetector)
      : FlowProfile(db, flawDetector) {
    initializeGraphs<EUROPA::BoostFlowProfileGraph>();
  }
};
}

#endif
