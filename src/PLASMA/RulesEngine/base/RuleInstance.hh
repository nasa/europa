#ifndef _H_RuleInstance
#define _H_RuleInstance

/**
 * @file RuleInstance.hh
 * @author Conor McGann
 * @date 2004
 */

#include "Entity.hh"
#include "Variable.hh"
#include "PlanDatabase.hh"
#include "RulesEngineDefs.hh"
#include "RulesEngine.hh"

#include <vector>

namespace EUROPA{

  /**
   * @class RuleInstance
   * @brief Provides the data access and operational interface for a Rule.
   * @see Rule, RulesEngine
   */
  class RuleInstance: public Entity{
  public:

    /**
     * @brief Constructor to construct an unguarded root rule context
     */
    RuleInstance(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb);

    /**
     * @brief Constructor to construct a guarded root rule context where the guard is triggered
     * whenever the guard variable is set to a singleton.
     * @param guard The variable which will be evaluated. trigger rule when set to any singleton value.
     */
    RuleInstance(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb,
                 const std::vector<ConstrainedVariableId>& guards);

    /**
     * @brief Constructor to construct a guarded root rule context where the guard
     * variable is bound to the given value.
     * @param guard The variable which will be evaluated
     * @param domain Fired when domain.isMember(guard.singletonValue)
     */
    RuleInstance(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb,
                 const ConstrainedVariableId& guard, const Domain& domain);

    /**
     * @brief Constructor to construct a rule instance from a parent. Must have a guard!
     */
    RuleInstance(const RuleInstanceId& parent, const std::vector<ConstrainedVariableId>& guards);

    /**
     * @brief Constructor to construct a rule instance from a parent. Must have a guard!
     */
    RuleInstance(const RuleInstanceId& parent, const std::vector<ConstrainedVariableId>& guards, const bool positive);

    /**
     * @brief Constructor to construct a rule instance from a parent. Must have a guard!
     * @param parent The parent rule instance.
     * @param guard The variable which will be evaluated
     * @param domain Fired when domain.isMember(guard.singletonValue)
     */
    RuleInstance(const RuleInstanceId& parent,
		 const ConstrainedVariableId& guard, const Domain& domain);

    /**
     * @brief Constructor to construct a rule instance from a parent. Must have a guard!
     * @param parent The parent rule instance.
     * @param guard The variable which will be evaluated
     * @param domain Fired when domain.isMember(guard.singletonValue) ^ !positive
		 * @param positive Flag to indicate whether the test is positive or not.
     */
    RuleInstance(const RuleInstanceId& parent,
		 const ConstrainedVariableId& guard, const Domain& domain, const bool positive);

    /**
     * Destructor
     */
    virtual ~RuleInstance();

    /**
     * @brief Id Accessor
     */
    const RuleInstanceId& getId() const;

    /**
     * @brief Accessor for the model rule governing evaluation and execution
     * @return An Id that must be valid.
     */
    const RuleId& getRule() const;

    /**
     * @brief Accessor for PlanDatabase
     */
    const PlanDatabaseId& getPlanDatabase() const;

    /**
     * @brief Accessor
     * @return The token to which the rule context applies. It must be ACTIVE.
     */
    const TokenId& getToken() const;

    /**
     * @brief Accessor
     * @return The slave tokens created by Rule execution.
     */
    const std::vector<TokenId> getSlaves() const;

    const RulesEngineId &getRulesEngine() const;

    void setRulesEngine(const RulesEngineId &rulesEngine);


    ConstrainedVariableId getVariable(const LabelStr& name) const;
    TokenId getSlave(const LabelStr& name) const;
    ConstraintId getConstraint(const LabelStr& name) const;

    /************** Call-backs from the rule variable listener **************/

    /**
     * @brief Test of the Rule has been executed.
     * @return true of the rule has been executed, otherwise false.
     */
    bool isExecuted() const;

    /**
     * @brief Tests if the condition, if there is one, is satisfied.
     * @param The guard variables to evaluate against
     */
    bool test(const std::vector<ConstrainedVariableId>& guards) const;

    /**
     * @brief Tests if condition is satisfied. Guards are internal
     */
    bool test() const;

    /**
     * Tests if the rule has been evaluated and is false
     */
    bool willNotFire() const;

    /**
     * Invoked by the RulesEngine or the RuleVariableListener in order to execute the subgoaling of the rule.
     */
    void execute();

    /**
     * Invoked by the RuleVariableListener in order to retract the subgoaling of the rule.
     */
    void undo();

    const std::vector<ConstrainedVariableId> &getGuards(void) const { return m_guards;}

    const std::vector<ConstrainedVariableId> &getVariables(void) const { return m_variables;}

    /**
     * @brief Constructs a vector of constrained variables by replacing names in the given
     * delimited string with variables from the scope of the rule.
     * @param delimitedVars A ":" delimited string of variable names.
     * @return A vector of variables from the scope.
     */
    std::vector<ConstrainedVariableId> getVariables(const std::string& delimitedVars) const;

    const std::vector<RuleInstanceId> &getChildRules(void) const {return m_childRules;}

    void addConstraint(const ConstraintId& constraint);

  protected:

    /**
     * @brief Handle rule related deallocations
     */
    virtual void handleDiscard();

