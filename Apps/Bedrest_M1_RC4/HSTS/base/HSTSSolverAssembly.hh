#ifndef _H_HSTSSolverAssembly
#define _H_HSTSSolverAssembly

#include "Schema.hh"
#include "Solver.hh"
#include "ConstraintEngine.hh"
#include "DefaultPropagator.hh"
#include "TemporalPropagator.hh"
#include "PlanDatabase.hh"
#include "RulesEngine.hh"

namespace EUROPA {
  namespace HSTS {
    class SolverAssembly {
    public:
      static void initialize();
      SolverAssembly(SchemaId schema);
      bool plan(const char* txSource, const char* sconfigSource);
    private:
      bool playTransactions(const char* txSource);

      ConstraintEngineId m_constraintEngine;
      PlanDatabaseId m_planDatabase;
      RulesEngineId m_rulesEngine;
    };
  }
}

#endif
