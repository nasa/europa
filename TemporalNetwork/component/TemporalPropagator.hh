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
    bool canPrecede(const TempVarId& first, const TempVarId& second);
    bool canFitBetween(const TempVarId& start, const TempVarId& end,
		       const TempVarId& predend, const TempVarId& succstart);
    void notifyDeleted(const TimepointId& tp);
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

    void checkAndAddTnetVariables(const ConstraintId& constraint);
    void addTnetConstraint(const ConstraintId& constraint);

    void handleTemporalAddition(const ConstraintId& constraint);
    void handleTemporalDeletion(const ConstraintId& constraint);

    void updateTnet();
    bool updateTempVar();

    TimepointId getTimepoint(const TempVarId& var);

    std::set<ConstraintId> m_agenda;
    int m_activeConstraint;

    bool m_updateRequired;
    bool m_fullRepropRequired;
    TemporalNetworkId m_tnet;
    std::map<int, TemporalConstraintId> m_tnetConstraints; // int denotes constraint key
    std::map<int, TemporalConstraintId> m_tnetVariableConstraints;
    std::map<int, TimepointId> m_tnetVariables; // int denotes tempvar key
    std::set<int> m_constraintsForDeletion; //!< Buffer deletions till you have to propagate.
    std::set<int> m_variablesForDeletion;  //!< Buffer deletions till you have to propagate.
    std::set<TemporalNetworkListenerId> m_listeners;

  };
}
#endif
