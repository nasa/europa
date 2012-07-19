#include "Utils.hh"
#include "Object.hh"
#include "NddlUtils.hh"

namespace NDDL {

  TokenId tok(const RuleInstanceId& rule, const std::string name) {
    TokenId token =rule->getSlave(LabelStr(name));
    if ((!token.isNoId()) && token->isMerged())
      return token->getActiveToken();
    else
      return token;
  }

  ConstrainedVariableId var(const RuleInstanceId& entity, const std::string name) {
    return entity->getVariable(LabelStr(name));
  }

  ConstrainedVariableId var(const TokenId& entity, const std::string name) {
        return entity->getVariable(LabelStr(name));
  }

}//namespace
