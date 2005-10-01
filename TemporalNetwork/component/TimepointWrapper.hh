#ifndef _H_TimePointWrapper
#define _H_TimePointWrapper

#include "TemporalNetwork.hh"
#include "TemporalNetworkDefs.hh"
#include "ConstrainedVariable.hh"

namespace EUROPA {

  /**
   * @brief Wraps a timepoint to manage deallocation in a staged manner that can be
   * used to synchronize the propagator state.
   */
  class TimepointWrapper : public Entity {
  public:
    TimepointWrapper(const TemporalPropagatorId& prop, const ConstrainedVariableId& tempVar, const TimepointId& point);
    virtual ~TimepointWrapper();
    inline const EntityId& getId() { return m_id; }
    inline const TimepointId& getTimepoint() const {return m_timepoint;}
    inline const ConstrainedVariableId& getTempVar() const {return m_tempVar;}
  private:
    void handleDiscard();
    TemporalPropagatorId m_propagator;
    ConstrainedVariableId m_tempVar;
    TimepointId m_timepoint;
    EntityId m_id;
  };
}

#endif
