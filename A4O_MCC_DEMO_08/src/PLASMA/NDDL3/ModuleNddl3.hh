
#ifndef _H_ModuleNddl3
#define _H_ModuleNddl3

#include "Module.hh"

namespace EUROPA {
  class ModuleNddl3 : public Module
  {
    public:
      ModuleNddl3();
      virtual ~ModuleNddl3();

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

  typedef Id<ModuleNddl3> ModuleNddl3Id;
}


#endif /* #ifndef _H_ModuleNddl3 */
