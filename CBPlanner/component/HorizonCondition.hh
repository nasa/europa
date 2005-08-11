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
   
    /**
     * @brief Specifies that we want to filter tokens that are necessarily outside
     * the planning horizon
     */
    void setNecessarilyOutsideHorizon();
    
    /**
     * @brief Specifies that we want to filter tokens that are necessarily outside
     * the planning horizon
     */
    void setPossiblyOutsideHorizon();
    
    /**
     * @brief returns true iff we are set to use the possibly filter defintion.
     */
    bool isPossiblyOutsideHorizon() const;

   /**
     * @brief returns true iff we are set to use the necessary filter defintion.
     */
    bool isNecessarilyOutsideHorizon() const;

    /**
     * @breif return true if token occurs "within" timeline, false otherwise. 
     * The defintion of "within" used is controlled by the setNecessarilyOutsideHorizon or
     * setPossiblityOutsideHorizon methods.
     * @param tok The id of the token we wish to test. Method will attempt to convert entity
     * to a tokenid. 
     */   

    bool test(const EntityId& entity);

    inline void print (std::ostream& os) { os << "HORIZON_CONDITION"; }
  private:
    bool testToken(const TokenId& tok);

    HorizonId m_horizon;
    bool m_possiblyOutside; /* flag controls which defintion of "within" horizon we use */
  };

} /* namespace Europa */
#endif

