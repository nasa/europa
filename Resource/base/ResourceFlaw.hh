#ifndef _H_ResourceFlaw
#define _H_ResourceFlaw

#include "ResourceProblem.hh"

namespace EUROPA {

  /**
   * @class ResourceFlaw
   * @brief Points where something needs to be done to produce stricter resource behavior.
   */
  class ResourceFlaw : public ResourceProblem {
  protected:
    friend class Instant;
    ResourceFlaw(Type type, const InstantId& instant) : ResourceProblem(type, instant) {}
    ResourceFlaw();
  };
}

#endif
