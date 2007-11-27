/*
 * PSEngine.cc
 *
 */
 
#include "PSEngineImpl.hh"
#include "ConstrainedVariable.hh"
#include "Entity.hh"
#include "LabelStr.hh"
#include "Object.hh"
#include "Token.hh"
#include "TokenVariable.hh"
#include "IntervalDomain.hh"
#include "EntityIterator.hh"
#include "Solver.hh"
#include "ConstraintEngine.hh"
#include "PlanDatabase.hh"
#include "RulesEngine.hh"
#include "Pdlfcn.hh"
#include "Schema.hh"
#include "Constraints.hh"
#include "NddlDefs.hh"
#include "ObjectFactory.hh"
#include "TokenFactory.hh"
#include "ConstraintLibrary.hh"
#include "Rule.hh"
#include "DbClient.hh"
#include "Filters.hh"
#include "DefaultPropagator.hh"
#include "TemporalPropagator.hh"
#include "STNTemporalAdvisor.hh"
#include "PlanDatabaseWriter.hh"
#include "SolverPartialPlanWriter.hh"

#include "ModuleConstraintEngine.hh"
#include "ModulePlanDatabase.hh"
#include "ModuleRulesEngine.hh"
#include "ModuleTemporalNetwork.hh"
//#include "ModuleResource.hh"
#include "ModuleSolvers.hh"
#include "ModuleNddl.hh"
//#include "ModuleAnml.hh"

#include <fstream>

namespace EUROPA {
  
  class BaseObjectWrapperGenerator : public ObjectWrapperGenerator 
  {
    public:
	  PSObject* wrap(const ObjectId& obj) {
		  return new PSObjectImpl(obj);
	  }
  };

  // TODO : allow user to register factories through configuration. dynamically load them.
  PSEngine* PSEngine::makeInstance()
  {
	  return new PSEngineImpl();
  }

  std::map<double, ObjectWrapperGenerator*>& PSEngineImpl::getObjectWrapperGenerators()
  {
      return m_objectWrapperGenerators;
  }
  
  std::map<double, LanguageInterpreter*>& PSEngineImpl::getLanguageInterpreters()
  {
      return m_languageInterpreters;
  }


  PSEngineImpl::PSEngineImpl() 
      : m_ppw(NULL)
  {
  }

  PSEngineImpl::~PSEngineImpl() 
  {
  }

  ModuleId PSEngineImpl::getModuleByName(const std::string& name) const
  {
	  for (unsigned int i=0;i<m_modules.size();i++) {
		  if (m_modules[i]->getName() == name)
			  return m_modules[i];
	  }
	  
	  return ModuleId::noId();
  }
  
  void PSEngineImpl::createModules()
  {
	  // TODO: make this data-driven
	  m_modules.push_back(new ModuleConstraintEngine()); 
	  m_modules.push_back(new ModuleConstraintLibrary());
	  m_modules.push_back(new ModulePlanDatabase());
	  m_modules.push_back(new ModuleRulesEngine());
	  m_modules.push_back(new ModuleTemporalNetwork());
      // TODO:	  m_modules.push_back(new ModuleResource());
	  m_modules.push_back(new ModuleSolvers());
	  m_modules.push_back(new ModuleNddl());	  
  }
  
  void PSEngineImpl::initializeModules()
  {
	  createModules();
	  
	  for (unsigned int i=0;i<m_modules.size();i++) {
		  m_modules[i]->initialize();
		  debugMsg("PSEngine","Initialized Module " << m_modules[i]->getName());		  
	  }	  
  }

  void PSEngineImpl::uninitializeModules()
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
  
