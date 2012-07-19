#include "AggregateListener.hh"
#include "EventAggregator.hh"

namespace EUROPA {
  AggregateListener::AggregateListener() : m_step(0), m_id(this) {
    check_error(EventAggregator::instance().isValid());
    check_error(m_id.isValid());
    EventAggregator::instance()->addListener(m_id);
  }

  AggregateListener::~AggregateListener() {
    check_error(EventAggregator::instance().isValid());
    check_error(m_id.isValid());
    EventAggregator::instance()->removeListener(m_id);
    m_id.remove();
  }
}
