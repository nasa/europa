#include "PlanDatabaseDefs.hh"
#include "RuleInstance.hh"
#include "PlanDatabase.hh"
#include "IntervalToken.hh"
#include "AtSubgoalRule.hh"
#include "Utils.hh"
#include "Object.hh"
#include "Token.hh"
#include "StringDomain.hh"

namespace EUROPA {

  class AtSubgoalRuleRoot : public RuleInstance {
  public:
    AtSubgoalRuleRoot(const RuleId& rule, const TokenId& token, const PlanDatabaseId& pdb) : RuleInstance(rule, token, pdb, token->getObject()) { 
    }
    void handleExecute();
  }; 

  RuleInstanceId AtSubgoalRule::createInstance(const TokenId& token, const PlanDatabaseId& pdb,
                                                 const RulesEngineId &rulesEngine) const {
    RuleInstanceId rootInstance = (new AtSubgoalRuleRoot(m_id, token, pdb))->getId();
    rootInstance->setRulesEngine(rulesEngine);
    return rootInstance;
  }

  AtSubgoalRule::AtSubgoalRule(const LabelStr& name) : Rule(name) {}

  AtSubgoalRule::~AtSubgoalRule() { }

  void AtSubgoalRuleRoot::handleExecute() {
    TokenId tok =  (new IntervalToken(m_token,  
				      "after",
				      LabelStr("Navigator.Going"), 
				      IntervalIntDomain(0, 100),
				      IntervalIntDomain(0, 100),
				      IntervalIntDomain(1, 100),
				      "nav1", false))->getId();
    std::list<ObjectId> results;
    getPlanDatabase()->getObjectsByType("Location",results);
    ObjectDomain allLocs(results,"Location");
    tok->addParameter(allLocs, LabelStr("from"));
    tok->addParameter(allLocs, LabelStr("to"));
    tok->close();
    ConstrainedVariableId vfrom = tok->getVariable("from");
    vfrom->specify(getPlanDatabase()->getObject("Loc1"));
    ConstrainedVariableId vto = tok->getVariable("to");
    vto->specify(getPlanDatabase()->getObject("Loc3"));
    addSlave(tok);
  }

}
