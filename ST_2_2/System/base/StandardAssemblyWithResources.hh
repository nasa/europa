#ifndef _H_StandardAssemblyWithResources
#define _H_StandardAssemblyWithResources

#include "StandardAssembly.hh"

namespace EUROPA {

  class StandardAssemblyWithResources : public virtual StandardAssembly {
  public:
    StandardAssemblyWithResources(const SchemaId& schema);
  };
}

#endif
