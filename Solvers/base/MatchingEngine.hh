#ifndef H_MatchingEngine
#define H_MatchingEngine

/**
 * @author Conor McGann
 * @file Declares the MatchingEngine class for matching variables and tokens in the database with rules.
 */

#include "SolverDefs.hh"
#include "XMLUtils.hh"
#include <map>
#include <set>
#include <typeinfo>

namespace EUROPA {
  namespace SOLVERS {

    class MatchFinder;
    typedef Id<MatchFinder> MatchFinderId;

    class MatchingEngine {
    public:
      MatchingEngine(const TiXmlElement& configData, const char* ruleTag = "MatchingRule");

      virtual ~MatchingEngine();

      const MatchingEngineId& getId() const;

      /**
       * @brief Retrieves all relevent matching rules for the given entity.
       * @note This method has no implementation.  It is expected that specializations will be
       *       written for each type.
       */
      template<typename T>
      void getMatches(const T& entity, std::vector<MatchingRuleId>& results) {
	checkError(ALWAYS_FAIL,
		   "Don't know how to match objects of type " << typeid(T).name());
      }

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

      static void addMatchFinder(const LabelStr& type, const MatchFinderId& finder);

    private:

      /**
       * @brief Utility method to add a rule to an index if it is required.
       */
      void addFilter(const LabelStr& label, const MatchingRuleId& rule, 
		     std::multimap<double,MatchingRuleId>& index);

      template<typename T>
      void getMatchesInternal(const T& entity, std::vector<MatchingRuleId>& results);

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

      static std::map<double, MatchFinderId> s_entityMatchers;
    };
    
    template<>
    void MatchingEngine::getMatches(const EntityId& entity,
				    std::vector<MatchingRuleId>& results);

    /**
     * @brief Retrives all relevant matching rules for the given variable
     */
    template<>
    void MatchingEngine::getMatches(const ConstrainedVariableId& var,
				    std::vector<MatchingRuleId>& results);
    
    /**
     * @brief Retrievs all relevant matching rules for te given token
     */
    template<>
    void MatchingEngine::getMatches(const TokenId& token,
				    std::vector<MatchingRuleId>& results);
    
    template<>
    void MatchingEngine::getMatchesInternal(const TokenId& token,
					    std::vector<MatchingRuleId>& results);

    /**
     * Class for 
     */

    class MatchFinder {
    public:
      MatchFinder() : m_id(this) {}
      const MatchFinderId& getId() const {return m_id;}
      virtual ~MatchFinder() { m_id.remove();}
      virtual void getMatches(const MatchingEngineId& engine, const EntityId& entity,
			      std::vector<MatchingRuleId>& results) = 0;
    private:
      MatchFinderId m_id;
    };

    class VariableMatchFinder : public MatchFinder {
    public:
      void getMatches(const MatchingEngineId& engine, const EntityId& entity,
		      std::vector<MatchingRuleId>& results);
    };

    class TokenMatchFinder : public MatchFinder {
    public:
      void getMatches(const MatchingEngineId& engine, const EntityId& entity,
		      std::vector<MatchingRuleId>& results);
    };

  }
}
#endif
