#include "ModuleRover.hh"
#include "RoverCustomCode.hh"

// Pieces necessary for various customizations:
#include "ConstraintFactory.hh"
#include "PSPlanDatabase.hh"
#include "TransactionInterpreter.hh"
#include "Schema.hh"
#include "FlawHandler.hh"


namespace EUROPA {

// static C init method to get handle when loading module as shared library
extern "C" 
{
	ModuleId initializeModule()
	{
          return (new ModuleRover())->getId();
	}
}

  static bool & RoverInitialized() {
    static bool sl_alreadyDone(false);
    return sl_alreadyDone;
  }

  ModuleRover::ModuleRover()
      : Module("Rover")
  {
  }

  ModuleRover::~ModuleRover()
  {	  
  }  
  
  void ModuleRover::initialize()
  {
      if(RoverInitialized())
    	  return;
	  RoverInitialized() = true;
  }  

  void ModuleRover::uninitialize()
  {
	  RoverInitialized() = false;
  }  
  
  void ModuleRover::initialize(EngineId engine)
  {
  }
  
  void ModuleRover::uninitialize(EngineId engine)
  {	  
  }  
}
