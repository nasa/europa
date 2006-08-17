#include "Utils.hh"
#include "DecisionPoint.hh"
#include "DbClient.hh"

namespace EUROPA {

  DecisionPoint::DecisionPoint(const DbClientId& dbClient, const EntityId& entity)
    : m_id(this), m_open(true), m_dbClient(dbClient), m_entity(entity), m_entityKey(entity->getKey()) {
    check_error(entity.isValid());
    check_error(m_id.isValid());
  }

  DecisionPoint::~DecisionPoint() { 
    check_error(m_id.isValid());
    m_id.remove(); 
  }

  const DecisionPointId& DecisionPoint::getId() const { return m_id; }

  int DecisionPoint::getEntityKey() const { return m_entityKey; }

  bool DecisionPoint::isOpen() const { return m_open; }

  void DecisionPoint::print(std::ostream& os) const {
    os << "Decision (" << getKey() << ")";
    os << std::endl;
  }

  std::ostream& operator <<(std::ostream& os, const DecisionPointId& decision) {
    if (decision.isNoId())
      os << " No Decision ";
    else 
      decision->print(os);
    return(os);
  }

}
