#ifndef _H_Constraint
#define _H_Constraint

/**
 * @file Constraint.hh
 * @author Conor McGann
 * @date August, 2003
 * @brief Declares the Constraint class
 * @ingroup ConstraintEngine
 */

#include "Entity.hh"
#include "ConstraintEngineDefs.hh"
#include "PSConstraintEngine.hh"
#include "DomainListener.hh"
#include "LabelStr.hh"

#include <vector>
#include <set>

namespace EUROPA {

  /**
   * @class Constraint
   * @brief Provides the base class from which all constraints are derived.
   *
   * The constraint class is an example of a Command Pattern. It is responsible for enforcing local consistency among
   * ConstrainedVariable instances. This class
   * provides a base class from which all constraints are derived. This class:
   * @li Ensures that a constraint belongs to exactly one ConstraintEngine
   * @li Ensures that a constraint is a named entity with at least one ConstrainedVariable for its scope.
   * @li Ensures that a constraint has exactly one propagator
   * @li Ensures that each ConstrainedVariable it constrains is properly linked in.
   * @li Defines the interface to plug in a Constraint to the propagation event model.
   *
   * The key aspects of this class are the protected virtual methods which show how custom constraints
   * are defined and integrated into the ConstraintEngine event model. Strict control of events in maintained in the ConstraintEngine.
   * However, carefully defined extension points ensure customization without compromising the integrity of the ConstraintEngine state model.
   * In particular:
   * @li Calls to execute a constraint must first be sent through the ConstraintEngine which alone has the access priveleges to
   * invoke the handleEvent methods defined in the custom class. This helps to ensure propagation control policies can be enforced.
   * @li It is possible to consider ignoring events generated during propagation if a constraint can deduce the event would
   * not impact the constraint
   *
   * @see canIgnore(), handleExecute()
   */

  class Constraint : public virtual PSConstraint, public Entity {
  public:
    DECLARE_ENTITY_TYPE(Constraint);

    /**
     * @brief Constructor for NARY constraint
     * @param name The logical identifier for the constraint. Names do not have to be unique, but all instances
     * with the same name are logically equivalent.
     * @param propagatorName The logical identifier for a propagator for its constraint. The propagator will provide meta
     * control of propagation of domain change events to the constraint.
     * @param constraintEngine The constraintEngine instance to wich the constraint belongs. There is exactly one.
     * @param variables The variables that will form the scope of the constraint.
     */
    Constraint(const LabelStr& name,
	       const LabelStr& propagatorName,
	       const ConstraintEngineId& constraintEngine,
	       const std::vector<ConstrainedVariableId>& variables);

    virtual ~Constraint();

    virtual const std::string& getEntityType() const;

    /**
     * @brief Accessor
     */
    const ConstraintId& getId() const;

    /**
     * @brief Accessor
     */
    const LabelStr& getName() const;

    /**
     * @brief Accessor
     */
    const PropagatorId& getPropagator() const;

    /**
     * @brief Accessor
     */
    const LabelStr& getCreatedBy() const;

    /**
     * @brief Test if the given variable is constrained by this constraint.
     * @param variable The variable to test.
     * @return true if var is a member of the constraint scope
     */
    bool isVariableOf(const ConstrainedVariableId& variable);

    /**
     * @brief Check if the constraint is to be actively used in propagation.
     * @return true if the constraint is activated. otherwise false.
     */
    virtual bool isActive() const {return m_deactivationRefCount == 0;}

    /**
     * @brief Check of the constraint is a unary constraint
     */
    inline bool isUnary() const{return m_isUnary;}

    /**
     * @brief This will turn off propagation of this constraint. Unlike deletion of a constraint, this is not
     * treated as a relaxation.
     *
     * Responsibility lies with the client to ensure that the correctness is not impacted by doing this.
     */
    virtual void deactivate();

    /**
     * @brief This will turn on propagation of this constraint.
     *
     * Has the same impact as if the constraint was added for the first time.
     */
    virtual void undoDeactivation();

    /**
     * @brief the number of outstanding deactivation calls
     */
    unsigned int deactivationCount() const;

    /**
     * @brief Accessor for the variable scope of the constraint
     */
    const std::vector<ConstrainedVariableId>& getScope() const;

    /**
     * @brief Informs the constraint that it is a copy of another constraint. It is up to the receiever
     * to use this information or disregard it.
     */
    virtual void setSource(const ConstraintId& sourceConstraint) {
      check_error(sourceConstraint.isValid());
    }

