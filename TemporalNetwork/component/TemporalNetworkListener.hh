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
    virtual ~TemporalNetworkListener();
    inline virtual void notifyTimepointAdded(const ConstrainedVariableId& var, const TimepointId& timepoint) {}
    inline virtual void notifyTimepointDeleted(const TimepointId& timepoint) {}
    inline virtual void notifyBaseDomainConstraintAdded(const ConstrainedVariableId& c, const TemporalConstraintId& constraint, Time lb, Time ub) {} ;
    inline virtual void notifyConstraintAdded(const ConstraintId c, const TemporalConstraintId& constraint, Time lb, Time ub) {} ;
    inline virtual void notifyConstraintDeleted(int key, const TemporalConstraintId& constraint) {} ;
    inline virtual void notifyBoundsRestricted(const ConstrainedVariableId& v, Time newlb, Time newub) {} ;
    inline virtual void notifyBoundsSame(const ConstrainedVariableId& v,  const TimepointId& timepoint) {}

    const TemporalNetworkListenerId& getId() const;

  protected:
    TemporalPropagatorId m_propagator;
    TemporalNetworkListener(const TemporalPropagatorId& prop);
    TemporalNetworkListenerId m_id;
  };
}
#endif
