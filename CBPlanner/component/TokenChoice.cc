#include "TokenChoice.hh"
#include "Token.hh"
#include "Object.hh"

namespace EUROPA {

  TokenChoice::TokenChoice(const DecisionPointId& decision, 
			   const ObjectId& obj, 
			   const TokenId& predecessor,
			   const TokenId& successor) 
    : Choice(decision), m_object(obj), m_predecessor(predecessor), m_successor(successor) { 
    // can't enforce successor.isValid() because decisions to insert at the end
    // include successor::noId() as successor.
    m_type = TOKEN;
  }

  TokenChoice::~TokenChoice() { }

  const ObjectId& TokenChoice::getObject() const { 
    check_error(m_type == TOKEN);
    return m_object; 
  }
  const TokenId& TokenChoice::getPredecessor() const { 
    check_error(m_type == TOKEN);
    return m_predecessor; 
  }
  const TokenId& TokenChoice::getSuccessor() const { 
    check_error(m_type == TOKEN);
    return m_successor; 
  }

  bool TokenChoice::operator==(const Choice& choice) const {
    check_error(m_type == TOKEN);
    const TokenChoice* oChoice = static_cast<const TokenChoice*>(&choice);
    check_error(oChoice != 0);
    check_error(oChoice->getObject().isNoId() || oChoice->getObject().isValid());
    check_error(oChoice->getPredecessor().isNoId() || oChoice->getPredecessor().isValid());
    check_error(oChoice->getSuccessor().isNoId() || oChoice->getSuccessor().isValid());
    return (m_decision == oChoice->getDecision() && 
	    m_predecessor == oChoice->getPredecessor() &&
	    m_successor == oChoice->getSuccessor() &&
	    m_object == oChoice->getObject());
  }

  void TokenChoice::print(std::ostream& os) const {
    check_error(m_id.isValid());
    check_error(m_type == TOKEN);
    if (!m_object.isNoId()) {
      os << "Object (" << m_object->getKey() << ") ";
      if (!m_successor.isNoId()) {
	os << "Token (" << m_successor->getKey() << ") ";
      }
      else {
	os << "Token (noId) ";
      }
    }
    else
      os << "Object (noId) [deleted]";
  }

  double TokenChoice::getValue() const {
    check_error(ALWAYS_FAILS);
    check_error(m_object.isNoId() || m_object.isValid());
    check_error(m_successor.isNoId() || m_successor.isValid());
    return m_object; /* Get the objectId encoded as a double */
  }
}
