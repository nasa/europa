#ifndef H_Condition
#define H_Condition

/**
 * @brief Declares a Condition class to define the interface for matching entities and to provide the basic static
 * matching behavior.
 * @author Conor McGann
 */

#include "SolverDefs.hh"
#include "ComponentFactory.hh"

namespace EUROPA {
namespace SOLVERS {

class MatchingRule: public Component {
 public:
  /**
   * @brief Construct a matching rule
   */
  MatchingRule(const TiXmlElement& configData);

  /**
   * @brief Sets link to matching engine to obtain cycle count
   */
  void initialize(const MatchingEngineId matchingEngine);

  /**
   * @brief Test if all conditions have been met for this rule, in this iteration of the matching engine
   */
  bool fire();

  /**
   * @brief Retrieves a string expression for the scope over which this filter is evaluated.
   */
  virtual std::string toString() const;

  /**
   * @brief The number of static filters on this rule
   */
  virtual unsigned int staticFilterCount() const;

  bool filteredByObjectType() const;

  const std::string& objectTypeFilter() const;

  bool filteredByPredicate() const;

  const std::string& predicateFilter() const;

  bool filteredByVariable() const;

  const std::string& variableFilter() const;

  bool filteredByMasterObjectType() const;

  const std::string& masterObjectTypeFilter() const;

  bool filteredByMasterPredicate() const;

  const std::string& masterPredicateFilter() const;

  bool filteredByMasterRelation() const;

  const std::string& masterRelationFilter() const;

  bool filteredByTokenName() const;

  const std::string& tokenNameFilter() const;

  ContextId getContext() const {return m_context;}
  virtual void setContext(ContextId ctx) {check_error(ctx != ContextId::noId()); m_context = ctx;}
 protected:
  void setExpression(const std::string& expression);
  ContextId m_context;
 private:
  MatchingRule(const MatchingRule&); /*!< NO IMPL */
  std::string m_label; /*!< Stores the label for the rule. Optional to set this. */
  std::string m_expression; /*!< Stores the matching expression as a string for display */

  std::string m_objectType;
  std::string m_predicate;
  std::string m_variable;
  std::string m_masterObjectType;
  std::string m_masterPredicate;
  std::string m_masterRelation;
  std::string m_tokenName;
  unsigned int m_staticFilterCount; /*!< Count of the number of static filters on this rule */
  unsigned int m_lastCycle; /*!< The last MatchingEngine cycle */
  unsigned int m_hitCount; /*!< Count of hits in the current matching cycle */
  MatchingEngineId m_matchingEngine;
};
  }
}
#endif
