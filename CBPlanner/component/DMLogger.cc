#include "DMLogger.hh"
#include "Condition.hh"
#include "DecisionPoint.hh"

namespace Prototype {

  DMLogger::DMLogger(std::ostream& os, const DecisionManagerId& dm) : DecisionManagerListener(dm), m_os(os) {}

  DMLogger::~DMLogger() {}

  void DMLogger::notifyConditionAdded(const ConditionId& cond){
    m_os << "DMLogger: Condition Added ";
    cond->print(m_os);
    m_os << std::endl;
  }

  void DMLogger::notifyConditionRemoved(const ConditionId& cond) {
    m_os << "DMLogger: Condition Removed ";
    cond->print(m_os);
    m_os << std::endl;
  }

  void DMLogger::notifyConditionsChanged() {
    m_os << "DMLogger: Conditions Changed " << std::endl;
  }

  void DMLogger::notifyPassed(const EntityId& entity, const ConditionId& cond) {
    m_os << "DMLogger: (" << entity->getKey() << ") Passed ";
    cond->print(m_os);
    m_os << std::endl;
  }
  void DMLogger::notifyFailed(const EntityId& entity, const ConditionId& cond) {
    m_os << "DMLogger: (" << entity->getKey() << ") Failed ";
    cond->print(m_os);
    m_os << std::endl;
  }
  void DMLogger::notifyNewUnitDecision(const DecisionPointId& dec) {
    m_os << "DMLogger: New Unit Decision ";
    dec->print(m_os);
    m_os << std::endl;
  }
  void DMLogger::notifyNewDecision(const DecisionPointId& dec) {
    m_os << "DMLogger: New Decision ";
    dec->print(m_os);
    m_os << std::endl;
  }
  void DMLogger::notifyRemovedDecision(const EntityId& entity) {
    m_os << "DMLogger: Removed Decision corresponding to (" << entity->getKey() << ") " << std::endl;
  }

}
