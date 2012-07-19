#ifndef _H_TestSubgoalRule
#define _H_TestSubgoalRule

#include "Rule.hh"

namespace EUROPA {

  class TestSubgoalRule : public Rule {
  public:
    RuleInstanceId createInstance(const TokenId& token, const PlanDatabaseId& pdb, const RulesEngineId &rulesEngine) const;
    TestSubgoalRule(const LabelStr& name);
    virtual ~TestSubgoalRule();
  };
}
#endif