  void PSEngineImpl::allocateComponents()
  {
	  m_constraintEngine = (new ConstraintEngine())->getId();	  
      m_planDatabase = (new PlanDatabase(m_constraintEngine, Schema::instance()))->getId();	
      m_rulesEngine = (new RulesEngine(m_planDatabase))->getId();	  
	  m_ppw = new SOLVERS::PlanWriter::PartialPlanWriter(m_planDatabase, m_constraintEngine, m_rulesEngine);

	  for (unsigned int i=0;i<m_modules.size();i++) {
		  m_modules[i]->initialize(getId());
		  debugMsg("PSEngine","Engine initialized by Module " << m_modules[i]->getName());		  
	  }	  	  
	  
	  // TODO: This needs to be done with a LanguageInterpreter for nddl-xml
      DbClientId client = m_planDatabase->getClient();
	  m_interpTransactionPlayer = new InterpretedDbClientTransactionPlayer(client);
	  m_transactionPlayer = new DbClientTransactionPlayer(client);
  }
  
  void PSEngineImpl::deallocateComponents()
  {
	  for (unsigned int i=m_modules.size();i>0;i--) {
		  unsigned int idx = i-1;
		  m_modules[idx]->uninitialize(getId());
		  debugMsg("PSEngine","Engine uninitialized by Module " << m_modules[idx]->getName());		  
	  }	  

	  Entity::purgeStarted();
      
      if(m_ppw != NULL) {
	     delete m_ppw;
	     m_ppw = NULL;
	  }

      if(m_rulesEngine.isValid()) {
	    delete (RulesEngine*) m_rulesEngine;
	    m_rulesEngine = RulesEngineId::noId();    
	  }
      
	  if(m_planDatabase.isValid()) {
	    delete (PlanDatabase*) m_planDatabase;
	    m_planDatabase = PlanDatabaseId::noId();
	  }
	  
	  if(m_constraintEngine.isValid()) {
	    delete (ConstraintEngine*) m_constraintEngine;
	    m_constraintEngine = ConstraintEngineId::noId();
	  }	  

	  Entity::purgeEnded();	  
  }
  
  EngineComponentId& PSEngineImpl::getComponent(const std::string& name)
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
  
  
  void PSEngineImpl::start() 
  {		
    Error::doThrowExceptions(); // throw exceptions!
    Error::doDisplayErrors();    
    check_runtime_error(m_constraintEngine.isNoId());
    check_runtime_error(m_planDatabase.isNoId());
    check_runtime_error(m_rulesEngine.isNoId());
    
    initializeModules();
    allocateComponents();   
    registerObjectWrappers();
  }

  void PSEngineImpl::registerObjectWrappers()
  {
      addObjectWrapperGenerator("Object", new BaseObjectWrapperGenerator());	  
  }
  
  //FIXME
  void PSEngineImpl::shutdown() 
  {
    deallocateComponents();    
    uninitializeModules();    
  }
    
  void PSEngineImpl::loadModel(const std::string& modelFileName) {
    void* libHandle = p_dlopen(modelFileName.c_str(), RTLD_NOW);
    checkError(libHandle != NULL,
	       "Error opening model " << modelFileName << ": " << p_dlerror());

    SchemaId (*fcn_schema)();
    fcn_schema = (SchemaId (*)()) p_dlsym(libHandle, "loadSchema");
    checkError(fcn_schema != NULL,
	       "Error locating symbol 'loadSchema' in " << modelFileName << ": " <<
	       p_dlerror());

    SchemaId schema = (*fcn_schema)();
    // TODO: need to make sure any previous initialization to Schema is replayed again, since the generated code calls Schema::reset()
    //  possibly the best thing to do is to change the generated code
  }

  void PSEngineImpl::executeTxns(const std::string& xmlTxnSource,bool isFile,bool useInterpreter) 
  {
    check_runtime_error(m_planDatabase.isValid());
    
    DbClientTransactionPlayerId player;      
    if (useInterpreter)
        player = m_interpTransactionPlayer;
    else
        player = m_transactionPlayer;

    std::istream* in;    
    if (isFile)
      in = new std::ifstream(xmlTxnSource.c_str());
    else
      in = new std::istringstream(xmlTxnSource);

    //check_runtime_error(*in);
    player->play(*in);

    delete in;    
  }

