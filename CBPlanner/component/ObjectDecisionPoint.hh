#ifndef _H_ObjectDecisionPoint
#define _H_ObjectDecisionPoint

#include "PlannerDefs.hh"
#include "DecisionPoint.hh"
#include "Token.hh" // for the comparator below

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

  /**
   * @brief Token Decision Points need to be compared with object key and
   * token key.  Comparator for ObjectDecisionSet.
   */
  class ObjectDecisionPointComparator {
  public:
    bool operator() (const ObjectDecisionPointId& o1, const ObjectDecisionPointId& o2) const {
      return o1->getEntityKey() == o2->getEntityKey() && o1->getToken()->getKey() == o2->getToken()->getKey();
    }
    bool operator==(const ObjectDecisionPointComparator& c) {return true;}
  };


  std::ostream& operator <<(std::ostream& os, const Id<ObjectDecisionPoint>&);

}
#endif
