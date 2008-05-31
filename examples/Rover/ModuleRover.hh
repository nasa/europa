#ifndef _H_ModuleRover
#define _H_ModuleRover

#include "Module.hh"

namespace EUROPA {
  class ModuleRover : public Module
  {
    public:
      ModuleRover();
      virtual ~ModuleRover();

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

  typedef Id<ModuleRover> ModuleRoverId;  
}  


#endif /* #ifndef _H_ModuleRover */
