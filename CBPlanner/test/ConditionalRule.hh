#ifndef _H_ConditionalRule
#define _H_ConditionalRule

#include "Rule.hh"
#include "IntervalIntDomain.hh"

namespace PLASMA {

  class ConditionalRule : public Rule {
  public:
    RuleInstanceId createInstance(const TokenId& token, const PlanDatabaseId& pdb, const RulesEngineId &rulesEngine) const;
    ConditionalRule(const LabelStr& name);
    virtual ~ConditionalRule();
  };
}
#endif
