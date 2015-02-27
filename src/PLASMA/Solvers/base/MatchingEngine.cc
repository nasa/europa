#include "MatchingEngine.hh"
#include "MatchingRule.hh"
#include "ConstrainedVariable.hh"
#include "Token.hh"
#include "Object.hh"
#include "RuleInstance.hh"
#include "Schema.hh"
#include "Utils.hh"
#include "SolverUtils.hh"
#include "tinyxml.h"

namespace EUROPA {
namespace SOLVERS {

MatchingEngine::MatchingEngine(EngineId engine,
                               const TiXmlElement& configData,
                               const char* ruleTag)
    : m_id(this)
    , m_engine(engine)
    , m_rulesByObjectType()
    , m_rulesByPredicate()
    , m_rulesByVariable()
    , m_rulesByMasterObjectType()
    , m_rulesByMasterPredicate()
    , m_rulesByMasterRelation()
    , m_rulesByTokenName()
    , m_cycleCount(1),
      m_rules(),
      m_rulesByExpression(),
      m_unfilteredRules() {
  // Now load all the flaw managers
  std::string ruleTagStr(ruleTag);

  for (TiXmlElement * child = configData.FirstChildElement();
       child != NULL;
       child = child->NextSiblingElement()) {
    const char* component = child->Attribute("component");

    if(ruleTagStr.find(child->Value(), 0) != std::string::npos){
      // If no component name is provided, register it with the tag name of configuration element
      // thus obtaining the default.
      if(component == NULL)
        child->SetAttribute("component", child->Value());

      // Now allocate the particular flaw manager using an abstract factory pattern.
      ComponentFactoryMgr* cfm =
          reinterpret_cast<ComponentFactoryMgr*>(engine->getComponent("ComponentFactoryMgr"));
      ComponentId componentInstance = cfm->createComponentInstance(*child);
      checkError(componentInstance.isValid(), componentInstance << ":" << child->Value());
      checkError(MatchingRuleId::convertable(componentInstance), "Bad component for " << child->Value());
      MatchingRuleId rule = componentInstance;
      rule->initialize(getId());
      debugMsg("MatchingEngine:MatchingEngine", "Adding " << rule->toString());
    }
  }
}

MatchingEngine::~MatchingEngine() {
  cleanup(m_rules);
  m_id.remove();
}

    MatchingEngineId MatchingEngine::getId() { return m_id; }

    void MatchingEngine::registerRule(const MatchingRuleId rule){
      checkError(m_rules.find(rule) == m_rules.end(), rule->toString() << " already registered.");
      debugMsg("MatchingEngine:registerRule", rule->toString());

      m_rules.insert(rule);

      std::string expression = rule->toString();
      std::string expressionLabel(expression);
      m_rulesByExpression.insert(std::make_pair(expressionLabel, rule));

      if(rule->staticFilterCount() == 0){
        m_unfilteredRules.push_back(rule);
        return;
      }

      addFilter(rule->objectTypeFilter(), rule, m_rulesByObjectType);
      addFilter(rule->predicateFilter(), rule, m_rulesByPredicate);
      addFilter(rule->variableFilter(), rule, m_rulesByVariable);
      addFilter(rule->masterObjectTypeFilter(), rule, m_rulesByMasterObjectType);
      addFilter(rule->masterPredicateFilter(), rule, m_rulesByMasterPredicate);
      addFilter(rule->tokenNameFilter(), rule, m_rulesByTokenName);

      if(rule->filteredByMasterRelation()){
        static const std::string BEFORE("before");
        static const std::string AFTER("after");
        static const std::string MEETS("meets");
        static const std::string MET_BY("met_by");

        const std::string& relation = rule->masterRelationFilter();
        addFilter(relation, rule, m_rulesByMasterRelation);

        // Post for matchable relations too
        if(relation == BEFORE)
          addFilter(MEETS, rule, m_rulesByMasterRelation);
        else if(relation == AFTER)
          addFilter(MET_BY, rule, m_rulesByMasterRelation);
      }
    }

void MatchingEngine::addFilter(const std::string& label, const MatchingRuleId rule, 
                               std::multimap<std::string,MatchingRuleId>& index){
  if(label != WILD_CARD()){
    debugMsg("MatchingEngine:addFilter",
             "Adding " << rule->toString() << " for label " << label);
    index.insert(std::make_pair(label, rule));
  }
}

bool MatchingEngine::hasRule(const std::string& expression) const {
  return m_rulesByExpression.find(expression) != m_rulesByExpression.end();
}