    /**
     * @brief Utility to capture the state of the constraint.
     */
    virtual std::string toString() const;

    /**
     * @brief Notification from a variable that its base domain has been retricted. May cause the constraint to be
     * deactivated
     */
    void notifyBaseDomainRestricted(const ConstrainedVariableId& var);

    /**
     * @brief Test the status of redundant flag
     */
    inline bool isRedundant() const {return m_isRedundant;}

    /**
     * @brief If the constraint is violated this must return a value >=0. It'll return 0 otherwise.
     */
    virtual double getViolation() const;

    /**
     * @brief string to present to the user describing what the violation of this constraint means
     * TODO: this must be allowed to be set from the model
     */
    virtual std::string getViolationExpl() const;

    // PS-Specific Methods:
    virtual PSList<PSVariable*> getVariables() const;

  protected:
    /**
     * @brief Base implementation will require all variables in scope have the base domains as singletons. Over-ride
     * for weaker criteria which may apply for certain constraints.
     */
    virtual bool testIsRedundant(const ConstrainedVariableId& var = ConstrainedVariableId::noId()) const;

    friend class ConstraintEngine; /**< Grant access to protected event handler methods handleExecute, and canIgnore */

    /**
     * @brief Accessor for derived classes to obtain the domain from the variable.
     *
     * Because the base class is a friend of the ConstrainedVariable, this gives it access to the protected method
     * on that interface. This wrapper function gives us a clean way to extend that specific access to derived classes.
     * @param var The variable whose domain is requested.
     * @return A mutable reference to the domain.
     */
    static AbstractDomain& getCurrentDomain(const ConstrainedVariableId& var);

    /**
     * @brief Wrapper for handleExecute calls, will set propagation context for all the variables in this constraint
     *
     */
    virtual void execute();
    virtual void execute(const ConstrainedVariableId& variable,int argIndex,const DomainListener::ChangeType& changeType);

    /**
     * @brief Called when no specific change event on a variable is reported.
     *
     * This mirrors the old way of calling a Constraint in Europa. Implementors of this method must quit as soon
     * as an empty domain is detected. This method may only be executed when the ConstraintEngine is in the pending state.
     * @see ConstraintEngine::execute(), ConstraintEngine::PENDING
     */
    virtual void handleExecute() = 0;

    /**
     * @brief Called to enforce consistency when a specific variable domain change event has occurred.
     *
     * Allows invoking the constraints consistency enforcemnent algorithm with more data about what changed and how.
     * This method may only be executed when the ConstraintEngine is in the pending state.
     * @param variable - the variable from within its scope that has been restricted.
     * @param argIndex - the position of the restricted variable within the constraint scope.
     * @param changeType - the nature of the change occuring in the given variable.
     * @see ConstraintEngine::execute(), ConstraintEngine::PENDING, DomainListener::ChangeType
     */
    virtual void handleExecute(const ConstrainedVariableId& variable,
			       int argIndex,
			       const DomainListener::ChangeType& changeType);

    /**
     * @brief Determine if a change notification can be ignored for further processing.
     *
     * Called during the propagation of notification events from a changed domain to its constraints. The ConstraintEngine
     * initiates this call to allow a derived class to possibly implement some checking that would prevent the Constraint
     * from being re-activated. Proceed with caution! If poorly implemented this may weaken propagation.
     * @param variable - the variable from within its scope that has been restricted.
     * @param argIndex - the position of the restricted variable within the constraint scope.
     * @param changeType - the nature of the change occuring in the given variable.
     * @return true if the event cannot impact the consistency of the Constraint given its current state. Othwerwise false.
     * @see ConstraintEngine::notify()
     */
    virtual bool canIgnore(const ConstrainedVariableId& variable,
			   int argIndex,
			   const DomainListener::ChangeType& changeType);


    /**
     * @brief Get the varibles in scope that might be modified by executing this constraint.
     *
     * Primarily used during network relaxation to determine which variables need to be relaxed
     * in response to a particular relaxation.
     * @param variable - the variable from within its scope that has been relaxed.
     * @return the vector of variables that this constraint modifies.  Defaults to the scope.
     * @see ConstraintEngine::getScope(), ConstraintEngine::handleRelax()
     */
    virtual const std::vector<ConstrainedVariableId>& getModifiedVariables(const ConstrainedVariableId& variable) const;

