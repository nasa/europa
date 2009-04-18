#ifndef _H_ConstrainedVariable
#define _H_ConstrainedVariable

/**
 * @file ConstrainedVariable.hh
 * @author Conor McGann
 * @brief Declares class ConstrainedVariable
 * @todo move handling of interaction between base, specified and derived domains to this class from Variable
 */

#include "ConstraintEngineDefs.hh"
#include "PSVarValue.hh"
#include "PSConstraintEngine.hh"
#include "Entity.hh"
#include "LabelStr.hh"
#include "AbstractDomain.hh"
#include <set>

namespace EUROPA {

  /**
   * @brief Allows connection to a variable and will be deleted on destruction of the variable
   */
  class ConstrainedVariableListener{
  public:
    virtual ~ConstrainedVariableListener();
    const ConstrainedVariableListenerId& getId() const;
    virtual void notifyDiscard() {}  // message for listener creator to immediately delete listener
    virtual void notifyConstraintAdded(const ConstraintId& constr, int argIndex) {}
    virtual void notifyConstraintRemoved(const ConstraintId& constr, int argIndex) {}
  protected:
    ConstrainedVariableListener(const ConstrainedVariableId& var);
    ConstrainedVariableListenerId m_id;
    ConstrainedVariableId m_var;
  };

  /**
   * @class ConstrainedVariable
   * @brief Provides the specification for a variable to interact in the ConstraintEngine.
   *
   * The ConstrainedVariable specifies the semantics of a variable from the perspective of a
   * ConstraintEngine and a Constraint. The public interface is very restricted, and specifies what can be
   * exposed to a customized Constraint. Much of the functionality of the interface is provided
   * through protected or private members.
   *
   * The ConstrainedVariable is an Abstract Class which must be extended to build a working system.
   * The responsibililties of this class are:
   * @li Ensure that each constrained variable instance is connected to exactly one ConstraintEngine instance.
   * @li Maintains the list of constraints posted on the variable.
   * @li Provides the core connection point for event propagation by connecting a DomainListener
   * from the ConstraintEngine to the domain of the variable. Thus, user changes on a variable will
   * trigger activity in the ConstraintEngine behind the scenes. The connection is guaranteed by the base class, or
   * a failure will occur - see isValid().
   * @see AbstractDomain, Constraint, ConstraintEngine, DomainListener, isValid
   */
  class ConstrainedVariable : public virtual PSVariable, public Entity {
  public:
    DECLARE_ENTITY_TYPE(ConstrainedVariable);

    static const LabelStr& NO_NAME() {
      static const LabelStr sl_noName(NO_VAR_NAME);
      return(sl_noName);
    }

    /**
     * Should not be called unless all constraints have been removed.
     */
    virtual ~ConstrainedVariable();

    /**
     * @brief Simple accessor.
     */
    const ConstrainedVariableId& getId() const;

    const std::string& getEntityType() const;

    const DataTypeId& getDataType() const;

    /**
     * @brief Validates the relationships of the class.
     *
     * Also delegates to the pure virtual function 'validate' which allows validity checking to be
     * extended without being compromised.
     * @return true if a valid ConstraintEngine is connected, all Constraints have this variable as expected,
     * and the listener is connected to the current domain. Otherwise return false.
     * @see validate(), lastDomain()
     */
    bool isValid() const;

    /**
     * @brief Retrieve all constraints on this variable.
     */
    void constraints(std::set<ConstraintId>& results) const;

    /**
     * @brief Retrieve all constraints on this variable.
     */
    void constraints(ConstraintSet& results) const;

    /**
     * @brief Retrive the count of constraints direclt on this variable
     */
    unsigned int constraintCount() const;

    /**
     * @brief Retrieve one constraint on this variable.  Returns noId if there are none.
     */
    const ConstraintId& getFirstConstraint() const;

    /**
     * @brief Test of the variable has at least one active constraint
     */
    bool hasActiveConstraint() const;

    /**
     * @brief test if the state is PROVEN_INCONSISTENT.
     * @see ConstraintEngine::provenInconsistent(), ConstraintEngine::State
     */
    bool provenInconsistent() const;

    /**
     * @brief test if the state is CONSTRAINT_CONSISTENT.
     * @see ConstraintEngine::constraintConsistent(), ConstraintEngine::State
     */
    bool constraintConsistent() const;

