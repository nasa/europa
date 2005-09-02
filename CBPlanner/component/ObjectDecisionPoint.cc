#include "ObjectDecisionPoint.hh"
#include "OpenDecisionManager.hh"
#include "DbClient.hh"
#include "Token.hh"
#include "Object.hh"
#include "Utils.hh"
#include "Debug.hh"

#include <list>

namespace EUROPA {

  ObjectDecisionPoint::ObjectDecisionPoint(const DbClientId& dbClient, const TokenId& token, const OpenDecisionManagerId& odm)
    : DecisionPoint(dbClient, token) {
    m_token = token;
    m_choiceIndex = 0;
    m_odm = odm;
  }

  ObjectDecisionPoint::~ObjectDecisionPoint() { }

  bool ObjectDecisionPoint::assign() {
    checkError(m_choiceIndex < m_choices.size(), "Choice index cannot exceeed choices. Bug in control loop.");
    checkError(!m_choices.empty(), "Should never call to assign if there are no coices. Bug in control loop.");
    check_error(m_token.isValid());

    debugMsg("CBPlanner:ObjectDecisionPoint", "nr of choices = " << m_choices.size());

    m_open = false;

    ObjectId object = m_choices[m_choiceIndex].first;
    TokenId predecessor = m_choices[m_choiceIndex].second.first;
    TokenId successor = m_choices[m_choiceIndex].second.second;
    checkError(predecessor == m_token || successor == m_token,
	       "Given token must be part of assignment.");
    m_dbClient->constrain(object, predecessor, successor);
    m_choiceIndex++;
    return true;
  }

  bool ObjectDecisionPoint::retract() {
    checkError(m_choiceIndex > 0, "Choice index must be at least 1. Bug in control loop.");
    ObjectId object = m_choices[m_choiceIndex-1].first;
    TokenId predecessor = m_choices[m_choiceIndex-1].second.first;
    TokenId successor = m_choices[m_choiceIndex-1].second.second;
    m_dbClient->free(object, predecessor, successor);
    
    m_open = true;
    return true;
  }

  bool ObjectDecisionPoint::hasRemainingChoices() {
    return m_choiceIndex < m_choices.size();
  }

  const TokenId& ObjectDecisionPoint::getToken() const {
    return m_token;
  }

  void ObjectDecisionPoint::print(std::ostream& os) const {
    check_error(m_id.isValid());
    if (!m_token.isNoId()) {
      os << "(" << getKey() << ") Object Token (" << m_entityKey << ") ";
      if (m_choiceIndex == 0) os << " Current Choice:  No Choice ";
      else { 
	os << " Current Choice: on Object (" << m_choices[m_choiceIndex-1].first->getKey() << ") ";
	os << " constrained (" << m_choices[m_choiceIndex-1].second.first->getKey() << ") and (";
	os << m_choices[m_choiceIndex-1].second.second->getKey() << ") ";
      }
      os << " Discarded: " << m_choiceIndex;
    }
    else {
      os << "(" << getKey() << ") Object Token (" << m_entityKey << ")  [deleted]  ";
    }
  }

  std::ostream& operator <<(std::ostream& os, const Id<ObjectDecisionPoint>& decision) {
    if (decision.isNoId())
      os << " No Decision ";
    else
      decision->print(os);
    return(os);
  }
}
