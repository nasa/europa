#ifndef H_TestSubgoalRule
#define H_TestSubgoalRule

#include "Rule.hh"

namespace EUROPA {

  class TestSubgoalRule : public Rule {
  public:
    RuleInstanceId createInstance(const TokenId token, const PlanDatabaseId pdb, const RulesEngineId &rulesEngine) const;
    TestSubgoalRule(const std::string& name);
    virtual ~TestSubgoalRule();
  };
}
#endif
