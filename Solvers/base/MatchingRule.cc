#include "MatchingRule.hh"
#include "MatchingEngine.hh"
#include "Object.hh"
#include "Token.hh"
#include "ConstrainedVariable.hh"
#include "SolverUtils.hh"
#include "Debug.hh"

/**
 * @author Conor McGann
 * @date May, 2005
 * @file Provides implementation for core rule matching classes.
 */
namespace EUROPA {
  namespace SOLVERS {

    MatchingRule::MatchingRule(const TiXmlElement& configData)
      : Component(configData), 
        m_objectType(WILD_CARD()), m_predicate(WILD_CARD()), m_variable(WILD_CARD()), 
        m_masterObjectType(WILD_CARD()), m_masterPredicate(WILD_CARD()), m_masterRelation(WILD_CARD()),
        m_staticFilterCount(0), m_lastCycle(0) {

      std::string expr;
      if(configData.Attribute("label") != NULL){
        m_label = configData.Attribute("label");
        expr = "[" + m_label.toString() + "]";
      }

      if(configData.Attribute("class-match") != NULL){
        m_objectType = configData.Attribute("class-match");
        m_staticFilterCount++;
      }
      else if(configData.Attribute("class") != NULL){
        m_objectType = configData.Attribute("class");
        m_staticFilterCount++;
      }

      if(configData.Attribute("predicate-match") != NULL){
        m_predicate = configData.Attribute("predicate-match");
        m_staticFilterCount++;
      }
      else if(configData.Attribute("predicate") != NULL){
        m_predicate = configData.Attribute("predicate");
        m_staticFilterCount++;
      }

      if(configData.Attribute("var-match") != NULL){
        m_variable = configData.Attribute("var-match");
        m_staticFilterCount++;m_variable.toString();
      }
      else if(configData.Attribute("variable") != NULL){
        m_variable = configData.Attribute("variable");
        m_staticFilterCount++;
      }

      if(configData.Attribute("masterRelation") != NULL){
        m_masterRelation = configData.Attribute("masterRelation");
        m_staticFilterCount++;
      }

      if(configData.Attribute("masterClass") != NULL){
        m_masterObjectType = configData.Attribute("masterClass");
        m_staticFilterCount++;
      }

      if(configData.Attribute("masterPredicate") != NULL){
        m_masterPredicate = configData.Attribute("masterPredicate");
        m_staticFilterCount++;
      }

      expr = expr + m_objectType.toString() + "." +
        m_predicate.toString() + "." +
        m_variable.toString() + "." +
        m_masterRelation.toString() + "." +
        m_masterObjectType.toString() + "." +
        m_masterPredicate.toString();

      setExpression(expr);
    }

    void MatchingRule::initialize(const MatchingEngineId& matchingEngine){
      m_matchingEngine = matchingEngine;
      m_matchingEngine->registerRule(getId());
    }

    bool MatchingRule::fire(){
      checkError(m_staticFilterCount > 0, "Should not be calling this if it has no filters.");
      debugMsg("MatchingRule:fire", "Firing rule " << toString() << " (hits " << m_hitCount << ", filters " << m_staticFilterCount << ") " <<
               "(last cycle " << m_lastCycle << ", cycle count " << m_matchingEngine->cycleCount() << ")");

      if(m_lastCycle < m_matchingEngine->cycleCount()){
        m_lastCycle = m_matchingEngine->cycleCount();
        m_hitCount = 0;
      }

      m_hitCount++;

      return m_hitCount == m_staticFilterCount;
    }

    unsigned int MatchingRule::staticFilterCount() const { return m_staticFilterCount;}

    void MatchingRule::setExpression(const std::string& expression){
      m_expression = expression;
      debugMsg("MatchingRule:setExpression", expression);
    }

    std::string MatchingRule::toString() const {return m_expression;}

    bool MatchingRule::filteredByObjectType() const { return m_objectType != WILD_CARD(); }

    const LabelStr& MatchingRule::objectTypeFilter() const{ return m_objectType;}

    bool MatchingRule::filteredByPredicate() const{ return m_predicate != WILD_CARD(); }

    const LabelStr& MatchingRule::predicateFilter() const{ return m_predicate; }

    bool MatchingRule::filteredByVariable() const{ return m_variable != WILD_CARD(); }

    const LabelStr& MatchingRule::variableFilter() const { return m_variable; }

    bool MatchingRule::filteredByMasterObjectType() const{ return m_objectType != WILD_CARD(); }

    const LabelStr& MatchingRule::masterObjectTypeFilter() const { return m_masterObjectType; }

    bool MatchingRule::filteredByMasterPredicate() const{ return m_objectType != WILD_CARD(); }

    const LabelStr& MatchingRule::masterPredicateFilter() const { return m_masterPredicate; }

    bool MatchingRule::filteredByMasterRelation() const{ return m_masterRelation != WILD_CARD(); }

    const LabelStr& MatchingRule::masterRelationFilter() const { return m_masterRelation; }

  }
}