    /**
     * @brief test if the state is PENDING.
     * @see ConstraintEngine::pending(), ConstraintEngine::State
     */
    bool pending() const;

    /**
     * @brief Forces propagation of any pending events.
     * @see ConstraintEngine::propagate()
     */
    void update();

    /**
     * @brief Withdraws a variable from active propagation. Takes immediate effect.
     * @see Constraint::deactivate, undoDeactivate
     */
    void deactivate();

    /**
     * @brief Removes a deactivation request. May result in activation of the variable for propagation.
     * @see deactivate, Constraint::undoDeactivate
     */
    void undoDeactivation();

    /**
     * @brief True if active in propagation. Otherwise false.
     * @see activate, deactivate
     */
    inline bool isActive() const {return m_deactivationRefCount == 0;}

    /**
     * @brief True if variable is internal to PLASMA and transactions are not recorded for it. Otherwise false.
     */
    inline bool isInternal() const {return m_internal;}

    /**
     * @brief the number of outstanding deactivation calls
     */
    unsigned int refCount() const;

    /**
     * Place holder for future work on lock management.
     *
     * @note I don't think this is adequate since it provides no
     * feedback whether the lock was acquired or not.
     * --wedgingt@ptolemy.arc.nasa.gov 2004 Feb 11
     *
     * CMG: Wrong. if you try lock, and already locked it will error
     * out. There is a method to test if a lock is already present.
     *
     * That is not enough in a multi-threaded environment.  The test &
     * lock operation must be atomic, or both threads could test
     * before either gets the lock and then both would try to get the
     * lock, causing one to error out despite following the
     * interface's requirement to test if unlocked first.
     * --wedgingt@ptolemy.arc.nasa.gov 2004 Apr 21
     *
     * This does not have any support in a multi-theraded environment. One must first
     * obtain a mutex elsewhere. Therefore, this is quite adequate.
     */
    void lock();
    void unlock();
    bool isLocked() const;

    /**
     * @brief Accessor for flag indicating a call to specify was made and not cleared by a reset
     */
    bool isSpecified() const;

    /**
     * @brief Accessor for the specified value. Will fail if the variable is not specified.
     */
    double getSpecifiedValue() const;

    /**
     * @brief Accessor for variable name.
     */
    const LabelStr& getName() const;

    /**
     * @brief Convenience method to get at parents of variables.  In some cases,
     * variables may not have a parent entity, so the default implementation
     * is to return noId.
     */
    const EntityId& parent() const;

    /**
     * @brief Accessor for the ConstraintEngine
     */
    const ConstraintEngineId& getConstraintEngine() const;

    /**
     * @brief Utility to capture the state of the constraint.
     */
    virtual std::string toString() const;
    virtual std::string toLongString() const;

    static const int NO_INDEX = -1;

    /**
     * @brief Access the index.
     * It is expected that variables will often be composed within other entities. To support this, we allow
     * a ConstrainedVariable to have an index reflecting its location in the parent.
     * @return NO_INDEX if there is no parent, otherwise a value > 0 indicating a position relevant to the caller.
     */
    int getIndex() const;

    /**
     * @brief Retrieve the last computed domain of the variable.
     * It should be used only when one cares to access last computed values.
     * @return The last computed domain of the variable.
     * @see derivedDomain
     */
    virtual const AbstractDomain& lastDomain() const = 0;

    /**
     * @brief Returns an up-to-date version of the derived domain.
     * @note May cause propagation.
     * @return The (completely propagated) derived domain.
     * @see lastDomain
     */
    virtual const AbstractDomain& derivedDomain() = 0;

    /**
     * @brief Retrieve the base domain.
     */
    virtual const AbstractDomain& baseDomain() const = 0;

    /**
     * @brief Restrict the base domain.
     * @param baseDomain The target restricted domain. It must be a subset of the current base domain.
     */
    void restrictBaseDomain(const AbstractDomain& baseDomain);

    /**
     * @brief Retract previously specified domain restriction.
     * @see specify()
     */
    virtual void reset();


    /**
     * @brief Restores the currentDomain to its full value.
     * It is up to the derived class to define this function. It is called by the ConstraintEngine. The behavior
     * is to relax the current domain to the largest appropriate set.
     * @see ConstraintEngine::propagate(), relax(), unspecify()
     * @todo Consider a version of this method that specifies an intermediary relaxtion target as part of smarter reprop.
     */
    virtual void relax();

