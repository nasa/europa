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

  void ObjectDecisionPoint::initializeChoices() {
    check_error(m_odm.isValid(), "must have a valid open decision manager before initializing choices");
    ObjectDecisionPointId dp(m_id);
    m_odm->initializeObjectChoices(dp);
  }

  const bool ObjectDecisionPoint::assign() {
    if (m_choiceIndex && m_choiceIndex >= m_choices.size()) return false;
    check_error(m_token.isValid());

    if (m_choiceIndex == 0) initializeChoices();

    debugMsg("CBPlanner:ObjectDecisionPoint", "nr of choices = " << m_choices.size());

    if (m_choices.empty()) return false; // unable to find any choices

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

  const bool ObjectDecisionPoint::retract() {
    ObjectId object = m_choices[m_choiceIndex-1].first;
    TokenId predecessor = m_choices[m_choiceIndex-1].second.first;
    TokenId successor = m_choices[m_choiceIndex-1].second.second;
    m_dbClient->free(object, predecessor, successor);
    
    m_open = true;

    if (hasRemainingChoices())
      return true;
    return false;
  }

  const bool ObjectDecisionPoint::hasRemainingChoices() {
    if (m_choiceIndex == 0) return true; // we have never assigned this decision  or initialized choices
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
