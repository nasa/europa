#ifndef _H_ENGINE
#define _H_ENGINE

#include <string>

namespace EUROPA {

  class EngineComponent 
  {
    public :
	  virtual ~EngineComponent() {}
	  
    protected:
  	  EngineComponent() {}  	  
  };

  typedef Id<EngineComponent> EngineComponentId;

  class Engine;
  typedef Id<Engine> EngineId;

  class Engine 
  {
    public :
	  virtual ~Engine() {}

	  EngineId& getId() { return m_id; }
	  
	  virtual EngineComponentId& getComponent(const std::string& name) = 0;
	  
    protected :
  	  Engine() : m_id(this) {}
  	  
      EngineId m_id;
  };
  
} // End namespace

#endif
