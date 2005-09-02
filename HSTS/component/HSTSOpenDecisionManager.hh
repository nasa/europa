#ifndef _H_HSTSOpenDecisionManager
#define _H_HSTSOpenDecisionManager

#include "CBPlannerDefs.hh"
#include "OpenDecisionManager.hh"
#include "HSTSHeuristics.hh"

namespace EUROPA {

  class HSTSOpenDecisionManager : public OpenDecisionManager {
  public:

    HSTSOpenDecisionManager(const PlanDatabaseId& db, const HSTSHeuristicsId& heur, bool strictHeuristics = true);
    ~HSTSOpenDecisionManager();

    virtual DecisionPointId getNextDecision();
    
  protected:

    virtual void initializeTokenChoices(const TokenDecisionPointId& tdp);

    virtual void initializeVariableChoices(const ConstrainedVariableDecisionPointId& vdp);

    virtual void initializeObjectChoices(const ObjectDecisionPointId& odp);

    /**
     * @brief Retrieve a decision that beats the given current best priority.
     * @param bestp The current best priority. May update this if we find a better decision.
     * @return A noId if no better option, otherwise a good decision point that needs initilization.
     */
    virtual DecisionPointId getBestObjectDecision(HSTSHeuristics::Priority& bestp);

    /**
     * @brief Retrieve a decision that beats the given current best priority.
     * @param bestp The current best priority. May update this if we find a better decision.
     * @return A noId if no better option, otherwise a good decision point that needs initilization.
     */
    virtual DecisionPointId getBestTokenDecision(HSTSHeuristics::Priority& bestp);

    /**
     * @brief Retrieve a decision that beats the given current best priority.
     * @param bestp The current best priority. May update this if we find a better decision.
     * @return A noId if no better option, otherwise a good decision point that needs initilization.
     */
    virtual DecisionPointId getBestNonUnitVariableDecision(HSTSHeuristics::Priority& bestp);

    DecisionPointId getNextDecisionLoose();

    DecisionPointId getNextDecisionStrict();

    HSTSHeuristicsId m_heur;
    bool m_strictHeuristics; /*<! if this flag is true, we ignore the implicit deicision heuristic order of
                              object, unit variable, token, non-unit variable and instead follow
                             the heuristics directly, with the exception of preferring unit variable decisions over all others.*/
  };

}

#endif
