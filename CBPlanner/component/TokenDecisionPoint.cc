#include "TokenDecisionPoint.hh"
#include "Token.hh"
#include "ValueChoice.hh"
#include "DbClient.hh"
#include "Utils.hh"

namespace PLASMA {

  TokenDecisionPoint::TokenDecisionPoint(const DbClientId& dbClient, const TokenId& tok)
    : DecisionPoint(dbClient, tok) { m_tok = tok; }

  TokenDecisionPoint::~TokenDecisionPoint() { }

  const bool TokenDecisionPoint::assign(const ChoiceId& choice) { 
    check_error (choice.isValid());
    check_error(choice->getType() == Choice::VALUE);

    const TokenId& tok = getToken();
    LabelStr state = Id<ValueChoice>(choice)->getValue();
    if(state == Token::ACTIVE)
      m_dbClient->activate(tok);
    else if(state == Token::MERGED)
      m_dbClient->merge(tok, Id<ValueChoice>(choice)->getToken());
    else if(state == Token::REJECTED)
      m_dbClient->reject(tok);
    else
      check_error(ALWAYS_FAILS, "Inavlid choice for token state assignment" + state.toString());

    return DecisionPoint::assign(choice);
  }

  const bool TokenDecisionPoint::retract() {
    m_dbClient->cancel(getToken());
    return DecisionPoint::retract();
  }

  std::list<ChoiceId>& TokenDecisionPoint::getChoices() {
    check_error(m_id.isValid());
    cleanup(m_choices);
    m_choices.clear();
    const AbstractDomain& dom(m_tok->getState()->lastDomain());
    check_error(!dom.isOpen());
    ValueChoice::makeChoices(DecisionPoint::getId(), dom, m_choices);
    return DecisionPoint::getChoices();
  }

  const TokenId& TokenDecisionPoint::getToken() const { return (m_tok); }

  void TokenDecisionPoint::print(std::ostream& os) const {
    check_error(m_id.isValid());
    if (m_tok.isValid())
      os << "(" << getKey() << ") Token (" << m_entityKey << ") ";
    else 
      os << "(" << getKey() << ") Token (" << m_entityKey << ") [deleted] ";
    os << " Current Choice: " << m_current;
    os << " Discarded: " << m_discarded.size();
  }

  std::ostream& operator <<(std::ostream& os, const Id<TokenDecisionPoint>& decision) {
    if (decision.isNoId())
      os << " No Decision ";
    else 
      decision->print(os);
    return(os);
  }

}
