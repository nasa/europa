
#include "EngineBase.hh"
#include "ModuleConstraintEngine.hh"
#include "ModulePlanDatabase.hh"
#include "ModuleRulesEngine.hh"
#include "ModuleTemporalNetwork.hh"
#include "ModuleSolvers.hh"
#include "ModuleNddl.hh"
#include "Pdlfcn.hh"
#ifndef NO_RESOURCES
#include "ModuleResource.hh"
#include "ModuleAnml.hh"
#endif

#include "ConstraintEngine.hh"
#include "PlanDatabase.hh"
#include "RulesEngine.hh"

#include <fstream>
#include <sstream>
#include <algorithm>

namespace EUROPA
{
    // TODO: must become instance var
    std::vector<ModuleId> EngineBase::m_modules;

    bool& isInitialized()
    {
      static bool sl_isInitialized(false);
      return sl_isInitialized;
    }

    EngineBase::EngineBase()
    {
    	m_started = false;
    }

    void EngineBase::initialize()
    {
    	if (!isInitialized()) {
    	    initializeModules();
    	    isInitialized() = true;
    	}
    }

    void EngineBase::terminate()
    {
    	if (isInitialized()) {
    	    uninitializeModules();
    	    isInitialized() = false;
    	}
    }

    ModuleId EngineBase::getModuleByName(const std::string& name)
    {
	    for (unsigned int i=0;i<m_modules.size();i++) {
	    	if (m_modules[i]->getName() == name)
	    		return m_modules[i];
	    }
	    return ModuleId::noId();
    }

    void EngineBase::createModules()
    {
	    // TODO: make this data-driven
	    m_modules.push_back(new ModuleConstraintEngine());
	    m_modules.push_back(new ModuleConstraintLibrary());
	    m_modules.push_back(new ModulePlanDatabase());
	    m_modules.push_back(new ModuleRulesEngine());
	    m_modules.push_back(new ModuleTemporalNetwork());
	    m_modules.push_back(new ModuleSolvers());
	    m_modules.push_back(new ModuleNddl());
#ifndef NO_RESOURCES
        m_modules.push_back(new ModuleResource());
	    m_modules.push_back(new ModuleAnml());
#endif
    }

    void EngineBase::initializeModule(ModuleId module)
    {
      	module->initialize();
      	debugMsg("EngineBase","Initialized Module " << module->getName());
    }

    void EngineBase::uninitializeModule(ModuleId module)
    {
    	module->uninitialize();
    	debugMsg("EngineBase","Uninitialized Module " << module->getName());
    	module.release();
    }

    void EngineBase::initializeModules()
    {
	    createModules();
	    for (unsigned int i=0;i<m_modules.size();i++)
	    	initializeModule(m_modules[i]);
    }

    void EngineBase::uninitializeModules()
    {
        Entity::purgeStarted();
        for (unsigned int i=m_modules.size(); i>0 ;i--)
        	uninitializeModule(m_modules[i-1]);
        Entity::purgeEnded();

        m_modules.clear();
    }


    bool EngineBase::isStarted()
    {
    	return m_started;
    }

    void EngineBase::doStart()
    {
    	if(!m_started)
    	{
    		allocateComponents();
    		initializeByModules();
    		m_started = true;
    	}
    }

    void EngineBase::doShutdown()
    {
    	if(m_started)
    	{
    		uninitializeByModules();
    		deallocateComponents();
    		m_started = false;
    	}
    }

	void EngineBase::addModule(ModuleId module)
	{
		m_modules.push_back(module);
		if(isInitialized())
		{
			initializeModule(module);
		}
		if(isStarted())
		{
			initializeByModule(module);
		}
	}

	void EngineBase::removeModule(ModuleId module)
	{
		std::vector<ModuleId>::iterator it = find(m_modules.begin(), m_modules.end(), module);
		checkError(it != m_modules.end(), "EngineBase: removeModule Module not found." << module->getName());
		if(isStarted())
		{
			uninitializeByModule(module);
		}
		if(isInitialized())
		{
			uninitializeModule(module);
		}
		m_modules.erase(it);
	}

