#ifndef _H_DMLogger
#define _H_DMLogger

#include "DecisionManagerListener.hh"
#include <iostream>

namespace Prototype{

  class DMLogger: public DecisionManagerListener {
  public:
    DMLogger(std::ostream& os, const DecisionManagerId& dm);
    ~DMLogger();
    void notifyConditionAdded(const ConditionId& cond);
    void notifyConditionRemoved(const ConditionId& cond);
    void notifyConditionsChanged();
    void notifyPassed(const EntityId& entity, const ConditionId& cond);
    void notifyFailed(const EntityId& entity, const ConditionId& cond);
    void notifyNewUnitDecision(const DecisionPointId& dec);
    void notifyNewDecision(const DecisionPointId& dec);
    void notifyRemovedDecision(const EntityId& entity);
    void notifyAssignNextStarted(const DecisionPointId& dec); // previous dec
    void notifyAssignNextFailed(const DecisionPointId& dec); // current dec
    void notifyAssignNextSucceeded(const DecisionPointId& dec); // current dec
    void notifyAssignCurrentStarted(const DecisionPointId& dec); // current dec
    void notifyAssignCurrentFailed(const DecisionPointId& dec); // current dec
    void notifyAssignCurrentSucceeded(const DecisionPointId& dec); // current dec
    void notifyRetractStarted(const DecisionPointId& dec); // current dec
    void notifyRetractFailed(const DecisionPointId& dec);  // current dec
    void notifyRetractSucceeded(const DecisionPointId& dec);  // current dec
  private:
    std::ostream& m_os;
  };

}

#endif
