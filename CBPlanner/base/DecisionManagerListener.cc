#include "DecisionManagerListener.hh"
#include "DecisionManager.hh"
#include "Debug.hh"

namespace EUROPA {

  DecisionManagerListener::DecisionManagerListener(const DecisionManagerId& dm)
    :m_id(this), m_dm(dm){
    check_error(m_dm.isValid());
    m_dm->add(m_id);
  }

  DecisionManagerListener::~DecisionManagerListener(){
    check_error(m_id.isValid());
    check_error(m_dm.isValid());
    m_dm->remove(m_id);
    m_id.remove();
  }

  const DecisionManagerListenerId& DecisionManagerListener::getId() const{return m_id;}

  void DecisionManagerListener::notifyConditionAdded(const ConditionId& cond) {}
  void DecisionManagerListener::notifyConditionRemoved(const ConditionId& cond) {}
  void DecisionManagerListener::notifyConditionsChanged() {}
  void DecisionManagerListener::notifyPassed(const EntityId& entity, const ConditionId& cond) {}
  void DecisionManagerListener::notifyFailed(const EntityId& entity, const ConditionId& cond) {
    debugMsg("DecisionManagerListener:notifyFailed", " Method called - method is a no-op");
  }
  void DecisionManagerListener::notifyNewUnitDecision(const DecisionPointId& dec) {}
  void DecisionManagerListener::notifyNewDecision(const DecisionPointId& dec) {}
  void DecisionManagerListener::notifyRemovedDecision(const EntityId& entity) {}
  void DecisionManagerListener::notifyAssignNextStarted(const DecisionPointId& dec) {} // previous dec
  void DecisionManagerListener::notifyAssignNextFailed(const DecisionPointId& dec) {} // current dec
  void DecisionManagerListener::notifyAssignNextSucceeded(const DecisionPointId& dec) {} // current dec
  void DecisionManagerListener::notifyAssignCurrentStarted(const DecisionPointId& dec) {} // current dec
  void DecisionManagerListener::notifyAssignCurrentFailed(const DecisionPointId& dec) {} // current dec
  void DecisionManagerListener::notifyAssignCurrentSucceeded(const DecisionPointId& dec) {} // current dec
  void DecisionManagerListener::notifyRetractStarted(const DecisionPointId& dec) {} // current dec
  void DecisionManagerListener::notifyRetractFailed(const DecisionPointId& dec) {}  // current dec
  void DecisionManagerListener::notifyRetractSucceeded(const DecisionPointId& dec) {}  // current dec
  void DecisionManagerListener::notifySearchFinished() {}
  void DecisionManagerListener::notifyPlannerTimeout() {}

}