  //FIXME
  std::string PSEngineImpl::executeScript(const std::string& language, const std::string& script) {
    std::map<double, LanguageInterpreter*>::iterator it =
      getLanguageInterpreters().find(LabelStr(language));
    checkRuntimeError(it != getLanguageInterpreters().end(),
		      "Cannot execute script of unknown language \"" << language << "\"");
    return it->second->interpret(script);
  }

  PSList<PSObject*> PSEngineImpl::getObjectsByType(const std::string& objectType) {
    check_runtime_error(m_planDatabase.isValid());
    
    PSList<PSObject*> retval;
    
    const ObjectSet& objects = m_planDatabase->getObjects();
    for(ObjectSet::const_iterator it = objects.begin(); it != objects.end(); ++it){
      ObjectId object = *it;
      if(Schema::instance()->isA(object->getType(), objectType.c_str()))
	retval.push_back(getObjectWrapperGenerator(object->getType())->wrap(object));
    }
    
    return retval;
  }

  PSObject* PSEngineImpl::getObjectByKey(PSEntityKey id) {
    check_runtime_error(m_planDatabase.isValid());

    EntityId entity = Entity::getEntity(id);
    check_runtime_error(entity.isValid());
    return new PSObjectImpl(entity);
  }

  PSObject* PSEngineImpl::getObjectByName(const std::string& name) {
    check_runtime_error(m_planDatabase.isValid());
    ObjectId obj = m_planDatabase->getObject(LabelStr(name));
    check_runtime_error(obj.isValid());
    return new PSObjectImpl(obj);
  }

  PSList<PSToken*> PSEngineImpl::getTokens() {
    check_runtime_error(m_planDatabase.isValid());

    const TokenSet& tokens = m_planDatabase->getTokens();
    PSList<PSToken*> retval;

    for(TokenSet::const_iterator it = tokens.begin(); it != tokens.end(); ++it) {
      PSToken* tok = new PSTokenImpl(*it);
      retval.push_back(tok);
    }
    return retval;
  }

  PSToken* PSEngineImpl::getTokenByKey(PSEntityKey id) {
    check_runtime_error(m_planDatabase.isValid());

    EntityId entity = Entity::getEntity(id);
    check_runtime_error(entity.isValid());
    return new PSTokenImpl(entity);
  }

  PSList<PSVariable*>  PSEngineImpl::getGlobalVariables() {
    check_runtime_error(m_planDatabase.isValid());

    const ConstrainedVariableSet& vars = m_planDatabase->getGlobalVariables();
    PSList<PSVariable*> retval;

    for(ConstrainedVariableSet::const_iterator it = vars.begin(); it != vars.end(); ++it) {
      PSVariable* v = new PSVariableImpl(*it);
      retval.push_back(v);
    }
    return retval;
  }  

  PSVariable* PSEngineImpl::getVariableByKey(PSEntityKey id)
  {
    check_runtime_error(m_planDatabase.isValid());
    EntityId entity = Entity::getEntity(id);
    check_runtime_error(entity.isValid());
    return new PSVariableImpl(entity);
  }

  // TODO: this needs to be optimized
  PSVariable* PSEngineImpl::getVariableByName(const std::string& name)
  {
    check_runtime_error(m_planDatabase.isValid());
    const ConstrainedVariableSet& vars = m_constraintEngine->getVariables();

    for(ConstrainedVariableSet::const_iterator it = vars.begin(); it != vars.end(); ++it) {
    	ConstrainedVariableId v = *it;
    	if (v->getName().toString() == name)
            return new PSVariableImpl(*it);
    }
    
    return NULL;
  }

