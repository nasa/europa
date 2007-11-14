#include "Rule.hh"
#include "RuleInstance.hh"
#include "RulesEngine.hh"
#include "RuleVariableListener.hh"
#include "Token.hh"
#include "PlanDatabase.hh"
#include "Schema.hh"

namespace EUROPA{

  Rule::Rule(const LabelStr& name)
    : Entity(), m_id(this), m_name(name), m_source("noSrc") {
    rulesByName().insert(std::pair<double, RuleId>(m_name.getKey(), m_id));
  }

  Rule::Rule(const LabelStr &name, const LabelStr &source)
    : Entity(), m_id(this), m_name(name), m_source(source) {
    rulesByName().insert(std::pair<double, RuleId>(m_name.getKey(), m_id));
  }

  Rule::~Rule(){
    check_error(m_id.isValid());

    bool purging = isPurging();

    if (!purging) {
      rulesByName().erase(m_name.getKey());
    }

    m_id.remove();
  }

  const RuleId& Rule::getId() const{return m_id;}

  const LabelStr& Rule::getName() const{return m_name;}

  std::multimap<double, RuleId>& Rule::rulesByName(){
    static std::multimap<double, RuleId> sl_rulesByName;
    return sl_rulesByName;
  }

  void Rule::getRules(const LabelStr& name, std::vector<RuleId>& results){
    SchemaId schema = Schema::instance();

    std::multimap<double, RuleId>::const_iterator it = rulesByName().find(name.getKey());
    while(it != rulesByName().end()){
      RuleId rule = it->second;
      check_error(rule.isValid());

      if(rule->getName() != name)
	break;
  
      results.push_back(rule);
      ++it;
    }

    // If the predicate is defined on the parent class, then
    // call this function recursively
    if(schema->hasParent(name))
      getRules(schema->getParent(name), results);
  }

  const std::multimap<double, RuleId>& Rule::getRules(){
    return rulesByName();
  }

  void Rule::purgeAll(){
    isPurging() = true;
    std::multimap<double, RuleId>& rules = rulesByName();
    for(std::multimap<double, RuleId>::const_iterator it = rules.begin(); it != rules.end(); ++it){
      RuleId rule = it->second;
      rule->discard();
    }

    rules.clear();
    isPurging() = false;
  }

  bool & Rule::isPurging(){
    static bool sl_purging(false);
    return sl_purging;
  }

}
