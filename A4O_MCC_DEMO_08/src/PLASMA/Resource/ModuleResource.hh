
#ifndef _H_ModuleResource
#define _H_ModuleResource

#include "Module.hh"

namespace EUROPA {
  class ModuleResource : public Module
  {
    public:
      ModuleResource();
      virtual ~ModuleResource();

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

  typedef Id<ModuleResource> ModuleResourceId;  
}  


#endif /* #ifndef _H_ModuleResource */