    /**
     * @brief Test if the variable has been closed, and is thus ready for use in propagation
     * @see close
     */
    bool isClosed() const;

    /**
     * @brief Insert the value into its domain. Variable must be dynamic.
     */
    virtual void insert(double value);

    /**
     * @brief Remove a value from its domain. Variable must be dynamic.
     */
    virtual void remove(double value);

    /**
     * @brief Restricts the domain to a singleton value.
     * @param singletonValue to specify it to.
     */
    virtual void specify(double singletonValue);

    /**
     * @brief Allows a client to close a variables domain.
     * @note The variable must be open.
     */
    virtual void close();

    /**
     * @brief Allows a client to open a variable's domain.
     * @note The variable must be closed.
     */
    virtual void open();

    /**
     * @brief Tests if the variable can be specified.
     * @return true if the variable can be specified and false otherwise.
     */
    bool canBeSpecified() const;

    /**
     * @brief Supports the merging of variables by propagating messages on restriction of a base domain of a variable
     */
    virtual void handleBase(const AbstractDomain&) {}

    /**
     * @brief Supports the merging of variables by propagating messages on specification of a variable
     */
    virtual void handleSpecified(double value) {}

    /**
     * @brief Supports the merging of variables by propagating message on reset of a specified domain
     */
    virtual void handleReset() {}

    /**
     * @brief Hook up a listener to propagate notifications of deletion
     */
    void notifyAdded(const ConstrainedVariableListenerId& listener);

    /**
     * @brief Remove a listener
     */
    void notifyRemoved(const ConstrainedVariableListenerId& listener);
    /**
     * @brief Utility to obtain a display version of a double encoded value.
     */
    virtual std::string toString(double value) const;

    /**
     * @brief Accessor
     */
    bool specifiedFlag() const;

    /**
     * @brief Sum of the violation values for all the constraints attached to this variable
     */
    virtual double getViolation() const;

    /**
     * @brief string to present to the user describing what the getViolation value means when it's > 0
     */
    virtual std::string getViolationExpl() const;

    /**
     * @brief Keeps track of who's the current propagating constraint, so if there is a violation the ConstraintEngine can get to it
     */
    void setCurrentPropagatingConstraint(ConstraintId c);
    ConstraintId getCurrentPropagatingConstraint() const;

    // PS Methods:
    virtual bool isEnumerated() const;
    virtual bool isInterval() const;

    virtual bool isNull() const;
    virtual bool isSingleton() const;

    virtual PSVarValue getSingletonValue() const;
    virtual PSVarType getType() const;

    virtual PSList<PSVarValue> getValues() const;
    virtual PSList<PSConstraint*> getConstraints() const;

    virtual double getLowerBound() const;  // if isSingleton()==false && isInterval() == true
    virtual double getUpperBound() const;  // if isSingleton()==false && isInterval() == true

    virtual void specifyValue(PSVarValue& v);

    virtual PSEntity* getParent() const;

  protected:
    /**
     * @brief Ensure the constrained variable is part of a ConstraintEngine.
     * @param constraintEngine to which the variable is to belong. Must be a valid id.
     * @param parent An optional parent entity reference.
     * @param index An optional index indicating the position in the parent entity collection.
     */
    ConstrainedVariable(const ConstraintEngineId& constraintEngine,
			const bool internal,
			bool canBeSpecified,
			const LabelStr& name,
			const EntityId& parent = EntityId::noId(),
			int index = NO_INDEX);

    /**
     * @brief Allows subclass to add specific extra-work without over-riding core behavior.
     */
    virtual void handleRestrictBaseDomain(const AbstractDomain& baseDomain) = 0;

    /**
     * @brief Handle De-allocation
     */
    void handleDiscard();

    /**
     * @brief Derived classes implement this to impose additional validation checks for correct semantics of a variable.
     * @return true if the derived class semantics are complied with. Otherwise false.
     */
    virtual bool validate() const;

    /**
     * @brief Retrieve a modifiable version of the base domain.
     */
    virtual AbstractDomain& internal_baseDomain() = 0;

    void internalSpecify(double singletonValue);

    friend class Constraint; /**< Grant access so that the relationships between Constraint and Variable can be constructed and validated
			       without exposiing such methods publically. @see Constraint::Constraint(), Constraint::~Constraint() */

