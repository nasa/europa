#ifndef _H_ResourceDefs
#define _H_ResourceDefs

#include "Id.hh"
#include "ConstraintEngineDefs.hh"
#include "Domain.hh"

namespace Prototype
{
  // Define constants for default values.
  //@todo Use EUROPA standard defaults for value and time
  const std::string NO_NAME("NO_NAME");
  const int LATEST_TIME = PLUS_INFINITY;
  const double LARGEST_VALUE = PLUS_INFINITY;

  class ResourceViolation;
  typedef Id<ResourceViolation> ResourceViolationId;

  class ResourceFlaw;
  typedef Id<ResourceFlaw> ResourceFlawId;

  class Instant;
  typedef Id<Instant> InstantId;

  class Transaction;
  typedef Id<Transaction> TransactionId;

  class Resource;
  typedef Id<Resource> ResourceId;

  class ResourceConstraint;
  typedef Id<ResourceConstraint> ResourceConstraintId;

  class ResourcePropagator;
  typedef Id<ResourcePropagator> ResourcePropagatorId;

  template<class DomainType> class TokenVariable;
  typedef Id< TokenVariable<IntervalDomain> > ResVarId;

  class ResourceListener;
  typedef Id<ResourceListener> ResourceListenerId;
  
}
#endif
