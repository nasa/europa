#ifndef _H_ENGINE
#define _H_ENGINE

#include <istream>
#include <map>
#include <string>
#include <vector>
#include "Id.hh"

namespace EUROPA {

  class LanguageInterpreter 
  {
    public:
      virtual ~LanguageInterpreter() {}
      virtual std::string interpret(std::istream& input, const std::string& source) = 0;
  };


  class EngineComponent; 
  typedef Id<EngineComponent> EngineComponentId;
  
  class EngineComponent 
  {
    public :
	  virtual ~EngineComponent() {}	  
	  
    protected:
  	  EngineComponent() {}  	  
  };


  class Module;
  typedef Id<Module> ModuleId;
  
  class Engine;
  typedef Id<Engine> EngineId;

  class Engine 
  {
    public :
	  virtual ~Engine() { m_id.remove(); }

	  EngineId& getId() { return m_id; }
	  
	  virtual void addComponent(const std::string& name,EngineComponent* component) = 0;
      virtual EngineComponent* removeComponent(const std::string& name) = 0;
      virtual EngineComponent* getComponent(const std::string& name) = 0; 
      virtual const EngineComponent* getComponent(const std::string& name) const = 0; 
      virtual const std::map<double, EngineComponent*>& getComponents() = 0;
	  
      virtual void addLanguageInterpreter(const std::string& language, LanguageInterpreter* interpreter) = 0;
      virtual void removeLanguageInterpreter(const std::string& language) = 0;
      virtual LanguageInterpreter* getLanguageInterpreter(const std::string& language) = 0;

    protected :
  	  Engine() : m_id(this) {}
  	  
      EngineId m_id;
  };
  
  class EngineBase : public Engine 
  {
    public:  
        EngineBase();          
        
        virtual void doStart();
        virtual void doShutdown();
        bool isStarted();
        
        virtual void addModule(ModuleId module);
        virtual void loadModule(const std::string& moduleFileName);
        virtual void removeModule(ModuleId module);
        virtual ModuleId& getModule(const std::string& moduleName);
        // TODO: add these
        //virtual void removeModule(const std::string& moduleName);
        //virtual void unloadModule(const std::string& moduleName);
        //virtual std::vector<std::string> getModuleNames(const std::string& moduleName);
              
        virtual void addComponent(const std::string& name,EngineComponent* component);
        virtual EngineComponent* removeComponent(const std::string& name);
        virtual EngineComponent* getComponent(const std::string& name);
        virtual const EngineComponent* getComponent(const std::string& name) const;
        virtual std::map<double, EngineComponent*>& getComponents();
        
        virtual std::string executeScript(const std::string& language, const std::string& script, bool isFile);        
        virtual void addLanguageInterpreter(const std::string& language, LanguageInterpreter* interpreter);
        virtual void removeLanguageInterpreter(const std::string& language);    
        virtual LanguageInterpreter* getLanguageInterpreter(const std::string& language);
        virtual std::map<double, LanguageInterpreter*>& getLanguageInterpreters();
        
    protected: 
        virtual ~EngineBase();

        void initializeModules();
        void initializeModule(ModuleId module);
        void uninitializeModules();
        void uninitializeModule(ModuleId module);
        void releaseModules();
        
        void initializeByModules();
        void initializeByModule(ModuleId module);
        void uninitializeByModules();        
        void uninitializeByModule(ModuleId module);
                
        // TODO: use Ids for languages and components
        std::vector<ModuleId> m_modules;
        std::map<double, LanguageInterpreter*> m_languageInterpreters;          
        std::map<double, EngineComponent*> m_components;          
        
    private:
        bool m_started;
  };
  
} // End namespace

#endif
