
#include "Engine.hh"
#include "Debug.hh"
#include "LabelStr.hh"
#include "Entity.hh"
#include "Module.hh"
#ifdef _MSC_VER
#include "Pdlfcn.hh"
#else
#include <dlfcn.h>
#endif

#include <fstream>
#include <sstream>
#include <algorithm>



namespace EUROPA
{
    void EngineComponent::setEngine(EngineId& engine)
    {
        m_engine = engine;
    }

    EngineId& EngineComponent::getEngine()
    {
        return m_engine;
    }

    EngineConfig::EngineConfig()
    {
    }

    EngineConfig::~EngineConfig()
    {
    }

    const std::string& EngineConfig::getProperty(const std::string& name) const
    {
        static std::string noValue="";
        const std::map<std::string,std::string>::const_iterator it = m_properties.find(name);

        if (it != m_properties.end())
            return it->second;

        return noValue;
    }

    void EngineConfig::setProperty(const std::string& name,const std::string& value)
    {
        m_properties[name] = value;
    }

	int EngineConfig::readFromXML(const char* file_name, bool isFile){

		bool loadStat = false;
		int status = 0;
		TiXmlElement * xml = 0;
		TiXmlDocument * m_doc = 0;
		if(isFile)
		{
			m_doc = new TiXmlDocument(file_name);
			loadStat = m_doc->LoadFile();
			checkError(!m_doc->Error(), "Invalid or malformed XML file '" << file_name << "' " << m_doc->ErrorDesc());
		}
		else
		{
			m_doc = new TiXmlDocument();
			m_doc->Parse(file_name);
			if(!m_doc->NoChildren())
				loadStat = true;
		}

		if(loadStat)
		{
			xml = m_doc->RootElement();
			if (xml == NULL) return 0;
			if(strcmp((const char *)xml->Value(),"EuropaConfig") == 0)
			{
				parseXML(xml);
				status = 1;
			}
			else
				debugMsg("EngineConfig","Expected <EuropaConfig>, Found: " << (const char *)xml->Value());
		}
		else
		{
			assert("LOAD FAIL");
		}
		delete m_doc;
		return status;

	}

	void EngineConfig::parseXML(TiXmlNode * pParent){
		TiXmlNode * pChild = 0;
		TiXmlNode * pChildEle = 0;
		const char * comp_name = "";
		const char * prop_name = "";
		const char * prop_value = "";
		while(( pChild = pParent->IterateChildren( pChild ) ))
		{
			if( pParent->Type() == TiXmlNode::ELEMENT )
			{
				if(strcmp((const char *)pChild->ToElement()->Value(),"Component") == 0
										&& pChild->ToElement())
				{
					comp_name = (const char *)pChild->ToElement()->FirstAttribute()->Value();
				}

				while((pChildEle = pChild->IterateChildren(pChildEle))
						&& strcmp((const char *)pChild->ToElement()->FirstChildElement()->Value(),"Property") == 0)
				{
					if(pChildEle->FirstChild())
					{
						prop_name = (const char *)pChildEle->ToElement()->FirstAttribute()->Value();
						prop_value = (const char *)pChildEle->FirstChild()->ToText()->Value();
						if(std::string(prop_name).compare("") != 0)
						{
							setProperty(std::string(comp_name).append(".").append(prop_name), std::string(prop_value));
						}
					}
				}
			}

		}
	}

