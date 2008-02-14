#include "SingleSolverController.hh"

namespace EUROPA {
  
  class ControllerFactory : public MasterControllerFactory
  {
  public:
      ControllerFactory() { MasterController::s_factory = this; }
      MasterController* createInstance() { return new SOLVERS::SingleSolverController(); }      
  };
  
  ControllerFactory s_factory;
}
