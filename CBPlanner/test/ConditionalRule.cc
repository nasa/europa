#include "PlanDatabaseDefs.hh"
#include "ConstraintLibrary.hh"
#include "RuleInstance.hh"
#include "PlanDatabase.hh"
#include "IntervalToken.hh"
#include "ConditionalRule.hh"
#include "Utils.hh"

namespace Prototype {

  class ConditionalRuleRoot : public RuleInstance {
  public:
    ConditionalRuleRoot(const RuleId& rule, const TokenId& token, const PlanDatabaseId& pdb) : RuleInstance(rule, token, pdb, token->getObject()) { }
    void handleExecute();
  }; 

  class ConditionalRule_true: public RuleInstance{
  public:
    ConditionalRule_true(const RuleInstanceId& parentInstance, const ConstrainedVariableId& guard, bool val)
      : RuleInstance(parentInstance, guard, val), m_localGuard(guard){}

    void handleExecute();
  private:
    ConstrainedVariableId m_localGuard;
  };

  class ConditionalRule_false: public RuleInstance{
  public:
    ConditionalRule_false(const RuleInstanceId& parentInstance, const ConstrainedVariableId& guard, bool val)
      : RuleInstance(parentInstance, guard, val), m_localGuard(guard){}

    void handleExecute();
  private:
    ConstrainedVariableId m_localGuard;
  };

  RuleInstanceId ConditionalRule::createInstance(const TokenId& token, const PlanDatabaseId& pdb, const RulesEngineId &rulesEngine) const {
    RuleInstanceId rootInstance = (new ConditionalRuleRoot(m_id, token, pdb))->getId();
    rootInstance->setRulesEngine(rulesEngine);
    return rootInstance;
  }

  ConditionalRule::ConditionalRule(const LabelStr& name) : Rule(name) {}

  ConditionalRule::~ConditionalRule() {}

  void ConditionalRuleRoot::handleExecute() {
    addVariable(BoolDomain(), true, LabelStr("OR"));
    addChildRule(new ConditionalRule_true(m_id, var("OR"), true));
    addChildRule(new ConditionalRule_false(m_id, var("OR"), false));
  }

  void ConditionalRule_true::handleExecute() {
    TokenId tok =  (new IntervalToken(m_token, LabelStr("P1True"), 
				      IntervalIntDomain(0, 10),
				      IntervalIntDomain(0, 20),
				      IntervalIntDomain(1, 1000),
				      LabelStr("Object1"), false))->getId();
    tok->addParameter(BoolDomain());
    tok->close();
    TokenId slave = addSlave(tok);

    addConstraint(LabelStr("custom"), makeScope(m_token->getParameters()[0],slave->getParameters()[0]));
  }

  void ConditionalRule_false::handleExecute() {
    TokenId slave = addSlave(new IntervalToken(m_token, LabelStr("P1False"), 
					       IntervalIntDomain(0, 10),
					       IntervalIntDomain(0, 200),
					       IntervalIntDomain(200, 200),
					       LabelStr("Timeline2")));

    addConstraint(LabelStr("eq"), makeScope(m_token->getStart(), slave->getStart()));
    addConstraint(LabelStr("eq"), makeScope(m_token->getEnd(), slave->getEnd()));
  }

}