    unsigned int MatchingEngine::cycleCount() const {
      return m_cycleCount;
    }

template<>
void MatchingEngine::getMatches(const EntityId entity,
                                std::vector<MatchingRuleId>& results) {
  std::map<std::string, MatchFinderId>::iterator it =
      getEntityMatchers().find(entity->entityType());
  checkError(it != getEntityMatchers().end(),
             "No way to match entities of type " << entity->entityType());
  it->second->getMatches(getId(), entity, results);
}

template<>
void MatchingEngine::getMatches(const ConstrainedVariableId var,
                                std::vector<MatchingRuleId>& results) {
  m_cycleCount++;
  results = m_unfilteredRules;

  // If it has a parent, then process that too
  if(var->parent().isId()){
    if(TokenId::convertable(var->parent()))
      getMatchesInternal(TokenId(var->parent()), results);
    else if(RuleInstanceId::convertable(var->parent()))
      getMatchesInternal(RuleInstanceId(var->parent())->getToken(), results);
    else if(ObjectId::convertable(var->parent())){
      ObjectId object = var->parent();
      trigger(object->getPlanDatabase()->getSchema()->getAllObjectTypes(object->getType()), m_rulesByObjectType, results);
    }
  }

  trigger(var->getName(), m_rulesByVariable, results);
}

    template<>
    void MatchingEngine::getMatches(const TokenId token, std::vector<MatchingRuleId>& results) {
      m_cycleCount++;
      results = m_unfilteredRules;
      getMatchesInternal(token, results);
    }

