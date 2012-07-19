#ifndef _H_EngineBase
#define _H_EngineBase

#include "Engine.hh"
#include "Module.hh"
#include "ConstraintEngineDefs.hh"
#include "PlanDatabaseDefs.hh"
#include "RulesEngineDefs.hh"

namespace EUROPA {

  class EngineBase : public Engine 
  {
    public:  
       	EngineBase();          
    	
    	static void initialize();
    	static void terminate();
    	
    	virtual void doStart();
    	virtual void doShutdown();
    	bool isStarted();
    	
    	virtual void addModule(ModuleId module);
    	virtual void removeModule(ModuleId module);
        virtual void loadModule(const std::string& moduleFileName);
      
        virtual EngineComponentId& getComponent(const std::string& name);
        
        virtual std::string executeScript(const std::string& language, const std::string& script, bool isFile);        
        virtual void addLanguageInterpreter(const std::string& language, LanguageInterpreter* interpreter);
        virtual void removeLanguageInterpreter(const std::string& language);    
        
        ConstraintEngineId& getConstraintEngine() { return m_constraintEngine; }
        PlanDatabaseId&     getPlanDatabase()     { return m_planDatabase; }
        RulesEngineId&      getRulesEngine()      { return m_rulesEngine; }
        
    protected: 
    	virtual ~EngineBase() {}

    	void initializeByModule(ModuleId module);
    	void uninitializeByModule(ModuleId module);
    	
    	void initializeByModules();
    	void uninitializeByModules();
    	
    	// TODO: Modules should not be static, they are for now because of all the singletons we still have laying around
    	static void createModules();
    	
    	static void initializeModule(ModuleId module);
    	static void uninitializeModule(ModuleId module);
    	
    	static void initializeModules();
    	static void uninitializeModules();
    	
    	static ModuleId getModuleByName(const std::string& name);
    	
    	virtual void allocateComponents();
    	virtual void deallocateComponents();
    			
	    std::map<double, LanguageInterpreter*>& getLanguageInterpreters();
	        
	    ConstraintEngineId m_constraintEngine;
	    PlanDatabaseId m_planDatabase;
	    RulesEngineId m_rulesEngine;	
	    std::map<double, LanguageInterpreter*> m_languageInterpreters;    	    
	    static std::vector<ModuleId> m_modules;
	    
    private:
	    bool m_started;
  };
}

#endif
