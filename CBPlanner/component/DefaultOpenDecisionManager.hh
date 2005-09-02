#ifndef _H_DefaultOpenDecisionManager
#define _H_DefaultOpenDecisionManager

#include "CBPlannerDefs.hh"
#include "OpenDecisionManager.hh"
#include "ObjectDecisionPoint.hh"
#include "TokenDecisionPoint.hh"
#include "ConstrainedVariableDecisionPoint.hh"
#include "Token.hh"
#include "Utils.hh"

namespace EUROPA {

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

    DefaultOpenDecisionManager(const PlanDatabaseId& db);
    ~DefaultOpenDecisionManager();

    virtual DecisionPointId getNextDecision();
    
    virtual void initializeTokenChoices(TokenDecisionPointId& tdp);
    virtual void initializeVariableChoices(ConstrainedVariableDecisionPointId& vdp);
    virtual void initializeObjectChoices(ObjectDecisionPointId& odp);
  };

}

#endif
