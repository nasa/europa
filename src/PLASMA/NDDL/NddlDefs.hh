#ifndef NDDL_DEFS_HH
#define NDDL_DEFS_HH

#include "RulesEngineDefs.hh"

using namespace EUROPA;

namespace NDDL {

  class NddlToken;
  typedef Id<NddlToken> NddlTokenId;

  class ObjectFilterConstraint;
  typedef Id<ObjectFilterConstraint> ObjectFilterConstraintId;

  class ObjectFilterCondition;
  typedef Id<ObjectFilterCondition> ObjectFilterConditionId;

  #define inf PLUS_INFINITY

} // namespace NDDL

#endif // NDDL_DEFS_HH