	void EngineConfig::writeFromXML(const char* file_name){
		TiXmlDocument * m_doc = new TiXmlDocument(file_name);
		TiXmlDeclaration * m_decl;
		m_decl = new TiXmlDeclaration("1.0", "", "" );
		m_doc->LinkEndChild(m_decl);
		TiXmlElement * root = new TiXmlElement("EuropaConfig");
		m_doc->LinkEndChild(root);
		TiXmlElement * elePtr = 0;
		TiXmlElement * propPtr = 0;
		TiXmlText * textPtr = 0;
		const char * COMPONENT = "Component";
		std::string  component_nm = "";
		std::string  property_attrib = "";
		const char *  PROPERTY = "Property";
		const char *  NAME = "name";
		std::string  value = "";
		std::map<std::string,std::string>::iterator it;
		std::map<std::string,std::string>::iterator next;
		for (it=m_properties.begin() ; it != m_properties.end(); it++ )
		{
			if(!component_nm.compare((*it).first.substr(0,(*it).first.find(".")))==0 && (*it).first.length()>0)
			{
				component_nm = (*it).first.substr(0,(*it).first.find("."));
				elePtr = new TiXmlElement(COMPONENT);
				elePtr->SetAttribute(NAME,component_nm.c_str());
				root->LinkEndChild(elePtr);
				propPtr = new TiXmlElement(PROPERTY);
				elePtr->LinkEndChild(propPtr);
				property_attrib = (*it).first.substr((*it).first.find(".")+1);
				propPtr->SetAttribute(NAME,property_attrib.c_str());
				value = (*it).second;
				textPtr = new TiXmlText(value.c_str());
				propPtr->LinkEndChild(textPtr);
			}else if(component_nm.compare((*it).first.substr(0,(*it).first.find(".")))==0 && (*it).first.length()>0)
			{
				propPtr = new TiXmlElement(PROPERTY);
				elePtr->LinkEndChild(propPtr);
				property_attrib = (*it).first.substr((*it).first.find(".")+1);
				propPtr->SetAttribute(NAME,property_attrib.c_str());
				value = (*it).second;
				textPtr = new TiXmlText(value.c_str());
				propPtr->LinkEndChild(textPtr);
			}
		}
		m_doc->SaveFile(file_name);
		delete m_doc;
		return;
	}

    EngineBase::EngineBase()
    {
    	m_started = false;
    	// TODO: make this data-driven so XML/database configs can be instanciated.
    	m_config = new EngineConfig();
    }

    EngineBase::~EngineBase()
    {
        releaseModules();
        delete m_config;
    }

    void EngineBase::releaseModules()
    {
        std::vector<ModuleId>::iterator it;
        for (it = m_modules.begin(); it != m_modules.end(); ++it) {
            ModuleId& m = *it;
            m.release();
        }
        m_modules.clear();
    }


    ModuleId& EngineBase::getModule(const std::string& name)
    {
        static ModuleId noId=ModuleId::noId();

	    for (unsigned int i=0;i<m_modules.size();i++) {
	    	if (m_modules[i]->getName() == name)
	    		return m_modules[i];
	    }

	    return noId;
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
    }

    void EngineBase::initializeModules()
    {
	    for (unsigned int i=0;i<m_modules.size();i++)
	    	initializeModule(m_modules[i]);
    }

    void EngineBase::uninitializeModules()
    {
        for (unsigned int i=m_modules.size(); i>0 ;i--)
        	uninitializeModule(m_modules[i-1]);
    }

    bool EngineBase::isStarted()
    {
    	return m_started;
    }

    void EngineBase::doStart()
    {
    	if(!m_started)
    	{
            initializeModules();
    		initializeByModules();
    		m_started = true;
    	}
    }

    void EngineBase::doShutdown()
    {
    	if(m_started)
    	{
            Entity::purgeStarted();
    		uninitializeByModules();
            uninitializeModules();
            Entity::purgeEnded();
            Entity::garbageCollect();
    		m_started = false;
    	}
    }

	void EngineBase::addModule(ModuleId module)
	{
		m_modules.push_back(module);

		if(isStarted()) {
            initializeModule(module);
			initializeByModule(module);
		}
	}

	void EngineBase::removeModule(ModuleId module)
	{
		std::vector<ModuleId>::iterator it = find(m_modules.begin(), m_modules.end(), module);
		checkError(it != m_modules.end(), "EngineBase: removeModule Module not found." << module->getName());

		if(isStarted()) {
			uninitializeByModule(module);
			uninitializeModule(module);
		}

		m_modules.erase(it);
	}

