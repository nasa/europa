#include "TemporalNetwork.hh"
#include "TemporalNetworkDefs.hh"

namespace Prototype {

class TimepointWrapper : public Entity {
public:
  TimepointWrapper(const TemporalPropagatorId& prop, const TimepointId& point);
  virtual ~TimepointWrapper();

  const EntityId& getId() { return m_id; }
private:
  TemporalPropagatorId m_propagator;
  TimepointId m_timepoint;
  EntityId m_id;
};

}
