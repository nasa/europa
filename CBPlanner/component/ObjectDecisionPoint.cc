#include "ObjectDecisionPoint.hh"
#include "TokenChoice.hh"
#include "DbClient.hh"
#include "Token.hh"
#include "Object.hh"
#include "Utils.hh"
#include <list>

namespace Prototype {

  ObjectDecisionPoint::ObjectDecisionPoint(const DbClientId& dbClient, const EntityId& entity, const TokenId& token)
    : DecisionPoint(dbClient, entity) {
    m_object = entity;
    m_token = token;
  }

  ObjectDecisionPoint::~ObjectDecisionPoint() { }

  const bool ObjectDecisionPoint::assign(const ChoiceId& choice) { 
    check_error(!choice.isNoId());
    check_error(Id<TokenChoice>::convertable(choice));
    m_dbClient->constrain(m_object, m_token, Id<TokenChoice>(choice)->getToken()); 
    return DecisionPoint::assign(choice);
  }

  const bool ObjectDecisionPoint::retract() {
    m_dbClient->free(m_object, m_token);
    return DecisionPoint::retract();
  }

  std::list<ChoiceId>& ObjectDecisionPoint::getChoices() {
    check_error(m_id.isValid());
    cleanup(m_choices);
    m_choices.clear();
    std::vector<TokenId> successors;
    m_object->getOrderingChoices(m_token, successors);
    std::vector<TokenId>::iterator it = successors.begin();
    //std::cout << "Choices for (" << getKey() << "):" << std::endl;
    for (; it != successors.end(); it++) {      
      TokenId token = *it;
      ChoiceId choice = Choice::makeChoiceId(m_id, token);
      //std::cout << choice << std::endl;
      m_choices.push_back(choice);
    }
    return DecisionPoint::getChoices();
  }

  const TokenId& ObjectDecisionPoint::getToken() const { 
    return m_token;
  }

  const ObjectId& ObjectDecisionPoint::getObject() const { 
    return m_object;
  }

  void ObjectDecisionPoint::print(std::ostream& os) const {
    check_error(m_id.isValid());
    if (!m_object.isNoId()) {
      os << "(" << getKey() << ") Object (" << m_entityKey << ") ";
      os << "Token (" << m_token->getKey() << ") ";
    }
    else {
      os << "(" << getKey() << ") Object (" << m_entityKey << ") ";
      os << "Token (" << m_token->getKey() << ") Flaw [deleted] ";
    }
    os << " Current Choice: " << m_current;
    os << " Discarded: " << m_discarded.size();
  }

  std::ostream& operator <<(std::ostream& os, const Id<ObjectDecisionPoint>& decision) {
    if (decision.isNoId())
      os << " No Decision ";
    else 
      decision->print(os);
    return(os);
  }

}
