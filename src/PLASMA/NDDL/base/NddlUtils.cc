#include "Utils.hh"
#include "Object.hh"
#include "NddlUtils.hh"
using namespace EUROPA;
namespace NDDL {

  /**
   * Look up the slave in the rule instance context.
   */
  TokenId tok(const RuleInstanceId rule, const std::string name) {
    return rule->getSlave(name);
  }

  ConstrainedVariableId var(const RuleInstanceId entity, const std::string name) {
    return entity->getVariable(name);
  }

  ConstrainedVariableId var(const TokenId entity, const std::string name) {
    return entity->getVariable(name);
  }

}//namespace
