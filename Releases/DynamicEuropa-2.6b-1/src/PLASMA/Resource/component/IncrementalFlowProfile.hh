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
      /**
       * @brief
       */
      IncrementalFlowProfile( const PlanDatabaseId db, const FVDetectorId flawDetector, const edouble initLevelLb = 0, const edouble initLevelUb = 0 );
      /**
       * @brief
       */
      virtual ~IncrementalFlowProfile();
      /**
       * @brief
       */
      void initRecompute( InstantId inst );
      /**
       * @brief
       */
      void initRecompute();
      /**
       * @brief
       */
      void recomputeLevels( InstantId prev, InstantId inst );
      /**
       * @brief
       */
      bool enableOrderings(  const InstantId& inst  );
    private:
      void recomputeLevels( InstantId inst, edouble lowerLevel, edouble upperLevel );


    };
}

#endif //INCREMENTAL_FLOW_PROFILE_HEADER__
