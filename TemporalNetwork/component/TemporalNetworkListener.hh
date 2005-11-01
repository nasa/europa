#ifndef _H_TemporalNetworkListener
#define _H_TemporalNetworkListener

#include "TemporalNetworkDefs.hh"
#include "TemporalNetwork.hh"
#include "ConstrainedVariable.hh"

namespace EUROPA{

  /**
   * @class TemporalNetworkListener
   * @brief The abstract interface for events published from the temporal network
   */
  class TemporalNetworkListener{
  public:

    /**
     * @brief destructor
     */
    virtual ~TemporalNetworkListener();

    /**
     * @brief Inform listeners that timpoint has been added
     * @param var
     * @param timepoint new timepoint 
     */
    inline virtual void notifyTimepointAdded(const ConstrainedVariableId& var, const TimepointId& timepoint) {}

    /**
     * @brief Inform listeners that timpoint has been deleted
     * @param timepoint deleted
     */
    inline virtual void notifyTimepointDeleted(const TimepointId& timepoint) {}

    /**
     * @brief Inform listeners that a constraint has been added to the base domain
     * @param c Constrained variable
     * @param constraint New constraint
     * @param lb lower bound
     * @param ub upper bound 
     */
    inline virtual void notifyBaseDomainConstraintAdded(const ConstrainedVariableId& c, const TemporalConstraintId& constraint, Time lb, Time ub) {} ;

    /**
     * @brief Inform listeners a constraint has been added.
     * @param c Constrained variable
     * @param constraint New constraint
     * @param lb lower bound
     * @param ub upper bound 
     */
    inline virtual void notifyConstraintAdded(const ConstraintId c, const TemporalConstraintId& constraint, Time lb, Time ub) {} ;

    /**
     * @brief Inform listeners that a constraint has been deleted
     * @param key
     * @param constraint constraint deleted 
     */
    inline virtual void notifyConstraintDeleted(int key, const TemporalConstraintId& constraint) {} ;

    /**
     * @brief Inform listeners that bounds have been restricted
     * @param v constrained variable restricted
     * @param newlb new lower bound
     * @param newup new upper bound
     */
    inline virtual void notifyBoundsRestricted(const ConstrainedVariableId& v, Time newlb, Time newub) {} ;

    /**
     * @brief Inform listeners that bounds are unchanged
     * @param v constrained variable where bounds remain the same
     * @param timepoint
     */
    inline virtual void notifyBoundsSame(const ConstrainedVariableId& v,  const TimepointId& timepoint) {}

    /**
     * @brief Get unique id of listener.
     * @return unique id of listener.
     */
    const TemporalNetworkListenerId& getId() const;

  protected:
    /**
     * @brief Constructor
     * @param prop identify of temporal propagator to which this listener is attached.
     */
    TemporalNetworkListener(const TemporalPropagatorId& prop);

    TemporalPropagatorId m_propagator;
    TemporalNetworkListenerId m_id;
  };
}
#endif
