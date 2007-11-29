/*
 * PSEngine.cc
 *
 */
 
#include "PSEngineImpl.hh"

// Utils
#include "LabelStr.hh"
#include "Pdlfcn.hh"

// ConstraintEngine
#include "PSConstraintEngineImpl.hh"
#include "ConstraintEngine.hh"
#include "ConstrainedVariable.hh"

// PlanDatabase
#include "PSPlanDatabaseImpl.hh"
#include "Schema.hh"
#include "PlanDatabase.hh"
#include "Object.hh"
#include "Token.hh"
#include "TokenVariable.hh"
#include "EntityIterator.hh"

// RulesEngine
#include "Rule.hh"

// Solver
#include "PSSolversImpl.hh"
#include "PlanDatabaseWriter.hh"
#include "SolverPartialPlanWriter.hh"

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

  PSEngineImpl::PSEngineImpl() 
      : m_ppw(NULL)
  {
  }

  PSEngineImpl::~PSEngineImpl() 
  {
  }

  std::map<double, ObjectWrapperGenerator*>& PSEngineImpl::getObjectWrapperGenerators()
  {
      return m_objectWrapperGenerators;
  }
  
  void PSEngineImpl::allocateComponents()
  {
	  EngineBase::allocateComponents();
	  m_ppw = new SOLVERS::PlanWriter::PartialPlanWriter(m_planDatabase, m_constraintEngine, m_rulesEngine);
	  // TODO: This needs to be done with LanguageInterpreters for nddl-xml and nddl-xml-txn
      DbClientId client = m_planDatabase->getClient();
	  m_interpTransactionPlayer = new InterpretedDbClientTransactionPlayer(client);
	  m_transactionPlayer = new DbClientTransactionPlayer(client);	  
  }
  
  void PSEngineImpl::deallocateComponents()
  {
	  if(m_ppw != NULL) {
	     delete m_ppw;
	     m_ppw = NULL;
	  }
	  
	  if(m_interpTransactionPlayer.isValid()) 
	     m_interpTransactionPlayer.release();
	  
	  if(m_transactionPlayer.isValid()) 
	     m_transactionPlayer.release();

	  EngineBase::deallocateComponents();
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

    player->play(*in);

    delete in;    
  }

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
  
}
