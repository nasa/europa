#include "Condition.hh"
#include "DecisionManager.hh"

namespace EUROPA {

  Condition::Condition(const DecisionManagerId& dm, bool isDynamic) 
    : m_id(this), m_isDynamic(isDynamic), m_decMgr(dm) {
    check_error(dm.isValid());
    m_decMgr->attach(m_id);
  }

  Condition::~Condition() { m_decMgr->detach(m_id); m_id.remove(); }

  const ConditionId& Condition::getId() const { return m_id; }

  bool Condition::isDynamic() const { 
    return m_isDynamic; 
  }

  void Condition::print(std::ostream &os) { 
    os << "CONDITION"; 
  }

  std::ostream& operator <<(std::ostream& os, const ConditionId& cond) {
    if (cond.isNoId())
      os << " No Condition ";
    else
      cond->print(os);
    return os;
  }
}
