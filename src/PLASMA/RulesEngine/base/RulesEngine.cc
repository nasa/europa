#include "RulesEngine.hh"
#include "RulesEngineListener.hh"
#include "PlanDatabase.hh"
#include "PlanDatabaseListener.hh"
#include "Rule.hh"
#include "RuleInstance.hh"
#include "Token.hh"
#include "ConstraintEngine.hh"
#include "Propagators.hh"
#include "Utils.hh"
#include "RuleVariableListener.hh"
#include "ProxyVariableRelation.hh"
#include "ConstraintFactory.hh"
#include <list>

namespace EUROPA{

  class DbRuleEngineConnector: public PlanDatabaseListener {

  private:
    friend class RulesEngine;

    DbRuleEngineConnector(const PlanDatabaseId& planDatabase, const RulesEngineId& rulesEngine)
      : PlanDatabaseListener(planDatabase), m_rulesEngine(rulesEngine){}
    void notifyActivated(const TokenId& token){m_rulesEngine->notifyActivated(token);}
    void notifyDeactivated(const TokenId& token){m_rulesEngine->notifyDeactivated(token);}
    void notifyTerminated(const TokenId& token){m_rulesEngine->notifyDeactivated(token);}
    const RulesEngineId m_rulesEngine;
  };

  RulesEngine::RulesEngine(const RuleSchemaId& schema, const PlanDatabaseId& planDatabase)
    : m_id(this)
    , m_schema(schema)
    , m_planDb(planDatabase)
    , m_deleted(false)
  {
    m_planDbListener = (new DbRuleEngineConnector(m_planDb, m_id))->getId();

    // Allocate an instance of Default Propagator to handle the rule related  contraint propagation.
    // Will be cleaned up automatically by the ConstraintEngine
    new DefaultPropagator("RulesEngine", m_planDb->getConstraintEngine());

    check_error(m_planDb->getTokens().empty());
  }

  RulesEngine::~RulesEngine(){
    check_error(m_planDbListener.isValid());

    // If we are not purging, then events should have propagated removal of all rule instances
    check_error(Entity::isPurging() || m_ruleInstancesByToken.empty());

    m_deleted = true;
    // Thus, only if we are in purge mode will we directly remove rule instances
    for(std::multimap<int, RuleInstanceId>::const_iterator it=m_ruleInstancesByToken.begin();it!=m_ruleInstancesByToken.end();++it){
      RuleInstanceId ruleInstance = it->second;
      check_error(ruleInstance.isValid());
      ruleInstance->discard();
    }

    delete (PlanDatabaseListener*) m_planDbListener;  // removes itself from the plan database set of listeners

    m_id.remove();
  }

  const RulesEngineId& RulesEngine::getId() const{return m_id;}

  const PlanDatabaseId& RulesEngine::getPlanDatabase() const {return m_planDb;}

  const RuleSchemaId& RulesEngine::getRuleSchema() const { return m_schema; }


  std::set<RuleInstanceId> RulesEngine::getRuleInstances() const{
    std::set<RuleInstanceId> ruleInstances;
    for(std::multimap<int, RuleInstanceId>::const_iterator it=m_ruleInstancesByToken.begin();it!=m_ruleInstancesByToken.end();++it)
      ruleInstances.insert(it->second);
    return ruleInstances;
  }

  void RulesEngine::getRuleInstances(const TokenId& token,std::set<RuleInstanceId>& results) const{
    check_error(token.isValid());
    std::multimap<int, RuleInstanceId>::const_iterator it = m_ruleInstancesByToken.find(token->getKey());
    while(it!=m_ruleInstancesByToken.end() && it->first == token->getKey()){
      results.insert(it->second);
      ++it;
    }
  }

  void RulesEngine::notifyExecuted(const RuleInstanceId &rule) {
    check_error(rule.isValid());
    for(std::set<RulesEngineListenerId>::iterator it = m_listeners.begin();
        it != m_listeners.end(); ++it) {
      RulesEngineListenerId listener = *it;
      listener->notifyExecuted(rule);
    }
  }

  void RulesEngine::notifyUndone(const RuleInstanceId &rule) {
    check_error(rule.isValid());
    for(std::set<RulesEngineListenerId>::iterator it = m_listeners.begin();
        it != m_listeners.end(); ++it) {
      RulesEngineListenerId listener = *it;
      listener->notifyUndone(rule);
    }
  }

  void RulesEngine::notifyActivated(const TokenId& token){
    check_error(token.isValid());
    check_error(token->isActive());
    check_error(m_ruleInstancesByToken.find(token->getKey()) == m_ruleInstancesByToken.end());

    // Allocate a rule instance for all rules that apply
    std::vector<RuleId> allRules;
    m_schema->getRules(getPlanDatabase(),token->getPredicateName(), allRules);
    for(std::vector<RuleId>::const_iterator it = allRules.begin(); it != allRules.end(); ++it){
      RuleId rule = *it;
      check_error(rule.isValid());
      RuleInstanceId ruleInstance = rule->createInstance(token, getPlanDatabase(), getId());
      m_ruleInstancesByToken.insert(std::pair<int, RuleInstanceId>(token->getKey(), ruleInstance));
    }
  }

  void RulesEngine::notifyDeactivated(const TokenId& token){
    check_error(!Entity::isPurging());
    cleanupRuleInstances(token);
  }

  void RulesEngine::notifyTerminated(const TokenId& token){
    cleanupRuleInstances(token);
  }

  void RulesEngine::cleanupRuleInstances(const TokenId& token){
    check_error(token.isValid());

    std::multimap<int, RuleInstanceId>::iterator it = m_ruleInstancesByToken.find(token->getKey());
    while(it!=m_ruleInstancesByToken.end() && it->first == token->getKey()){
      RuleInstanceId ruleInstance = it->second;
      check_error(ruleInstance.isValid());
      ruleInstance->discard();
      m_ruleInstancesByToken.erase(it++);
    }

  }

  bool RulesEngine::hasPendingRuleInstances(const TokenId& token) const {
    check_error(token.isValid());
    std::multimap<int, RuleInstanceId>::const_iterator it = m_ruleInstancesByToken.find(token->getKey());
    while(it!=m_ruleInstancesByToken.end() && it->first == token->getKey()){
      RuleInstanceId r = it->second;
      if(isPending(r))
	return true;
      ++it;
    }

    return false;
  }

  bool RulesEngine::isPending(const RuleInstanceId& r) const {
    // Try r directly
    if(!r->isExecuted() && !r->willNotFire()){
      debugMsg("RulesEngine:isPending", "Found pending rule:" << r->getKey() << " for " << r->getToken()->toString());
      return true;
    }

    // No iterate over all child rule instances
    const std::vector<RuleInstanceId>& childRules =  r->getChildRules();
    for(std::vector<RuleInstanceId>::const_iterator it = childRules.begin(); it != childRules.end(); ++it){
      RuleInstanceId child = *it;
      if(isPending(child))
	return true;
    }

    return false;
  }

  void RulesEngine::add(const RulesEngineListenerId &listener) {
    check_error(listener.isValid());
    check_error(m_listeners.count(listener) == 0);
    m_listeners.insert(listener);
  }

  void RulesEngine::remove(const RulesEngineListenerId &listener) {
    check_error(m_listeners.count(listener) == 1);
    if(!m_deleted)
      m_listeners.erase(listener);
  }
}
