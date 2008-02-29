
#ifndef _H_ModuleRulesEngine
#define _H_ModuleRulesEngine

#include "Module.hh"

namespace EUROPA {
  class ModuleRulesEngine : public Module
  {
    public:
      ModuleRulesEngine();
      virtual ~ModuleRulesEngine();

      /**
       * @brief Initialize all default elements of the module 
       */
	  virtual void initialize();
	  /**
	   * @brief Uninitialize all default elements of the module 
	   */
	  virtual void uninitialize();   

	  virtual void initialize(EngineId engine);   // initialization of a particular engine instance

	  virtual void uninitialize(EngineId engine); // cleanup of a particular engine instance	  
  };

  typedef Id<ModuleRulesEngine> ModuleRulesEngineId;  
}  


#endif /* #ifndef _H_ModuleRulesEngine */
