#ifndef _H_DefaultOpenDecisionManager
#define _H_DefaultOpenDecisionManager

#include "CBPlannerDefs.hh"
#include "OpenDecisionManager.hh"
#include "Token.hh"

namespace Prototype {

  class ObjectDecisionPointComparator {
  public:
    virtual bool operator()(const ObjectDecisionPointId& o1, const ObjectDecisionPointId& o2) const = 0;
    bool operator==(const ObjectDecisionPointComparator& c) { return true; }
  };

  class TokenDecisionPointComparator {
  public:
    virtual bool operator()(const TokenDecisionPointId& t1, const TokenDecisionPointId& t2) const = 0;
    bool operator==(const TokenDecisionPointComparator& c) { return true; }
  };

  class ConstrainedVariableDecisionPointComparator {
  public:
    virtual bool operator()(const ConstrainedVariableDecisionPointId& t1, const ConstrainedVariableDecisionPointId& t2) const =0;
    bool operator==(const ConstrainedVariableDecisionPointComparator& c) { return true; }
  };

  class DefaultObjectDecisionPointComparator : ObjectDecisionPointComparator {
  public:
    bool operator()(const ObjectDecisionPointId& o1, const ObjectDecisionPointId& o2) const {
      return o1->getKey() < o2->getKey();
    }
  };

  class DefaultTokenDecisionPointComparator : TokenDecisionPointComparator {
  public:
    bool operator()(const TokenDecisionPointId& t1, const TokenDecisionPointId& t2) const {
      return t1->getKey() < t2->getKey();
    }
  };

  class DefaultConstrainedVariableDecisionPointComparator : ConstrainedVariableDecisionPointComparator {
  public:
    bool operator()(const ConstrainedVariableDecisionPointId& t1, const ConstrainedVariableDecisionPointId& t2) const {
      return t1->getKey() < t2->getKey();
    }
  };

  typedef std::set<ObjectDecisionPointId, DefaultObjectDecisionPointComparator> ObjectDecisionSet;
  typedef std::set<TokenDecisionPointId, DefaultTokenDecisionPointComparator> TokenDecisionSet;
  typedef std::set<ConstrainedVariableDecisionPointId, DefaultConstrainedVariableDecisionPointComparator> VariableDecisionSet;

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
    virtual void addActive(const TokenId& token);
    virtual void condAdd(const TokenId& token);
    virtual void add(const TokenId& token);
    virtual void condAdd(const ConstrainedVariableId& var, const bool units);
    virtual void add(const ConstrainedVariableId& variable);

    virtual void removeVar(const ConstrainedVariableId& var, const bool deleting);
    virtual const bool removeVarDP(const ConstrainedVariableId& var, const bool deleting, std::map<int,ConstrainedVariableDecisionPointId>& varMap, VariableDecisionSet& sortedVars);
    virtual void condRemoveVar(const ConstrainedVariableId& var);

    virtual void removeActive(const TokenId& tok, const bool deleting);
    virtual void removeToken(const TokenId& tok, const bool deleting);
    virtual const bool removeTokenDP(const TokenId& tok, const bool deleting, std::map<int,TokenDecisionPointId>& tokMap, TokenDecisionSet& sortedToks);
    virtual void removeObject(const ObjectId& object, const TokenId& token, const bool deleting);

    /* the following are the decision caches */
    std::map<int,TokenDecisionPointId> m_tokDecs;
    std::map<int,ConstrainedVariableDecisionPointId> m_nonUnitVarDecs;
    std::map<int,ConstrainedVariableDecisionPointId> m_unitVarDecs;
    std::map<int,ObjectDecisionPointId> m_objDecs;

    /* the following are the heuristically ordered sets of decisions */
    /* note that object decisions are not sorted, and we don't handle unit
       tok decs at the moment. */
    VariableDecisionSet m_sortedUnitVarDecs;
    VariableDecisionSet m_sortedNonUnitVarDecs;
    TokenDecisionSet m_sortedTokDecs;
  };

}

#endif
