#include "ModuleUBO.hh"
#include "UBOCustomCode.hh"

// Pieces necessary for various customizations:
#include "ConstraintLibrary.hh"
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
          return (new ModuleUBO())->getId();
	}
}

  static bool & UBOInitialized() {
    static bool sl_alreadyDone(false);
    return sl_alreadyDone;
  }

  ModuleUBO::ModuleUBO()
      : Module("UBO")
  {
  }

  ModuleUBO::~ModuleUBO()
  {	  
  }  
  
  void ModuleUBO::initialize()
  {
      if(UBOInitialized())
    	  return;
	  UBOInitialized() = true;
  }  

  void ModuleUBO::uninitialize()
  {
	  UBOInitialized() = false;
  }  
  
  void ModuleUBO::initialize(EngineId engine)
  {
  }
  
  void ModuleUBO::uninitialize(EngineId engine)
  {	  
  }  
}
