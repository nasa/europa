#ifndef _H_TemporalVariableFilter
#define _H_TemporalVariableFilter

#include "Condition.hh"
#include <map>
#include <set>

namespace EUROPA {

  /**
   * @brief A static filter to exclude temporal variables as flaws.
   */
  class TemporalVariableFilter : public Condition {
  public:
    TemporalVariableFilter(const DecisionManagerId& dm);

    virtual ~TemporalVariableFilter();
   
    bool test(const EntityId& entity);

    inline void print (std::ostream& os) { os << "TEMPORAL_VARIABLE_FILTER"; }

    static bool isTemporalVariable(const EntityId& entity);
  };

} /* namespace Europa */
#endif

