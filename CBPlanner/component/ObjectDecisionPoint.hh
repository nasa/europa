#ifndef _H_ObjectDecisionPoint
#define _H_ObjectDecisionPoint

#include "CBPlannerDefs.hh"
#include "DecisionPoint.hh"

namespace Prototype {

  class ObjectDecisionPoint : public DecisionPoint {
  public:
    virtual ~ObjectDecisionPoint();

    const bool assign(const ChoiceId&);
    const bool retract();
    std::list<ChoiceId>& getChoices();
    const TokenId& getToken() const;
    const ObjectId& getObject() const;

    void print(std::ostream& os) const;
  private:
    friend class OpenDecisionManager;

    ObjectDecisionPoint(const DbClientId& dbClient, const EntityId&, const TokenId&);
    const bool testIfExhausted();

    ObjectId m_object;
    TokenId m_token;
  };

  std::ostream& operator <<(std::ostream& os, const Id<ObjectDecisionPoint>&);

}
#endif
