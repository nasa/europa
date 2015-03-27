#ifndef H_TemporalNetworkListener
#define H_TemporalNetworkListener

#include "TemporalNetworkDefs.hh"
#include "TemporalNetwork.hh"
#include "ConstrainedVariable.hh"

namespace EUROPA{

  /**
   * @class TemporalNetworkListener
   * @brief The abstract interface for events published from the temporal network
   * @ingroup TemporalNetwork
   */
  class TemporalNetworkListener{
  public:

    /**
     * @brief destructor
     */
    virtual ~TemporalNetworkListener();

    /**
     * @brief Inform listeners that timpoint has been added
     * @param var The temporal variable from the constraint network
     * @param timepoint new timepoint 
     */
    virtual void notifyTimepointAdded(const ConstrainedVariableId var,
                                      const TimepointId timepoint);

    /**
     * @brief Inform listeners that timpoint has been deleted
     * @param timepoint deleted
     */
    virtual void notifyTimepointDeleted(const TimepointId timepoint);

    /**
     * @brief Inform listeners that a constraint has been added to the base domain
     * @param c Constrained variable
     * @param constraint New constraint
     * @param lb lower bound
     * @param ub upper bound 
     */
    virtual void notifyBaseDomainConstraintAdded(const ConstrainedVariableId c,
                                                 const TemporalConstraintId constraint,
                                                 Time lb, Time ub);

    /**
     * @brief Inform listeners a constraint has been added.
     * @param c Constrained variable
     * @param constraint New constraint
     * @param lb lower bound
     * @param ub upper bound 
     */
    virtual void notifyConstraintAdded(const ConstraintId c,
                                       const TemporalConstraintId constraint,
                                       Time lb, Time ub);

    /**
     * @brief Inform listeners that a constraint has been deleted
     * @param key The key of the deleted constraint
     * @param constraint constraint deleted 
     */
    virtual void notifyConstraintDeleted(eint key, const TemporalConstraintId constraint);

    /**
     * @brief Inform listeners that bounds have been restricted
     * @param v constrained variable restricted
     * @param newlb new lower bound
     * @param newub new upper bound
     */
    virtual void notifyBoundsRestricted(const ConstrainedVariableId v, Time newlb, Time newub);

    /**
     * @brief Inform listeners that bounds are unchanged
     * @param v constrained variable where bounds remain the same
     * @param timepoint The corresponding timepoint
     */
    virtual void notifyBoundsSame(const ConstrainedVariableId v,  const TimepointId timepoint);

    /**
     * @brief Get unique id of listener.
     * @return unique id of listener.
     */
    const TemporalNetworkListenerId getId() const;

  protected:
    /**
     * @brief Constructor
     * @param prop identify of temporal propagator to which this listener is attached.
     */
    TemporalNetworkListener(const TemporalPropagatorId prop);

    TemporalPropagatorId m_propagator;
    TemporalNetworkListenerId m_id;
  };
}
#endif
