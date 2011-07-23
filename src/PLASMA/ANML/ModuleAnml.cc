#include "ModuleAnml.hh"
#include "AnmlInterpreter.hh"

namespace EUROPA {

  static bool & anmlInitialized() {
    static bool sl_alreadyDone(false);
    return sl_alreadyDone;
  }

  ModuleAnml::ModuleAnml()
      : Module("ANML")
  {	  
  }

  ModuleAnml::~ModuleAnml()
  {	  
  }  
    
  void ModuleAnml::initialize()
  {
	  if (anmlInitialized())
		  return;
	  
	  anmlInitialized() = true;
  }  

  void ModuleAnml::uninitialize()
  {
	  if (!anmlInitialized()) 		  
	      return;
  }
  
  void ModuleAnml::initialize(EngineId engine)
  {
	  engine->addLanguageInterpreter("anml", new AnmlInterpreter(engine));
  }
  
  void ModuleAnml::uninitialize(EngineId engine)
  {
	  LanguageInterpreter *old = engine->removeLanguageInterpreter("anml");
	  if (old)
		  delete old;
  }
}
