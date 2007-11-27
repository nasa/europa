#include "ModuleSolvers.hh"
#include "SolverConfig.hh"

namespace EUROPA {

  static bool & SolversInitialized() {
    static bool sl_alreadyDone(false);
    return sl_alreadyDone;
  }

  ModuleSolvers::ModuleSolvers()
      : Module("Solvers")
  {
	  
  }

  ModuleSolvers::~ModuleSolvers()
  {	  
  }  
  
  void ModuleSolvers::initialize()
  {
      if(SolversInitialized())
    	  return;

      SOLVERS::SolverConfig::init();
      
      SolversInitialized() = true;
  }  

  void ModuleSolvers::uninitialize()
  {
      if(!SolversInitialized())
    	  return;
      
	  SOLVERS::SolverConfig::uninit();	  
	  SolversInitialized() = false;
  }  
  
  void ModuleSolvers::initialize(EngineId engine)
  {
  }
  
  void ModuleSolvers::uninitialize(EngineId engine)
  {	 
  }  
}