  PSSolver* PSEngineImpl::createSolver(const std::string& configurationFile) {
    TiXmlDocument* doc = new TiXmlDocument(configurationFile.c_str());
    doc->LoadFile();

    SOLVERS::SolverId solver =
      (new SOLVERS::Solver(m_planDatabase, *(doc->RootElement())))->getId();
    return new PSSolverImpl(solver,configurationFile, m_ppw);
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
   
   std::string PSEngineImpl::planDatabaseToString() {
      PlanDatabaseWriter* pdw = new PlanDatabaseWriter();
      std::string planOutput = pdw->toString(m_planDatabase);
      delete pdw;
      return planOutput;
   }

  void PSEngineImpl::addLanguageInterpreter(const std::string& language, LanguageInterpreter* interpreter) 
  {
    std::map<double, LanguageInterpreter*>::iterator it = getLanguageInterpreters().find(LabelStr(language));
    if(it == getLanguageInterpreters().end())
      getLanguageInterpreters().insert(std::make_pair(LabelStr(language), interpreter));
    else {
      delete it->second;
      it->second = interpreter;
    }
  }

  void PSEngineImpl::removeLanguageInterpreter(const std::string& language) 
  {
    std::map<double, LanguageInterpreter*>::iterator it = getLanguageInterpreters().find(LabelStr(language));
    if(it != getLanguageInterpreters().end()) {
      delete it->second;
      getLanguageInterpreters().erase(it);
    }
  }
  
  void PSEngineImpl::addObjectWrapperGenerator(const LabelStr& type,
					   ObjectWrapperGenerator* wrapper) {
    std::map<double, ObjectWrapperGenerator*>::iterator it =
      getObjectWrapperGenerators().find(type);
    if(it == getObjectWrapperGenerators().end())
      getObjectWrapperGenerators().insert(std::make_pair(type, wrapper));
    else {
      delete it->second;
      it->second = wrapper;
    }
  }

  ObjectWrapperGenerator* PSEngineImpl::getObjectWrapperGenerator(const LabelStr& type) {
    const std::vector<LabelStr>& types = Schema::instance()->getAllObjectTypes(type);
    for(std::vector<LabelStr>::const_iterator it = types.begin(); it != types.end(); ++it) {
      std::map<double, ObjectWrapperGenerator*>::iterator wrapper =
	getObjectWrapperGenerators().find(*it);
      if(wrapper != getObjectWrapperGenerators().end())
	return wrapper->second;
    }
    checkRuntimeError(ALWAYS_FAIL,
		      "Don't know how to wrap objects of type " << type.toString());
    return NULL;
  }

  PSEntity::PSEntity(const EntityId& entity) : m_entity(entity) {}

  PSEntityKey PSEntity::getKey() const {return PSEntityKey(m_entity->getKey());}
  
  const std::string& PSEntity::getName() const {return m_entity->getName().toString();}

  const std::string UNKNOWN("UNKNOWN");
  
  const std::string& PSEntity::getEntityType() const {return UNKNOWN;}
  
  std::string PSEntity::toString()
  {
  	std::ostringstream os;
  	
  	os << "Entity(" << getKey() << "," << getName() << ")";
  	return os.str();
  }

  PSObjectImpl::PSObjectImpl(const ObjectId& obj) : PSObject(obj), m_obj(obj) {
  }

  PSObjectImpl::~PSObjectImpl() {
  }

  PSList<PSVariable*> PSObjectImpl::getMemberVariables() {
    PSList<PSVariable*> retval;
    const std::vector<ConstrainedVariableId>& vars = m_obj->getVariables();
    for(std::vector<ConstrainedVariableId>::const_iterator it = vars.begin(); it != vars.end();
	++it) {
      PSVariable* var = new PSVariableImpl(*it); 
      check_runtime_error(var != NULL);
      retval.push_back(var);
    }

    return retval;
  }

  const std::string OBJECT_STR("OBJECT");
  const std::string& PSObjectImpl::getEntityType() const 
  {
  	return OBJECT_STR;
  }

  std::string PSObjectImpl::getObjectType() const 
  {
  	return m_obj->getType().toString();
  }

  PSVariable* PSObjectImpl::getMemberVariable(const std::string& name) {
    LabelStr realName(name);
    PSVariable* retval = NULL;
    const std::vector<ConstrainedVariableId>& vars = m_obj->getVariables();
    for(std::vector<ConstrainedVariableId>::const_iterator it = vars.begin(); it != vars.end();
	++it) {
      if((*it)->getName() == realName) {
	retval = new PSVariableImpl(*it);
	break;
      }
    }
    return retval;
  }

  PSList<PSToken*> PSObjectImpl::getTokens() {
    PSList<PSToken*> retval;
    const TokenSet& tokens = m_obj->getTokens();
    for(TokenSet::const_iterator it = tokens.begin(); it != tokens.end(); ++it) {
      PSToken* tok = new PSTokenImpl(*it);
      check_runtime_error(tok != NULL);
      retval.push_back(tok);
    }
    return retval;
  }


  void PSObjectImpl::addPrecedence(PSToken* pred,PSToken* succ)
  {
	  TokenId p = m_obj->getPlanDatabase()->getEntityByKey(pred->getKey());
	  TokenId s = m_obj->getPlanDatabase()->getEntityByKey(succ->getKey());
	  m_obj->constrain(p,s);
	  // TODO: this needs to be done on demand from outside
	  m_obj->getPlanDatabase()->getConstraintEngine()->propagate();
  }
  
  void PSObjectImpl::removePrecedence(PSToken* pred,PSToken* succ)
  {
	  TokenId p = m_obj->getPlanDatabase()->getEntityByKey(pred->getKey());
	  TokenId s = m_obj->getPlanDatabase()->getEntityByKey(succ->getKey());
	  m_obj->free(p,s);	  
	  // TODO: this needs to be done on demand from outside
	  m_obj->getPlanDatabase()->getConstraintEngine()->propagate();
  }

  
  PSTokenImpl::PSTokenImpl(const TokenId& tok) : PSToken(tok), m_tok(tok) {
  }

  const std::string TOKEN_STR("TOKEN");
  const std::string& PSTokenImpl::getEntityType() const 
  {
  	return TOKEN_STR;
  }

  std::string PSTokenImpl::getTokenType() const 
  {
  	return m_tok->getUnqualifiedPredicateName().toString();
  }

  PSObject* PSTokenImpl::getOwner() {
    if(!m_tok->isAssigned())
      return NULL;
      
    ObjectVarId objVar = m_tok->getObject();
    return new PSObjectImpl(ObjectId(objVar->lastDomain().getSingletonValue()));
  }
  
  PSToken* PSTokenImpl::getMaster() {
  	TokenId master = m_tok->getMaster();
  	if (master.isNoId())
  	    return NULL;
  	
  	return new PSTokenImpl(master);    
  }
  
  PSList<PSToken*> PSTokenImpl::getSlaves() {
    const TokenSet& tokens = m_tok->getSlaves();
    PSList<PSToken*> retval;

    for(TokenSet::const_iterator it = tokens.begin(); it != tokens.end(); ++it) {
      PSToken* tok = new PSTokenImpl(*it);
      retval.push_back(tok);
    }
    return retval;    	
  }  

  // TODO: Implement these
  double PSTokenImpl::getViolation() const 
  {
	  return m_tok->getViolation();
  }
  
  std::string PSTokenImpl::getViolationExpl() const 
  { 
	  return m_tok->getViolationExpl();
  }

  PSList<PSVariable*> PSTokenImpl::getParameters() {
    PSList<PSVariable*> retval;
    const std::vector<ConstrainedVariableId>& vars = m_tok->getVariables();
    for(std::vector<ConstrainedVariableId>::const_iterator it = vars.begin(); it != vars.end();
	++it) {
      PSVariable* var = new PSVariableImpl(*it);
      check_runtime_error(var != NULL);
      retval.push_back(var);
    }
    return retval;
  }

  PSVariable* PSTokenImpl::getParameter(const std::string& name) {
    LabelStr realName(name);
    PSVariable* retval = NULL;
    const std::vector<ConstrainedVariableId>& vars = m_tok->getVariables();
    for(std::vector<ConstrainedVariableId>::const_iterator it = vars.begin(); it != vars.end();
	++it) {
      if((*it)->getName() == realName) {
	retval = new PSVariableImpl(*it);
	break;
      }
    }
    return retval;
  }

  bool PSTokenImpl::isFact()
  {
  	return m_tok->isFact();
  }
  
  std::string PSTokenImpl::toString()
  {
  	std::ostringstream os;
  	
  	os << "Token(" << PSEntity::toString() << ") {" << std::endl;
  	os << "    isFact:" << isFact() << std::endl;
  	
  	if (m_tok->isMerged())
  	    os << "    mergedInto:" << m_tok->getActiveToken()->getKey() << std::endl;

	PSList<PSVariable*> vars = getParameters();
  	for (int i=0;i<vars.size();i++) {
  	    os << "    " << vars.get(i)->toString() << std::endl;
	    delete vars.get(i);
  	}
  	
  	os << "}" << std::endl;
  	
  	return os.str();
  }


  PSVariableImpl::PSVariableImpl(const ConstrainedVariableId& var) : PSVariable(var), m_var(var) {
    check_runtime_error(m_var.isValid());
    if(m_var->baseDomain().isString())
      m_type =  STRING;
    else if(m_var->baseDomain().isSymbolic()) {
      if(m_var->baseDomain().isEmpty() || LabelStr::isString(m_var->baseDomain().getLowerBound()))
	m_type =  STRING;
      else
	m_type =  OBJECT; //this may not be the best assumption ~MJI
    }
    else if(m_var->baseDomain().isBool())
      m_type =  BOOLEAN;
    else if(m_var->baseDomain().isNumeric()) {
      if(m_var->baseDomain().minDelta() < 1)
	m_type =  DOUBLE;
      else
	m_type =  INTEGER;
    }
    else {
      checkError(ALWAYS_FAIL, "Failed to correctly determine the type of " << var->toString());
    }
  }

  const std::string VARIABLE_STR("VARIABLE");
  const std::string& PSVariableImpl::getEntityType() const 
  {
  	return VARIABLE_STR;
  }
  
  bool PSVariableImpl::isEnumerated() {
    check_runtime_error(m_var.isValid());
    return m_var->baseDomain().isEnumerated();
  }

  bool PSVariableImpl::isInterval() {
    check_runtime_error(m_var.isValid());
    return m_var->baseDomain().isInterval();
  }
  
  PSVarType PSVariableImpl::getType() {
    check_runtime_error(m_var.isValid());
    return m_type;
  }

  bool PSVariableImpl::isNull() {
    check_runtime_error(m_var.isValid());
    return m_var->lastDomain().isEmpty() && !m_var->isSpecified();
  }

  bool PSVariableImpl::isSingleton() {
    check_runtime_error(m_var.isValid());
    return m_var->isSpecified() || m_var->lastDomain().isSingleton();
  }

  PSVarValue PSVariableImpl::getSingletonValue() {
    check_runtime_error(m_var.isValid());
    check_runtime_error(isSingleton());
    
    if (m_var->isSpecified())
      return PSVarValue(m_var->getSpecifiedValue(), getType());
    else
      return PSVarValue(m_var->lastDomain().getSingletonValue(), getType());
  }

  PSList<PSVarValue> PSVariableImpl::getValues() {
    check_runtime_error(m_var.isValid());
    check_runtime_error(!isSingleton() && isEnumerated());
    PSList<PSVarValue> retval;
    std::list<double> values;
    m_var->lastDomain().getValues(values);
    PSVarType type = getType();

    for(std::list<double>::const_iterator it = values.begin(); it != values.end(); ++it) {
      PSVarValue value(*it, type);
      retval.push_back(value);
    }
    return retval;
  }


  double PSVariableImpl::getLowerBound() {
    check_runtime_error(m_var.isValid());
    check_runtime_error(isInterval());
    return m_var->lastDomain().getLowerBound();
  }

  double PSVariableImpl::getUpperBound() {
    check_runtime_error(m_var.isValid());
    check_runtime_error(isInterval());
    return m_var->lastDomain().getUpperBound();
  }

  void PSVariableImpl::specifyValue(PSVarValue& v) {
    check_runtime_error(m_var.isValid());
    check_runtime_error(getType() == v.getType());

    debugMsg("PSVariable:specify","Specifying var:" << m_var->toString() << " to value:" << v.toString());
    
    // If specifying to the same value it already has, do nothing
    if (m_var->isSpecified() && (m_var->getSpecifiedValue() == v.asDouble())) {
        debugMsg("PSVariable:specify","Tried to specify to same value, so bailing out without doing any work");
        return;
    }
    else {
      // TODO: reset is only strictly necessary if the new value would conlict with the current domain	
      m_var->reset();
      debugMsg("PSVariable:specify","After reset for var:" << m_var->toString());
      //only propagate if we aren't allowing violations
      //this can save us an extra call to propagate()
      if(!m_var->getConstraintEngine()->getAllowViolations()) {
          m_var->getConstraintEngine()->propagate();
          debugMsg("PSVariable:specify","After propagate for var:" << m_var->toString());
      }
    }
        
    m_var->specify(v.asDouble());
    debugMsg("PSVariable:specify","After specify for var:" << m_var->toString() << " to value:" << v.toString());
    m_var->getConstraintEngine()->propagate();
    debugMsg("PSVariable:specify","After propagate for var:" << m_var->toString());
  }

  void PSVariableImpl::reset() {
    check_runtime_error(m_var.isValid());
    debugMsg("PSVariable:reset",
	     "Re-setting " << m_var->toString());
    m_var->reset();
    m_var->getConstraintEngine()->propagate();
  }

  double PSVariableImpl::getViolation() const
  {
    check_runtime_error(m_var.isValid());
    return m_var->getViolation();
  }
  
  std::string PSVariableImpl::getViolationExpl() const 
  { 
    check_runtime_error(m_var.isValid());
    return m_var->getViolationExpl();
  }
  
  PSEntity* PSVariableImpl::getParent() {
    EntityId parent(m_var->getParent());
    if(parent.isNoId())
      return NULL;
    else if(TokenId::convertable(parent))
      return new PSTokenImpl((TokenId) parent);
    else if(ObjectId::convertable(parent))
      return new PSObjectImpl((ObjectId) parent);
    else if(RuleInstanceId::convertable(parent))
      return new PSTokenImpl(((RuleInstanceId)parent)->getToken());
    else {
      checkRuntimeError(ALWAYS_FAIL,
			"Variable " << toString() << " has a parent that isn't a token, " <<
			"object, or rule: " << m_var->getParent()->toString());
    }
    return NULL;
  }

  std::string PSVariableImpl::toString() {
    check_runtime_error(m_var.isValid());
    std::ostringstream os;
    
    if (isNull())
        os << "NULL";
    else if (isSingleton()) 
    	os << getSingletonValue().toString();    	    
    else if (isInterval()) 
        os << "[" << getLowerBound() << "," << getUpperBound() << "]";
    else if (isEnumerated()) {
    	os << "{";
    	PSList<PSVarValue> values = getValues();
    	for (int i=0;i<values.size();i++) {
    		if (i > 0)
    		    os << ", ";
    		os << values.get(i).toString();    
    	}
    	os << "}";
    }
    else 
        os << "ERROR!";    
    
    return os.str();
  }

  PSVarValue::PSVarValue(const double val, const PSVarType type) : m_val(val), m_type(type) {}

  PSVarType PSVarValue::getType() const {return m_type;}
  
  PSObject* PSVarValue::asObject() const {
    check_runtime_error(m_type == OBJECT);
    return new PSObjectImpl(ObjectId(m_val));
  }

  int PSVarValue::asInt() const {check_runtime_error(m_type == INTEGER); return (int) m_val;}
  
  double PSVarValue::asDouble() const {return m_val;}

  bool PSVarValue::asBoolean() const {check_runtime_error(m_type == BOOLEAN); return (bool) m_val;}

  const std::string& PSVarValue::asString() const {
    check_runtime_error(m_type == STRING);
    return LabelStr(m_val).toString();
  }

  std::string PSVarValue::toString() const {
  	std::ostringstream os;
  	
  	switch (m_type) {
  		case INTEGER:
            os << asInt();
  		    break;
  		case DOUBLE:
            os << asDouble();
  		    break;
  		case BOOLEAN:
            os << asBoolean();
  		    break;
  		case STRING:
            os << asString();
  		    break;
  		case OBJECT:
  		    {
  		        PSObject* obj = asObject();
                os << "OBJECT:" << obj->getName() << "(" << obj->getKey() << ")";
                delete obj;
  		    }
  		    break;
  		
  		default:
  		    check_error(ALWAYS_FAILS, "Unknown type");    
  	}
  	  	
  	return os.str();
  }
   
  PSSolverImpl::PSSolverImpl(const SOLVERS::SolverId& solver, const std::string& configFilename,
		     SOLVERS::PlanWriter::PartialPlanWriter* ppw) 
      : m_solver(solver) 
      , m_configFile(configFilename),
	m_ppw(ppw)
  {
    m_ppw->setSolver(m_solver);
  }

  PSSolverImpl::~PSSolverImpl() {
    if(m_solver.isValid())
      destroy();
  }

  void PSSolverImpl::step() {
    m_solver->step();
  }

  void PSSolverImpl::solve(int maxSteps, int maxDepth) {
    m_solver->solve(maxSteps, maxDepth);
  }

  void PSSolverImpl::reset() {
    m_solver->reset();
  }

  void PSSolverImpl::destroy() {
    m_ppw->clearSolver();
    delete (SOLVERS::Solver*) m_solver;
    m_solver = SOLVERS::SolverId::noId();
  }

  int PSSolverImpl::getStepCount() {
    return (int) m_solver->getStepCount();
  }

  int PSSolverImpl::getDepth() {
    return (int) m_solver->getDepth();
  }

  bool PSSolverImpl::isExhausted() {
    return m_solver->isExhausted();
  }

  bool PSSolverImpl::isTimedOut() {
    return m_solver->isTimedOut();
  }

  bool PSSolverImpl::isConstraintConsistent() {
    return m_solver->isConstraintConsistent();
  }

  bool PSSolverImpl::hasFlaws() {
    return !m_solver->noMoreFlaws();
  }

  int PSSolverImpl::getOpenDecisionCnt() {
    int count = 0;
    IteratorId flawIt = m_solver->createIterator();
    while(!flawIt->done()) {
      count++;
      flawIt->next();
    }
    delete (Iterator*) flawIt;
    return count;
  }

  PSList<std::string> PSSolverImpl::getFlaws() {
    PSList<std::string> retval;

    /*    
    IteratorId flawIt = m_solver->createIterator();
    while(!flawIt->done()) {
      EntityId entity = flawIt->next();
      std::string flaw = entity->toString();
      retval.push_back(flaw);
    }
    delete (Iterator*) flawIt;
    */  

    std::multimap<SOLVERS::Priority, std::string> priorityQueue = m_solver->getOpenDecisions();
    for(std::multimap<SOLVERS::Priority, std::string>::const_iterator it=priorityQueue.begin();it!=priorityQueue.end(); ++it) {
        std::stringstream os;      
        os << it->second << " PRIORITY==" << it->first; 
        retval.push_back(os.str());
    }
    
    return retval;
  }

  std::string PSSolverImpl::getLastExecutedDecision() {
    return m_solver->getLastExecutedDecision();
  }

  const std::string& PSSolverImpl::getConfigFilename() {return m_configFile;}

  int PSSolverImpl::getHorizonStart() {
    return (int) SOLVERS::HorizonFilter::getHorizon().getLowerBound();
  }

  int PSSolverImpl::getHorizonEnd() {
    return (int) SOLVERS::HorizonFilter::getHorizon().getUpperBound();
  }

  void PSSolverImpl::configure(int horizonStart, int horizonEnd) {
    check_runtime_error(horizonStart <= horizonEnd);
    SOLVERS::HorizonFilter::getHorizon().reset(IntervalIntDomain());
    SOLVERS::HorizonFilter::getHorizon().intersect(horizonStart, horizonEnd);
  }
}
