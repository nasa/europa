#ifndef _H_ResourceViolation
#define _H_ResourceViolation

#include "ResourceDefs.hh"
#include "ResourceProblem.hh"

namespace EUROPA {

  /**
   * @class Violation
   * @brief Characterize violations that might be generated for a resource.
   *    
   * Violations are generated when the resource is Violated. A Violation means that no completion of the current
   * partial plan can lead to a consistent solution. Backtracking is essential.
   */
  class ResourceViolation : public ResourceProblem {
  protected:
    friend class Instant;
    ResourceViolation(ResourceProblem::Type type, const InstantId& instant) : ResourceProblem(type, instant) {}
    ResourceViolation();
  protected:
    friend class Resource;
    ResourceViolation(ResourceProblem::Type type) : ResourceProblem(type) {} // global violation on Resource
  };

}

#endif
