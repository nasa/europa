#ifndef _H_EventDefs
#define _H_EventDefs

#include "Id.hh"

namespace Prototype {
  class EventAggregator;
  typedef Id<EventAggregator> EventAggregatorId;
  
  class AggregateListener;
  typedef Id<AggregateListener> AggregateListenerId;
}
#endif
