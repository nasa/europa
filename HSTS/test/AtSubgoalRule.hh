#ifndef _H_AtSubgoalRule
#define _H_AtSubgoalRule

#include "Rule.hh"

namespace EUROPA {

  class AtSubgoalRule : public Rule {
  public:
    RuleInstanceId createInstance(const TokenId& token, const PlanDatabaseId& pdb, const RulesEngineId &rulesEngine) const;
    AtSubgoalRule(const LabelStr& name);
    virtual ~AtSubgoalRule();
  };
}
#endif
