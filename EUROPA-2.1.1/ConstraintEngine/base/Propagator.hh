#ifndef _H_Propagator
#define _H_Propagator

/**
 * @file Propagator.hh
 * @author Conor McGann
 * @date August, 2003
 */

#include "Entity.hh"
#include "ConstraintEngineDefs.hh"
#include "DomainListener.hh"
#include "LabelStr.hh"

namespace EUROPA {
  /**
   * @class Propagator
   * @brief A mechanism for extendible, maintanable propagation event (i.e. agenda)  management
   *
   * The main features of this base class are:
   * @li Provides event listener interfaces for receiving propagation event notifications (i.e. notify() ).
   * @li provides query interfaces so that state can be polled by the ConstraintEngine (i.e. updateRequired() )
   * @li Provides command interfaces so propagation execution can be initiated by the ConstraintEngine, which is the
   * main controller (i.e. execute() )
   * @li Provides for correct linkage to constraint engine by construction ( i.e. Propagator() )
   *
   */
  class Propagator: public Entity
  {
  public:
    /**
     * @brief Retrieve the ConstraintEngine to which this Propagator belongs.
     * @return A valid ConstraintEngine reference
     */
    const ConstraintEngineId& getConstraintEngine() const;

    /**
     * @brief Name of propagator. Used for configuration
     */
    const LabelStr& getName() const;

    /**
     * @brief Obtain the list of all constraints managed by this Propagator
     */
    const std::set<ConstraintId>& getConstraints() const;

    /**
     * @brief tests if the propagatpr is enabled
     */
    bool isEnabled() const;

    /**
     * @brief Enable the propagator
     */
    void enable();

    /**
     * @brief Disable the propagator
     */
    void disable();
    /**
     * @brief Reference to self
     */
    const PropagatorId& getId() const;

  protected:
    friend class ConstraintEngine; /**< Grant access so protected members can be used to enforce collaboration model
                                      without exposing details publically. */

    /**
     * @brief Constructor guarantees the Propagator belongs to exactly one ConstraintEngine
     * @param constraintEngine The engine to place it in. Must be a valid id.
     */
    Propagator(const LabelStr& name, const ConstraintEngineId& constraintEngine);

    /**
     * @brief Destructor - will remove the Propagator from the ConstraintEngine.
     */
    virtual ~Propagator();


    /**
     * @brief Constraint may be added to the Propagator by the ConstraintEngine.
     * @param constraint The Constraint to be added.
     */
    void addConstraint(const ConstraintId& constraint);

    /**
     * @brief Constraint may be removed from the Propagator by the ConstraintEngine.
     * @param constraint The Constraint to be removed. It must be present.
     */
    void removeConstraint(const ConstraintId& constraint);

    /**
     * @brief Handler interface for variable change events. 
     *
     * Purpose is to facilitate agenda synchronization.These events will always be restrictions.
     * This notification is information rich, so that agenda management and constraints can take advantage
     * of as much data as possibel if they are sophisticated enough.
     * @param variable The variable that has changed
     * @param argIndiex The position of the variable in the scope of the given Constraint.
     * @param constraint The constraint that may need to be woken up.
     * @param changeType The nature of the change on the variable. Will always be a restriction
     * @see ConstraintEngine::notify()
     */
    virtual void handleNotification(const ConstrainedVariableId& variable, 
                                    int argIndex, 
                                    const ConstraintId& constraint, 
                                    const DomainListener::ChangeType& changeType) = 0;
    /**
     * @brief Instruction from ConstraintEngine to commence execution of pending propagation events.
     */
    virtual void execute() = 0;

    /**
     * @brief test of the Propagator has any pending propagation events.
     * @return true of there are any changes pending, false otherwise.
     * @see ConstraintEngine::constraintConsistent(), ConstraintEngine::propagate()
     */
    virtual bool updateRequired() const = 0;

    /**
     * @brief Allow custom processing when a Constraint is added to the Propagator.
     * @param constraint The Constraint to be added.
     * @see addConstraint()
     */
    virtual void handleConstraintAdded(const ConstraintId& constraint) = 0;

    /**
     * @brief Allow custom processing when a Constraint is removed from the Propagator.
     * @param constraint The Constraint to be removed.
     * @see removeConstraint()
     */
    virtual void handleConstraintRemoved(const ConstraintId& constraint) = 0;

    /**
     * @brief Hook to allow a Propagator to update state if one of its constraints has been deactivated.
     * @param constraint The constraint that has been deactivated.
     */
    virtual void handleConstraintDeactivated(const ConstraintId& constraint) = 0;

    /**
     * @brief Hook to allow a Propagator to update state if one of its constraints has been activated.
     * @param constraint The constraint that has been activated.
     */
    virtual void handleConstraintActivated(const ConstraintId& constraint) = 0;

    /**
     * @brief Handle a variable deactivation.
     * @param var The inactive variable
     */
    virtual void handleVariableDeactivated(const ConstrainedVariableId& var){}

    /**
     * @brief Handle a variable Activation.
     * @param var The active variable
     */
    virtual void handleVariableActivated(const ConstrainedVariableId& var){}

    /**
     * @brief Request execution of a Constraint.
     *
     * It is not possible to directly execute a Constraint. This is because we want to ensure control policies defined
     * can be enforced and not circumvented in derived classes. This call will delegate execution to the ConstraintEngine.
     * @param The constraint to be executed. It must be part in m_constraints.
     */
    virtual void execute(const ConstraintId& constraint);

    static AbstractDomain& getCurrentDomain(const ConstrainedVariableId& var);
    
    // Constraint Violation Mgmt
    virtual void notifyConstraintViolated(ConstraintId c);
    virtual void notifyVariableEmptied(ConstrainedVariableId v);

  private:
    Propagator(); /**< NO IMPL - MUST HAVE A ConstraintEngine. */

    std::set<ConstraintId> m_constraints; /**< The list of all constraints (should be a set) managed by this Propagator. */
    PropagatorId m_id; /**< Self reference. */
    const LabelStr m_name;
    const ConstraintEngineId& m_constraintEngine; /**< The ConstraintEngine to which this Propagator belongs. Must be valid. */
    bool m_enabled; /**< Indicates if the propagator is enabled or not */
  };
}
#endif
