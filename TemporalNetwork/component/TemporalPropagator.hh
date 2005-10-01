#ifndef _H_TemporalPropagator
#define _H_TemporalPropagator

#include "ConstrainedVariable.hh"
#include "Propagator.hh"
#include "TemporalNetworkDefs.hh"
#include "TimepointWrapper.hh"
#include "IntervalIntDomain.hh"

#include <set>

/**
 * @author Conor McGann & Sailesh Ramakrishnan
 */

namespace EUROPA {

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
    bool canPrecede(const ConstrainedVariableId& first, const ConstrainedVariableId& second);

    /**
     * @see TemporalAdvisor
     */
    bool canFitBetween(const ConstrainedVariableId& start, const ConstrainedVariableId& end,
		       const ConstrainedVariableId& predend, const ConstrainedVariableId& succstart);

    /**
     * @see TemporalAdvisor
     */
    bool canBeConcurrent(const ConstrainedVariableId& first, const ConstrainedVariableId& second);

    /**
     * @see TemporalAdvisor
     */
    const IntervalIntDomain getTemporalDistanceDomain(const ConstrainedVariableId& first, 
						      const ConstrainedVariableId& second, const bool exact);


    /**
     * @see TemporalAdvisor
     */
    unsigned int mostRecentRepropagation() const;

    void getTemporalNogood(const ConstrainedVariableId& useAsOrigin,
                           std::vector<ConstrainedVariableId>& fromvars,
                           std::vector<ConstrainedVariableId>& tovars,
                           std::vector<long>& lengths
                           );

    void addListener(const TemporalNetworkListenerId& listener);

  protected:
    void handleDiscard();
    void handleConstraintAdded(const ConstraintId& constraint);
    void handleConstraintRemoved(const ConstraintId& constraint);
    void handleConstraintActivated(const ConstraintId& constraint);
    void handleConstraintDeactivated(const ConstraintId& constraint);
    void handleVariableDeactivated(const ConstrainedVariableId& var);
    void handleVariableActivated(const ConstrainedVariableId& var);
    void handleNotification(const ConstrainedVariableId& variable, 
			    int argIndex, 
			    const ConstraintId& constraint, 
			    const DomainListener::ChangeType& changeType);

  private:
    friend class TimepointWrapper;
    
    TemporalConstraintId addSpecificationConstraint(const TemporalConstraintId& tc, const TimepointId& tp, const Time lb, const Time ub);

    void notifyDeleted(const ConstrainedVariableId& tempVar, const TimepointId& tp);

    void addTimepoint(const ConstrainedVariableId& var);
    void addTemporalConstraint(const ConstraintId& constraint);

    bool isEqualToConstraintNetwork();
    bool isConsistentWithConstraintNetwork();

    inline static const TimepointId& getTimepoint(const ConstrainedVariableId& var) {
      static const TimepointId sl_noId;
      check_error(var->getExternalEntity().isNoId() || var->getExternalEntity().isValid());
      const TimepointWrapperId wrapper(var->getExternalEntity());

      if(wrapper.isNoId())
	return sl_noId;

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
    void updateTimepoint(const ConstrainedVariableId& var);

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
    TemporalConstraintId updateConstraint(const ConstrainedVariableId& var,
					  const TemporalConstraintId& tnetConstraint, 
					  Time lb,
					  Time ub);

    /**
     * @brief Buffer the variable in either the new variable buffer or the change variable buffer
     * depending on its state - i.e. if timepoint already created or not.
     */
    void buffer(const ConstrainedVariableId& var);

    /**
     * @brief Test that the buffer status is correct prior to propagation
     */
    bool isValidForPropagation() const;

    TemporalNetworkId m_tnet; /*!< Temporal Network does all the propagation */

    /*!< Synchronization data structures */
    std::map<int, ConstrainedVariableId> m_activeVariables; /**< Maintain the set of active start and end variables. 
							     Duration handled in constraints */
    std::map<int, ConstrainedVariableId> m_changedVariables; /*!< Manage the set of changed variables to be synchronized */
    std::set<ConstraintId, EntityComparator<EntityId> > m_changedConstraints; /*!< Constraint Agenda */
    std::set<TemporalConstraintId, EntityComparator<EntityId> > m_constraintsForDeletion; /*!< Buffer deletions till you have to propagate. */
    std::set<TimepointId> m_variablesForDeletion; /*!< Buffer timepoints for deletion till we propagate. */
    std::set<EntityId> m_wrappedTimepoints;
    std::set<TemporalNetworkListenerId> m_listeners;
    unsigned int m_mostRecentRepropagation;
  };
}
#endif