	// Basically a copy of PSEngineImpl::loadModel
	void EngineBase::loadModule(const std::string& moduleFileName)
	{
		check_runtime_error(m_started,"Engine has not been started");

	#ifdef _MSC_VER
		void* libHandle = p_dlopen(moduleFileName.c_str(), RTLD_NOW);
		checkRuntimeError(libHandle != NULL,
				"Error opening module " << moduleFileName << ": " << p_dlerror());

		ModuleId (*fcn_module)();
		fcn_module = (ModuleId (*)()) p_dlsym(libHandle, "initializeModule");
		checkError(fcn_module != NULL,
				"Error locating symbol 'initializeModule' in " << moduleFileName << ": " <<
				p_dlerror());
	#else
		void* libHandle = dlopen(moduleFileName.c_str(), RTLD_NOW);
		checkRuntimeError(libHandle != NULL,
				"Error opening module " << moduleFileName << ": " << dlerror());

		ModuleId (*fcn_module)();
		fcn_module = (ModuleId (*)()) dlsym(libHandle, "initializeModule");
		checkError(fcn_module != NULL,
				"Error locating symbol 'initializeModule' in " << moduleFileName << ": " <<
				dlerror());
	#endif

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

    std::string EngineBase::executeScript(const std::string& language, const std::string& script, bool isFile)
    {
      std::map<edouble, LanguageInterpreter*>::iterator it = getLanguageInterpreters().find(LabelStr(language));
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

    LanguageInterpreter *EngineBase::addLanguageInterpreter(const std::string& language, LanguageInterpreter* interpreter)
    {
      LanguageInterpreter *old = NULL;
      std::map<edouble, LanguageInterpreter*>::iterator it = getLanguageInterpreters().find(LabelStr(language));
      if(it == getLanguageInterpreters().end())
        getLanguageInterpreters().insert(std::make_pair(LabelStr(language), interpreter));
      else {
    	old = it->second;
        it->second = interpreter;
      }
      interpreter->setEngine(getId());
      return old;
    }

    LanguageInterpreter* EngineBase::removeLanguageInterpreter(const std::string& language)
    {
      LanguageInterpreter *old = NULL;
      std::map<edouble, LanguageInterpreter*>::iterator it = getLanguageInterpreters().find(LabelStr(language));
      if(it != getLanguageInterpreters().end()) {
    	old = it->second;
        getLanguageInterpreters().erase(it);
      }
      return old;
    }

    LanguageInterpreter* EngineBase::getLanguageInterpreter(const std::string& language)
    {
        std::map<edouble, LanguageInterpreter*>::iterator it = getLanguageInterpreters().find(LabelStr(language));
        if(it != getLanguageInterpreters().end()) 
            return it->second;

        return NULL;
    }

    std::map<edouble, LanguageInterpreter*>& EngineBase::getLanguageInterpreters()
    {
        return m_languageInterpreters;
    }

    void EngineBase::addComponent(const std::string& name,EngineComponent* component)
    {
        std::map<edouble, EngineComponent*>::iterator it = getComponents().find(LabelStr(name));
        if(it == getComponents().end())
          getComponents().insert(std::make_pair(LabelStr(name), component));
        else {
          delete it->second;
          it->second = component;
        }
        component->setEngine(getId());
    }

    EngineComponent* EngineBase::getComponent(const std::string& name)
    {
      std::map<edouble, EngineComponent*>::iterator it = getComponents().find(LabelStr(name));
      if(it != getComponents().end()) 
          return it->second;

  	  return NULL;
    }

    const EngineComponent* EngineBase::getComponent(const std::string& name) const
    {
      std::map<edouble, EngineComponent*>::const_iterator it = m_components.find(LabelStr(name));
      if(it != m_components.end()) 
          return it->second;

      return NULL;
    }

    EngineComponent* EngineBase::removeComponent(const std::string& name)
    {
        EngineComponent* c = getComponent(name);

        if (c != NULL)
            getComponents().erase(LabelStr(name));

        static EngineId s_nullEngineId;
        c->setEngine(s_nullEngineId);
        return c;
    }    
    
    std::map<edouble, EngineComponent*>& EngineBase::getComponents()
    {
        return m_components;
    }

    void LanguageInterpreter::setEngine(EngineId& engine)
    {
        m_engine = engine;
    }

    EngineId& LanguageInterpreter::getEngine()
    {
        return m_engine;
    }
}

