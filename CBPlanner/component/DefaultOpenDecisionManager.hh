#ifndef _H_DefaultOpenDecisionManager
#define _H_DefaultOpenDecisionManager

#include "CBPlannerDefs.hh"
#include "OpenDecisionManager.hh"

namespace Prototype {

  class DefaultOpenDecisionManager : public OpenDecisionManager {
  public:

    DefaultOpenDecisionManager(const DecisionManagerId& dm);
    ~DefaultOpenDecisionManager();

    virtual DecisionPointId& getNextDecision();
    virtual const ChoiceId& getNextChoice();
    virtual const int getNumberOfDecisions();

    virtual void getOpenDecisions(std::list<DecisionPointId>& decisions);
    virtual void printOpenDecisions(std::ostream& os = std::cout);
  protected:
    friend class DecisionManager;

    virtual void cleanupAllDecisionCaches();

    virtual void deleteAllMatchingObjects(const ObjectId& object, const TokenId& token);
    virtual void add(const ObjectId& object, const TokenId& token);
    virtual void add(const ObjectId& object);
    virtual void condAdd(const TokenId& token);
    virtual void add(const TokenId& token);
    virtual void condAdd(const ConstrainedVariableId& var, const bool units);
    virtual void add(const ConstrainedVariableId& variable);

    virtual void removeVar(const ConstrainedVariableId& var, const bool deleting);
    virtual const bool removeVarDP(const ConstrainedVariableId& var, const bool deleting, std::map<int,ConstrainedVariableDecisionPointId>& varMap, VariableDecisionSet& sortedVars);
    virtual void condRemoveVar(const ConstrainedVariableId& var);

    virtual void removeToken(const TokenId& tok, const bool deleting);
    virtual const bool removeTokenDP(const TokenId& tok, const bool deleting, std::map<int,TokenDecisionPointId>& tokMap, TokenDecisionSet& sortedToks);
    virtual void removeObject(const ObjectId& object, const TokenId& token, const bool deleting);

    /* the following are the decision caches */
    std::map<int,TokenDecisionPointId> m_nonUnitTokDecs;
    std::map<int,TokenDecisionPointId> m_unitTokDecs;
    std::map<int,ConstrainedVariableDecisionPointId> m_nonUnitVarDecs;
    std::map<int,ConstrainedVariableDecisionPointId> m_unitVarDecs;
    std::multimap<int,ObjectDecisionPointId> m_objDecs;

    /* the following are the heuristically ordered sets of decisions */
    /* note that object decisions are not sorted */
    VariableDecisionSet m_sortedUnitVarDecs;
    VariableDecisionSet m_sortedNonUnitVarDecs;
    TokenDecisionSet m_sortedUnitTokDecs;
    TokenDecisionSet m_sortedNonUnitTokDecs;
  };

}

#endif
