#include "TimepointWrapper.hh"
#include "TemporalPropagator.hh"

namespace Prototype {

TimepointWrapper::TimepointWrapper(const TemporalPropagatorId& prop, const TimepointId& point) :m_propagator(prop), m_timepoint(point), m_id(this) {
  //std::cout << "timepoint wrapper new: timepoint key" << m_timepoint->getKey() << std::endl;
}

TimepointWrapper::~TimepointWrapper() { 
  //std::cout << "timepoint wrapper delete: timepoint key" << m_timepoint->getKey() << std::endl;
  m_propagator->notifyDeleted(m_timepoint); 
  m_id.remove(); 
}

}
