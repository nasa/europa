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
#include "ConstraintType.hh"
#include <list>

#include <algorithm>

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

  class RulesEngineCallback : public PostPropagationCallback {
  public:
    RulesEngineCallback(const ConstraintEngineId& ce, const RulesEngineId& re) : PostPropagationCallback(ce), m_re(re) {}

    bool operator()() {
      if(m_re->hasWork()) {
        debugMsg("RulesEngineCallback", "Rules engine has work to do.");
        return m_re->doRules();
      }
      return false;
    }
  private:
    RulesEngineId m_re;
  };

  RulesEngine::RulesEngine(const RuleSchemaId& schema, const PlanDatabaseId& planDatabase)
    : m_id(this)
    , m_schema(schema)
    , m_planDb(planDatabase)
    , m_deleted(false)
  {
    m_planDbListener = (new DbRuleEngineConnector(m_planDb, m_id))->getId();
    m_callback = (new RulesEngineCallback(m_planDb->getConstraintEngine(), m_id))->getId();
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
    for(std::multimap<eint, RuleInstanceId>::const_iterator it=m_ruleInstancesByToken.begin();it!=m_ruleInstancesByToken.end();++it){
      RuleInstanceId ruleInstance = it->second;
      check_error(ruleInstance.isValid());
      ruleInstance->discard();
    }

    delete (PlanDatabaseListener*) m_planDbListener;  // removes itself from the plan database set of listeners
    delete (PostPropagationCallback*) m_callback;
    m_id.remove();
  }

  const RulesEngineId& RulesEngine::getId() const{return m_id;}

  const PlanDatabaseId& RulesEngine::getPlanDatabase() const {return m_planDb;}

  const RuleSchemaId& RulesEngine::getRuleSchema() const { return m_schema; }


  std::set<RuleInstanceId> RulesEngine::getRuleInstances() const{
    std::set<RuleInstanceId> ruleInstances;
    for(std::multimap<eint, RuleInstanceId>::const_iterator it=m_ruleInstancesByToken.begin();it!=m_ruleInstancesByToken.end();++it)
      ruleInstances.insert(it->second);
    return ruleInstances;
  }

  void RulesEngine::getRuleInstances(const TokenId& token,std::set<RuleInstanceId>& results) const{
    check_error(token.isValid());
    std::multimap<eint, RuleInstanceId>::const_iterator it = m_ruleInstancesByToken.find(token->getKey());
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
      m_ruleInstancesByToken.insert(std::make_pair(token->getKey(), ruleInstance));
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

    std::multimap<eint, RuleInstanceId>::iterator it = m_ruleInstancesByToken.find(token->getKey());
    while(it!=m_ruleInstancesByToken.end() && it->first == token->getKey()){
      RuleInstanceId ruleInstance = it->second;
      check_error(ruleInstance.isValid());
      ruleInstance->discard();
      m_ruleInstancesByToken.erase(it++);
    }

  }

  bool RulesEngine::hasPendingRuleInstances(const TokenId& token) const {
    check_error(token.isValid());
    std::multimap<eint, RuleInstanceId>::const_iterator it = m_ruleInstancesByToken.find(token->getKey());
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

  void RulesEngine::scheduleForExecution(const RuleInstanceId& r) {
    debugMsg("RulesEngine:scheduleForExecution", "Scheduling rule " << r->toString());
    m_ruleInstancesToExecute.push_back(r);
  }

  void RulesEngine::scheduleForUndoing(const RuleInstanceId& r) {
    debugMsg("RulesEngine:scheduleForUndoing", "Scheduling rule " << r->toString());
    m_ruleInstancesToUndo.push_back(r);
  }

  bool RulesEngine::doRules() {
    std::sort(m_ruleInstancesToExecute.begin(), m_ruleInstancesToExecute.end(), EntityComparator<RuleInstanceId>());
    std::sort(m_ruleInstancesToUndo.begin(), m_ruleInstancesToUndo.end(), EntityComparator<RuleInstanceId>());
    std::vector<RuleInstanceId>::iterator execEnd = std::unique(m_ruleInstancesToExecute.begin(),
                                                                m_ruleInstancesToExecute.end(),
                                                                EntityComparator<RuleInstanceId>());
    std::vector<RuleInstanceId>::iterator undoEnd = std::unique(m_ruleInstancesToUndo.begin(),
                                                                m_ruleInstancesToUndo.end(),
                                                                EntityComparator<RuleInstanceId>());
    bool retval = false;

    for(std::vector<RuleInstanceId>::iterator it = m_ruleInstancesToExecute.begin(); it != execEnd; ++it)
      if(!(*it)->isExecuted() && (*it)->test()) {
        debugMsg("RulesEngine:doRules", "Executing rule " << (*it)->toString());
        (*it)->execute();
        retval = true;
      }
    for(std::vector<RuleInstanceId>::iterator it = m_ruleInstancesToUndo.begin(); it != undoEnd; ++it)
      if((*it)->isExecuted() && !(*it)->test() && !(*it)->hasEmptyGuard()) {
        debugMsg("RulesEngine:doRules", "Undoing rule " << (*it)->toString());
        (*it)->undo();
        retval = true;
      }
    m_ruleInstancesToExecute.clear();
    m_ruleInstancesToUndo.clear();
    return retval;
  }

  bool RulesEngine::hasWork() const {
    return !m_ruleInstancesToExecute.empty() || !m_ruleInstancesToUndo.empty();
  }
}
