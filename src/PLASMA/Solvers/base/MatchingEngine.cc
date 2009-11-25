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


    MatchingEngine::MatchingEngine(EngineId& engine,const TiXmlElement& configData, const char* ruleTag)
      : m_id(this)
      , m_engine(engine)
      , m_cycleCount(1)
    {
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
          ComponentFactoryMgr* cfm = (ComponentFactoryMgr*)engine->getComponent("ComponentFactoryMgr");
          ComponentId componentInstance = cfm->createInstance(*child);
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

    MatchingEngineId& MatchingEngine::getId() { return m_id; }

    void MatchingEngine::registerRule(const MatchingRuleId& rule){
      checkError(m_rules.find(rule) == m_rules.end(), rule->toString() << " already regisered.");
      debugMsg("MatchingEngine:registerRule", rule->toString());

      m_rules.insert(rule);

      std::string expression = rule->toString();
      LabelStr expressionLabel(expression);
      m_rulesByExpression.insert(std::make_pair(expressionLabel.getKey(), rule));

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
                                   std::multimap<edouble,MatchingRuleId>& index){
      if(label != WILD_CARD()){
        debugMsg("MatchingEngine:addFilter", "Adding " << rule->toString() << " for label " << label.toString());
        index.insert(std::make_pair(label.getKey(), rule));
      }
    }

    bool MatchingEngine::hasRule(const LabelStr& expression) const {
      return m_rulesByExpression.find(expression.getKey()) != m_rulesByExpression.end();
    }

    unsigned int MatchingEngine::cycleCount() const {
      return m_cycleCount;
    }

    template<>
    void MatchingEngine::getMatches(const EntityId& entity,
				    std::vector<MatchingRuleId>& results) {
      std::map<edouble, MatchFinderId>::iterator it =
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
      if(token->master().isId()){
        debugMsg("MatchingEngine:getMatchesInternal", "Triggering matches for master object types (" << token->master()->getBaseObjectType().toString() << ")");
        trigger(schema->getAllObjectTypes(token->master()->getBaseObjectType()),
                m_rulesByMasterObjectType, results);
        debugMsg("MatchingEngine:getMatchesInternal", "Triggering matches for master predicate " << token->master()->getUnqualifiedPredicateName().toString());
        trigger(token->master()->getUnqualifiedPredicateName(), m_rulesByMasterPredicate, results);
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
                                 const std::multimap<edouble, MatchingRuleId>& rules,
                                 std::vector<MatchingRuleId>& results){
      debugMsg("MatchingEngine:trigger", "Searching with label " << lbl.toString());
      debugMsg("MatchingEngine:verboseTrigger", "Searching in " << std::endl << rulesToString(rules));
      unsigned int addedCount = 0;
      edouble key = lbl.getKey();
      std::multimap<edouble, MatchingRuleId>::const_iterator it = rules.find(key);
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
                                 const std::multimap<edouble, MatchingRuleId>& rules,
                                 std::vector<MatchingRuleId>& results){
      for(std::vector<LabelStr>::const_iterator it = labels.begin(); it != labels.end(); ++it){
        const LabelStr& label = *it;
        trigger(label, rules, results);
      }
    }

    std::string MatchingEngine::rulesToString(const std::multimap<edouble, MatchingRuleId>& rules) {
      std::stringstream str;
      edouble current = -1.0; 
      for(std::multimap<edouble, MatchingRuleId>::const_iterator it = rules.begin(); it != rules.end(); ++it) {
        if(it->first != current) {
          current = it->first;
          str << "'" << LabelStr(current).toString() << "':" << std::endl;
        }
        str << "  " <<it->second->toString() << std::endl;
      }
      return str.str();
    }
    
    std::map<edouble, MatchFinderId>& MatchingEngine::getEntityMatchers() 
    { 
        MatchFinderMgr* mfm = (MatchFinderMgr*)m_engine->getComponent("MatchFinderMgr");        
        return mfm->getEntityMatchers(); 
    }    
    
    MatchFinderMgr::MatchFinderMgr()
    {
    }

    MatchFinderMgr::~MatchFinderMgr()
    {
        purgeAll();
    }

    void MatchFinderMgr::addMatchFinder(const LabelStr& type, const MatchFinderId& finder) {
        // Remove first in case one already exists
        removeMatchFinder(type);
        getEntityMatchers().insert(std::make_pair(type, finder));
    }

    void MatchFinderMgr::removeMatchFinder(const LabelStr& type)
    {
      std::map<edouble, MatchFinderId>::iterator it = getEntityMatchers().find(type);
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
    
    std::map<edouble, MatchFinderId>& MatchFinderMgr::getEntityMatchers() { return m_entityMatchers; }
    
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
