#ifndef _H_ModuleBlocksWorld
#define _H_ModuleBlocksWorld

#include "Module.hh"

namespace EUROPA {
  class ModuleBlocksWorld : public Module
  {
    public:
      ModuleBlocksWorld();
      virtual ~ModuleBlocksWorld();

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

  typedef Id<ModuleBlocksWorld> ModuleBlocksWorldId;  
}  


#endif /* #ifndef _H_ModuleBlocksWorld */
