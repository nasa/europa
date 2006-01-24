#include "Utils.hh"
#include "Debug.hh"
#include "PlanDatabase.hh"
#include "TokenDecisionPoint.hh"
#include "OpenDecisionManager.hh"
#include "UnifyMemento.hh"
#include "Token.hh"
#include "TokenVariable.hh"
#include "DbClient.hh"

namespace EUROPA {

  TokenDecisionPoint::TokenDecisionPoint(const DbClientId& dbClient, const TokenId& tok, const OpenDecisionManagerId& odm)
    : DecisionPoint(dbClient, tok) { 
    m_tok = tok; 
    m_odm = odm;
    m_choiceIndex = 0;
    m_mergeIndex = 0;
  }

  TokenDecisionPoint::~TokenDecisionPoint() { }

  std::vector<LabelStr>& TokenDecisionPoint::getChoices() {
    return m_choices;
  }

  unsigned int TokenDecisionPoint::getCurrentChoice() const {
    return m_choiceIndex;
  }

  bool TokenDecisionPoint::assign() { 
    checkError(m_choiceIndex < m_choices.size() || m_choices.empty(), 
	       "Cannor be assigning if we have no choices. A control loop bug.");

    check_error(m_tok.isValid());

    m_open = false;

    LabelStr state = m_choices[m_choiceIndex];
    if(state == Token::MERGED) {
      if(m_mergeIndex < m_compatibleTokens.size()) {
        debugMsg("TokenDecisionPoint", "Merging " << m_tok->getPredicateName().toString() << " with " 
                 << m_compatibleTokens[m_mergeIndex]->getPredicateName().toString());

        debugMsg("CBPlannerDecisionPoint:assign", "For " << m_tok->getPredicateName().toString() << "(" <<
                 m_tok->getKey() << "), assigning MERGED onto " << m_compatibleTokens[m_mergeIndex]->getPredicateName().toString() << 
                 "(" << m_compatibleTokens[m_mergeIndex]->getKey() << ").");
        m_dbClient->merge(m_tok, m_compatibleTokens[m_mergeIndex]);
        m_mergeIndex++;
        if (m_mergeIndex == m_compatibleTokens.size())
          m_choiceIndex++;
        return true;
      } else {
        m_choiceIndex++;
        if (m_choiceIndex == m_choices.size()) {
          debugMsg("TokenDecisionPoint", "Assigning m_open to true because we ran out of things with which to merge.");
          m_open = true;
          return false;
        }
        state = m_choices[m_choiceIndex];
      }
    }
    else if(state == Token::ACTIVE) {
      debugMsg("TokenDecisionPoint", "Activating token " << m_tok->getPredicateName().toString());
      debugMsg("CBPlannerDecisionPoint:assign", "For " << m_tok->getPredicateName().toString() << "(" <<
               m_tok->getKey() << "), assigning ACTIVE.");
      m_dbClient->activate(m_tok);
    }
    else if(state == Token::REJECTED) {
      debugMsg("TokenDecisionPoint", "Rejecting token " << m_tok->getPredicateName().toString());
      debugMsg("CBPlannerDecisionPoint:assign", "For " << m_tok->getPredicateName().toString() << "(" <<
               m_tok->getKey() << "), assigning REJECTED.");
      m_dbClient->reject(m_tok);
    }
    else
      check_error(ALWAYS_FAILS, "Inavlid choice for token state assignment" + state.toString());
    m_choiceIndex++;

    return true;
  }

  bool TokenDecisionPoint::retract() {
    debugMsg("CBPlannerDecisionPoint:retract", "Retracting open condition decision on " << m_tok->getPredicateName().toString() <<
             "(" << m_tok->getKey() << ").");
    bool retval = hasRemainingChoices();
    if(m_tok->getState()->isSpecified())
      m_dbClient->cancel(m_tok);
    m_open = true;
    return retval;
  }

  bool TokenDecisionPoint::hasRemainingChoices() {
    check_error(m_choiceIndex <= m_choices.size(), "choices index exceeded number of choices");
    check_error(m_mergeIndex <= m_compatibleTokens.size(), "merge index exceeded number of compatible tokens");
    
    if(m_choices.empty() || //if we have no choices
       (m_choices.size() == 1 &&   //or there's only one choice
        (double) (*(m_choices.begin())) == (double) Token::MERGED && //and that choice is to merge
        m_compatibleTokens.empty())) {
      debugMsg("TokenDecisionPoint:hasRemainingChoices", "Following the path that *surely* must be correct.");
      return false; //but there are no compatible tokens, return false
    }

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
      os << m_tok->toString();
      os << " Current Choice: ";

      if (m_tok->isMerged())
        os << "merged with token (" << m_tok->getActiveToken() << ") ";
      else  if (m_choices.empty())
        os << "No Choices";
      else if (m_choiceIndex == 0)
        os << LabelStr(m_choices[m_choiceIndex]).toString() << " ";
      else
        os << LabelStr(m_choices[m_choiceIndex-1]).toString() << " ";

      os << " Discarded: " << m_choiceIndex+m_mergeIndex;
    }
    else 
      os << "(" << getKey() << ") Token (" << m_tok->toString() << ") [deleted]  ";
  }

  std::ostream& operator <<(std::ostream& os, const Id<TokenDecisionPoint>& decision) {
    if (decision.isNoId())
      os << " No Decision ";
    else 
      decision->print(os);
    return(os);
  }

}
