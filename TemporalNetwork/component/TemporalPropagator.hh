#ifndef _H_TemporalPropagator
#define _H_TemporalPropagator

#include "Propagator.hh"
#include "PlanDatabaseDefs.hh"
#include "TemporalNetworkDefs.hh"
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
    static const TimepointId& getTimepoint(const TempVarId& var);

    void checkAndAddTnetVariables(const ConstraintId& constraint);
    void addTnetConstraint(const ConstraintId& constraint);

    void handleTemporalAddition(const ConstraintId& constraint);
    void handleTemporalDeletion(const ConstraintId& constraint);

    /**
     * @brief update timepoints in the Temporal Network with changes in the Constraint Engine's variables.
     */
    void updateTnet();

    /**
     * @brief update variables in the constraint engine with changes due to Temporal Propagation
     */
    bool updateTempVar();

    /*
     * @brief Update the time point in the tnet from the given CE variable
     */
    void updateTimepoint(const TempVarId& var);

    /**
     * @brief Update duration constraint in the TNET due to change in duration variable.
     */
    void updateDurationConstraint(const ConstraintId& constraint);

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

    TemporalNetworkId m_tnet; /*!< Temporal Network does all the propagation */
    ConstraintId m_activeConstraint;

    /*!< Synchronization data structures */
    std::set<TempVarId> m_activeVariables; /*!< Maintain the set of active start and end variables. Duration handled in consrinats */
    std::set<TempVarId> m_changedVariables; /*!< Manage the set of changed variables to be synchronized */
    std::set<ConstraintId> m_constraintsForExecution; /*!< StartEndDurationRelation stored here to allow write-back of duration to CE. */
    std::set<ConstraintId> m_durationChanges; /*!< StartEndDuration stored here if duration var changed to update TNET constraint */
    std::set<ConstraintId> m_constraintsForAddition; /*!< Buffer insertions till we have to propagate */
    std::set<TemporalConstraintId> m_constraintsForDeletion; /*!< Buffer deletions till you have to propagate. */
    std::set<TimepointId> m_variablesForDeletion; /*!< Buffer timepoints for deletion till we propagate. */

    std::set<TemporalNetworkListenerId> m_listeners;

    static const int DURATION_VAR_INDEX = 2; /*!< Position in token vector of variables */
  };
}
#endif
