
#ifndef _H_ModuleSolvers
#define _H_ModuleSolvers

#include "Module.hh"

namespace EUROPA {
  class ModuleSolvers : public Module
  {
    public:
      ModuleSolvers();
      virtual ~ModuleSolvers();

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

  typedef Id<ModuleSolvers> ModuleSolversId;  
}  


#endif /* #ifndef _H_ModuleSolvers */
