#ifndef _H_HSTSOpenDecisionManager
#define _H_HSTSOpenDecisionManager

#include "CBPlannerDefs.hh"
#include "DefaultOpenDecisionManager.hh"
#include "HeuristicsEngine.hh"

namespace EUROPA {

  typedef std::set<ObjectDecisionPointId, DefaultObjectDecisionPointComparator> ObjectDecisionSet;

  class HSTSOpenDecisionManager : public DefaultOpenDecisionManager {
  public:

    HSTSOpenDecisionManager(const DecisionManagerId& dm, const HeuristicsEngineId& heur, const bool strictHeuristics = false);
    ~HSTSOpenDecisionManager();

    virtual DecisionPointId getNextDecision();


    /**
     * Helper method to establish the base set of token choices. No Pruning.
     */
    void initializeTokenChoicesInternal(const TokenDecisionPointId& tdp);

    /**
     * Helper method to establish the base set of object choices. No Pruning.
     */
    void initializeObjectChoicesInternal(const ObjectDecisionPointId& odp);

    virtual void initializeTokenChoices(const TokenDecisionPointId& tdp);
    virtual void initializeVariableChoices(const ConstrainedVariableDecisionPointId& vdp);
    virtual void initializeObjectChoices(const ObjectDecisionPointId& odp);

    /**
     * @brief Tests if configured to use auto allocation of a new
     * active token as a proxy when resolving an open condition.
     */
    bool isAutoAllocationEnabled() const;
    
  protected:
    friend class DecisionManager;

    // need to add sorted object decs
    virtual void cleanupAllDecisionCaches();

    virtual void addActive(const TokenId& token);
    virtual void condAddActive(const TokenId& token);
    virtual void removeActive(const TokenId& tok, const bool deleting);

    virtual void getBestObjectDecision(DecisionPointId& bestDec, Priority& bestp);
    virtual void getBestTokenDecision(DecisionPointId& bestDec, Priority& bestp);
    virtual void getBestUnitVariableDecision(DecisionPointId& bestDec, Priority& bestp);
    virtual void getBestNonUnitVariableDecision(DecisionPointId& bestDec, Priority& bestp);
    DecisionPointId getNextDecisionLoose();
    DecisionPointId getNextDecisionStrict();

    ObjectDecisionSet m_sortedObjectDecs;

    HeuristicsEngineId m_heur;
    bool m_strictHeuristics; /*<! if this flag is true, we ignore the implicit deicision heuristic order of
                              object, unit variable, token, non-unit variable and instead follow
                             the heuristics directly, with the exception of preferring unit variable decisions over all others.*/
  };

}

#endif