    friend class ConstraintEngine; /**< Grant access so the ConstraintEngine can reset the variable domain without exposing this behaviour
				     publicly.*/

    friend class Propagator; /*<! Grant access so the Propagator can reset the variable domain without exposing this behaviour
				     publicly.*/

    /**
     * @brief This is the only method to return a mutable copy of the domain.
     * Purpose is to restrict access to the current domain to avoid improper use. Note that we ensure
     * the current domain has a listener attached to connect the Constraint Engine. Use is intended strictly for Constraints
     * and ConstraintEngine, as well as for access in derived classes.
     * @return A reference to the domain under management of the ConstraintEngine.
     * @see lastDomain(), AbstractDomain
     */
    virtual AbstractDomain& getCurrentDomain() = 0;

    ConstrainedVariableId m_id; /**< Id of this. */

    DomainListenerId m_listener; /**< The listener provided by the Constraint Engine. This is the key connection point.
				    We require and ensure that the listener is in fact connected to the
				    current domain defined by the derived class.*/

    /**
     * @brief Called by Constraint on construction to impose the Constraint on the variable.
     * @param constraint - must be a valid id.
     * @param argIndex - the position of this variable in the scope of the constraint.
     */
    void addConstraint(const ConstraintId& constraint, int argIndex);

    /**
     * @brief Called by Constraint on destruction to retract the constraint from the variable.
     * @param constraint - must be a valid id.
     * @param argIndex - the position of this variable in the scope of the constraint.
     */
    void removeConstraint(const ConstraintId& constraint, int argIndex);

    /**
     * @brief Allow derived class to implement additional functionality for
     * addition of a constraint.
     */
    virtual void handleConstraintAdded(const ConstraintId&){}

    /**
     * @brief Allow derived class to implement additional functionality for
     *  removal of a constraint.
     */
    virtual void handleConstraintRemoved(const ConstraintId&){}

    /**
     * @brief Helper method to reset to a specific domain
     */
    void reset(const AbstractDomain& domain);

    // keeps track of who's the current propagating constraint, in case there is a violation
    ConstraintId m_propagatingConstraint;

  private:
    /**
     * @brief An internal utility to ensure the relationship between the constraints and the variable are valid.
     * @param constraint The constraint to ensure is a member of the constraint list.
     * @return true if present in the list, otherwise false.
     * @see Constraint::isVariableOf()
     */
    bool isConstrainedBy(const ConstraintId& constraint);

    /**
     * @brief Called by the ConstraintEngine to update the cycle count.
     * @param cycleCount The current relaxation cycle counter in the ConstraintEngine.
     * @see ConstraintEngine::notifyRelaxed
     */
    void updateLastRelaxed(int cycleCount);

    /**
     * @brief Accessor to the current cycle count stored on the variable
     * @return m_lastRelaxed the last propagation cycle in which the variable was relaxed.
     */
    int lastRelaxed() const;



    int m_lastRelaxed; /**< Holds the cycle in which the variable was last relaxed */
    const ConstraintEngineId m_constraintEngine; /**< Reference to the ConstraintEngine to which this variable belongs.
						    The  construction model of this class ensures
						    that it is always set to a valid ConstraintEngineId. */
    LabelStr m_name;
    const bool m_internal;
    const bool m_canBeSpecified;
    bool m_specifiedFlag; /**< True of internalSpecify is called.
			   It is possible that !canBeSpecified() && m_hasBeenSpecified */
    double m_specifiedValue; /**< Only meaningful if specifiedFlag set */
    const int m_index; /**< Locator for variable if constained by some entity. Default is NO_INDEX */
    const EntityId m_parent;
    unsigned int m_deactivationRefCount;/*!< The number of outstanding deactivation requests. */
    bool m_deleted; /*!< True when constraint is in the destructor. Otherwise false. */

    std::set<ConstrainedVariableListenerId> m_listeners; /**< Collection of listeners to variable changes */
    ConstraintList m_constraints; /**< Holds the list of Constraint/Argument pairs. The argument indicates the
				    index within the constraint scope, allowing for more efficient notification.
				    @see reset() */
  };

  /**
   * @brief Helper method to cast singleton values
   */
  template<class T>
  Id<T> id(const ConstrainedVariableId& var){
    return var->baseDomain().getSingletonValue();
  }



}
#endif
