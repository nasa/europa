#include "PlanDatabase.hh"
#include "TokenDecisionPoint.hh"
#include "OpenDecisionManager.hh"
#include "Token.hh"
#include "DbClient.hh"
#include "Utils.hh"
#include "Debug.hh"

namespace EUROPA {

  TokenDecisionPoint::TokenDecisionPoint(const DbClientId& dbClient, const TokenId& tok, const OpenDecisionManagerId& odm)
    : DecisionPoint(dbClient, tok) { 
      m_tok = tok; 
      m_odm = odm;
      m_choiceIndex = 0;
      m_mergeIndex = 0;
    }

  TokenDecisionPoint::~TokenDecisionPoint() { }

  void TokenDecisionPoint::initializeChoices() {
    check_error(m_odm.isValid());
    TokenDecisionPointId dp(m_id);
    m_odm->initializeTokenChoices(dp);
  }

  const bool TokenDecisionPoint::assign() { 
    if (m_choiceIndex && m_choiceIndex >= m_choices.size()) return false;
    check_error(m_tok.isValid());

    if (m_choiceIndex == 0 && m_mergeIndex == 0) initializeChoices();

    if (m_choices.empty()) return false; // unable to find any choices

    m_open = false;

    LabelStr state = m_choices[m_choiceIndex];
    if(state == Token::MERGED) {
      if(m_mergeIndex < m_compatibleTokens.size()) {
	m_dbClient->merge(m_tok, m_compatibleTokens[m_mergeIndex]);
	m_mergeIndex++;
	if (m_mergeIndex == m_compatibleTokens.size())
	  m_choiceIndex++;
	return true;
      } else {
	m_choiceIndex++;
	if (m_choiceIndex == m_choices.size()) {
	  m_open = true;
	  return false;
	}
	state = m_choices[m_choiceIndex];
      }
    }
    if(state == Token::ACTIVE)
      m_dbClient->activate(m_tok);
    else if(state == Token::REJECTED)
      m_dbClient->reject(m_tok);
    else
      check_error(ALWAYS_FAILS, "Inavlid choice for token state assignment" + state.toString());
    m_choiceIndex++;

    return true;
  }

  const bool TokenDecisionPoint::retract() {
    bool retval = hasRemainingChoices();
    m_dbClient->cancel(m_tok);
    m_open = true;
    return retval;
  }

  const bool TokenDecisionPoint::hasRemainingChoices() {
    check_error(m_choiceIndex <= m_choices.size(), "choices index exceeded number of choices");
    check_error(m_mergeIndex <= m_compatibleTokens.size(), "merge index exceeded number of compatible tokens");
    if (m_choiceIndex == 0 && m_mergeIndex == 0) return true; // we have never assigned this decision  or initialized choices
    if (m_choiceIndex == m_choices.size())
      if (!m_tok->isMerged())
	return false;
      else if(m_mergeIndex == m_compatibleTokens.size())
	return false;
    return true;
  }

  const TokenId& TokenDecisionPoint::getToken() const { return (m_tok); }

  void TokenDecisionPoint::print(std::ostream& os) const {
    check_error(m_id.isValid());
    if (m_tok.isValid()) {
      os << "(" << getKey() << ") Token (" << m_entityKey << ") ";
      os << " Current Choice: ";
      if (m_tok->isMerged())
	os << "merged with token (" << m_compatibleTokens[m_mergeIndex-1]->getKey() << ") ";
      else  if (m_choiceIndex == 0)
	os << " No Choice ";
      else
	os << LabelStr(m_choices[m_choiceIndex-1]).toString() << " "; 
      os << " Discarded: " << m_choiceIndex+m_mergeIndex;
    }
    else 
      os << "(" << getKey() << ") Token (" << m_entityKey << ") [deleted]  ";
  }

  std::ostream& operator <<(std::ostream& os, const Id<TokenDecisionPoint>& decision) {
    if (decision.isNoId())
      os << " No Decision ";
    else 
      decision->print(os);
    return(os);
  }

}
