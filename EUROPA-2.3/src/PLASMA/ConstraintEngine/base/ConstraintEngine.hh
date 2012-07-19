#ifndef _H_ConstraintEngine
#define _H_ConstraintEngine

/**
 * @file ConstraintEngine.hh
 * @author Conor McGann
 * @brief Defines the kernel for the Constraint Engine Framework
 * @date August, 2003
 * @ingroup ConstraintEngine
 */

#include "ConstraintEngineDefs.hh"
#include "PSConstraintEngine.hh"
#include "CESchema.hh"
#include "DomainListener.hh"
#include "LabelStr.hh"
#include "ConstraintEngineListener.hh"
#include "Entity.hh"
#include "Engine.hh"

#include <set>
#include <map>
#include <string>

namespace EUROPA {

  class ViolationMgr
  {
  	public:
  	  virtual unsigned int getMaxViolationsAllowed() = 0;
  	  virtual void setMaxViolationsAllowed(unsigned int i) = 0;

  	  virtual double getViolation() const = 0;
  	  virtual PSList<std::string> getViolationExpl() const = 0;
  	  virtual PSList <PSConstraint*> getAllViolations() const = 0;

  	  virtual bool handleEmpty(ConstrainedVariableId v) = 0;
  	  virtual bool handleRelax(ConstrainedVariableId v) = 0;
      virtual bool canContinuePropagation() = 0;

  	  virtual bool isViolated(ConstraintId c) const = 0;
  	  virtual void addViolatedConstraint(ConstraintId c) = 0;
	  virtual void removeViolatedConstraint(ConstraintId c) = 0;

	  virtual bool isEmpty(ConstrainedVariableId v) const = 0;
  	  virtual void addEmptyVariable(ConstrainedVariableId c) = 0;
  	  virtual const ConstrainedVariableSet& getEmptyVariables() const = 0;
  	  virtual void clearEmptyVariables() = 0;
      virtual void relaxEmptyVariables() = 0;

  	protected:
  	   ViolationMgr() {}
  	   virtual ~ViolationMgr() {}

  	   friend class ConstraintEngine;
  };

  /**
   * @class ConstraintEngine
   * @brief Base Class from which specific Constraint Engine Framework Instances can be derived.
   *
   * The ConstraintEngine is the core of the framework for building particular constraint processing
   * components. It defines a public interface which presents a state model, and s small set of operations to
   * query state and take appropriate action. Much of the functionality of the ConstraintEngine is provided by its
   * protected interface and its friends. The responsibilities of this class are:
   * @li Enforce the semantics of the state model. The states are defined by ConstraintEngine::State
   * @li Enforce the propagation control policy i.e. who gets notified, of what, when. And also who gets invoked, when, for what
   * action.
   * @li Enforce the proper relationships between the component classes
   * @li Define and enforce the boundaries for extension and customization
   *
   * The ConstraintEngine plays the role of
   * mediator between Constraint, ConstrainedVariable, AbstractDomain, VariableChangeListener, and Propagator. As such, it is
   * the central controller for the state of the constraint network and the sle delegator of responsibility for managing
   * state changes. This allows the ConstraintEngine to enforce many relationships and policies that are hard to enforce otherwise.
   * See subsequent sections for further details.
   * The mediation is accomplished (and protected) through private members by all parties and a hub and spoke friendship
   * model. The ConstraintEngine (the hub) is a friend of each spoke, and each spoke is a friend of the ConstraintEngine
   * The Mediation algorithms are also an example of the Template Pattern, since it defines a template for
   * collaboration between components which can be customized be changing the handling of the leaf calls, e.g. canIgnore(),
   * rather than core inner loops.
   *
   * @par Relationships Enforced
   * @li Each spoke component (ConstrainedVariable, Constraint, Propagator) instance belongs to exactly one ConstraintEngine (hub) instance.
   * @li Each Constraint instance belongs to exaclty one Propagator instance.
   * @li There is exactly one Propagator instance which will accept any given Constraint instance.  The criteria for acceptance of a
   * Constraint by a Propagator is a completely customizable feature of the Propagator class. However, a safety check is conducted
   * when running under normal compilation which ensures the integrity of the relationship.
   *
   * @par Propagation Policy Constraints Enforced
   * While we may wish to permit a range of policies for scheduling notification and processing or propagation events, the framework
   * must ensure that models for customization and extension in this regard to not cede control so much so that we lose the ability to
   * enforce the semantics of the overall network, or a global policy for scheduling.
   * @li Propagation may only occur when no variable domain is empty.
   * @li Propagation will terminate immediately if a variable domain is emptied. Consequently, there is at most one empty domain
   * possible at a time. This is accomplished since the ConstraintEngine is immediately notified the instant a domain has been emptied and
   * all subsequent calls routed through the ConstraintEngine can be checked for this.
   * @li During propagation, all Propagator instances are executed in prioriy order until quiescence, where priority is based on the
   * order of insertion into the ConstraintEngine. Thus, the configuration of Propagator instances is the primary means of customization
   * of propagation.
   *
   * @par Extension and Customization
   * @li Implement custom propagator to define agenda management scheme for targetted constraints.
   * @li Implement custom constraints. Constraints may only make restrictions to their domains during propagation. This is enforced
   * by ConstraintEngine as it process events on the domains.
   * @li Implement custom event handling for constraints to propagate messages i.e. Constraint::canIgnore()
   * @li Construct different ConstraintEngine configurations w.r.t Propagators.
   * @see Constraint, ConstrainedVariable, AbstractDomain, VariableChangeListener, Propagator
   * @see "Propagator: A Family of Patterns, Peter H. Feiler, Walter F. Tichy"
   */
  class ConstraintEngine : public PSConstraintEngine {

