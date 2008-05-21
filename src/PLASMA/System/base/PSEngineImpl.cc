/*
 * PSEngine.cc
 *
 */
 
#include "PSEngineImpl.hh"

#include "LabelStr.hh"
#include "Pdlfcn.hh"
#include "Schema.hh"

#include "Constraint.hh"
#include "PlanDatabase.hh"
#include "PSPlanDatabaseImpl.hh"
#include "PSSolversImpl.hh"

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
  {
  }	 
  
  PSEngineImpl::~PSEngineImpl() 
  {
	  shutdown();
  }

  void PSEngineImpl::initialize()
  {
    Error::doThrowExceptions(); // throw exceptions!
    Error::doDisplayErrors();    
  }
  
  void PSEngineImpl::terminate()
  {
  }
 

  void PSEngineImpl::start() 
  {		
	  doStart();   
  }

  void PSEngineImpl::shutdown() 
  {
	  doShutdown();    
  }
    
  void PSEngineImpl::addModule(ModuleId module)
  {
	  EuropaEngineBase::addModule(module);
  }
  void PSEngineImpl::removeModule(ModuleId module)
  {
	  EuropaEngineBase::removeModule(module);
  }
  void PSEngineImpl::loadModule(const std::string& moduleFileName) 
  {
	  EuropaEngineBase::loadModule(moduleFileName);
  }

  void PSEngineImpl::loadModel(const std::string& modelFileName) 
   {
	   check_runtime_error(isStarted(),"PSEngine has not been started");

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
    return EuropaEngineBase::executeScript(language,script,isFile);
  }

  // Plan Database methods
  PSList<PSObject*> PSEngineImpl::getObjectsByType(const std::string& objectType) 
  {
    check_runtime_error(isStarted(),"PSEngine has not been started");
    return getPlanDatabase()->getObjectsByType(objectType);  
  }

  PSObject* PSEngineImpl::getObjectByKey(PSEntityKey id) 
  {
    check_runtime_error(isStarted(),"PSEngine has not been started");
    return getPlanDatabase()->getObjectByKey(id);  
  }

  PSObject* PSEngineImpl::getObjectByName(const std::string& name) 
  {
    check_runtime_error(isStarted(),"PSEngine has not been started");
    return getPlanDatabase()->getObjectByName(name);  
  }

  PSList<PSToken*> PSEngineImpl::getTokens() 
  {
    check_runtime_error(isStarted(),"PSEngine has not been started");
    return getPlanDatabase()->getAllTokens();  
  }

  PSToken* PSEngineImpl::getTokenByKey(PSEntityKey id) 
  {
    check_runtime_error(isStarted(),"PSEngine has not been started");
    return getPlanDatabase()->getTokenByKey(id);  
  }

  PSList<PSVariable*>  PSEngineImpl::getGlobalVariables() 
  {
    check_runtime_error(isStarted(),"PSEngine has not been started");
    return getPlanDatabase()->getAllGlobalVariables();  
  }  

  std::string PSEngineImpl::planDatabaseToString() 
  {
    check_runtime_error(isStarted(),"PSEngine has not been started");
	return getPlanDatabase()->toString();  
  }

  // Constraint Engine methods
  PSVariable* PSEngineImpl::getVariableByKey(PSEntityKey id)
  {
    check_runtime_error(isStarted(),"PSEngine has not been started");
	return getConstraintEngine()->getVariableByKey(id);  
  }

  PSVariable* PSEngineImpl::getVariableByName(const std::string& name)
  {
    check_runtime_error(isStarted(),"PSEngine has not been started");
	return getConstraintEngine()->getVariableByName(name);  
  }

  // TODO: these 2 need to be pushed into the PSConstraintEngine
  PSConstraint* PSEngineImpl::addConstraint(const std::string& type, PSList<PSVariable*> args)
  {
      std::vector<ConstrainedVariableId> variables;
      for (int i=0;i<args.size();i++) {
          ConstrainedVariableId arg = getPlanDatabase()->getEntityByKey(args.get(i)->getKey());
          variables.push_back(arg);
      }
      
      ConstraintId c = getPlanDatabase()->getClient()->createConstraint(type.c_str(), variables);

      // TODO: this must be pushed to the CE
      if (getAutoPropagation())
          propagate();
      
      return (Constraint*)c;      
  }
  
  void PSEngineImpl::removeConstraint(PSEntityKey id)
  {
      ConstraintId c = getPlanDatabase()->getEntityByKey(id);
      getPlanDatabase()->getClient()->deleteConstraint(c);      
      // TODO: this must be pushed to the CE
      if (getAutoPropagation())
          propagate();      
  }
  
  bool PSEngineImpl::getAutoPropagation() const
  {
    return getConstraintEnginePtr()->getAutoPropagation();
  }

  void PSEngineImpl::setAutoPropagation(bool v)      
  {
    getConstraintEngine()->setAutoPropagation(v);
  }

  bool PSEngineImpl::propagate() 
  {
    return getConstraintEngine()->propagate();
  }
    
  bool PSEngineImpl::getAllowViolations() const
  {
  	return getConstraintEnginePtr()->getAllowViolations();
  }

  void PSEngineImpl::setAllowViolations(bool v)
  {
    getConstraintEngine()->setAllowViolations(v);
  }

  double PSEngineImpl::getViolation() const
  {
  	return getConstraintEnginePtr()->getViolation();
  }
   
  std::string PSEngineImpl::getViolationExpl() const
  {
  	return getConstraintEnginePtr()->getViolationExpl();
  }

  // Solver methods
  PSSolver* PSEngineImpl::createSolver(const std::string& configurationFile) 
  {
    check_runtime_error(isStarted(),"PSEngine has not been started");
    return ((PSSolverManager*)getComponent("PSSolverManager"))->createSolver(configurationFile);
  }
}

