#ifndef _H_HSTSOpenDecisionManager
#define _H_HSTSOpenDecisionManager

#include "CBPlannerDefs.hh"
#include "DefaultOpenDecisionManager.hh"
#include "HSTSHeuristics.hh"

namespace PLASMA {

  typedef std::set<ObjectDecisionPointId, DefaultObjectDecisionPointComparator> ObjectDecisionSet;

  class HSTSOpenDecisionManager : public DefaultOpenDecisionManager {
  public:

    HSTSOpenDecisionManager(const DecisionManagerId& dm, const HSTSHeuristicsId& heur);
    ~HSTSOpenDecisionManager();

    virtual DecisionPointId getNextDecision();
    virtual const ChoiceId getNextChoice();

  protected:
    friend class DecisionManager;

    // need to add sorted object decs
    virtual void cleanupAllDecisionCaches();

    virtual void addActive(const TokenId& token);
    virtual void condAddActive(const TokenId& token);
    virtual void removeActive(const TokenId& tok, const bool deleting);

    virtual void getBestObjectDecision(DecisionPointId& bestDec, HSTSHeuristics::Priority& bestp);
    virtual void getBestTokenDecision(DecisionPointId& bestDec, HSTSHeuristics::Priority& bestp);
    virtual void getBestVariableDecision(DecisionPointId& bestDec, HSTSHeuristics::Priority& bestp);

    virtual void HSTSOpenDecisionManager::initializeNumberToBeat(const HSTSHeuristics::CandidateOrder& order, int& numberToBeat);

    virtual void HSTSOpenDecisionManager::compareTokensAccordingToOrder(const HSTSHeuristics::CandidateOrder& order, const ChoiceId& choice, ChoiceId& bestChoice, const int est, const int lst, int& numberToBeat); 

    ObjectDecisionSet m_sortedObjectDecs;

    HSTSHeuristicsId m_heur;
  };

}

#endif
