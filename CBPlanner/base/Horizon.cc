#include "Horizon.hh"
#include "Condition.hh"

namespace EUROPA {

  Horizon::Horizon() : m_id(this), m_start(MIN_FINITE_TIME), m_end(MAX_FINITE_TIME) {}

  Horizon::Horizon(const int& start, const int& end) : m_id(this), m_start(start), m_end(end) {} 

  Horizon::~Horizon() { 
    m_id.remove(); 
  }

  const HorizonId& Horizon::getId() { return m_id; }

  void Horizon::setHorizon(const int& start, const int& end) { 
    check_error(start >= MIN_FINITE_TIME && start <= MAX_FINITE_TIME);
    check_error(end >= MIN_FINITE_TIME && end <= MAX_FINITE_TIME);
    if (m_start < start) {
      m_start = start;
    }
    else if (m_start > start) {
      m_start = start;
    }
    if (m_end < end) {
      m_end = end;
    }
    else if (m_end > end) {
      m_end = end;
    }
  }

  void
  Horizon::getHorizon(int& start, int& end) { 
    start = m_start;
    end = m_end;
  }

  inline void Horizon::print (std::ostream& os) {
    os << "HORIZON [" << m_start << " - " << m_end << "]";
  }

}