    /**
     * @brief provide implementatin for this method for firing the rule
     */
    virtual void handleExecute() = 0;

    /*!< Helper methods */
    TokenId addSlave(Token* slave);
    TokenId addSlave(Token* slave, const LabelStr& name);
    ConstrainedVariableId varfromtok(const TokenId& tok, const std::string varstring) ;

    /**
     * @brief Obtains a variable which represents the set of all values for an object field. May retrieve
     * from local scope or allocate anew if retrieved for the first time.
     * @param objectString The name of the object variable in the local scope.
     * @param varString The moniker for a field variable.
     * @param canBeSpecified Indicate if an allocated variable should be specifiable.
     */
    ConstrainedVariableId varFromObject(const std::string& objectString,
					const std::string& varString,
					bool canBeSpecified = false);

    /**
     * @brief Allocates a proxy variable and constraint to maintain  the relation
     * between the set of objects and the aggregated field values.
     * @param object The object variable to synchronize.
     * @param varString The moniker for a field variable.
     * @param fullName The full name used for resolution. Should always contain varString.
     * @param canBeSpecified Indicate if the allocated variable should be specifiable.
     */
    ConstrainedVariableId varFromObject(const ConstrainedVariableId& object,
					const std::string& varString,
					const std::string& fullName,
					bool canBeSpecified = false);

    /**
     * @brief Insert an already existing variable into the scope to be accessed by
     * name.
     * @param var The variable to be added to scope.
     * @param name The name of the variable to use for lookup
     */
    void addVariable(const ConstrainedVariableId& var, const LabelStr& name);

    template<class DomainType>
    ConstrainedVariableId addVariable( const DomainType& baseDomain,
				       bool canBeSpecified,
				       const LabelStr& name){
      // If there is already a name-value pair for retrieving a variable by name,
      // we erase it. Though we do not erase the actual variable stored in the list since it still
      // has to be cleaned up when the rule instance is undone. This is done reluctantly, since it
      // is based on assumptions that there will be no child rules. This is all required to support the
      // looping construct used to implement the 'foreach' semantics. Therefore, we overwrite the old
      // value with the new value.
      if(!getVariable(name).isNoId())
        m_variablesByName.erase(name.getKey());

      ConstrainedVariableId localVariable = (new Variable<DomainType>(m_planDb->getConstraintEngine(),
                                                                      baseDomain,
                                                                      false, // TODO: Maybe true?
                                                                      canBeSpecified,
                                                                      name,
                                                                      m_id))->getId();
      // Only allowed add a variable for an executed rule instance
      check_error(isExecuted());

      m_variables.push_back(localVariable);
      addVariable(localVariable, name);
      return localVariable;
    }

    void addConstraint(const LabelStr& name, const std::vector<ConstrainedVariableId>& scope);
    void addChildRule(RuleInstance* instance);
    void clearLoopVar(const LabelStr& loopVarName);
    std::string makeImplicitVariableName();
    ConstraintId constraint(const std::string& name) const;


    RuleInstanceId m_id;
    const RuleId m_rule;
    const TokenId m_token;
    const PlanDatabaseId& m_planDb;
    RulesEngineId m_rulesEngine;
    RuleInstanceId m_parent;

  private:
    /**
     * @brief Invoked by derived classes to set the guard variable where the test criteria will be that
     * the specified domain of the guard is a singleton. Private since should onle be called from constructor.
     */
    void setGuard(const std::vector<ConstrainedVariableId>& guards);

    /**
     * @brief Invoked by derived classes to set the guard variable where the test criteria will be that
     * the specified domain of the guard is a singleton, and equals the given value. Private since should
     * only be called from constructor.
     */
    void setGuard(const ConstrainedVariableId& guard, const Domain& domain);


    /**
     * @brief Implement in order to track discarded slaves and constraints
     */
    void notifyDiscarded(const Entity* entity);

    bool isValid() const;
    void commonInit();

    /**
     * @brief Test of a constraint is connected to a given token. This is true if any variable in the scope
     * of the constraint belongs to the token.
     */
    bool connectedToToken(const ConstraintId& constraint, const TokenId& token) const;

    /** ANALYSIS ROUTINES FOR DEBUGGING **/
    std::string ruleExecutionContext() const;

    std::vector<ConstrainedVariableId> m_guards; /*!< Guard variables for implicit and explcit guards */
    Domain* m_guardDomain; /*!< If an explicit equality test, will ahve this be non-null */
    ConstraintId m_guardListener; /*!< If guarded, listener is a constraint */

  protected:
    bool m_isExecuted; /*!< Indicates if the rule has been fired */
    bool m_isPositive; /*!< If this is false, the rule's guard is on a negative test. */
    std::vector<ConstraintId> m_constraints; /*!< Constraints introduced through rule execution */
    std::vector<RuleInstanceId> m_childRules; /*!< Child rules introduced through rule execution */
    std::vector<ConstrainedVariableId> m_variables; /*< Local variables introduced through rule execution */
    std::vector<TokenId> m_slaves; /*< Slaves introduced through rule execution */
    std::map<edouble, ConstrainedVariableId> m_variablesByName; /*!< Context lookup */
    std::map<edouble, TokenId> m_slavesByName; /*!< Context lookup */
    std::map<edouble, ConstraintId> m_constraintsByName; /*!< Context lookup */
  };
}
#endif