  public:
    enum State { PROVEN_INCONSISTENT = 0, /**< A domain has been emptied, and no subsequent retractions have occurred yet. */
                 CONSTRAINT_CONSISTENT, /**< No domains are empty, and all constraints are satisfied. All events have been processed. */
                 PENDING /**< No domains are empty, but there are domain change events awaiting processing. */
    };

    enum Event { UPPER_BOUND_DECREASED = 0, /**< If the upper bound of an interval domain is reduced. */
                 LOWER_BOUND_INCREASED, /**< If the lower bound of an interval domain is increased. */
                 BOUNDS_RESTRICTED, /**< Both upper and lower are decreased and increased respectively. */
                 VALUE_REMOVED, /**< A restriction to an enumerated domain. */
                 RESTRICT_TO_SINGLETON, /**< A restriction of the domain to a singleton value through inference. */
                 SET_TO_SINGLETON, /**< Special case restriction when the domain is set to a singleton i.e. instantiated. */
                 RESET, /**< Special case of an external relaxation. */
                 RELAXED, /**< Inferred relaxtion to the domain is indicated by this type of change. */
                 CLOSED, /**< If a dynamic domain is closed this event will be generated. */
                 OPENED, /**< If a closed domain is re-opened this event will be generated. */
                 EMPTIED, /**< If a domain is emptied, indicating an inconsistency, then this event is generated. */
                 PROPAGATION_COMMENCED, /**< Starting propagation. */
                 PROPAGATION_COMPLETED, /**< Completed propagation. */
                 PROPAGATION_PREEMPTED, /**< Propagation pre-empted and incomplete. */
                 CONSTRAINT_ADDED, /**< A constraint was created. */
                 CONSTRAINT_REMOVED, /**< A constraint was removed. */
                 CONSTRAINT_EXECUTED, /**< A constraint was used during propagation. */
                 VARIABLE_ADDED, /**< A variable was created. */
                 VARIABLE_REMOVED, /**< A variable was removed. */
                 LAST_EVENT /**< Use only for EVENT_COUNT. @see EVENT_COUNT */
    };

    /**
     * @brief Count of possible constraint events.
     * @note Depends on the first Event having an int value of 0.
     */
    static const int EVENT_COUNT = (int)LAST_EVENT;

    /**
     * @brief Constructor currently creates a basic configuration of propagators.
     */
    ConstraintEngine(const CESchemaId& schema);

    /**
     * @brief Destructor.
     */
    virtual ~ConstraintEngine();

    /**
     * @brief Id self reference.
     */
    const ConstraintEngineId& getId() const;

    /**
     * @brief purge all elements from the Engine. Will delete all variables, constraints, and propagators.
     */
    void purge();

