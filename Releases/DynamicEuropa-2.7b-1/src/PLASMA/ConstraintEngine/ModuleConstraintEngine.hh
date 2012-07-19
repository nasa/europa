
#ifndef _H_ModuleConstraintEngine
#define _H_ModuleConstraintEngine

#include "Module.hh"

namespace EUROPA {
  class ModuleConstraintEngine : public Module
  {
    public:
      ModuleConstraintEngine();
      virtual ~ModuleConstraintEngine();

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

  typedef Id<ModuleConstraintEngine> ModuleConstraintEngineId;
  
  class ModuleConstraintLibrary : public Module
  {
    public:
      ModuleConstraintLibrary();
      virtual ~ModuleConstraintLibrary();

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

  typedef Id<ModuleConstraintLibrary> ModuleConstraintLibraryId;
}  


#endif /* #ifndef _H_ModuleConstraintEngine */
