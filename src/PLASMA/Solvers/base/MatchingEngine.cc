#include "MatchingEngine.hh"
#include "MatchingRule.hh"
#include "ConstrainedVariable.hh"
#include "Token.hh"
#include "Object.hh"
#include "RuleInstance.hh"
#include "Schema.hh"
#include "Utils.hh"
#include "SolverUtils.hh"

namespace EUROPA {
  namespace SOLVERS {

    
    std::map<double, MatchFinderId>& MatchingEngine::getEntityMatchers() 
    { 
        static std::map<double, MatchFinderId> entityMatchers;
    	
    	return entityMatchers; 
    }

    MatchingEngine::MatchingEngine(const TiXmlElement& configData, const char* ruleTag) 
      : m_id(this), m_cycleCount(1) {
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
          ComponentId componentInstance = Component::AbstractFactory::allocate(*child);
          checkError(componentInstance.isValid(), componentInstance << ":" << child->Value());
          checkError(MatchingRuleId::convertable(componentInstance), "Bad component for " << child->Value());
          MatchingRuleId rule = componentInstance;
          rule->initialize(getId());
          debugMsg("MatchingEngine:MatchingEngine", "Adding " << rule->toString());
        }
      }
    }

    MatchingEngine::~MatchingEngine(){
      cleanup(m_rules);
      m_id.remove();
    }

    const MatchingEngineId& MatchingEngine::getId() const { return m_id; }

    void MatchingEngine::registerRule(const MatchingRuleId& rule){
      checkError(m_rules.find(rule) == m_rules.end(), rule->toString() << " already regisered.");
      debugMsg("MatchingEngine:registerRule", rule->toString());

      m_rules.insert(rule);

      std::string expression = rule->toString();
      LabelStr expressionLabel(expression);
      m_rulesByExpression.insert(std::pair<double, MatchingRuleId>(expressionLabel.getKey(), rule));

      if(rule->staticFilterCount() == 0){
        m_unfilteredRules.push_back(rule);
        return;
      }

      addFilter(rule->objectTypeFilter(), rule, m_rulesByObjectType);
      addFilter(rule->predicateFilter(), rule, m_rulesByPredicate);
      addFilter(rule->variableFilter(), rule, m_rulesByVariable);

      if(rule->filteredByMasterRelation()){
        static const LabelStr BEFORE("before");
        static const LabelStr AFTER("after");
        static const LabelStr MEETS("meets");
        static const LabelStr MET_BY("met_by");
        const LabelStr& relation = rule->masterRelationFilter();
        addFilter(relation, rule, m_rulesByMasterRelation);

        // Post for matchable relations too
        if(relation == BEFORE)
          addFilter(MEETS, rule, m_rulesByMasterRelation);
        else if(relation == AFTER)
          addFilter(MET_BY, rule, m_rulesByMasterRelation);
      }

      addFilter(rule->masterObjectTypeFilter(), rule, m_rulesByMasterObjectType);

      addFilter(rule->masterPredicateFilter(), rule, m_rulesByMasterPredicate);
    }

    void MatchingEngine::addFilter(const LabelStr& label, const MatchingRuleId& rule, 
                                   std::multimap<double,MatchingRuleId>& index){
      if(label != WILD_CARD()){
        debugMsg("MatchingEngine:addFilter", "Adding " << rule->toString() << " for label " << label.toString());
        index.insert(std::pair<double,MatchingRuleId>(label.getKey(), rule));
      }
    }

    bool MatchingEngine::hasRule(const LabelStr& expression) const {
      return m_rulesByExpression.find(expression.getKey()) != m_rulesByExpression.end();
    }

    unsigned int MatchingEngine::cycleCount() const {
      return m_cycleCount;
    }

    void MatchingEngine::addMatchFinder(const LabelStr& type, const MatchFinderId& finder) {
      checkError(getEntityMatchers().find(type) == getEntityMatchers().end(),
		 "Already know how to match entities of type " << type.toString());
      getEntityMatchers().insert(std::make_pair((double) type, finder));
    }

    void MatchingEngine::removeMatchFinder(const LabelStr& type) {
      checkError(getEntityMatchers().find(type) != getEntityMatchers().end(),
         "Could not find matcher for entities of  type " << type.toString());
      getEntityMatchers().erase((double) type);
    }

    template<>
    void MatchingEngine::getMatches(const EntityId& entity,
				    std::vector<MatchingRuleId>& results) {
      std::map<double, MatchFinderId>::iterator it =
	getEntityMatchers().find(entity->entityType());
      checkError(it != getEntityMatchers().end(),
		 "No way to match entities of type " << entity->entityType().toString());
      it->second->getMatches(getId(), entity, results);
    }

    template<>
    void MatchingEngine::getMatches(const ConstrainedVariableId& var,
				    std::vector<MatchingRuleId>& results) {
      m_cycleCount++;
      results = m_unfilteredRules;
      
      // If it has a parent, then process that too
      if(var->getParent().isId()){
        if(TokenId::convertable(var->getParent()))
          getMatchesInternal(TokenId(var->getParent()), results);
        else if(RuleInstanceId::convertable(var->getParent()))
          getMatchesInternal(RuleInstanceId(var->getParent())->getToken(), results);
        else if(ObjectId::convertable(var->getParent())){
          ObjectId object = var->getParent();
          trigger(Schema::instance()->getAllObjectTypes(object->getType()), m_rulesByObjectType, results);
        }
      }

      trigger(var->getName(), m_rulesByVariable, results);
    }

    template<>
    void MatchingEngine::getMatches(const TokenId& token, std::vector<MatchingRuleId>& results) {
      m_cycleCount++;
      results = m_unfilteredRules;
      getMatchesInternal(token, results);
    }

    unsigned int MatchingEngine::ruleCount() const {
      return m_rules.size();
    }

    /**
     * @brief todo. Fire for all cases
     */
    template<>
    void MatchingEngine::getMatchesInternal(const TokenId& token, std::vector<MatchingRuleId>& results){
      // Fire for predicate
      LabelStr unqualifiedName = token->getUnqualifiedPredicateName();
      debugMsg("MatchingEngine:getMatchesInternal", "Triggering matches for predicate " << unqualifiedName.toString());
      trigger(unqualifiedName, m_rulesByPredicate, results);

      SchemaId schema = token->getPlanDatabase()->getSchema();
      
      // Fire for class and all super classes
      debugMsg("MatchingEngine:getMatchesInternal", "Triggering matches for object types (" << token->getBaseObjectType() << ")");
      trigger(schema->getAllObjectTypes(token->getBaseObjectType()), 
              m_rulesByObjectType, results);

      // If it has a master, trigger on the relation
      if(token->getMaster().isId()){
        debugMsg("MatchingEngine:getMatchesInternal", "Triggering matches for master object types (" << token->getMaster()->getBaseObjectType().toString() << ")");
        trigger(schema->getAllObjectTypes(token->getMaster()->getBaseObjectType()), 
                m_rulesByMasterObjectType, results);
        debugMsg("MatchingEngine:getMatchesInternal", "Triggering matches for master predicate " << token->getMaster()->getUnqualifiedPredicateName().toString());
        trigger(token->getMaster()->getUnqualifiedPredicateName(), m_rulesByMasterPredicate, results);
        debugMsg("MatchingEngine:getMatchesInternal", "Triggering matches for master relation " << token->getRelation().toString());
        trigger(token->getRelation(), m_rulesByMasterRelation, results);
      }
      else { // Trigger for those registered for 'none' explicitly
        static const LabelStr none("none");
        debugMsg("MatchingEngine:getMatchesInternal", "Triggering matches for 'none' master relation.");
        trigger(none, m_rulesByMasterRelation, results);
      }
    }

    void MatchingEngine::trigger(const LabelStr& lbl, 
                                 const std::multimap<double, MatchingRuleId>& rules,
                                 std::vector<MatchingRuleId>& results){
      debugMsg("MatchingEngine:trigger", "Searching with label " << lbl.toString());
      debugMsg("MatchingEngine:verboseTrigger", "Searching in " << std::endl << rulesToString(rules));
      unsigned int addedCount = 0;
      double key = lbl.getKey();
      std::multimap<double, MatchingRuleId>::const_iterator it = rules.find(key);
      while(it != rules.end() && it->first == key){
        MatchingRuleId rule = it->second;
        if(rule->fire()) {
          results.push_back(rule);
          addedCount++;
        }
        ++it;
      }

      debugMsg("MatchingEngine:trigger", 
               "Found " << results.size() << " matches for " << lbl.toString() << " so far.  Added " << addedCount);
    }

    void MatchingEngine::trigger(const std::vector<LabelStr>& labels, 
                                 const std::multimap<double, MatchingRuleId>& rules,
                                 std::vector<MatchingRuleId>& results){
      for(std::vector<LabelStr>::const_iterator it = labels.begin(); it != labels.end(); ++it){
        const LabelStr& label = *it;
        trigger(label, rules, results);                  
      }
    }

    std::string MatchingEngine::rulesToString(const std::multimap<double, MatchingRuleId>& rules) {
      std::stringstream str;
      double current = -1.0; 
      for(std::multimap<double, MatchingRuleId>::const_iterator it = rules.begin(); it != rules.end(); ++it) {
        if(it->first != current) {
          current = it->first;
          str << "'" << LabelStr(current).toString() << "':" << std::endl;
        }
        str << "  " <<it->second->toString() << std::endl;
      }
      return str.str();
    }

    void VariableMatchFinder::getMatches(const MatchingEngineId& engine, const EntityId& entity,
					 std::vector<MatchingRuleId>& results) {
      engine->getMatches(ConstrainedVariableId(entity), results);
    }

    void TokenMatchFinder::getMatches(const MatchingEngineId& engine, const EntityId& entity,
				      std::vector<MatchingRuleId>& results) {
      engine->getMatches(TokenId(entity), results);
    }
  }
}
