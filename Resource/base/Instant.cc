#include "Utils.hh"
#include "Debug.hh"
#include "Instant.hh"

#include <sstream>

namespace EUROPA {

  Instant::Instant(int time)
  : Entity(),
    m_id(this),
    m_time(time),

	m_lowerMin(0),
	m_lowerMax(0),
	m_upperMin(0),
	m_upperMax(0)
  {
    check_error(isValid());
  }

  Instant::~Instant() {
    cleanup(m_violations);
	cleanup(m_flaws);
    m_id.remove();
  }

  void Instant::insert(const TransactionId& tx) {
    m_transactions.insert(tx);
  }

  bool Instant::remove(const TransactionId& tx) {
    unsigned int old_size = m_transactions.size();
    m_transactions.erase(tx);
    reset();
    return(m_transactions.size() < old_size);
  }

  void Instant::addResourceViolation(ResourceProblem::Type type) {
    ResourceViolation* violation = new ResourceViolation(type, m_id);
    m_violations.push_back(violation->getId());
  }

  void Instant::addResourceFlaw(ResourceProblem::Type type) {
    ResourceFlaw* flaw = new ResourceFlaw(type, m_id);
    m_flaws.push_back(flaw->getId());
  }

  // Resets the violations and flaws
  void Instant::reset(){
    cleanup(m_violations);
    cleanup(m_flaws);
  }

  std::string Instant::toString() const {
    std::stringstream sstr;
    print(sstr);
    return sstr.str();
  }

  void Instant::print(std::ostream& os) const {
    if (m_violations.size() > 0)
      os << "XX ";
    if (m_flaws.size() > 0)
      os << "FF ";

    if(m_time == PLUS_INFINITY)
      os << "+inf";
    else if (m_time == MINUS_INFINITY)
      os << "-inf";
    else
      os << m_time;
    os <<  ":["  << m_lowerMin << ", " << m_lowerMax << ", "  << m_upperMin << ", " << m_upperMax << "] ";
	for ( std::list<ResourceViolationId>::const_iterator it=m_violations.begin(); 
		  it!=m_violations.end(); ++it ) {
	  os << "v" << (*it)->getString() <<" ";
	}
	for ( std::list<ResourceFlawId>::const_iterator it=m_flaws.begin(); 
		  it!=m_flaws.end(); ++it ) {
	  os << "f" << (*it)->getString() <<" ";
	}
  }

  void Instant::updateBounds( double lowerMin, double lowerMax, double upperMin, double upperMax ) {
	m_lowerMin = lowerMin;
	m_lowerMax = lowerMax;
	m_upperMin = upperMin;
	m_upperMax = upperMax;
	debugMsg("Instant:updateBounds", toString());
  }

  bool Instant::isValid() const {
    for (TransactionSet::const_iterator it = m_transactions.begin();
         it != m_transactions.end(); ++it) {
      TransactionId tx = *it;
      check_error(tx.isValid());
      check_error(tx->getTime()->lastDomain().isMember(m_time));
      check_error(tx->getResource().isNoId() || tx->getResource().isValid());
    }

    return(true);
  }

}
