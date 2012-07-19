#ifndef _H_MODULE
#define _H_MODULE

#include <map>
#include <string>
#include "Id.hh"
#include "Engine.hh"

namespace EUROPA {

  class Module 
  {
    public :
	  Module(const std::string& name) : m_name(name) {}
	  virtual ~Module() {}
	  
	  const std::string getName() const { return m_name; }
	  
	  virtual void initialize() = 0;                  // module initialization  
	  virtual void uninitialize() = 0;                // module cleanup

	  virtual void initialize(EngineId engine) = 0;   // initialization of a particular engine instance
	  virtual void uninitialize(EngineId engine) = 0; // cleanup of a particular engine instance
	  
    protected :
      std::string m_name; 	
  };

  typedef Id<Module> ModuleId;
  
} // End namespace

#endif
