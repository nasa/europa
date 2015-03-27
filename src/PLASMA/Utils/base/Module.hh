#ifndef H_MODULE
#define H_MODULE

#include <string>
#include "Id.hh"

namespace EUROPA {
class Engine;
typedef Id<Engine> EngineId;

class Module;
typedef Id<Module> ModuleId;

class Module {
 public :
  Module(const std::string& name) : m_id(this), m_name(name) {}
  virtual ~Module() {}
	  
  ModuleId getId() { return m_id; }
  const std::string getName() const { return m_name; }
	  
  virtual void initialize() = 0;                  // module initialization  
  virtual void uninitialize() = 0;                // module cleanup

  virtual void initialize(EngineId engine) = 0;   // initialization of a particular engine instance
  virtual void uninitialize(EngineId engine) = 0; // cleanup of a particular engine instance
	  
 protected :
  ModuleId m_id;  
  std::string m_name; 	
};

  
} // End namespace

#endif
