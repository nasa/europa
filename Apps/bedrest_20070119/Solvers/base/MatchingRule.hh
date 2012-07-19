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
      void initialize(const MatchingEngineId& matchingEngine);

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

      const LabelStr& objectTypeFilter() const;

      bool filteredByPredicate() const;

      const LabelStr& predicateFilter() const;

      bool filteredByVariable() const;

      const LabelStr& variableFilter() const;

      bool filteredByMasterObjectType() const;

      const LabelStr& masterObjectTypeFilter() const;

      bool filteredByMasterPredicate() const;

      const LabelStr& masterPredicateFilter() const;

      bool filteredByMasterRelation() const;

      const LabelStr& masterRelationFilter() const;

      ContextId getContext() {return m_context;}
      void setContext(ContextId ctx) {m_context = ctx;}
    protected:
      void setExpression(const std::string& expression);
      ContextId m_context;
    private:
      MatchingRule(const MatchingRule&); /*!< NO IMPL */
      LabelStr m_label; /*!< Stores the label for the rule. Optional to set this. */
      std::string m_expression; /*!< Stores the matching expression as a string for display */

      LabelStr m_objectType;
      LabelStr m_predicate;
      LabelStr m_variable;
      LabelStr m_masterObjectType;
      LabelStr m_masterPredicate;
      LabelStr m_masterRelation;
      unsigned int m_staticFilterCount; /*!< Count of the number of static filters on this rule */
      unsigned int m_lastCycle; /*!< The last MatchingEngine cycle */
      unsigned int m_hitCount; /*!< Count of hits in the current matching cycle */
      MatchingEngineId m_matchingEngine;
    };
  }
}
#endif
