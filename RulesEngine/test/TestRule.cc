#include "TestRule.hh"
#include "RuleContext.hh"
#include "RuleInstance.hh"
#include "Token.hh"
#include "Object.hh"
#include "IntervalToken.hh"
#include "PlanDatabase.hh"
#include "../ConstraintEngine/ConstraintLibrary.hh"
#include "../ConstraintEngine/IntervalIntDomain.hh"
#include "../ConstraintEngine/Variable.hh"
#include "../ConstraintEngine/Utils.hh"
#include "TokenTemporalVariable.hh"

namespace Prototype {

  /**
   * @brief Defines members in addition to containers to aid comprehension
   */
  class TestContext: public RuleContext{
  public:
    TestContext(const RuleInstanceId& ruleInstance, const IntervalIntDomain& guardBaseDomain)
      : RuleContext(ruleInstance), m_guardBaseDomain(guardBaseDomain){}

    /**
     * @brief Initialize the context with some variables from the token and add a local variable for the rule too. This
     * will test cleanup.
     */
    void initialize(std::vector<ConstrainedVariableId>& guards) {
      assert(getGuards().empty());
      assert(guards.empty());
      guards.push_back(objectGuard = getToken()->getObject());
      guards.push_back(stateGuard = getToken()->getState());

      localGuard = 
	(new Variable<IntervalIntDomain>(getPlanDatabase()->getConstraintEngine(), 
					 m_guardBaseDomain,
					 true,
					 LabelStr("RuleGuardLocal"),
					 m_id))->getId();
      guards.push_back(localGuard);
    }

    /**
     * @brief Fires when all variables are set to sinlgeton.
     */
    bool test(int index, const ConstrainedVariableId& var) {
      return var->specifiedDomain().isSingleton();
    }

    void fire(std::vector<TokenId>& newTokens,
	      std::vector<ConstrainedVariableId>& newVariables,
	      std::vector<ConstraintId>& newConstraints) {
      // Allocate a new slave Token
      slave = (new IntervalToken(getToken(),
				 LabelStr("Predicate")))->getId();
      newTokens.push_back(slave);

      // Allocate a new constraint equating the start variable of the new token with the end variable of
      // the existing token
      ConstraintId meets = ConstraintLibrary::createConstraint(LabelStr("eq"),
							       getPlanDatabase()->getConstraintEngine(),
							       makeScope(getToken()->getEnd(), slave->getStart()));
      newConstraints.push_back(meets);

      // Allocate a bogus intermeidate variable that is not decideable
      interimVariable = (new Variable<IntervalIntDomain>(getPlanDatabase()->getConstraintEngine(), 
							 IntervalIntDomain(),
							 false,
							 LabelStr("BogusRuleVariable"),
							 m_id))->getId();

      // Allocate an interim constraint
      ConstraintId interimConstraint = ConstraintLibrary::createConstraint(LabelStr("eq"),
									   getPlanDatabase()->getConstraintEngine(),
									   makeScope(localGuard, interimVariable));
      newConstraints.push_back(interimConstraint);
      newVariables.push_back(interimVariable);

      // Allocate a constraint restricting the duration of the slave using the guard variable
      ConstraintId restrictDuration = ConstraintLibrary::createConstraint(LabelStr("eq"),
									  getPlanDatabase()->getConstraintEngine(),
									  makeScope(slave->getDuration(), interimVariable));
      newConstraints.push_back(restrictDuration);

      if(!getRule()->getChildren().empty())
	createChild(getRule()->getChildren().front());
    }

  private:
    const IntervalIntDomain m_guardBaseDomain;

    /*!< Guard variables */
    ConstrainedVariableId objectGuard;
    ConstrainedVariableId stateGuard;
    ConstrainedVariableId localGuard;

    /*!< Members created from firing the rule */
    TokenId slave;
    ConstrainedVariableId interimVariable;
  };

  /**
   * @brief allocates a special derived class which has explicit member variables in addition to the lists otherwise used.
   * @see TestContext
   */
  RuleContextId TestRule::createContext(const RuleInstanceId& ruleInstance) const{
    RuleContextId context = (new TestContext(ruleInstance, m_guardBaseDomain))->getId();
    return context;
  }

  /**
   * @brief Constructor as the root has a child rule.
   */
  TestRule::TestRule(const LabelStr& name, const IntervalIntDomain& guardBaseDomain)
    :Rule(name), m_guardBaseDomain(guardBaseDomain){
    new TestRule(getId(), m_guardBaseDomain);
  }

  /**
   * @brief Constructor as a child rule does not have any further children. This allows rule expansion
   * to terminate after 1 level.
   */
  TestRule::TestRule(const RuleId& parent, const IntervalIntDomain& guardBaseDomain)\
  : Rule(parent), m_guardBaseDomain(guardBaseDomain){}
}
