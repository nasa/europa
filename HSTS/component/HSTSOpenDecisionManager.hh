#ifndef _H_HSTSOpenDecisionManager
#define _H_HSTSOpenDecisionManager

#include "CBPlannerDefs.hh"
#include "DefaultOpenDecisionManager.hh"
#include "HSTSHeuristics.hh"

namespace EUROPA {

  typedef std::set<ObjectDecisionPointId, DefaultObjectDecisionPointComparator> ObjectDecisionSet;

  class HSTSOpenDecisionManager : public DefaultOpenDecisionManager {
  public:

    HSTSOpenDecisionManager(const DecisionManagerId& dm, const HSTSHeuristicsId& heur, const bool strictHeuristics = false);
    ~HSTSOpenDecisionManager();

    virtual DecisionPointId getNextDecision();

    virtual void initializeTokenChoices(TokenDecisionPointId& tdp);
    virtual void initializeVariableChoices(ConstrainedVariableDecisionPointId& vdp);
    virtual void initializeObjectChoices(ObjectDecisionPointId& odp);
    
  protected:
    friend class DecisionManager;

    // need to add sorted object decs
    virtual void cleanupAllDecisionCaches();

    virtual void addActive(const TokenId& token);
    virtual void condAddActive(const TokenId& token);
    virtual void removeActive(const TokenId& tok, const bool deleting);

    virtual void getBestObjectDecision(DecisionPointId& bestDec, HSTSHeuristics::Priority& bestp);
    virtual void getBestTokenDecision(DecisionPointId& bestDec, HSTSHeuristics::Priority& bestp);
    virtual void getBestUnitVariableDecision(DecisionPointId& bestDec, HSTSHeuristics::Priority& bestp);
    virtual void getBestNonUnitVariableDecision(DecisionPointId& bestDec, HSTSHeuristics::Priority& bestp);
    DecisionPointId getNextDecisionLoose();
    DecisionPointId getNextDecisionStrict();

    ObjectDecisionSet m_sortedObjectDecs;

    HSTSHeuristicsId m_heur;
    bool m_strictHeuristics; /*<! if this flag is true, we ignore the implicit deicision heuristic order of
                              object, unit variable, token, non-unit variable and instead follow
                             the heuristics directly, with the exception of preferring unit variable decisions over all others.*/
  };

}

#endif
