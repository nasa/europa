#ifndef _H_TemporalPropagator
#define _H_TemporalPropagator

#include "Propagator.hh"
#include "PlanDatabaseDefs.hh"
#include "TemporalNetworkDefs.hh"
#include "TimepointWrapper.hh"
#include "TokenVariable.hh"
#include "IntervalIntDomain.hh"
#include <set>

namespace Prototype {

  class TemporalPropagator: public Propagator
  {
  public:
    TemporalPropagator(const LabelStr& name, const ConstraintEngineId& constraintEngine);
    virtual ~TemporalPropagator();
    void execute();
    bool updateRequired() const;

    /**
     * @see TemporalAdvisor
     */
    bool canPrecede(const TempVarId& first, const TempVarId& second);

    /**
     * @see TemporalAdvisor
     */
    bool canFitBetween(const TempVarId& start, const TempVarId& end,
		       const TempVarId& predend, const TempVarId& succstart);

    /**
     * @see TemporalAdvisor
     */
    bool canBeConcurrent(const TempVarId& first, const TempVarId& second);

    void addListener(const TemporalNetworkListenerId& listener);

  protected:
    void handleConstraintAdded(const ConstraintId& constraint);
    void handleConstraintRemoved(const ConstraintId& constraint);
    void handleConstraintActivated(const ConstraintId& constraint);
    void handleConstraintDeactivated(const ConstraintId& constraint);
    void handleNotification(const ConstrainedVariableId& variable, 
			    int argIndex, 
			    const ConstraintId& constraint, 
			    const DomainListener::ChangeType& changeType);

  private:
    friend class TimepointWrapper;
    void notifyDeleted(const TempVarId& tempVar, const TimepointId& tp);

    void addTimepoint(const TempVarId& var);
    void addTemporalConstraint(const ConstraintId& constraint);

    inline static const TimepointId& getTimepoint(const TempVarId& var) {
      check_error(var->getIndex() != DURATION_VAR_INDEX);
      check_error(var->getExternalEntity().isValid());
      const TimepointWrapperId wrapper(var->getExternalEntity());
      return wrapper->getTimepoint();
    }

    void handleTemporalAddition(const ConstraintId& constraint);
    void handleTemporalDeletion(const ConstraintId& constraint);

    /**
     * @brief update timepoints in the Temporal Network with changes in the Constraint Engine's variables.
     */
    void updateTnet();

    /**
     * @brief update variables in the constraint engine with changes due to Temporal Propagation
     */
    void updateTempVar();

    /**
     * @brief Update the time point in the tnet from the given CE variable
     */
    void updateTimepoint(const TempVarId& var);

    /**
     * @brief update a constraint in the tnet - before, concurrent, startEndDuration
     */
    void updateTemporalConstraint(const ConstraintId& constraint);

    /**
     * @brief Shared method to handle update to a constraint in the temporal network.
     * @param var The Temporal Variable driving the change
     * @param tnetConstraint The constraint used to enforce prior (or current) values.
     * @param lb The new lower bound
     * @param ub The new upper bound
     * @return TemporalConstraintId::noId() if the constrant is not deleted. Otherwise it will return the
     * new replacement constraint.
     */
    TemporalConstraintId updateConstraint(const TempVarId& var,
					  const TemporalConstraintId& tnetConstraint, 
					  Time lb,
					  Time ub);

    /**
     * @brief Buffer the variable in either the new variable buffer or the change variable buffer
     * depending on its state - i.e. if timepoint already created or not.
     */
    void buffer(const TempVarId& var);

    /**
     * @brief Test that the buffer status is correct prior to propagation
     */
    bool isValidForPropagation() const;

    TemporalNetworkId m_tnet; /*!< Temporal Network does all the propagation */

    /*!< Synchronization data structures */
    std::set<TempVarId> m_activeVariables; /*!< Maintain the set of active start and end variables. Duration handled in consrinats */
    std::set<TempVarId> m_changedVariables; /*!< Manage the set of changed variables to be synchronized */
    std::set<ConstraintId> m_changedConstraints; /*!< Constraint Agenda */
    std::set<TemporalConstraintId> m_constraintsForDeletion; /*!< Buffer deletions till you have to propagate. */
    std::set<TimepointId> m_variablesForDeletion; /*!< Buffer timepoints for deletion till we propagate. */
    std::set<EntityId> m_wrappedTimepoints;
    std::set<TemporalNetworkListenerId> m_listeners;

    static const int DURATION_VAR_INDEX = 2; /*!< Position in token vector of variables */
    static const LabelStr& durationConstraintName();
  };
}
#endif
