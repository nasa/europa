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
  }

  /**
   * @see Mergemento::merge
   */
  void RuleVariableListener::setSource(const ConstraintId& sourceConstraint){
    check_error(sourceConstraint.isValid());
    check_error(Id<RuleVariableListener>::convertable(sourceConstraint), 
		"Supposed to be sourced from constraint of same type.");
    check_error(m_ruleInstance.isNoId(), "Rule Instance should not be set when this is called");

    // Now obtain the rule instance from the source
    RuleVariableListener* source = (RuleVariableListener*) sourceConstraint;
    m_ruleInstance = source->m_ruleInstance;
  }

  /**
   * @brief Handle all behaviour immediately on set or reset operations
   * so that rule execution is not subject to the vagaries of propagtion timing
   * @return true
   */
  bool RuleVariableListener::canIgnore(const ConstrainedVariableId& variable, 
				       int argIndex,
				       const DomainListener::ChangeType& changeType){
    debugMsg("RuleVariableListener:canIgnore", "Checking canIgnore for guard listener for rule " <<
	     m_ruleInstance->getRule()->getName() << " from source " << m_ruleInstance->getRule()->getName());
    // If a Reset has occurred, and the rule has been fired, we may have to do something right now
    if(m_ruleInstance->isExecuted() && 
       changeType == DomainListener::RESET && 
       !m_ruleInstance->test()){
      m_ruleInstance->undo();
      return true;
    }

    // If not executed, and the specified domain is a singleton and the rule test passses,
    // then execute the rule.
    if(variable->isSpecified() &&
       !m_ruleInstance->isExecuted() && 
       m_ruleInstance->test())
      m_ruleInstance->execute();

    return true;
  }

  const RuleInstanceId& RuleVariableListener::getRuleInstance() const {return m_ruleInstance;}

  void RuleVariableListener::handleExecute() {}
}
