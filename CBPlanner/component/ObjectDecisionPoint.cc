#include "ObjectDecisionPoint.hh"
#include "TokenChoice.hh"
#include "DbClient.hh"
#include "Token.hh"
#include "Object.hh"
#include "Utils.hh"
#include "Debug.hh"

#include <list>

namespace EUROPA {

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
    m_dbClient->constrain(tChoice->getObject(), tChoice->getPredecessor(), tChoice->getSuccessor());
    check_error(choice.isValid());
    return DecisionPoint::assign(choice);
  }

  const bool ObjectDecisionPoint::retract() {
    check_error(Id<TokenChoice>::convertable(m_current));
    Id<TokenChoice> tChoice = m_current;
    m_dbClient->free(tChoice->getObject(), tChoice->getPredecessor(), tChoice->getSuccessor());
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
      std::vector<std::pair<TokenId, TokenId> > tuples;
      obj->getOrderingChoices(m_token, tuples);
      std::vector<std::pair<TokenId, TokenId> >::iterator it = tuples.begin();
      debugMsg("ObjectDecisionPoint:getChoices", "Choices constraining (" << m_token->getKey() << ")");
      for (; it != tuples.end(); it++) {
	TokenId predecessor = it->first;
	TokenId successor = it->second;
	check_error(predecessor.isValid());
	check_error(successor.isValid());
	ChoiceId choice = (new TokenChoice(m_id, obj, predecessor, successor))->getId();
	debugMsg("ObjectDecisionPoint:getChoices", "  constrain(" << predecessor->getKey() << ", " << successor->getKey() << ")");
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