    /**
     * @brief test if the state is PROVEN_INCONSISTENT.
     */
    bool provenInconsistent() const;

    /**
     * @brief test if the state is CONSTRAINT_CONSISTENT.
     */
    bool constraintConsistent() const;

    /**
     * @brief test if the state is PENDING.
     */
    bool pending() const;

    /**
     * @brief test if the constraint engine is actively propagating.
     */
    bool isPropagating() const;

    /**
     * @brief Retrieve the current propagation cycle count.
     */
    inline unsigned int cycleCount() const {
      return m_cycleCount;
    }

    /**
     * @brief Restrive the cycle of the most recent repropagation.
     */
    unsigned int mostRecentRepropagation() const;

    /**
     * @brief Initiate propagation of any pending domain change events.
     * Engine must be in a PENDING or CONSTRAINT_CONSISTENT state.
     * @return true if the resulting state is CONSTRAINT_CONSISTENT,
     * otherwise return false, indicating the state is
     * PROVEN_INCONSISTENT.
     */
    virtual bool propagate();

    /**
     * @brief Indicates whether the ConstraintEngine is able to continue propagation.
     * should be invoked after the constraint has been proven inconsistent so that Propagators
     * and other classes can decide whether to keep relevant state for when propagation resumes
     */
    bool canContinuePropagation() const;

    /**
     * @brief Accessor for all constrained variables.
     */
    const ConstrainedVariableSet& getVariables() const;

    /**
     * @brief Get variable based on its position in the set of variables.
     * @note This is not a const in-case it manages some internal cache to optimize later
     */
    ConstrainedVariableId getVariable(unsigned int index);

    /**
     * @brief Get index based on its position in the set of variables.
     * @note This is not a const in-case it manages some internal cache to optimize later
     */
    unsigned int getIndex(const ConstrainedVariableId& var);

    /**
     * @brief Accessor for all constraints.
     */
    const ConstraintSet& getConstraints() const;

    /**
     * @brief Get constraint based on its position in the set of constraints.
     */
    ConstraintId getConstraint(unsigned int index);

    /**
     * @brief Get index based on its position in the set of constraints.
     */
    unsigned int getIndex(const ConstraintId& constr);

    /**
     * @brief Accessor for all propagators
     */
    const PropagatorId& getPropagatorByName(const LabelStr& name)  const;

    /**
     * @see If AutoPropagation is enabled, the constraint engine will automatically invoke propagate()
     * after variable or constraint modifications (create/delete/change) caused through calls in the public API
     * (mainly PSConstraintEngine, PSVariable, PSConstraint, although calls in some other classes may indirectly trigger propagation in the future)
     */
    virtual void setAutoPropagation(bool v);

    /**
     * @see setAutoPropagation
     */
    virtual bool getAutoPropagation() const;

    /**
     * @brief Integrates flag for control of auto propagation with tests for not already propagating
     */
    bool shouldAutoPropagate() const;

    /**
     * @brief If violations are allowed propagation will continue and the constraint engine will keep track of violated constraints
     */
    virtual void setAllowViolations(bool v);

    /**
     * @see setAllowViolations
     */
    virtual bool getAllowViolations() const;

    /**
     * @brief returns total violation in the constraint engine
     */
    virtual double getViolation() const;

    /**
     * @brief returns string explanation for total violation in the constraint engine
     */
    virtual PSList<std::string> getViolationExpl() const;

    /**
     * @brief returns set of constraint violations
     */
  	virtual PSList<PSConstraint*> getAllViolations() const;

    /**
     * @brief is constraint c violated?
     */
    bool isViolated(ConstraintId c) const;

    /**
     * @brief Test of the network is in a relaxed state
     */
    bool isRelaxed() const;

    const CESchemaId& getCESchema() const;

    // PSConstraintEngine methods
    virtual PSVariable* getVariableByKey(PSEntityKey id);
    virtual PSVariable* getVariableByName(const std::string& name);


    /**
     * @brief Create a variable
     */
    ConstrainedVariableId createVariable(const char* typeName,
                                                const bool internal = false,
                                                bool canBeSpecified = true,
                                                const char* name = NO_VAR_NAME,
                                                const EntityId& parent = EntityId::noId(),
                                                int index = ConstrainedVariable::NO_INDEX);

