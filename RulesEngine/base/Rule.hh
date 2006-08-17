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
#include "Entity.hh"
#include "LabelStr.hh"
#include <vector>
#include <map>

namespace EUROPA {

  /**
   * @class Rule
   * @brief Defines an abstract base class that is implemented by a provider of rules.
   * @see RuleContext, RulesEngine
   */
  class Rule: public Entity {
  public:

    /**
     * @brief Accessor
     */
    const RuleId& getId() const;

    /**
     * @brief Accessor
     * @return The predicate for which this rule applies.
     */
    const LabelStr& getName() const;

    const LabelStr &getSource() const {return m_source;};

    /**
     * Destructor
     */
    virtual ~Rule();

    virtual RuleInstanceId createInstance(const TokenId& token, const PlanDatabaseId& planDb, const RulesEngineId &rulesEngine) const = 0;

    /**
     * @brief Retrieve all registered rules for the given predicate. This will include rules
     * defined in ancestors of the current predicate also.
     */
    static void getRules(const LabelStr& predicate, std::vector<RuleId>& results);

    static const std::multimap<double, RuleId>& getRules();

    /**
     * @brief Delete all rule instances stored. Should only be used to support testing, since
     * rules should remain for all instances of the plan database in the same process.
     */
    static void purgeAll();

  protected:

    /**
     * @brief Constructor.
     * @param rulesEngine The Rules Engine to which this rule will belong.
     * @param name A unique name for the rule.
     */
    Rule(const LabelStr& name);

    Rule(const LabelStr& name, const LabelStr &src);

    /**
     * @brief static accessor for getting the rule ids by name. Map is a static local variable of the function.
     */
    static std::multimap<double, RuleId>& rulesByName();

    static bool & isPurging();

    RuleId m_id; /*!< Id for reference */
    const LabelStr m_name; /*! Unique name for the rule */
    const LabelStr m_source;
  };
}

#endif
