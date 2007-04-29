#ifndef _H_SolverAssemblyWithResources
#define _H_SolverAssemblyWithResources

#include "SolverAssembly.hh"
#include "StandardAssemblyWithResources.hh"

namespace EUROPA {

  class SolverAssemblyWithResources : public StandardAssemblyWithResources,
				      public SolverAssembly {
  public:
    SolverAssemblyWithResources(const SchemaId& schema);
    static void initialize() {SolverAssembly::initialize();}
    static const char* TX_LOG() {return SolverAssembly::TX_LOG();}
  private:
    static bool s_initialized;
  };

}

#endif
