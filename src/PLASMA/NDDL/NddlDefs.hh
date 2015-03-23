#ifndef NDDL_DEFS_HH
#define NDDL_DEFS_HH

#include "RulesEngineDefs.hh"

namespace EUROPA {
class ObjectFilterConstraint;
typedef Id<ObjectFilterConstraint> ObjectFilterConstraintId;

class ObjectFilterCondition;
typedef Id<ObjectFilterCondition> ObjectFilterConditionId;
}

namespace NDDL {

class NddlToken;
typedef EUROPA::Id<NddlToken> NddlTokenId;


} // namespace NDDL

#endif // NDDL_DEFS_HH
