#ifndef _H_EqualityConstraintPropagator
#define _H_EqualityConstraintPropagator

/**
 * @file EqualityCOnstraintPropagator.hh
 * @author Conor McGann
 * @date August, 2003
 */

#include "Propagator.hh"
#include "EquivalenceClassCollection.hh"
#include <map>
#include <set>

namespace Prototype{

  /**
   * @class EqualityConstraintPropagator
   * @brief Responsible for propagation management of all EqualConstraints when registered.
   *
   * Achieves this by organizing variables constrained by an EqualConstraint into different
   * Equivalence Classes. This is derived from the approach in Europa today, though the implementation 
   * details are a little different.
   * @par Key Points
   * @li Addition of a Constraint causes an incremental update to the equivalence class collection
   * @li Removal of a Constraint will cause a complete recomputation of equivalence classes.
   * @li The 'agenda' is based on e change to an equivalence class.
   * @li When a constraint is removed, no agenda is required since we recompute all.
   * @see EquivalenceClassCollection
   */
  class EqualityConstraintPropagator: public Propagator {
  public:
    EqualityConstraintPropagator(const LabelStr& name, const ConstraintEngineId& constraintEngine);
    ~EqualityConstraintPropagator();
    void execute();
    bool updateRequired() const;

  private:
    void handleConstraintAdded(const ConstraintId& constraint);
    void handleConstraintRemoved(const ConstraintId& constraint);
    void handleConstraintActivated(const ConstraintId& constrain);
    void handleConstraintDeactivated(const ConstraintId& constraint);
    void handleNotification(const ConstrainedVariableId& variable, 
			    int argIndex, 
			    const ConstraintId& constraint, 
			    const DomainListener::ChangeType& changeType);
    bool isAcceptable(const ConstraintId& constraint) const;

    /**
     * Does the real work of propagating an equivalence class
     * @param scope the scope of variables to be equated
     */
    void equate(const std::set<ConstrainedVariableId>& scope);

    bool m_fullReprop; /*!< True if a constraint has been removed. Otherwise false. */
    bool m_active; /*!< True if we are in the execute method. Otherwise false. Used to prevent additions to the agenda
		     while we are actively propagating */

    EquivalenceClassCollection m_eqClassCollection; /*!< Does all the real work of managing Equivalence Classes */
    std::set<int> m_eqClassAgenda; /*!< Inbound notifications are translated into the affected equivalence class and stored here for
				     processing during execution */
  };

}
#endif
