#include "TemporalNetworkListener.hh"
#include "TemporalPropagator.hh"
namespace EUROPA{

  TemporalNetworkListener::TemporalNetworkListener(const TemporalPropagatorId prop) : m_propagator(prop), m_id(this) {
    m_propagator->addListener(m_id);
  }

  TemporalNetworkListener::~TemporalNetworkListener() {
    std::cout << "TemporalNetworkListener destructor " << std::endl;
    std::cout << "Done TemporalNetworkListener destructor " << std::endl;
  }

void TemporalNetworkListener::notifyTimepointAdded(const ConstrainedVariableId,
                                                   const TimepointId) {}

void TemporalNetworkListener::notifyTimepointDeleted(const TimepointId) {}

void TemporalNetworkListener::notifyBaseDomainConstraintAdded(const ConstrainedVariableId,
                                                              const TemporalConstraintId,
                                                              Time, Time) {}
void TemporalNetworkListener::notifyConstraintAdded(const ConstraintId,
                                                    const TemporalConstraintId,
                                                    Time , Time) {}

void TemporalNetworkListener::notifyConstraintDeleted(eint,
                                                      const TemporalConstraintId) {}

void TemporalNetworkListener::notifyBoundsRestricted(const ConstrainedVariableId,
                                                     Time, Time) {}

void TemporalNetworkListener::notifyBoundsSame(const ConstrainedVariableId,
                                               const TimepointId) {}


}