    /**
     * @brief Create a variable
     */
    ConstrainedVariableId createVariable(const char* typeName,
                                                const AbstractDomain& baseDomain,
                                                const bool internal = false,
                                                bool canBeSpecified = true,
                                                const char* name = NO_VAR_NAME,
                                                const EntityId& parent = EntityId::noId(),
                                                int index = ConstrainedVariable::NO_INDEX);


    ConstraintId createConstraint(const LabelStr& name,
                     const std::vector<ConstrainedVariableId>& scope);

    void deleteConstraint(const ConstraintId& c);

    /**
     * @brief Create a value for a string
     */
    double createValue(const char* typeName, const std::string& value);

  protected:

    /**
     * @brief Creates a listener for the specified variable thus
     * 'plugging it in' to the propagation events.
     * The implementation of this listener will determine much of the
     * propagation control policy. This method is not virtual at the
     * moment, but we clearly want a way to customize this behavior.
     * @param variable the variable to be listened to.
     * @param constraints the reference to the list of constraints of the variable.
     * @return A reference to the listener to be registered with the variable.
     */
    DomainListenerId allocateVariableListener(const ConstrainedVariableId& variable,
                                              const ConstraintList& constraints) const;

    ViolationMgr& getViolationMgr() { return *m_violationMgr; }
    const ViolationMgr& getViolationMgr() const { return *m_violationMgr; }

  private:
    friend class Constraint;
    friend class VariableChangeListener;
    friend class Propagator;
    friend class ConstraintEngineListener;
    friend class ViolationMgrImpl;

    /**
     * @brief Constraint Constructor will call this method to establish linkage with the ConstraintEngine.
     * @param constraint The constraint to be added.
     */
    void add(const ConstraintId& constraint, const LabelStr& propagatorName);

    /**
     * @brief Constraint destructor will call this method to withdraw links with the ConstraintEngine.
     * @param constraint The constraint to be removed.
     */
    void remove(const ConstraintId& constraint);

    /**
     * @brief Called by the VariableListener when a change occurs on its varible.
     *
     * This method provides the core event control logic of the ConstraintEngine.
     * @param listener The listener with the necessary data for the variable.
     * @param change Type the type of change that occured on the variable.
     * @see DomainListener::ChangeType, handleEmptied, handleRelaxed, Propagator::handleNotification(), Constraint::canIgnore()
     */
    void notify(const ConstrainedVariableId& source, const DomainListener::ChangeType& changeType);

    /**
     * @brief Update appropriately when a variabe domain has been emptied.
     * @param the variable that has been emptied.
     * @see notify(const ConstrainedVariableId& source, const DomainListener::ChangeType& changeType)
     */
    void handleEmpty(const ConstrainedVariableId& variable);

    /**
     * @brief Update appropriately when a variable has been relaxed.
     * @param the variable that has been emptied.
     * @see notify(const ConstrainedVariableId& source, const DomainListener::ChangeType& changeType)
     */
    void handleRelax(const ConstrainedVariableId& variable);

    /**
     * @brief Propagator constructor will call this method to register the Propagator with the ConstraintEngine.
     * @param propagator the propagator to be added.
     */
    void add(const PropagatorId& propagator);

    /**
     * @brief Notification from a constraint when it has been deactivated.
     */
    void notifyDeactivated(const ConstraintId& deactivatedConstraint);

    /**
     * @brief Notification from a constraint when it has been activated.
     */
    void notifyActivated(const ConstraintId& activatedConstraint);

    /**
     * @brief Notification that a constraint has become redundant.
     */
    void notifyRedundant(const ConstraintId& redundantConstraint);

    /**
     * @breif Notification from a variable when it has been deactivated
     */
    void notifyDeactivated(const ConstrainedVariableId& var);

    /**
     * @brief Notification from a variable when it has been activated
     */
    void notifyActivated(const ConstrainedVariableId& var);

    /**
     * @brief Invokes execution event handler on the designated constraint.
     * Called by the propagator as it processes its agenda. This
     * method is used where specific domain changes are ignored or
     * unavailable.
     * @param constraint the constraint to be executed.
     */
    void execute(const ConstraintId& constraint);

