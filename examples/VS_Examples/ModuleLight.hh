#ifndef _H_ModuleLight
#define _H_ModuleLight

#include "Module.hh"

namespace EUROPA {
  class ModuleLight : public Module
  {
    public:
      ModuleLight();
      virtual ~ModuleLight();

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

  typedef Id<ModuleLight> ModuleLightId;  
}  


#endif /* #ifndef _H_ModuleLight */
