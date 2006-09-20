#include "PlanDatabaseDefs.hh"
#include "ConstraintEngineDefs.hh"
#include "RulesEngineDefs.hh"
#include "SolverDefs.hh"

using namespace EUROPA::SOLVERS;

namespace EUROPA {
  namespace DSA {

    class DSA {
    public:
      static DSA& instance(){
	static DSA sl_instance;
	return sl_instance;
      }

      /** Calls rapped by JNI **/
      void load(const char* model);
      void loadTransactions(const char* txSource);
      void queryGetComponents();
      void solverConfigure(const char* source, int horizonStart, int horizonEnd);
      void solverSolve(int maxSteps, int maxDepth);
      void solverStep();
      void solverReset();
      void solverClear();

    private:
      DSA();
      void init();
      void unload();
      void loadModelLibrary(const char* model);
      void writeSolverState();

      ConstraintEngineId m_ce; /*!< A Constraint Engine for propagation of relations */
      PlanDatabaseId m_db; /*!< A PlanDatabase as central state representation */
      RulesEngineId m_re; /*!< A Rules Engine to enforce model rules. */
      SolverId m_solver; /*!< At most, one solver allowed */
      void* m_libHandle;
    };

  }
}
