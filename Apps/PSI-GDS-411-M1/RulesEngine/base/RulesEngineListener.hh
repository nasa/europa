#ifndef _H_RulesEngineListener
#define _H_RulesEngineListener

#include "RulesEngineDefs.hh"

namespace EUROPA {
  class RulesEngineListener {
  public:
    virtual ~RulesEngineListener();
    virtual void notifyExecuted(const RuleInstanceId &rule){}
    virtual void notifyUndone(const RuleInstanceId &rule){}
    const RulesEngineListenerId &getId() const;

  protected:
    RulesEngineListener(const RulesEngineId &rulesEngine);
    RulesEngineListenerId m_id;
    RulesEngineId m_rulesEngine;
  };
}

#endif
