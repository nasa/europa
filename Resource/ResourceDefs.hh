#ifndef _H_ResourceDefs
#define _H_ResourceDefs

#include "Id.hh"
#include "../ConstraintEngine/ConstraintEngineDefs.hh"
#include "../ConstraintEngine/Domain.hh"

namespace Prototype
{
  // Define constants for default values.
  //@todo Use EUROPA standard defaults for value and time
  const std::string NO_NAME("NO_NAME");
  const int LATEST_TIME = PLUS_INFINITY;
  const double LARGEST_VALUE = PLUS_INFINITY;

  class Violation;
  typedef Id<Violation> ViolationId;

  class Instant;
  typedef Id<Instant> InstantId;

  class Transaction;
  typedef Id<Transaction> TransactionId;

  class Resource;
  typedef Id<Resource> ResourceId;

  class ResourceConstraint;
  typedef Id<ResourceConstraint> ResourceConstraintId;

  template<class DomainType> class TokenVariable;
  typedef Id< TokenVariable<IntervalDomain> > ResVarId;
  
}
#endif
