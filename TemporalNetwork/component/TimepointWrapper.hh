#include "TemporalNetwork.hh"
#include "TemporalNetworkDefs.hh"

namespace Prototype {

  /**
   * @brief Wraps a timepoint to manage deallocation in a staged manner that can be
   * used to synchronize the propagator state.
   */
  class TimepointWrapper : public Entity {
  public:
    TimepointWrapper(const TemporalPropagatorId& prop, const TempVarId& tempVar, const TimepointId& point);
    virtual ~TimepointWrapper();

    const EntityId& getId() { return m_id; }
    const TimepointId& getTimepoint() const {return m_timepoint;}
    const TempVarId& getTempVar() const {return m_tempVar;}
  private:
    TemporalPropagatorId m_propagator;
    TempVarId m_tempVar;
    TimepointId m_timepoint;
    EntityId m_id;
  };
}
