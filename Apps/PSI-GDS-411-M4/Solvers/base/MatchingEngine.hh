#ifndef H_MatchingEngine
#define H_MatchingEngine

/**
 * @author Conor McGann
 * @file Declares the MatchingEngine class for matching variables and tokens in the database with rules.
 */

#include "SolverDefs.hh"
#include "XMLUtils.hh"
#include "SAVH_ResourceDefs.hh"
#include <map>
#include <set>

namespace EUROPA {
  namespace SOLVERS {

    class MatchingEngine {
    public:
      MatchingEngine(const TiXmlElement& configData, const char* ruleTag = "MatchingRule");

      virtual ~MatchingEngine();

      const MatchingEngineId& getId() const;

      /**
       * @brief Retrives all relevant matching rules for the given variable
       */
      void getVariableMatches(const ConstrainedVariableId& var, std::vector<MatchingRuleId>& results);

      /**
       * @brief Retrievs all relevant matching rules for te given token
       */
      void getTokenMatches(const TokenId& token, std::vector<MatchingRuleId>& results);

      /**
       * @brief Retrieves all relevant matching rules for the given instant.
       */
      void getInstantMatches(const SAVH::InstantId& inst, std::vector<MatchingRuleId>& results);

      /**
       * @brief Adds a rule. Details of how it is indexed are internal
       */
      void registerRule(const MatchingRuleId& rule);

      /**
       * @brief test if a given rule expression is present.
       * @param expression A string expression for the rule
       * @see MatchingRule::toString(), MatchingRule::setExpression
       */
      bool hasRule(const LabelStr& expression) const;

      /**
       * @brief The last count of matches tried.
       */
      unsigned int cycleCount() const;

      /**
       * @brief Get the total number of registered rules.
       */
      unsigned int ruleCount() const;

      const std::set<MatchingRuleId>& getRules() const {return m_rules;}

    private:

      /**
       * @brief Utility method to add a rule to an index if it is required.
       */
      void addFilter(const LabelStr& label, const MatchingRuleId& rule, 
		     std::multimap<double,MatchingRuleId>& index);

      void getMatchesInternal(const TokenId& token, std::vector<MatchingRuleId>& results);

      void getMatchesInternal(const SAVH::InstantId& inst, std::vector<MatchingRuleId>& results);

      /**
       * @brief Utility method to trigger rules along a given index.
       */
      void trigger(const LabelStr& lbl, 
		   const std::multimap<double, MatchingRuleId>& rules,
		   std::vector<MatchingRuleId>& results);

      /**
       * @brief Utility method to trigger rules along a given index for each element in the vector
       */
      void trigger(const std::vector<LabelStr>& labels, 
		   const std::multimap<double, MatchingRuleId>& rules,
		   std::vector<MatchingRuleId>& results);

      std::string rulesToString(const std::multimap<double, MatchingRuleId>& rules);

      /**
       * @brief Utility to handle the recursive triggering for a class and its super class.
       */
      MatchingEngineId m_id;

      /**
       * Declarations basically support indexing by each criteria.
       */
      std::multimap<double, MatchingRuleId> m_rulesByObjectType;
      std::multimap<double, MatchingRuleId> m_rulesByPredicate;
      std::multimap<double, MatchingRuleId> m_rulesByVariable;
      std::multimap<double, MatchingRuleId> m_rulesByMasterObjectType;
      std::multimap<double, MatchingRuleId> m_rulesByMasterPredicate;
      std::multimap<double, MatchingRuleId> m_rulesByMasterRelation;

      unsigned int m_cycleCount; /*!< Used to reset all rule firing data. Updated on each call to match. */
      std::set<MatchingRuleId> m_rules; /*!< The set of all rules. */
      std::multimap<double, MatchingRuleId> m_rulesByExpression; /*!< All rules by expression */
      std::vector<MatchingRuleId> m_unfilteredRules; /*!< All rules without filters */
    };
  }
}
#endif
