#include "Utils.hh"
#include "Object.hh"
#include "NddlUtils.hh"

namespace NDDL {

  /**
   * @todo Swtich these interface to use const LabelStr&
   */

  /**
   * Look up the slave in the rule instance context.
   */
  TokenId tok(const RuleInstanceId& rule, const std::string name) {
    return rule->getSlave(LabelStr(name));
  }

  ConstrainedVariableId var(const RuleInstanceId& entity, const std::string name) {
    return entity->getVariable(LabelStr(name));
  }

  ConstrainedVariableId var(const TokenId& entity, const std::string name) {
        return entity->getVariable(LabelStr(name));
  }

}//namespace
