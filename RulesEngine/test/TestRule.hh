#ifndef _H_TestRule
#define _H_TestRule

#include "Rule.hh"
#include "../ConstraintEngine/IntervalIntDomain.hh"

namespace Prototype {

  /**
   * @brief Rule designed to test plan database handling and support flaw testing
   * independent of a model.
   * 
   * The elements of this rule are:
   * 1. Has guard variables based on the Token (object and state variables) as well as a new locally introduced gurad.
   * 2. Rule fires when all guars are specified to singletons.
   * 3. By default, the local variable is a singleton on construction. To change it, pass base domain that is not a singleton. This
   * will allow testing of flws for local variables on rule instantiation.
   * 4. The rule has one level of composition i.e. it will generate one child rule on first firing which will fire on same criteria.
   * 5. Usage should ensure that the Rule is registered for Tokens with a matching predicate.
   * 6. Rule execution also introduces a local variable that is NOT DECIDEABLE!
   */
  class TestRule: public Rule {
  public:
    RuleContextId createContext(const RuleInstanceId& ruleInstance) const;

    TestRule(const LabelStr& name, const IntervalIntDomain& guardBaseDomain = IntervalIntDomain(1,1));

    TestRule(const RuleId& parent, const IntervalIntDomain& guardBaseDomain = IntervalIntDomain(1,1));

  private:
    const IntervalIntDomain m_guardBaseDomain; /*!< The base domain for the local guard variable */
  };
}
#endif
