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

namespace EUROPA {

  /**
   * @brief Initialize all default elements of the module e.g. factories
   */
  void initNDDL();

  /**
   * @brief Uninitialize all default elements of the module e.g. factories
   */
  void uninitNDDL();

}

#endif // NDDL_DEFS_HH
