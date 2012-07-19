#ifndef _H_ENGINE
#define _H_ENGINE

#include <istream>
#include <map>
#include <string>
#include <vector>
#include "Id.hh"
#include "tinyxml.h"

namespace EUROPA {

  class Engine;
  typedef Id<Engine> EngineId;

  class EngineComponent;
  typedef Id<EngineComponent> EngineComponentId;

  class Module;
  typedef Id<Module> ModuleId;

  class LanguageInterpreter
  {
    public:
      virtual ~LanguageInterpreter() {}
      virtual std::string interpret(std::istream& input, const std::string& source) = 0;

      void setEngine(EngineId& engine);
      EngineId& getEngine();

    protected:
      EngineId m_engine;
  };

  class EngineComponent
  {
    public :
	  virtual ~EngineComponent() {}

	  void setEngine(EngineId& engine);
	  EngineId& getEngine();

    protected:
  	  EngineComponent() {}

  	  EngineId m_engine;
  };

  class EngineConfig
  {
    public:
      EngineConfig();
      virtual ~EngineConfig();

      const std::string& getProperty(const std::string& name) const;
      void setProperty(const std::string& name,const std::string& value);
      int readFromXML(const char* file_name, bool isFile);
	  void writeFromXML(const char* file_name);
	  void parseXML(TiXmlNode * pParent);

    protected:
      std::map<std::string,std::string> m_properties;
  };

  class Engine
  {
    public :
	  virtual ~Engine() { m_id.remove(); }

	  EngineId& getId() { return m_id; }

	  virtual void addComponent(const std::string& name,EngineComponent* component) = 0;
      virtual EngineComponent* removeComponent(const std::string& name) = 0;
      virtual EngineComponent* getComponent(const std::string& name) = 0;
      virtual const EngineComponent* getComponent(const std::string& name) const = 0;
      virtual const std::map<edouble, EngineComponent*>& getComponents() = 0;

      /** Returns an old interpreter, if any */
      virtual LanguageInterpreter* addLanguageInterpreter(const std::string& language, LanguageInterpreter* interpreter) = 0;
      /** Returns the removed interpreter, if any */
      virtual LanguageInterpreter* removeLanguageInterpreter(const std::string& language) = 0;
      virtual LanguageInterpreter* getLanguageInterpreter(const std::string& language) = 0;

      virtual EngineConfig* getConfig() = 0;

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
        virtual std::map<edouble, EngineComponent*>& getComponents();

        virtual std::string executeScript(const std::string& language, const std::string& script, bool isFile);
        /** Returns an old interpreter, if any */
        virtual LanguageInterpreter *addLanguageInterpreter(const std::string& language, LanguageInterpreter* interpreter);
        /** Returns the removed interpreter, if any */
        virtual LanguageInterpreter* removeLanguageInterpreter(const std::string& language);
        virtual LanguageInterpreter* getLanguageInterpreter(const std::string& language);
        virtual std::map<edouble, LanguageInterpreter*>& getLanguageInterpreters();

        virtual EngineConfig* getConfig() { return m_config; }

    protected:
        virtual ~EngineBase();

        virtual void initializeModules();
        virtual void initializeModule(ModuleId module);
        virtual void uninitializeModules();
        virtual void uninitializeModule(ModuleId module);
        virtual void releaseModules();

        virtual void initializeByModules();
        virtual void initializeByModule(ModuleId module);
        virtual void uninitializeByModules();
        virtual void uninitializeByModule(ModuleId module);

        // TODO: use Ids for languages and components
        EngineConfig* m_config;
        std::vector<ModuleId> m_modules;
        std::map<edouble, LanguageInterpreter*> m_languageInterpreters;          
        std::map<edouble, EngineComponent*> m_components;          
        
    private:
        bool m_started;
  };

} // End namespace

#endif
