#include "ValueChoice.hh"
#include "Token.hh"
#include "TokenDecisionPoint.hh"
#include "ConstrainedVariableDecisionPoint.hh"
#include "PlanDatabase.hh"
#include "Object.hh"

namespace PLASMA {

  void ValueChoice::makeChoices(const DecisionPointId& decision, const AbstractDomain& domain, std::list<ChoiceId>& choices) {
    check_error(!domain.isEmpty());
    check_error(domain.isFinite(), "Unable to handle infinite decisions");
    std::list<double> values;
    domain.getValues(values);
    //    std::cout << " Choice.cc::getChoices() domain size = " << values.size() << std::endl;
    const AbstractDomain::DomainType& type = domain.getType();
    //    std::cout << " Choice.cc::getChoices() type = " << type << std::endl;
    // Create choices for values. Order matters! We want to prefer merging over activation. This should really
    // be handled in a heuristic.
    std::list<double>::iterator it = values.begin();
    for ( ; it != values.end(); it++) {
      if (type == AbstractDomain::REAL_ENUMERATION && TokenDecisionPointId::convertable(decision) && *it == Token::MERGED) {
	TokenDecisionPointId tokDec = decision;
	TokenId tok = tokDec->getToken();
	check_error(tok->isInactive());
	std::vector<TokenId> compats;
	tok->getPlanDatabase()->getCompatibleTokens(tok, compats);
	// std::cout << " Choice.cc::getChoices() found " << compats.size() << " compatible tokens" << std::endl;
	if (!compats.empty()) {
	  std::vector<TokenId>::iterator it2 = compats.begin();
	  check_error((*it2)->isActive());
	  for ( ; it2 != compats.end(); it2++) 
	    choices.push_back((new ValueChoice(decision, *it, *it2))->getId());
	}
      }
      else {
	choices.push_back((new ValueChoice(decision, *it))->getId());
      }
    }
  }

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
