#include "Module%%Project%%.hh"
#include "%%Project%%CustomCode.hh"

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
		return new Module%%Project%%();
	}
}

  static bool & %%Project%%Initialized() {
    static bool sl_alreadyDone(false);
    return sl_alreadyDone;
  }

  Module%%Project%%::Module%%Project%%()
      : Module("%%Project%%")
  {
  }

  Module%%Project%%::~Module%%Project%%()
  {	  
  }  
  
  void Module%%Project%%::initialize()
  {
      if(%%Project%%Initialized())
    	  return;
      
      // Example of how to incorporate the ExampleConstraint declared in %%Project%%CustomCode.hh
      // REGISTER_CONSTRAINT(ExampleConstraint, "example", "Default");
	  %%Project%%Initialized() = true;
  }  

  void Module%%Project%%::uninitialize()
  {
	  %%Project%%Initialized() = false;
  }  
  
  void Module%%Project%%::initialize(EngineId engine)
  {
  }
  
  void Module%%Project%%::uninitialize(EngineId engine)
  {	  
  }  
}
