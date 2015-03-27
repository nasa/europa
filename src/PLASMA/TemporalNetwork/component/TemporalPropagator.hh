#ifndef H_TemporalPropagator
#define H_TemporalPropagator

#include "ConstrainedVariable.hh"
#include "Propagator.hh"
#include "TemporalNetworkDefs.hh"
#include "Domains.hh"
#include "Token.hh"

#include <set>
#include <map>

namespace EUROPA {
  /**
   * @class TemporalPropagator
   * @author Conor McGann & Sailesh Ramakrishnan
   * @brief Propagation event manager specialized for temporal constraints. See parent class for more information.
   * @ingroup TemporalNetwork
   */
  class TemporalPropagator: public Propagator
  {
  public:
    TemporalPropagator(const std::string& name, const ConstraintEngineId constraintEngine);
    virtual ~TemporalPropagator();
    void execute();
    bool updateRequired() const;

    /**
     * @see TemporalAdvisor::canPrecede
     */
    bool canPrecede(const ConstrainedVariableId first, const ConstrainedVariableId second);
    bool mustPrecede(const ConstrainedVariableId first, const ConstrainedVariableId second);

    /**
     * @see TemporalAdvisor::canFitBetween
     */
    bool canFitBetween(const ConstrainedVariableId start, const ConstrainedVariableId end,
		       const ConstrainedVariableId predend, const ConstrainedVariableId succstart);

    /**
     * @see TemporalAdvisor::canBeConcurrent
     */
    bool canBeConcurrent(const ConstrainedVariableId first, const ConstrainedVariableId second);

    /**
     * @see TemporalAdvisor::getTemporalDistanceDomain
     */
    const IntervalIntDomain getTemporalDistanceDomain(const ConstrainedVariableId first,
						      const ConstrainedVariableId second, const bool exact);
    /**
     * @see STNTemporalAdvisor::getTemporalDistanceDomains
     */
    void getTemporalDistanceDomains(const ConstrainedVariableId first,
                                    const std::vector<ConstrainedVariableId>&
                                    seconds,
                                    std::vector<IntervalIntDomain>& domains);

     /**
     * @see STNTemporalAdvisor::getTemporalDistanceSigns
     */
    void getTemporalDistanceSigns(const ConstrainedVariableId first,
                                  const std::vector<ConstrainedVariableId>&
                                  seconds,
                                  std::vector<Time>& lbs,
                                  std::vector<Time>& ubs);

    /**
     * @brief When refpoint is set up, getReferenceTime(var) values
     * will be a minperturb solution, where preferred times of
     * timevars are given as constraints from the refpoint var.(PHM
     * Support for reftime calculations.)
     */

    Time getReferenceTime(const ConstrainedVariableId var);

    /**
     * @brief Designate the refpoint var.  The noId() default removes
     * a previous designation. (PHM Support for reftime calculations.)
     */

    void setRefpointVar(const ConstrainedVariableId
			var = ConstrainedVariableId::noId());

    /**
     * @see TemporalAdvisor::mostRecentReprogation
     */
    unsigned int mostRecentRepropagation() const;

    void getTemporalNogood(const ConstrainedVariableId useAsOrigin,
                           std::vector<ConstrainedVariableId>& fromvars,
                           std::vector<ConstrainedVariableId>& tovars,
                           //std::vector<long>& lengths
			   //std::vector<int>& lengths
                           std::vector<Time>& lengths
                           );

    void getMinPerturbTimes(const std::vector<ConstrainedVariableId>& timevars,
                            const std::vector<Time>& oldreftimes,
                            std::vector<Time>& newreftimes);

    void addListener(const TemporalNetworkListenerId listener);

  protected:
    void handleDiscard();
    void handleConstraintAdded(const ConstraintId constraint);
    void handleConstraintRemoved(const ConstraintId constraint);
    void handleConstraintActivated(const ConstraintId constraint);
    void handleConstraintDeactivated(const ConstraintId constraint);
    void handleVariableDeactivated(const ConstrainedVariableId var);
    void handleVariableActivated(const ConstrainedVariableId var);
    void handleVariableRemoved(const ConstrainedVariableId var);
    void handleNotification(const ConstrainedVariableId variable,
			    unsigned int argIndex,
			    const ConstraintId constraint,
			    const DomainListener::ChangeType& changeType);

  private:

    TemporalConstraintId addSpecificationConstraint(const TemporalConstraintId tc, const TimepointId tp, const Time lb, const Time ub);

