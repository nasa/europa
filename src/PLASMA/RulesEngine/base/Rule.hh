#ifndef _H_Rule
#define _H_Rule


/**
 * @file Constraint.hh
 * @author Conor McGann
 * @date November, 2003
 * @brief Declares the Rule base class which is the point of integration for external rules
 * languages.
 */

#include "RulesEngineDefs.hh"
#include "LabelStr.hh"
#include "Engine.hh"
#include <vector>
#include <map>

namespace EUROPA {

  /**
   * @class RuleSchema
   * @brief Meta-info about rules
   */
  class RuleSchema : public EngineComponent
  {
    public:
      RuleSchema();
      ~RuleSchema();

      /**
       * @brief Accessor
       */
      const RuleSchemaId& getId() const;

      void registerRule(const RuleId& rule);

      /**
       * @brief Retrieve all registered rules for the given predicate. This will include rules
       * defined in ancestors of the current predicate also.
       */
      void getRules(const PlanDatabaseId& pdb, const LabelStr& predicate, std::vector<RuleId>& results);

      const std::multimap<edouble, RuleId>& getRules();

      /**
       * @brief Delete all rules stored.
       */
      void purgeAll();

    protected:
      RuleSchemaId m_id; /*!< Id for reference */
      std::multimap<edouble, RuleId> m_rulesByName;
  };

  /**
   * @class Rule
   * @brief Defines an abstract base class that is implemented by a provider of rules.
   * @see RuleContext, RulesEngine
   */
  class Rule
  {
    public:
      virtual ~Rule();

      /**
       * @brief Accessor
       */
      const RuleId& getId() const;

      /**
       * @brief Accessor
       * @return The predicate for which this rule applies.
       */
      const LabelStr& getName() const;

      const LabelStr& getSource() const;

      virtual RuleInstanceId createInstance(const TokenId& token,
                                            const PlanDatabaseId& planDb,
                                            const RulesEngineId &rulesEngine) const = 0;

      virtual std::string toString() const;

    protected:
      /**
       * @brief Constructor.
       * @param name A unique name for the rule.
       */
      Rule(const LabelStr& name);
      Rule(const LabelStr& name, const LabelStr &src);

      RuleId m_id; /*!< Id for reference */
      const LabelStr m_name; /*! Unique name for the rule */
      const LabelStr m_source;
  };
}

#endif