    unsigned long MatchingEngine::ruleCount() const {
      return m_rules.size();
    }

namespace {
std::string rulesToString(const std::multimap<std::string, MatchingRuleId>& rules) {
  std::stringstream str;
  std::string current = "";
  for(std::multimap<std::string, MatchingRuleId>::const_iterator it = rules.begin();
      it != rules.end(); ++it) {
    if(it->first != current) {
      current = it->first;
      str << "'" << current << "':" << std::endl;
    }
    str << "  " <<it->second->toString() << std::endl;
  }
  return str.str();
}

bool matches(MatchingRuleId rule, const std::string& value, const std::string& key) {
  if (rule->filteredByTokenName()) {
    bool result = key.find(value) != std::string::npos;
    debugMsg("MatchingEngine:tokenName",
             "result=" << result << " key=" << key << " value=" << value);
    return result;
  }
  else
    return value==key;
}
  
void triggerTokenByName(const std::string& lbl,
                        const std::multimap<std::string, MatchingRuleId>& rules,
                        std::vector<MatchingRuleId>& results) {
  debugMsg("MatchingEngine:trigger", "Searching with label " << lbl);
  debugMsg("MatchingEngine:verboseTrigger", "Searching in " << std::endl << rulesToString(rules));
  unsigned int addedCount = 0;
  std::multimap<std::string, MatchingRuleId>::const_iterator it = rules.begin();
  while(it != rules.end()) {
    if (matches(it->second,it->first,lbl)) {
      MatchingRuleId rule = it->second;
      if(rule->fire()) {
        results.push_back(rule);
        addedCount++;
      }
    }
    ++it;
  }

  debugMsg("MatchingEngine:trigger",
           "Found " << results.size() << " matches for " << lbl <<
           " so far.  Added " << addedCount);
}
}
    /**
     * @brief todo. Fire for all cases
     */
    template<>
    void MatchingEngine::getMatchesInternal(const TokenId token, std::vector<MatchingRuleId>& results){
      // Fire for predicate
      std::string unqualifiedName = token->getUnqualifiedPredicateName();
      debugMsg("MatchingEngine:getMatchesInternal", "Triggering matches for predicate " << unqualifiedName);
      trigger(unqualifiedName, m_rulesByPredicate, results);

      // Fire for tokenName
      std::string tokenName = token->getName();
      debugMsg("MatchingEngine:getMatchesInternal", "Triggering matches for tokenName " << tokenName);
      triggerTokenByName(tokenName, m_rulesByTokenName, results);

      SchemaId schema = token->getPlanDatabase()->getSchema();

      // Fire for class and all super classes
      debugMsg("MatchingEngine:getMatchesInternal", "Triggering matches for object types (" << token->getBaseObjectType() << ")");
      trigger(schema->getAllObjectTypes(token->getBaseObjectType()),
              m_rulesByObjectType, results);

      // If it has a master, trigger on the relation
      if(token->master().isId()){
        debugMsg("MatchingEngine:getMatchesInternal", "Triggering matches for master object types (" << token->master()->getBaseObjectType() << ")");
        trigger(schema->getAllObjectTypes(token->master()->getBaseObjectType()),
                m_rulesByMasterObjectType, results);
        debugMsg("MatchingEngine:getMatchesInternal", "Triggering matches for master predicate " << token->master()->getUnqualifiedPredicateName());
        trigger(token->master()->getUnqualifiedPredicateName(), m_rulesByMasterPredicate, results);
        debugMsg("MatchingEngine:getMatchesInternal", "Triggering matches for master relation " << token->getRelation());
        trigger(token->getRelation(), m_rulesByMasterRelation, results);
      }
      else { // Trigger for those registered for 'none' explicitly
        static const std::string none("none");
        debugMsg("MatchingEngine:getMatchesInternal", "Triggering matches for 'none' master relation.");
        trigger(none, m_rulesByMasterRelation, results);
      }
    }

void MatchingEngine::trigger(const std::string& lbl, 
                             const std::multimap<std::string, MatchingRuleId>& rules,
                             std::vector<MatchingRuleId>& results){
  debugMsg("MatchingEngine:trigger", "Searching with label " << lbl);
  debugMsg("MatchingEngine:verboseTrigger", "Searching in " << std::endl << rulesToString(rules));
  unsigned int addedCount = 0;

  std::multimap<std::string, MatchingRuleId>::const_iterator it = rules.find(lbl);
  while(it != rules.end() && it->first==lbl) {
    MatchingRuleId rule = it->second;
    if(rule->fire()) {
      results.push_back(rule);
      addedCount++;
    }
    ++it;
  }

  debugMsg("MatchingEngine:trigger",
           "Found " << results.size() << " matches for " << lbl <<
           " so far.  Added " << addedCount);
}

void MatchingEngine::trigger(const std::vector<std::string>& labels, 
                             const std::multimap<std::string, MatchingRuleId>& rules,
                             std::vector<MatchingRuleId>& results) {
  for(std::vector<std::string>::const_iterator it = labels.begin(); it != labels.end(); ++it){
    const std::string& label = *it;
    trigger(label, rules, results);
  }
}

std::map<std::string, MatchFinderId>& MatchingEngine::getEntityMatchers() { 
  MatchFinderMgr* mfm =
      reinterpret_cast<MatchFinderMgr*>(m_engine->getComponent("MatchFinderMgr")); 
  return mfm->getEntityMatchers(); 
}    
    
MatchFinderMgr::MatchFinderMgr() : m_entityMatchers() {}

MatchFinderMgr::~MatchFinderMgr() {
  purgeAll();
}

    void MatchFinderMgr::addMatchFinder(const std::string& type, const MatchFinderId finder) {
        // Remove first in case one already exists
        removeMatchFinder(type);
        getEntityMatchers().insert(std::make_pair(type, finder));
    }

void MatchFinderMgr::removeMatchFinder(const std::string& type) {
  std::map<std::string, MatchFinderId>::iterator it = getEntityMatchers().find(type);
  if(it != getEntityMatchers().end()) {
    MatchFinderId oldId = it->second;
    getEntityMatchers().erase(type);
    oldId.release();
  }
}

    void MatchFinderMgr::purgeAll()
    {
        while (getEntityMatchers().size() > 0)
            removeMatchFinder(getEntityMatchers().begin()->first);    
    }    
    
std::map<std::string, MatchFinderId>& MatchFinderMgr::getEntityMatchers() { return m_entityMatchers; }
    
    void VariableMatchFinder::getMatches(const MatchingEngineId engine, const EntityId entity,
					 std::vector<MatchingRuleId>& results) {
      engine->getMatches(ConstrainedVariableId(entity), results);
    }

    void TokenMatchFinder::getMatches(const MatchingEngineId engine, const EntityId entity,
				      std::vector<MatchingRuleId>& results) {
      engine->getMatches(TokenId(entity), results);
    }
  }
}
