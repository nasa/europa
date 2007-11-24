#include "ModuleSolvers.hh"

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
      
      SolversInitialized() = true;
  }  

  void ModuleSolvers::uninitialize()
  {
	  SolversInitialized() = false;
  }  
  
  void ModuleSolvers::initialize(EngineId engine)
  {
  }
  
  void ModuleSolvers::uninitialize(EngineId engine)
  {	 
  }  
}
