#include "ResourceFlawChoice.hh"
#include "ResourceFlawDecisionPoint.hh"
#include "Resource.hh"

namespace Prototype {

  ResourceFlawChoice::ResourceFlawChoice(const ResourceFlawDecisionPointId& decision, const TransactionId& before, const TransactionId& after) : Choice(decision), m_before(before), m_after(after) { 
    check_error(before.isValid());
    check_error(after.isValid());
    m_type = USER;
  }

  ResourceFlawChoice::ResourceFlawChoice(const ResourceFlawDecisionPointId& decision, const TransactionId& after) : Choice(decision), m_after(after) { 
    // consumer should go after the horizon
    check_error( after.isValid() );
    m_type = RESOURCE;
  }

  ResourceFlawChoice::~ResourceFlawChoice() { }

  bool ResourceFlawChoice::operator==(const Choice& choice) const {
    check_error(m_type == RESOURCE);
    const ResourceFlawChoice* oChoice = static_cast<const ResourceFlawChoice*>(&choice);
    check_error(oChoice != 0);
    if ( m_decision == oChoice->getDecision() && 
	 (m_before==TransactionId::noId() && oChoice->m_before==TransactionId::noId() || m_before == oChoice->m_before) &&
	 m_after == oChoice->m_after )
      return true;
    return false;
  }

  void ResourceFlawChoice::print(std::ostream& os) const {
    check_error(m_id.isValid());
    check_error(m_type == USER);
    if ( m_before==TransactionId::noId() ) os << "HORIZON";
    else m_before->print(os); 
	os << " before ";
	m_after->print(os);
  }

  double ResourceFlawChoice::getValue() const {
    check_error(ALWAYS_FAILS, "No implementation for ResourceFlawChoice::getValue()");
    return -1;
  }

}
