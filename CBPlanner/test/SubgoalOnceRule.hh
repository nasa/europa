#ifndef _H_SubgoalOnceRule
#define _H_SubgoalOnceRule

#include "Rule.hh"

namespace EUROPA {

  /**
   * @brief Hand built rule to force backtracking after one level of subgoaling
   */
  class SubgoalOnceRule : public Rule {
  public:
    RuleInstanceId createInstance(const TokenId& token, const PlanDatabaseId& pdb, const RulesEngineId &rulesEngine) const;
    SubgoalOnceRule(const LabelStr& name, int count);
    virtual ~SubgoalOnceRule();
  private:
    static int m_count;
  };
}
#endif
