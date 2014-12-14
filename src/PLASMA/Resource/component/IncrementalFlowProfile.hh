#ifndef INCREMENTAL_FLOW_PROFILE_HEADER__
#define INCREMENTAL_FLOW_PROFILE_HEADER__

/**
 * @file IncrementalFlowProfile.hh
 * @author David Rijsman
 * @brief Defines the public interface for a maximum flow algorithm
 * @date April 2006
 * @ingroup Resource
 */

#include "FlowProfile.hh"

namespace EUROPA
{
    class IncrementalFlowProfile:
      public FlowProfile
    {
    public:
      IncrementalFlowProfile( const PlanDatabaseId db, const FVDetectorId flawDetector);
      virtual ~IncrementalFlowProfile();
      void initRecompute( InstantId inst );
      void initRecompute();
      void recomputeLevels( InstantId prev, InstantId inst );
      bool enableOrderings(  const InstantId inst  );
    private:
      void recomputeLevels( InstantId inst, edouble lowerLevel, edouble upperLevel );


    };
}

#endif //INCREMENTAL_FLOW_PROFILE_HEADER__
