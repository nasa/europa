#include "ObjectDecisionPoint.hh"
#include "TokenChoice.hh"
#include "DbClient.hh"
#include "Token.hh"
#include "Object.hh"
#include "Utils.hh"
#include <list>

namespace PLASMA {

  ObjectDecisionPoint::ObjectDecisionPoint(const DbClientId& dbClient, const TokenId& token)
    : DecisionPoint(dbClient, token) {
    m_token = token;
  }

  ObjectDecisionPoint::~ObjectDecisionPoint() { }

  const bool ObjectDecisionPoint::assign(const ChoiceId& choice) { 
    check_error(choice.isValid());
    check_error(Id<TokenChoice>::convertable(choice));
    const Id<TokenChoice>& tChoice = choice;
    check_error(choice.isValid());
    m_dbClient->constrain(tChoice->getObject(), m_token, tChoice->getSuccessor()); 
    check_error(choice.isValid());
    return DecisionPoint::assign(choice);
  }

  const bool ObjectDecisionPoint::retract() {
    check_error(Id<TokenChoice>::convertable(m_current));
    Id<TokenChoice> tChoice = m_current;
    m_dbClient->free(tChoice->getObject(), m_token);
    return DecisionPoint::retract();
  }

  std::list<ChoiceId>& ObjectDecisionPoint::getChoices() {
    check_error(m_id.isValid());
    cleanup(m_choices);
    m_choices.clear();
    std::list<double> values;
    m_token->getObject()->getLastDomain().getValues(values);
    std::list<double>::iterator it = values.begin();
    for ( ; it != values.end(); ++it) {
      ObjectId obj = *it;
      check_error(obj.isValid());
      std::vector<TokenId> successors;
      obj->getOrderingChoices(m_token, successors);
      std::vector<TokenId>::iterator it = successors.begin();
      //std::cout << "Choices for (" << getKey() << "):" << std::endl;
      for (; it != successors.end(); it++) {      
	TokenId token = *it;
	check_error(token.isValid() || token.isNoId());
	ChoiceId choice = (new TokenChoice(m_id, obj, token))->getId();
	//std::cout << choice << std::endl;
	m_choices.push_back(choice);
      }
    }
    return DecisionPoint::getChoices();
  }

  const TokenId& ObjectDecisionPoint::getToken() const { 
    return m_token;
  }

  void ObjectDecisionPoint::print(std::ostream& os) const {
    check_error(m_id.isValid());
    if (!m_token.isNoId()) {
      os << "(" << getKey() << ") Object Token (" << m_entityKey << ") ";
    }
    else {
      os << "(" << getKey() << ") Object Token (" << m_entityKey << ")  [deleted] ";
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
