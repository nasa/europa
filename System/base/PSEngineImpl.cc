/*
 * PSEngine.cc
 *
 */
 
#include "PSEngineImpl.hh"

#include "LabelStr.hh"
#include "Pdlfcn.hh"
#include "Schema.hh"

#include "PSConstraintEngineImpl.hh"
#include "PSPlanDatabaseImpl.hh"
#include "PSSolversImpl.hh"
// TODO: get rid of all the #ifdefs 
#ifndef NO_RESOURCES
#include "PSResourceImpl.hh"
#endif

namespace EUROPA {
  
  void PSEngine::initialize()
  {
	PSEngineImpl::initialize();  
  }
  
  void PSEngine::terminate()
  {
	PSEngineImpl::terminate();  
  }
  
  PSEngine* PSEngine::makeInstance()
  {
	  return new PSEngineImpl();
  }

  PSEngineImpl::PSEngineImpl() 
      : m_started(false)
  {
  }

  PSEngineImpl::~PSEngineImpl() 
  {
	  if (m_started)
		  shutdown();
  }

  void PSEngineImpl::initialize()
  {
    Error::doThrowExceptions(); // throw exceptions!
    Error::doDisplayErrors();    
	EngineBase::initialize();  
  }
  
  void PSEngineImpl::terminate()
  {
	EngineBase::terminate();  
  }
  
  void PSEngineImpl::start() 
  {		
	if (m_started)
		return;
	
    doStart();   
    m_started = true;
  }

  void PSEngineImpl::shutdown() 
  {
	if (!m_started)
		return;
	
    doShutdown();    
    m_started = false;
  }
    
  void PSEngineImpl::allocateComponents()
  {
	  EngineBase::allocateComponents();
	  m_psConstraintEngine = new PSConstraintEngineImpl(m_constraintEngine);
	  m_psPlanDatabase = new PSPlanDatabaseImpl(m_planDatabase);
	  m_psSolverManager = new PSSolverManagerImpl(m_constraintEngine,m_planDatabase,m_rulesEngine);
#ifndef NO_RESOURCES
	  m_psResourceManager = new PSResourceManagerImpl(m_planDatabase);
#endif	  
  }
  
  void PSEngineImpl::deallocateComponents()
  {
	  delete m_psConstraintEngine;
	  delete m_psPlanDatabase;
	  delete m_psSolverManager;
#ifndef NO_RESOURCES
	  delete m_psResourceManager;
#endif	  
	  EngineBase::deallocateComponents();
  }
  
  EngineComponent* PSEngineImpl::getComponentPtr(const std::string& name)
  {
  	  if (name == "PSConstraintEngine")
  		  return (EngineComponent*)m_psConstraintEngine;
  	  if (name == "PSPlanDatabase")
  		  return (EngineComponent*)m_psPlanDatabase;
  	  if (name == "PSSolverManager")
  		  return (EngineComponent*)m_psSolverManager;
  	  
      return EngineBase::getComponent(name);  	
  }   
  
  void PSEngineImpl::loadModel(const std::string& modelFileName) 
  {
    check_runtime_error(m_started,"PSEngine has not been started");
	    
    void* libHandle = p_dlopen(modelFileName.c_str(), RTLD_NOW);
    checkRuntimeError(libHandle != NULL,
	       "Error opening model " << modelFileName << ": " << p_dlerror());

    SchemaId (*fcn_schema)();
    fcn_schema = (SchemaId (*)()) p_dlsym(libHandle, "loadSchema");
    checkError(fcn_schema != NULL,
	       "Error locating symbol 'loadSchema' in " << modelFileName << ": " <<
	       p_dlerror());

    SchemaId schema = (*fcn_schema)();
  }

  std::string PSEngineImpl::executeScript(const std::string& language, const std::string& script, bool isFile) 
  {
    return EngineBase::executeScript(language,script,isFile);
  }

  // Plan Database methods
  PSList<PSObject*> PSEngineImpl::getObjectsByType(const std::string& objectType) 
  {
    check_runtime_error(m_started,"PSEngine has not been started");
    return m_psPlanDatabase->getObjectsByType(objectType);  
  }

  PSObject* PSEngineImpl::getObjectByKey(PSEntityKey id) 
  {
    check_runtime_error(m_started,"PSEngine has not been started");
    return m_psPlanDatabase->getObjectByKey(id);  
  }

  PSObject* PSEngineImpl::getObjectByName(const std::string& name) 
  {
    check_runtime_error(m_started,"PSEngine has not been started");
    return m_psPlanDatabase->getObjectByName(name);  
  }

  PSList<PSToken*> PSEngineImpl::getTokens() 
  {
    check_runtime_error(m_started,"PSEngine has not been started");
    return m_psPlanDatabase->getTokens();  
  }

  PSToken* PSEngineImpl::getTokenByKey(PSEntityKey id) 
  {
    check_runtime_error(m_started,"PSEngine has not been started");
    return m_psPlanDatabase->getTokenByKey(id);  
  }

  PSList<PSVariable*>  PSEngineImpl::getGlobalVariables() 
  {
    check_runtime_error(m_started,"PSEngine has not been started");
    return m_psPlanDatabase->getGlobalVariables();  
  }  

  std::string PSEngineImpl::planDatabaseToString() 
  {
    check_runtime_error(m_started,"PSEngine has not been started");
	return m_psPlanDatabase->toString();  
  }

  // Constraint Engine methods
  PSVariable* PSEngineImpl::getVariableByKey(PSEntityKey id)
  {
    check_runtime_error(m_started,"PSEngine has not been started");
	return m_psConstraintEngine->getVariableByKey(id);  
  }

  PSVariable* PSEngineImpl::getVariableByName(const std::string& name)
  {
    check_runtime_error(m_started,"PSEngine has not been started");
	return m_psConstraintEngine->getVariableByName(name);  
  }

  bool PSEngineImpl::getAllowViolations() const
  {
  	return m_constraintEngine->getAllowViolations();
  }

  void PSEngineImpl::setAllowViolations(bool v)
  {
    m_constraintEngine->setAllowViolations(v);
  }

  double PSEngineImpl::getViolation() const
  {
  	return m_constraintEngine->getViolation();
  }
   
  std::string PSEngineImpl::getViolationExpl() const
  {
  	return m_constraintEngine->getViolationExpl();
  }

  // Solver methods
  PSSolver* PSEngineImpl::createSolver(const std::string& configurationFile) 
  {
    check_runtime_error(m_started,"PSEngine has not been started");
    return m_psSolverManager->createSolver(configurationFile);
  }

#ifndef NO_RESOURCES
   PSList<PSResource*> PSEngineImpl::getResourcesByType(const std::string& objectType) {
     check_runtime_error(m_started,"PSEngine has not been started");
     return m_psResourceManager->getResourcesByType(objectType);
   }
   
   PSResource* PSEngineImpl::getResourceByKey(PSEntityKey id) {
     check_runtime_error(m_started,"PSEngine has not been started");
     return m_psResourceManager->getResourceByKey(id);
   }    
#else
   PSList<PSResource*> PSEngineImpl::getResourcesByType(const std::string& objectType) {
     check_runtime_error(ALWAYS_FAIL,"PSEngine was built without resource module");     
     PSList<PSResource*> retval;
     return retval;
   }
   
   PSResource* PSEngineImpl::getResourceByKey(PSEntityKey id) {
     check_runtime_error(ALWAYS_FAIL,"PSEngine was built without resource module");
     return NULL;
   }      
#endif        
}

