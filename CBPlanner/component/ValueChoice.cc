#include "ValueChoice.hh"
#include "Token.hh"

namespace Prototype {

  ValueChoice::ValueChoice(const DecisionPointId& decision, const double val) : Choice(decision), m_value(val) { 
    m_type = value; 
  }

  ValueChoice::ValueChoice(const DecisionPointId& decision, const double val, const TokenId& tok) : Choice(decision), m_value(val), m_token(tok) { 
    check_error(tok.isValid());
    m_type = value; 
  }

  ValueChoice::~ValueChoice() {}

  const double ValueChoice::getValue() const { 
    check_error(m_type == value);
    return m_value; 
  }

  const TokenId& ValueChoice::getToken() const { 
    check_error(m_type == value);
    return m_token; 
  }

  bool ValueChoice::operator==(const Choice& choice) const {
    check_error(m_type == value);
    const ValueChoice* vChoice = static_cast<const ValueChoice*>(&choice);
    check_error(vChoice != 0);
    if (m_decision == vChoice->getDecision() && m_value == vChoice->getValue() && m_token == vChoice->getToken())
      return true;
    return false;
  }

  void ValueChoice::print(std::ostream& os) const {
    check_error(m_id.isValid());
    check_error(m_type == value);
    if (!m_token.isNoId())
      os << "Value (" << m_value << ") Token (" << m_token->getKey() << ") ";
    else 
      os << "Value (" << m_value << ") ";
  }
}
