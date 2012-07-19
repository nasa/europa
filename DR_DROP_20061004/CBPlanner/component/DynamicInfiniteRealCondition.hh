#ifndef _H_DynamicInfiniteRealCondition
#define _H_DynamicInfiniteRealCondition

#include "Condition.hh"

namespace EUROPA {

    class DynamicInfiniteRealCondition : public Condition {
    public:
      DynamicInfiniteRealCondition(const DecisionManagerId& dm);
      virtual ~DynamicInfiniteRealCondition();

      bool test(const EntityId& entity);
      void print (std::ostream& os = std::cout);
      
      // Turn off dynamic exclusion. This should be done before planning begins.
      void disableDynamicExclusion();
      bool isDynamicExclusionEnabled();

    private:
      bool m_applyDynamicExclusion; // flag to control application of dynamic exclusinon policy
    };

}
#endif
