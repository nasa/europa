/*
 * PSEngine.cc
 *
 */

#include "PSEngineImpl.hh"

#include "LabelStr.hh"
#include "Pdlfcn.hh"
#include "Schema.hh"
#include "Rule.hh"

#include "Constraint.hh"
#include "PlanDatabase.hh"
#include "PSSolversImpl.hh"

namespace EUROPA {

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

  void PSEngineImpl::start()
  {
	  doStart();
  }

  void PSEngineImpl::shutdown()
  {
	  doShutdown();
  }

  EngineConfig* PSEngineImpl::getConfig()
  {
      return EuropaEngine::getConfig();
  }

  void PSEngineImpl::addModule(Module* module)
  {
	  EuropaEngine::addModule(module->getId());
  }
  void PSEngineImpl::removeModule(Module* module)
  {
	  EuropaEngine::removeModule(module->getId());
  }
  void PSEngineImpl::loadModule(const std::string& moduleFileName)
  {
	  EuropaEngine::loadModule(moduleFileName);
  }

  void PSEngineImpl::loadModel(const std::string& modelFileName)
   {
	   check_runtime_error(isStarted(),"PSEngine has not been started");

	   void* libHandle = p_dlopen(modelFileName.c_str(), RTLD_NOW);
	   checkRuntimeError(libHandle != NULL,
			   "Error opening model " << modelFileName << ": " << p_dlerror());

	   SchemaId (*fcn_schema)(const SchemaId&,const RuleSchemaId&);
	   fcn_schema = (SchemaId (*)(const SchemaId&,const RuleSchemaId&)) p_dlsym(libHandle, "loadSchema");
	   checkError(fcn_schema != NULL,
			   "Error locating symbol 'loadSchema' in " << modelFileName << ": " <<
			   p_dlerror());

	   SchemaId schema = ((Schema*)getComponent("Schema"))->getId();
       RuleSchemaId ruleSchema = ((RuleSchema*)getComponent("RuleSchema"))->getId();
	   (*fcn_schema)(schema,ruleSchema);
   }

  std::string PSEngineImpl::executeScript(const std::string& language, const std::string& script, bool isFile)
  {
    return EuropaEngine::executeScript(language,script,isFile);
  }

  // Plan Database methods
  PSList<PSObject*> PSEngineImpl::getObjects() {
    check_runtime_error(isStarted(), "PSEngine has not been started");
    return getPlanDatabase()->getAllObjects();
  }

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

  PSSchema* PSEngineImpl::getPSSchema()
  {
	  return getPlanDatabase()->getSchema();
  }


  PSPlanDatabaseClient* PSEngineImpl::getPlanDatabaseClient()
  {
     check_runtime_error(isStarted(),"PSEngine has not been started");
     return getPlanDatabase()->getPDBClient();
  }

  void PSEngineImpl::addPlanDatabaseListener(PSPlanDatabaseListener& listener)
  {
      check_runtime_error(isStarted(),"PSEngine has not been started");
      listener.setPlanDatabase(EuropaEngine::getPlanDatabase());
  }

  void PSEngineImpl::addConstraintEngineListener(PSConstraintEngineListener& listener)
  {
      check_runtime_error(isStarted(),"PSEngine has not been started");
      listener.setConstraintEngine(EuropaEngine::getConstraintEngine());
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

  PSList<std::string> PSEngineImpl::getViolationExpl() const
  {
  	return getConstraintEnginePtr()->getViolationExpl();
  }

  PSList<PSConstraint*> PSEngineImpl::getAllViolations() const
  {
	  return getConstraintEnginePtr()->getAllViolations();
  }

  // Solver methods
  PSSolver* PSEngineImpl::createSolver(const std::string& configurationFile)
  {
    check_runtime_error(isStarted(),"PSEngine has not been started");
    return ((PSSolverManager*)getComponent("PSSolverManager"))->createSolver(configurationFile);
  }

}

