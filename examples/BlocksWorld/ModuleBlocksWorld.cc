#include "ModuleBlocksWorld.hh"
#include "BlocksWorldCustomCode.hh"

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
          return (new ModuleBlocksWorld())->getId();
	}
}

  static bool & BlocksWorldInitialized() {
    static bool sl_alreadyDone(false);
    return sl_alreadyDone;
  }

  ModuleBlocksWorld::ModuleBlocksWorld()
      : Module("BlocksWorld")
  {
  }

  ModuleBlocksWorld::~ModuleBlocksWorld()
  {	  
  }  
  
  void ModuleBlocksWorld::initialize()
  {
      if(BlocksWorldInitialized())
    	  return;
	  BlocksWorldInitialized() = true;
  }  

  void ModuleBlocksWorld::uninitialize()
  {
	  BlocksWorldInitialized() = false;
  }  
  
  void ModuleBlocksWorld::initialize(EngineId engine)
  {
  }
  
  void ModuleBlocksWorld::uninitialize(EngineId engine)
  {	  
  }  
}
