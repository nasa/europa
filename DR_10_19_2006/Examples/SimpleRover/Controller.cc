#include "SingleSolverController.hh"

namespace EUROPA {

  MasterController* MasterController::createInstance(){
    return new SOLVERS::SingleSolverController();
  }

}
