#include "TokenChoice.hh"
#include "Token.hh"

namespace Prototype {

  TokenChoice::TokenChoice(const DecisionPointId& decision, const TokenId& tok) : Choice(decision), m_token(tok) { 
    // can't enforce tok.isValid() because decisions to insert at the end
    // include tok::noId() as successor.
    m_type = TOKEN; 
  }

  TokenChoice::~TokenChoice() { }

  const TokenId& TokenChoice::getToken() const { 
    check_error(m_type == TOKEN);
    return m_token; 
  }

  bool TokenChoice::operator==(const Choice& choice) const {
    check_error(m_type == TOKEN);
    const TokenChoice* tChoice = static_cast<const TokenChoice*>(&choice);
    check_error(tChoice != 0);
    check_error(tChoice->getToken().isNoId() || tChoice->getToken().isValid());
    if (m_decision == tChoice->getDecision() && 
	((m_token.isNoId() && tChoice->getToken().isNoId()) || m_token == tChoice->getToken()))
      return true;
    return false;
  }

  void TokenChoice::print(std::ostream& os) const {
    check_error(m_id.isValid());
    check_error(m_type == TOKEN);
    if (!m_token.isNoId())
      os << "Token (" << m_token->getKey() << ") ";
    else
      os << "Token (noId) ";
  }

}
