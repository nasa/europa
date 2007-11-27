
#ifndef _H_ModuleAnml
#define _H_ModuleAnml

#include "Module.hh"

namespace EUROPA {
  class ModuleAnml : public Module
  {
    public:
      ModuleAnml();
      virtual ~ModuleAnml();

	  virtual void initialize();  // module initialzation

	  virtual void uninitialize(); // module cleanup   

	  virtual void initialize(EngineId engine);   // initialization of a particular engine instance

	  virtual void uninitialize(EngineId engine); // cleanup of a particular engine instance	  
  };

  typedef Id<ModuleAnml> ModuleAnmlId;  
}  


#endif /* #ifndef _H_ModuleAnml */
