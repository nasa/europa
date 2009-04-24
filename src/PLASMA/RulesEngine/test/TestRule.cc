#include "TestRule.hh"
#include "RuleInstance.hh"
#include "Token.hh"
#include "TokenVariable.hh"
#include "Object.hh"
#include "IntervalToken.hh"
#include "PlanDatabase.hh"
#include "ConstraintFactory.hh"
#include "Domains.hh"
#include "Variable.hh"
#include "Utils.hh"

namespace EUROPA {
  /**
   * @brief Declaration for rule instance root - guarded by object variable
   */
  class TestRule_Root: public RuleInstance{
  public:
    TestRule_Root(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
      : RuleInstance(rule, token, planDb, makeScope(token->getObject())){}
    void handleExecute();
  };

  /**
   * @brief First child of root instance
   */
  class TestRule_0: public RuleInstance{
  public:
    TestRule_0(const RuleInstanceId& parentInstance, const ConstrainedVariableId& guard)
      : RuleInstance(parentInstance, makeScope(guard)), m_localGuard(guard){}

    void handleExecute();
  private:
    ConstrainedVariableId m_localGuard;
  };

  TestRule::TestRule(const LabelStr& name, const IntervalIntDomain& guardBaseDomain)
    : Rule(name), m_guardBaseDomain(guardBaseDomain){}

  RuleInstanceId TestRule::createInstance(const TokenId& token, const PlanDatabaseId& planDb,
                                          const RulesEngineId &rulesEngine) const{
    RuleInstanceId rootInstance = (new TestRule_Root(m_id, token, planDb))->getId();
    rootInstance->setRulesEngine(rulesEngine);
    return rootInstance;
  }

  /**
   * @brief Execution adds a new rule instance, guarded by a local variable
   */
  void TestRule_Root::handleExecute(){
    TokenId slave = addSlave(new IntervalToken(m_token, "met_by", LabelStr("AllObjects.Predicate")));
    addConstraint(LabelStr("eq"), makeScope(m_token->end(), slave->start()));

    Id<TestRule> rule = m_rule;
    addChildRule(new TestRule_0(m_id, addVariable(rule->getGuardBaseDomain(), true, LabelStr("RuleGuardLocal"))));
  }

  void TestRule_0::handleExecute(){
    TokenId slave = addSlave(new IntervalToken(m_token, "met_by", LabelStr("AllObjects.Predicate")));
    addConstraint(LabelStr("eq"), makeScope(m_token->end(), slave->start()));
    ConstrainedVariableId interimVariable = addVariable(IntervalIntDomain(), false, LabelStr("BogusRuleVariable"));
    addConstraint(LabelStr("eq"), makeScope(interimVariable, m_localGuard));
    addConstraint(LabelStr("eq"), makeScope(interimVariable, slave->duration()));
  }
}
