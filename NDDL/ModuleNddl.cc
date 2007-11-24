#include "ModuleNddl.hh"
#include "BoolTypeFactory.hh"
#include "StringTypeFactory.hh"
#include "SymbolTypeFactory.hh"
#include "intType.hh"
#include "floatType.hh"

namespace EUROPA {

  static bool & nddlInitialized() {
    static bool sl_alreadyDone(false);
    return sl_alreadyDone;
  }

  ModuleNddl::ModuleNddl()
      : Module("NDDL")
  {
	  
  }

  ModuleNddl::~ModuleNddl()
  {	  
  }  
  
  void initNDDL()
  {
    if(!nddlInitialized()){
      nddlInitialized() = true;

      /* Allocate NDDL Type Factories */
      // Default names are uppercase, TODO: change default names so that this isn't necessary
      new BoolTypeFactory("bool");
      new StringTypeFactory("string");
      new SymbolTypeFactory("symbol");
      
      // These are Nddl specific, so they belong here
      new intTypeFactory();
      new floatTypeFactory();
    }
  }

  void uninitNDDL()
  {
    if(nddlInitialized()){
      TypeFactory::purgeAll();
      nddlInitialized() = false;
    }
  }
  
  // TODO: remove initNDDL() and uninitNDDL()
  void ModuleNddl::initialize()
  {
	  initNDDL();
  }  

  void ModuleNddl::uninitialize()
  {
	  uninitNDDL();
  }
  
  void ModuleNddl::initialize(EngineId engine)
  {
  }
  
  void ModuleNddl::uninitialize(EngineId engine)
  {	  
  }  
}
