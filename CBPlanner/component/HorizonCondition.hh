#ifndef _H_HorizonCondition
#define _H_HorizonCondition

#include "Condition.hh"
#include "Horizon.hh"
#include <map>
#include <set>

namespace EUROPA {

  class HorizonCondition : public Condition {
  public:
    HorizonCondition(const HorizonId& hor, const DecisionManagerId& dm);
    virtual ~HorizonCondition();

    void setNecessarilyOutsideHorizon();
    void setPossiblyOutsideHorizon();
    bool isPossiblyOutsideHorizon() const;
    bool isNecessarilyOutsideHorizon() const;

    bool test(const EntityId& entity);

    inline void print (std::ostream& os) { os << "HORIZON_CONDITION"; }
  private:
    bool testToken(const TokenId& tok);

    HorizonId m_horizon;
    bool m_possiblyOutside;
  };

} /* namespace Europa */
#endif

