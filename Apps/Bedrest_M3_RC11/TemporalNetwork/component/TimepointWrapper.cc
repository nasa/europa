#include "TimepointWrapper.hh"
#include "TemporalPropagator.hh"

namespace EUROPA {

  TimepointWrapper::TimepointWrapper(const TemporalPropagatorId& prop, const ConstrainedVariableId& tempVar, const TimepointId& point) 
    :m_propagator(prop), m_tempVar(tempVar), m_timepoint(point), m_id(this) {}

  TimepointWrapper::~TimepointWrapper() {
    discard(false);
    m_id.remove();
  }

  void TimepointWrapper::handleDiscard(){
    if(!Entity::isPurging())
      m_propagator->notifyDeleted(m_tempVar, m_timepoint);

    Entity::handleDiscard();
  }
}
