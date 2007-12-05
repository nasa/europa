
#include "EngineBase.hh"
#include "ModuleConstraintEngine.hh"
#include "ModulePlanDatabase.hh"
#include "ModuleRulesEngine.hh"
#include "ModuleTemporalNetwork.hh"
#include "ModuleSolvers.hh"
#include "ModuleNddl.hh"
#ifndef NO_RESOURCES
#include "ModuleResource.hh"
#include "ModuleAnml.hh"
#endif

#include "ConstraintEngine.hh"
#include "PlanDatabase.hh"
#include "RulesEngine.hh"

#include <fstream>
#include <sstream>

namespace EUROPA 
{
    std::vector<ModuleId> EngineBase::m_modules;

    bool& isInitialized()
    {
      static bool sl_isInitialized(false);
      return sl_isInitialized;
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

    void EngineBase::initializeModules()
    {
	    createModules();
	  
	    for (unsigned int i=0;i<m_modules.size();i++) {
	    	m_modules[i]->initialize();
	    	debugMsg("PSEngine","Initialized Module " << m_modules[i]->getName());		  
	    }	  
    }

    void EngineBase::uninitializeModules()
    {
        Entity::purgeStarted();      
        for (unsigned int i=m_modules.size();i>0;i--) {
        	unsigned int idx = i-1;
        	m_modules[idx]->uninitialize();
        	debugMsg("PSEngine","Uninitialized Module " << m_modules[idx]->getName());
        	m_modules[idx].release();
        }	  
        Entity::purgeEnded();	  

        m_modules.clear();	  
    }

    void EngineBase::allocateComponents()
    {
	    m_constraintEngine = (new ConstraintEngine())->getId();	  
	    m_planDatabase = (new PlanDatabase(m_constraintEngine, Schema::instance()))->getId();	
	    m_rulesEngine = (new RulesEngine(m_planDatabase))->getId();	  

	    for (unsigned int i=0;i<m_modules.size();i++) {
	    	m_modules[i]->initialize(getId());
	    	debugMsg("PSEngine","Engine initialized by Module " << m_modules[i]->getName());		  
	    }	  	  
    }	  
    
    void EngineBase::deallocateComponents()
    {
  	  for (unsigned int i=m_modules.size();i>0;i--) {
  		  unsigned int idx = i-1;
  		  m_modules[idx]->uninitialize(getId());
  		  debugMsg("PSEngine","Engine uninitialized by Module " << m_modules[idx]->getName());		  
  	  }	  

  	  Entity::purgeStarted();
        
      if(m_rulesEngine.isValid()) delete (RulesEngine*) m_rulesEngine;
  	  if(m_planDatabase.isValid()) delete (PlanDatabase*) m_planDatabase; 
  	  if(m_constraintEngine.isValid()) delete (ConstraintEngine*) m_constraintEngine;

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
  	  
  	  if (name == "ConstraintEngine")
  		  return (EngineComponentId&)m_constraintEngine;
  	  if (name == "PlanDatabase")
  		  return (EngineComponentId&)m_planDatabase;
  	  if (name == "RulesEngine")
  		  return (EngineComponentId&)m_constraintEngine;

  	  return noId;
    }            
}

