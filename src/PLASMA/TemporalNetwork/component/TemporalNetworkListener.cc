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
                                                   Timepoint* const) {}

void TemporalNetworkListener::notifyTimepointDeleted(Timepoint* const) {}

void TemporalNetworkListener::notifyBaseDomainConstraintAdded(const ConstrainedVariableId,
                                                              TemporalConstraint* const,
                                                              Time, Time) {}
void TemporalNetworkListener::notifyConstraintAdded(const ConstraintId,
                                                    TemporalConstraint* const,
                                                    Time , Time) {}

void TemporalNetworkListener::notifyConstraintDeleted(eint,
                                                      TemporalConstraint* const) {}

void TemporalNetworkListener::notifyBoundsRestricted(const ConstrainedVariableId,
                                                     Time, Time) {}

void TemporalNetworkListener::notifyBoundsSame(const ConstrainedVariableId,
                                               Timepoint* const) {}


}

