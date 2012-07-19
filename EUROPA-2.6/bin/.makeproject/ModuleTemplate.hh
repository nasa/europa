#ifndef _H_Module%%Project%%
#define _H_Module%%Project%%

#include "Module.hh"

namespace EUROPA {
  class Module%%Project%% : public Module
  {
    public:
      Module%%Project%%();
      virtual ~Module%%Project%%();

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

  typedef Id<Module%%Project%%> Module%%Project%%Id;  
}  


#endif /* #ifndef _H_Module%%Project%% */
