#ifndef H_SingleSolverController
#define H_SingleSolverController

#include "MasterController.hh"

namespace EUROPA {
  namespace SOLVERS {

    class SingleSolverController: public MasterController {
    public:
      SingleSolverController();
      ~SingleSolverController();

    private:
      /**
       * @brief Called after model is loaded
       */
      void handleRegistration();

      /**
       * @brief Called after the initial transactions are loaded into the database.
       */
      void configureSolvers(const char* configPath);

      /**
       * @brief Advance 1 step in the solution process
       */
      Status handleNext();

      SolverId m_solver;


      unsigned int m_maxDepth;
      unsigned int m_maxSteps;
    };
  }
}

#endif
