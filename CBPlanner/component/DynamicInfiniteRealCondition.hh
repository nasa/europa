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
    };

}
#endif
