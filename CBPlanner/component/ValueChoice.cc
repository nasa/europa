#include "ValueChoice.hh"
#include "Token.hh"
#include "ConstrainedVariableDecisionPoint.hh"
#include "Object.hh"

namespace PLASMA {

  ValueChoice::ValueChoice(const DecisionPointId& decision, const double val) : Choice(decision), m_value(val) { 
    m_type = VALUE; 
    //    std::cout << " creating choice ";
    //    printValue(std::cout);
    //    std::cout << std::endl;
  }

  ValueChoice::ValueChoice(const DecisionPointId& decision, const double val, const TokenId& tok) : Choice(decision), m_value(val), m_token(tok) { 
    check_error(tok.isValid());
    m_type = VALUE; 
    //    std::cout << " creating choice ";
    //    printValue(std::cout);
    //    std::cout << std::endl;
  }

  ValueChoice::~ValueChoice() {}

  double ValueChoice::getValue() const { 
    check_error(m_type == VALUE);
    return m_value;
  }

  const TokenId& ValueChoice::getToken() const { 
    check_error(m_type == VALUE);
    return m_token; 
  }

  bool ValueChoice::operator==(const Choice& choice) const {
    check_error(m_type == VALUE);
    const ValueChoice* vChoice = static_cast<const ValueChoice*>(&choice);
    check_error(vChoice != 0);
    if (m_decision == vChoice->getDecision() && m_value == vChoice->getValue() && m_token == vChoice->getToken())
      return true;
    return false;
  }

  void ValueChoice::printValue(std::ostream& os) const {
    if (ConstrainedVariableDecisionPointId::convertable(m_decision)) {
      ConstrainedVariableDecisionPointId cvdec(m_decision);
      const AbstractDomain& dom = cvdec->getVariable()->specifiedDomain();
      if (Schema::instance()->isObjectType(dom.getTypeName())) {
	ObjectId obj(m_value);
	os << obj->getName().c_str();
      }
      else if (dom.isNumeric())
	os << m_value;
      else if (LabelStr::isString(m_value))
	os << LabelStr(m_value).c_str();
      else os << m_value;
    }
    else if (LabelStr::isString(m_value))
      os << LabelStr(m_value).c_str();
    else
      os << m_value;
  }

  void ValueChoice::print(std::ostream& os) const {
    check_error(m_id.isValid());
    check_error(m_type == VALUE);
    if (!m_token.isNoId()) {
      os << "Value (";
      printValue(os);
      os << ") Token (" << m_token->getKey() << ") ";
    }
    else {
      os << "Value ("; 
      printValue(os);
      os << ") ";
    }
  }
}
