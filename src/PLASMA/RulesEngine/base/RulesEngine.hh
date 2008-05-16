#ifndef _H_RulesEngine
#define _H_RulesEngine

/**
 * @file RulesEngine.hh
 * @author Conor McGann
 * @date December, 2003
 * @brief Defines Rules Engine interface
 * @ingroup RulesEngine
 */

#include "RulesEngineDefs.hh"
#include <map>
#include <set>
#include"Engine.hh"

namespace EUROPA {

  /**
   * @class RulesEngine
   * @brief Provides the control model to integrate plan database events with model rules
   * @see RuleVariableListener, PlanDatabaseListener, RuleInstance, Rule
   */
  class RulesEngine : public EngineComponent {
  public:
    RulesEngine(const PlanDatabaseId& planDatabase);
    ~RulesEngine();
    const RulesEngineId& getId() const;
    
    const PlanDatabaseId& getPlanDatabase() const;

    void notifyActivated(const TokenId& token);
    void notifyDeactivated(const TokenId& token);
    void notifyTerminated(const TokenId& token);

    std::set<RuleInstanceId> getRuleInstances() const;
    void getRuleInstances(const TokenId& token,std::set<RuleInstanceId>& results) const;
    bool hasPendingRuleInstances(const TokenId& token) const;

  private:
    friend class RulesEngineListener;
    friend class RuleInstance;

    void add(const RulesEngineListenerId &listener);
    void remove(const RulesEngineListenerId &listener);
    void notifyExecuted(const RuleInstanceId &rule);
    void notifyUndone(const RuleInstanceId &rule);
    void cleanupRuleInstances(const TokenId& token);
    bool isPending(const RuleInstanceId& r) const;
    RulesEngineId m_id;
    const PlanDatabaseId m_planDb;
    PlanDatabaseListenerId m_planDbListener;
    std::multimap<int, RuleInstanceId> m_ruleInstancesByToken;
    std::set<RulesEngineListenerId> m_listeners;
    bool m_deleted;
  };
}
#endif