    void notifyDeleted(const ConstrainedVariableId tempVar, const TimepointId tp);

    void addTimepoint(const ConstrainedVariableId var);
    void addTemporalConstraint(const ConstraintId constraint);

    bool isEqualToConstraintNetwork();
    bool isConsistentWithConstraintNetwork();

    inline const TimepointId getTimepoint(const ConstrainedVariableId var) {
      std::map<ConstrainedVariableId, TimepointId>::const_iterator it =
          m_varToTimepoint.find(var);
      return (it == m_varToTimepoint.end() ? TimepointId() : it->second);
    }

    void handleTemporalAddition(const ConstraintId constraint);
    void handleTemporalDeletion(const ConstraintId constraint);

    /**
     * @brief update timepoints in the Temporal Network with changes in the Constraint Engine's variables.
     */
    void updateTnet();
    void processConstraintDeletions();
    void processVariableDeletions();
    void processVariableChanges();
    void processConstraintChanges();

    /**
     * @brief update variables in the constraint engine with changes due to Temporal Propagation
     */
    void updateCnet();

    /**
     * @brief Update the time point in the tnet from the given CE variable
     */
    void updateTimepoint(const ConstrainedVariableId var);

    /**
     * @brief update a constraint in the tnet - before, concurrent, startEndDuration
     */
    void updateTemporalConstraint(const ConstraintId constraint);

    /**
     * @brief Shared method to handle update to a constraint in the temporal network.
     * @param var The Temporal Variable driving the change
     * @param tnetConstraint The constraint used to enforce prior (or current) values.
     * @param lb The new lower bound
     * @param ub The new upper bound
     * @return TemporalConstraintId::noId() if the constrant is not deleted. Otherwise it
     * will return the new replacement constraint.
     */
    TemporalConstraintId updateConstraint(const ConstrainedVariableId var,
					  const TemporalConstraintId tnetConstraint,
					  Time lb,
					  Time ub);

    /**
     * @brief Buffer the variable in either the new variable buffer or the change variable buffer
     * depending on its state - i.e. if timepoint already created or not.
     */
    void buffer(const ConstrainedVariableId var);

    /**
     * @brief Test that the buffer status is correct prior to propagation
     */
    bool isValidForPropagation() const;

    /**
     * @brief Update duration bounds
     */
    void updateTnetDuration(const ConstraintId c);  // incoming bounds from cnet
    void updateCnetDuration(const TokenId token) const; // outgoing bounds after tnet propagation

    bool wasRelaxed(const ConstrainedVariableId var);

    void handleViolations();
    void collectViolations(ConstrainedVariableId var);

    void mapVariable(const ConstrainedVariableId var, const TimepointId tp);
    void unmap(const ConstrainedVariableId var);
    void unmap(const TimepointId tp);
    void mapConstraint(const ConstraintId constr, const TemporalConstraintId temp);
    void unmap(const ConstraintId constr);
    void unmap(const TemporalConstraintId temp);
    
    void incrementRefCount(const ConstrainedVariableId var);
    void decrementRefCount(const ConstrainedVariableId var);

    TemporalNetworkId m_tnet; /*!< Temporal Network does all the propagation */

    /*!< Synchronization data structures */
    std::map<eint, ConstrainedVariableId> m_activeVariables; /**< Maintain the set of active start and end variables. 
							     Duration handled in constraints */
    std::map<eint, ConstrainedVariableId> m_changedVariables; /*!< Manage the set of changed variables to be synchronized */

    typedef std::set<ConstraintId, EntityComparator<EntityId> > ConstraintsSet;
    ConstraintsSet m_changedConstraints; /*!< Constraint Agenda */

    typedef  std::set<TemporalConstraintId> TemporalConstraintsSet;
    TemporalConstraintsSet m_constraintsForDeletion; /*!< Buffer deletions till you have to propagate. */

    std::set<TimepointId> m_variablesForDeletion; /*!< Buffer timepoints for deletion till we propagate. */
    std::set<TemporalNetworkListenerId> m_listeners;
    std::map<ConstrainedVariableId, TimepointId> m_varToTimepoint;
    std::map<TimepointId, ConstrainedVariableId> m_timepointToVar;
    std::map<ConstraintId, TemporalConstraintId> m_constrToTempConstr;
    std::map<TemporalConstraintId, ConstraintId> m_tempConstrToConstr;
    std::map<ConstrainedVariableId, unsigned int> m_refCount;
    
    unsigned int m_mostRecentRepropagation;
  };
}
#endif
