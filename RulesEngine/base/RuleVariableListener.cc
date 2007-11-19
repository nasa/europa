#include "RuleVariableListener.hh"
#include "RuleInstance.hh"
#include "DomainListener.hh"
#include "ConstrainedVariable.hh"
#include "ConstraintLibrary.hh"
#include "LabelStr.hh"
#include "Rule.hh"

namespace EUROPA {

  class RuleVariableLocalStatic {
  public:
    RuleVariableLocalStatic(){
      static bool sl_boolean = false;
      check_error(sl_boolean == false, "Should only be called once");
      if(sl_boolean == false){
	// Register built in constraint with expected logical propagator name
	REGISTER_SYSTEM_CONSTRAINT(RuleVariableListener, 
				   RuleVariableListener::CONSTRAINT_NAME(),
				   RuleVariableListener::PROPAGATOR_NAME());
        sl_boolean = true;
      }
    }
  };

  RuleVariableLocalStatic s_ruleVariableLocalStatic;


  RuleVariableListener::RuleVariableListener(const LabelStr& name,
					     const LabelStr& propagatorName,
					     const ConstraintEngineId& constraintEngine, 
					     const std::vector<ConstrainedVariableId>& scope)
    : Constraint(name, propagatorName, constraintEngine, scope){}


  RuleVariableListener::RuleVariableListener(const ConstraintEngineId& constraintEngine,
					     const RuleInstanceId& ruleInstance,
					     const std::vector<ConstrainedVariableId>& scope)
    : Constraint(CONSTRAINT_NAME(), PROPAGATOR_NAME(), constraintEngine, scope), 
      m_ruleInstance(ruleInstance){
    check_error(! m_ruleInstance->isExecuted(), 
		"A Rule Instance should never be already executed when we construct the constraint!");

    // Add rule variable listener as a dependent of the rule instance to receive discard notifications
    m_ruleInstance->addDependent(this);
  }

  /**
   * @see Mergemento::merge
   */
  void RuleVariableListener::setSource(const ConstraintId& sourceConstraint){
    check_error(sourceConstraint.isValid());
    checkError(RuleVariableListenerId::convertable(sourceConstraint), 
		"Supposed to be sourced from constraint of same type." << sourceConstraint->toString());
    checkError(m_ruleInstance.isNoId(), "Rule Instance should not be set when this is called");

    // Now obtain the rule instance from the source
    RuleVariableListener* source = (RuleVariableListener*) sourceConstraint;
    m_ruleInstance = source->m_ruleInstance;

    // It is possible that this constraint is being migrated even though the master has been terminated. If that were the case then the rule
    // instance may have been cleared. So we have to check for that.
    if(m_ruleInstance.isId())
      m_ruleInstance->addDependent(this);
  }

  /**
   * @brief Handle all behaviour immediately on set or reset operations
   * so that rule execution is not subject to the vagaries of propagtion timing
   * @return true
   */
  bool RuleVariableListener::canIgnore(const ConstrainedVariableId& variable, 
				       int argIndex,
				       const DomainListener::ChangeType& changeType){
    checkError(m_ruleInstance.isValid(), getKey() << " has lost its rule instance:" << m_ruleInstance);

    if(m_ruleInstance.isNoId())
      return true;

    debugMsg("RuleVariableListener:canIgnore", "Checking canIgnore for guard listener for rule " <<
	     m_ruleInstance->getRule()->getName() << " from source " << m_ruleInstance->getRule()->getName());
    // If a Reset has occurred, and the rule has been fired, we may have to do something right now
    if(m_ruleInstance->isExecuted() && 
       changeType == DomainListener::RESET && 
       !m_ruleInstance->test(getScope())){
      m_ruleInstance->undo();
      return true;
    }

    // If not executed, and the specified domain is a singleton and the rule test passses,
    // then execute the rule.
    if(variable->isSpecified() &&
       !m_ruleInstance->isExecuted() && 
       m_ruleInstance->test(getScope()))
      m_ruleInstance->execute();

    return true;
  }

  const RuleInstanceId& RuleVariableListener::getRuleInstance() const {return m_ruleInstance;}

  void RuleVariableListener::handleExecute() {}

  void RuleVariableListener::notifyDiscarded(const Entity*){
    m_ruleInstance = RuleInstanceId::noId();
    if(isActive()) 
      deactivate();
  }

  void RuleVariableListener::handleDiscard(){
    if(!Entity::isPurging() && m_ruleInstance.isId())
      m_ruleInstance->removeDependent(this);

    Constraint::handleDiscard();
  }

  /**
   * @brief If the base class test passes, then we need to see if there is any more information contained in the rule that
   * has not been applied. This will be the case if the rule has not fired yet and the test indicates it could.
   */
  bool RuleVariableListener::testIsRedundant(const ConstrainedVariableId& var) const{
    return Constraint::testIsRedundant(var) && (m_ruleInstance.isNoId() || m_ruleInstance->isExecuted() || !m_ruleInstance->test(getScope()));
  }
}