    /**
     * @brief Invokes execution event handler on the designated constraint.
     *
     * Called by the propagator as it processes its agenda. This method is used where specific domain changes are available.
     * @param constraint the constraint to be executed.
     * @param variable the variable which is the source of the event.
     * @param argIndex the position of the variable in the scope of the constraint.
     * @param changeType the nature of change that occurred on the domain.
     */
    void execute(const ConstraintId& constraint,
                 const ConstrainedVariableId& variable,
                 int argIndex,
                 const DomainListener::ChangeType& changeType);

    friend class ConstrainedVariable;

    /**
     * @brief Called by ConstrainedVariable constructor to link the variable with the constraint.
     * @param variable the variable to be constrained.
     */
    void add(const ConstrainedVariableId& variable);

    /**
     * @brief Called by ConstrainedVariable destructor to unlink the variable with the constraint.
     * @param variable the variable to be removed.
     */
    void remove(const ConstrainedVariableId& variable);

    /**
     * @brief Called by ConstraintEngineListener constructor.
     * @param listener The listener to add
     */
    void add(const ConstraintEngineListenerId& listener);

    /**
     * @brief Called by ConstraintEngineListener destructor.
     * @param listener The listener to remove.
     */
    void remove(const ConstraintEngineListenerId& listener);

    /**
     * @brief Accessor to retrieve the next active propagator in the
     * list of active propagators.
     * @return noId if no propagator is active, otherwise return the
     * highest priority active propagator.
     */
    PropagatorId getNextPropagator() const;

    /**
     * @brief Helper method to encapsulate increments in the propagation cycle count.
     * @see m_cycleCount
     */
    void incrementCycle();

    /**
     * @brief Internal helper methods
     */
    bool hasEmptyVariables() const;
    void clearEmptyVariables();
    bool doPropagate();


    /**
     * @brief Deactivate redundant constraints buffered for pprocessing after successful propagation
     */
    void processRedundantConstraints();

    /**
     * @brief Allow ViolationMgr to inform us of updates we should publish
     */
    void notifyViolationAdded(ConstraintId variable);
    void notifyViolationRemoved(ConstraintId variable);


    // debug methods
    std::string dumpPropagatorState(const std::list<PropagatorId>& propagators) const;

    int addLinkedVarsForRelaxation(const ConstrainedVariableId& var,
				   std::list<ConstrainedVariableId>& dest,
				   std::list<ConstrainedVariableId>::iterator pos,
				   ConstrainedVariableSet& visitedVars);


    ConstraintEngineId m_id;
    ConstrainedVariableSet m_variables; /*!< The set of all variables under the control of the ConstraintEngine. */
    ConstraintSet m_constraints; /*!< The set of all constraints. */
    std::list<PropagatorId> m_propagators; /*!< The list of all propagators.
					     Position in the list indicates execution priority. This
					     is determined by the order of construction. */
    std::map<double, PropagatorId> m_propagatorsByName; /*!< Support configuration and lookup by name. */

    bool m_relaxing;
    bool m_relaxingViolation; /*!< Flag to record if relax events should be ignored by the ViolationMgr */
    bool m_relaxed;           /*!< Set when a domain is RELAXED. Implies PENDING. */
    bool m_propInProgress;    /*!< Set true when doing propagation, otherwise false. */
    bool m_deleted;           /*!< Used to control cleanup, preventing cycles. */
    bool m_purged;            /*!< Indicates if the engine has been purged of its data */
    bool m_dirty;              /*!< Flag to record if any messages handled without propagating consequences */

    int m_cycleCount; /*!< A monotonically increasing count of propagation cycles. Identifies
                         when propagation events have already been queued or handled. */
    unsigned int m_mostRecentRepropagation; /*!< A monotonically increasing record of cycles where a relaxation occurred. */

    std::set<ConstraintEngineListenerId> m_listeners; /*!< Stores the set of registered listeners. */

    ConstraintSet m_redundantConstraints; /*!< Pending redundant constraints awaiting deactivation */

    ViolationMgr* m_violationMgr;
    bool m_autoPropagate;

    const CESchemaId& m_schema;
  };
}
#endif
