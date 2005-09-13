#ifndef _H_HSTSOpenDecisionManager
#define _H_HSTSOpenDecisionManager

#include "CBPlannerDefs.hh"
#include "OpenDecisionManager.hh"
#include "HSTSDefs.hh"

namespace EUROPA {

  class HSTSOpenDecisionManager : public OpenDecisionManager {
  public:

    HSTSOpenDecisionManager(const PlanDatabaseId& db, const HeuristicsEngineId& heur, bool strictHeuristics = true);

    ~HSTSOpenDecisionManager();
    
  protected:

    /**
     * @brief Obtain the heuristic value for the given UnboundVariable.
     * @param var The given unbound variable
     */
    virtual Priority getPriorityForUnboundVariable(const ConstrainedVariableId& var) const;

    /**
     * @brief Obtain the heuristic value for the given inactive token.
     */
    virtual Priority getPriorityForOpenCondition(const TokenId& token) const;

    /**
     * @brief Obtain the heuristic value for the given threatened token (one that requires ordering)
     */
    virtual Priority getPriorityForThreat(const TokenId& token) const;

    virtual void initializeTokenChoices(const TokenDecisionPointId& tdp);

    virtual void initializeVariableChoices(const ConstrainedVariableDecisionPointId& vdp);

    virtual void initializeObjectChoices(const ObjectDecisionPointId& odp);

    HeuristicsEngineId m_heur; /*!< Used to compute priorities */ 

    bool m_strictHeuristics; /*!< if this flag is true, we ignore the implicit deicision heuristic order of
                              object, unit variable, token, non-unit variable and instead follow
                             the heuristics directly, with the exception of preferring unit variable 
			     decisions over all others.*/
  };

}

#endif
