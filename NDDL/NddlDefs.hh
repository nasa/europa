#ifndef NDDL_DEFS_HH
#define NDDL_DEFS_HH

#include "RulesEngineDefs.hh"
#include "ResourceDefs.hh"

using namespace Prototype;

namespace NDDL {

  class NddlToken;
  typedef Id<NddlToken> NddlTokenId;

  class NddlSchema;
  typedef Id<NddlSchema> NddlSchemaId;

  class NddlResource;
  typedef Id<NddlResource> NddlResourceId;

  class NddlResourceTransaction;
  typedef Id<NddlResourceTransaction> NddlResourceTransactionId;

  class ObjectFilterConstraint;
  typedef Id<ObjectFilterConstraint> ObjectFilterConstraintId;

  class ObjectFilterCondition;
  typedef Id<ObjectFilterCondition> ObjectFilterConditionId;

} // namespace NDDL

#endif // NDDL_DEFS_HH
