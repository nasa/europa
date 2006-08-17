#ifndef _H_DecisionManagerListener
#define _H_DecisionManagerListener

#include "CBPlannerDefs.hh"

namespace EUROPA {

  class DecisionManagerListener {
  public:
    virtual ~DecisionManagerListener();

    const DecisionManagerListenerId& getId() const;
    
    virtual void notifyConditionAdded(const ConditionId& cond);
    virtual void notifyConditionRemoved(const ConditionId& cond);
    virtual void notifyConditionsChanged();
    virtual void notifyPassed(const EntityId& entity, const ConditionId& cond);
    virtual void notifyFailed(const EntityId& entity, const ConditionId& cond);
    virtual void notifyNewUnitDecision(const DecisionPointId& dec);
    virtual void notifyNewDecision(const DecisionPointId& dec);
    virtual void notifyRemovedDecision(const EntityId& entity);
    virtual void notifyAssignNextStarted(const DecisionPointId& dec); // previous dec
    virtual void notifyAssignNextFailed(const DecisionPointId& dec); // current dec
    virtual void notifyAssignNextSucceeded(const DecisionPointId& dec); // current dec
    virtual void notifyAssignCurrentStarted(const DecisionPointId& dec); // current dec
    virtual void notifyAssignCurrentFailed(const DecisionPointId& dec); // current dec
    virtual void notifyAssignCurrentSucceeded(const DecisionPointId& dec); // current dec
    virtual void notifyRetractStarted(const DecisionPointId& dec); // current dec
    virtual void notifyRetractFailed(const DecisionPointId& dec);  // current dec
    virtual void notifyRetractSucceeded(const DecisionPointId& dec);  // current dec
    virtual void notifySearchFinished();
    virtual void notifyPlannerTimeout();
  protected:
    DecisionManagerListener(const DecisionManagerId& dm);
    DecisionManagerListenerId m_id;
    const DecisionManagerId m_dm;
  };

}

#endif
