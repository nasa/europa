#ifndef _H_ENGINE
#define _H_ENGINE

#include <string>
#include <istream>
#include "Id.hh"

namespace EUROPA {

  class LanguageInterpreter 
  {
    public:
      virtual ~LanguageInterpreter() {}
      virtual std::string interpret(std::istream& input, const std::string& source) = 0;
  };


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
	  virtual ~Engine() { m_id.remove(); }

	  EngineId& getId() { return m_id; }
	  
	  virtual EngineComponentId& getComponent(const std::string& name) = 0;
	  
      virtual void addLanguageInterpreter(const std::string& language, LanguageInterpreter* interpreter) = 0;
      virtual void removeLanguageInterpreter(const std::string& language) = 0;

    protected :
  	  Engine() : m_id(this) {}
  	  
      EngineId m_id;
  };
  
} // End namespace

#endif
