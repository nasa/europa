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
  private:
    std::ostream& m_os;
  };

}

#endif