	// Basically a copy of PSEngineImpl::loadModel
	void EngineBase::loadModule(const std::string& moduleFileName)
	{
		check_runtime_error(m_started,"PSEngine has not been started");

		void* libHandle = p_dlopen(moduleFileName.c_str(), RTLD_NOW);
		checkRuntimeError(libHandle != NULL,
				"Error opening module " << moduleFileName << ": " << p_dlerror());

		ModuleId (*fcn_module)();
		fcn_module = (ModuleId (*)()) p_dlsym(libHandle, "initializeModule");
		checkError(fcn_module != NULL,
				"Error locating symbol 'initializeModule' in " << moduleFileName << ": " <<
				p_dlerror());

		ModuleId module = (*fcn_module)();
		addModule(module);
	}


    void EngineBase::initializeByModule(ModuleId module)
    {
    	module->initialize(getId());
    	debugMsg("EngineBase","Engine initialized by Module " << module->getName());
    }

    void EngineBase::uninitializeByModule(ModuleId module)
    {
    	module->uninitialize(getId());
    	debugMsg("EngineBase","Engine uninitialized by Module " << module->getName());
    }

    void EngineBase::initializeByModules()
    {
	    for (unsigned int i=0;i<m_modules.size();i++)
	    	initializeByModule(m_modules[i]);
    }

    void EngineBase::uninitializeByModules()
    {
    	for (int i=m_modules.size(); i>0; i--)
    		uninitializeByModule(m_modules[i-1]);
    }

    void EngineBase::allocateComponents()
    {
        m_schema = (new Schema("EngineSchema"))->getId(); // TODO: use engine name
	    m_constraintEngine = (new ConstraintEngine())->getId();
	    m_planDatabase = (new PlanDatabase(m_constraintEngine, m_schema))->getId();
	    m_rulesEngine = (new RulesEngine(m_planDatabase))->getId();
    }

    void EngineBase::deallocateComponents()
    {
  	  Entity::purgeStarted();

      if(m_rulesEngine.isValid()) delete (RulesEngine*) m_rulesEngine;
  	  if(m_planDatabase.isValid()) delete (PlanDatabase*) m_planDatabase;
  	  if(m_constraintEngine.isValid()) delete (ConstraintEngine*) m_constraintEngine;
      if(m_schema.isValid()) delete (Schema*) m_schema;           

  	  Entity::purgeEnded();
    }

    std::string EngineBase::executeScript(const std::string& language, const std::string& script, bool isFile)
    {
      std::map<double, LanguageInterpreter*>::iterator it = getLanguageInterpreters().find(LabelStr(language));
      checkRuntimeError(it != getLanguageInterpreters().end(),
  		      "Cannot execute script for unknown language \"" << language << "\"");


      std::istream *in;
      std::string source;
      if (isFile) {
        in = new std::ifstream(script.c_str());
        checkRuntimeError(in->good(), "Cannot read script from location \"" << script << "\"");
        source = script;
      }
      else {
        in = new std::istringstream(script);
        source = "<eval>";
      }

      std::string retval = it->second->interpret(*in, source);
      delete in;

      return retval;
    }

    std::map<double, LanguageInterpreter*>& EngineBase::getLanguageInterpreters()
    {
        return m_languageInterpreters;
    }

    void EngineBase::addLanguageInterpreter(const std::string& language, LanguageInterpreter* interpreter)
    {
      std::map<double, LanguageInterpreter*>::iterator it = getLanguageInterpreters().find(LabelStr(language));
      if(it == getLanguageInterpreters().end())
        getLanguageInterpreters().insert(std::make_pair(LabelStr(language), interpreter));
      else {
        delete it->second;
        it->second = interpreter;
      }
    }

    void EngineBase::removeLanguageInterpreter(const std::string& language)
    {
      std::map<double, LanguageInterpreter*>::iterator it = getLanguageInterpreters().find(LabelStr(language));
      if(it != getLanguageInterpreters().end()) {
        delete it->second;
        getLanguageInterpreters().erase(it);
      }
    }

    EngineComponentId& EngineBase::getComponent(const std::string& name)
    {
  	  static EngineComponentId noId = EngineComponentId::noId();

      if (name == "Schema")
          return (EngineComponentId&)m_schema;
  	  if (name == "ConstraintEngine")
  		  return (EngineComponentId&)m_constraintEngine;
  	  if (name == "PlanDatabase")
  		  return (EngineComponentId&)m_planDatabase;
  	  if (name == "RulesEngine")
  		  return (EngineComponentId&)m_constraintEngine;

  	  return noId;
    }
}