    /**
     * @brief Allow implementation class to take action in the event of activation
     */
    virtual void handleActivate(){}

    /**
     * @brief Allow implementation class to take action in the event of deactivation
     * @param delegate The constraint which will justify deactivation
     */
    virtual void handleDeactivate(){}

    /**
     * @brief Handle De-allocation
     */
    virtual void handleDiscard();

    const LabelStr m_name; /**< Name used on stratup to bind to the correct factory and then present as a debugging aid. */
    const ConstraintEngineId m_constraintEngine; /**< The owner ConstraintEngine */
    std::vector<ConstrainedVariableId> m_variables; /**< The variable scope of the Constraint. */

    virtual void notifyViolated();
    virtual void notifyNoLongerViolated();

  private:
    /**
     * @brief Called by the ConstraintEngine during the construction hand-shaking that goes on to set up the correct
     * relationships.
     * @param propagator The single Propagator in the ConstraintEngine that can accept this constraint.
     */
    void setPropagator(const PropagatorId& propagator);

    bool isValid() const;

    Constraint(); // NO IMPL

    ConstraintId m_id; /**< Self reference */
    PropagatorId m_propagator; /**< The owner Propagator. */
    bool m_isUnary; /**< True if constructed with a single variable */
    const LabelStr m_createdBy; /**< Populated on construction. Indicates the user that created the constraint. */
    unsigned int m_deactivationRefCount; /*!< Tracks number of outstanding deactivation calls */
    bool m_isRedundant; /*!< True of the constraint is redundant */
  };

  std::vector<ConstrainedVariableId>& makeScope(const ConstrainedVariableId& arg1);

  std::vector<ConstrainedVariableId>& makeScope(const ConstrainedVariableId& arg1,
						const ConstrainedVariableId& arg2);

  std::vector<ConstrainedVariableId>& makeScope(const ConstrainedVariableId& arg1,
						const ConstrainedVariableId& arg2,
						const ConstrainedVariableId& arg3);

  std::vector<ConstrainedVariableId>& makeScope(const ConstrainedVariableId& arg1,
						const ConstrainedVariableId& arg2,
						const ConstrainedVariableId& arg3,
						const ConstrainedVariableId& arg4);

  std::vector<ConstrainedVariableId>& makeScope(const ConstrainedVariableId& arg1,
						const ConstrainedVariableId& arg2,
						const ConstrainedVariableId& arg3,
						const ConstrainedVariableId& arg4,
						const ConstrainedVariableId& arg5);

  std::vector<ConstrainedVariableId>& makeScope(const ConstrainedVariableId& arg1,
						const ConstrainedVariableId& arg2,
						const ConstrainedVariableId& arg3,
						const ConstrainedVariableId& arg4,
						const ConstrainedVariableId& arg5,
						const ConstrainedVariableId& arg6);

  std::vector<ConstrainedVariableId>& makeScope(const ConstrainedVariableId& arg1,
						const ConstrainedVariableId& arg2,
						const ConstrainedVariableId& arg3,
						const ConstrainedVariableId& arg4,
						const ConstrainedVariableId& arg5,
						const ConstrainedVariableId& arg6,
						const ConstrainedVariableId& arg7);

  std::vector<ConstrainedVariableId>& makeScope(const ConstrainedVariableId& arg1,
						const ConstrainedVariableId& arg2,
						const ConstrainedVariableId& arg3,
						const ConstrainedVariableId& arg4,
						const ConstrainedVariableId& arg5,
						const ConstrainedVariableId& arg6,
						const ConstrainedVariableId& arg7,
						const ConstrainedVariableId& arg8);

  std::vector<ConstrainedVariableId>& makeScope(const ConstrainedVariableId& arg1,
						const ConstrainedVariableId& arg2,
						const ConstrainedVariableId& arg3,
						const ConstrainedVariableId& arg4,
						const ConstrainedVariableId& arg5,
						const ConstrainedVariableId& arg6,
						const ConstrainedVariableId& arg7,
						const ConstrainedVariableId& arg8,
						const ConstrainedVariableId& arg9,
						const ConstrainedVariableId& arg10,
						const ConstrainedVariableId& arg11,
						const ConstrainedVariableId& arg12,
						const ConstrainedVariableId& arg13);
}
#endif
