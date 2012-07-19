#include "PlanDatabaseDefs.hh"
#include "PlanDatabase.hh"
#include "RuleInstance.hh"
#include "TestSubgoalRule.hh"
#include "IntervalToken.hh"
#include "TokenVariable.hh"

namespace EUROPA {

  class TestSubgoalRuleRoot : public RuleInstance {
  public:
    TestSubgoalRuleRoot(const RuleId& rule, const TokenId& token, const PlanDatabaseId& pdb) : RuleInstance(rule, token, pdb) { 
    }
    void handleExecute();
  }; 

  RuleInstanceId TestSubgoalRule::createInstance(const TokenId& token, const PlanDatabaseId& pdb,
                                                 const RulesEngineId &rulesEngine) const {
    RuleInstanceId rootInstance = (new TestSubgoalRuleRoot(m_id, token, pdb))->getId();
    rootInstance->setRulesEngine(rulesEngine);
    return rootInstance;
  }

  TestSubgoalRule::TestSubgoalRule(const LabelStr& name) : Rule(name) {}

  TestSubgoalRule::~TestSubgoalRule() { }

  void TestSubgoalRuleRoot::handleExecute() {
    TokenId slave =  (new IntervalToken(m_token,  
				      "meets",
				      LabelStr("Objects.PredicateB"), 
				      IntervalIntDomain(0, 100),
				      IntervalIntDomain(0, 100),
				      IntervalIntDomain(1, 100),
				      "o2", false))->getId();
    slave->addParameter(IntervalIntDomain(0,10),"IntervalParam");
    slave->close();
    addSlave(slave);
    addConstraint("eq", makeScope(m_token->getEnd(), slave->getStart()));
  }

}
