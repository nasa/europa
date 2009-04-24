#include "ModuleShopping.hh"
#include "ShoppingCustomCode.hh"

// Pieces necessary for various customizations:
#include "PSPlanDatabase.hh"


namespace EUROPA {

// static C init method to get handle when loading module as shared library
extern "C"
{
	ModuleId initializeModule()
	{
          return (new ModuleShopping())->getId();
	}
}

  static bool & ShoppingInitialized() {
    static bool sl_alreadyDone(false);
    return sl_alreadyDone;
  }

  ModuleShopping::ModuleShopping()
      : Module("Shopping")
  {
  }

  ModuleShopping::~ModuleShopping()
  {
  }

  void ModuleShopping::initialize()
  {
      if(ShoppingInitialized())
    	  return;
	  ShoppingInitialized() = true;
  }

  void ModuleShopping::uninitialize()
  {
	  ShoppingInitialized() = false;
  }

  void ModuleShopping::initialize(EngineId engine)
  {
  }

  void ModuleShopping::uninitialize(EngineId engine)
  {
  }
}
