#ifndef _H_TemporalVariableCondition
#define _H_TemporalVariableCondition

#include "Condition.hh"
#include "Horizon.hh"

namespace Prototype {

  class TemporalVariableCondition : public Condition {
  public:
    TemporalVariableCondition(const HorizonId& horizon, const DecisionManagerId& dm);
    virtual ~TemporalVariableCondition();

    bool isStartIgnored() const;
    bool isEndIgnored() const;
    bool isDurationIgnored() const;
    bool isTemporalIgnored() const;
    void setIgnoreStart(const bool& start);
    void setIgnoreEnd(const bool& end);
    void setIgnoreDuration(const bool& duration);
    void setIgnoreTemporal(const bool& value);
    void allowHorizonOverlap();
    void disallowHorizonOverlap();
    bool isHorizonOverlapAllowed() const;

    bool test(const EntityId& entity);

    void print (std::ostream& os = std::cout);

  private:
    bool domainContainsHorizonEnd(const ConstrainedVariableId& var);
    bool m_start;
    bool m_end;
    bool m_duration;
    bool m_allowOverlap;
    HorizonId m_horizon;
  };

}
#endif
