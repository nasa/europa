#include "TemporalNetworkListener.hh"
#include "TemporalPropagator.hh"

namespace PLASMA{

  TemporalNetworkListener::TemporalNetworkListener(const TemporalPropagatorId& prop) : m_propagator(prop), m_id(this) {
    m_propagator->addListener(m_id);
  }

  TemporalNetworkListener::~TemporalNetworkListener() {
    std::cout << "TemporalNetworkListener destructor " << std::endl;
    if (!m_id.isNoId())
      m_id.remove();
    std::cout << "Done TemporalNetworkListener destructor " << std::endl;
  }

}

