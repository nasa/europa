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

// TODO: get rid of all the #ifdefs by providing a query language on the PlanDatabase
// so, for instance, ResourceProfiles can just be a query on a Resource object
#ifndef NO_RESOURCES
#include "PSResourceImpl.hh"
#include "SAVH_Resource.hh"
#endif

#include <fstream>

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
      , m_ppw(NULL)
  {
  }

  PSEngineImpl::~PSEngineImpl() 
  {
	  if (m_started)
		  shutdown();
  }

  void PSEngineImpl::initialize()
  {
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
	
    Error::doThrowExceptions(); // throw exceptions!
    Error::doDisplayErrors();    
    check_runtime_error(m_constraintEngine.isNoId());
    check_runtime_error(m_planDatabase.isNoId());
    check_runtime_error(m_rulesEngine.isNoId());
    
    allocateComponents();   
    registerObjectWrappers();
    m_started = true;
  }

  void PSEngineImpl::shutdown() 
  {
	if (!m_started)
		return;
	
    deallocateComponents();    
    m_started = false;
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
  
  void PSEngineImpl::loadModel(const std::string& modelFileName) {
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

  void PSEngineImpl::executeTxns(const std::string& xmlTxnSource,bool isFile,bool useInterpreter) 
  {
    check_runtime_error(m_started,"PSEngine has not been started");
    
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

  std::string PSEngineImpl::executeScript(const std::string& language, const std::string& script) 
  {
    std::map<double, LanguageInterpreter*>::iterator it =
      getLanguageInterpreters().find(LabelStr(language));
    checkRuntimeError(it != getLanguageInterpreters().end(),
		      "Cannot execute script of unknown language \"" << language << "\"");
    return it->second->interpret(script);
  }

  PSList<PSObject*> PSEngineImpl::getObjectsByType(const std::string& objectType) {
    check_runtime_error(m_started,"PSEngine has not been started");
    
    PSList<PSObject*> retval;
    
    const ObjectSet& objects = m_planDatabase->getObjects();
    for(ObjectSet::const_iterator it = objects.begin(); it != objects.end(); ++it){
      ObjectId object = *it;
      if(Schema::instance()->isA(object->getType(), objectType.c_str()))
	retval.push_back(getObjectWrapperGenerator(object->getType())->wrap(object));
    }
    
    return retval;
  }

  PSObject* PSEngineImpl::getObjectByKey(PSEntityKey id) 
  {
    check_runtime_error(m_started,"PSEngine has not been started");

    EntityId entity = Entity::getEntity(id);
    check_runtime_error(entity.isValid());
    return new PSObjectImpl(entity);
  }

  PSObject* PSEngineImpl::getObjectByName(const std::string& name) {
    check_runtime_error(m_started,"PSEngine has not been started");
    ObjectId obj = m_planDatabase->getObject(LabelStr(name));
    check_runtime_error(obj.isValid());
    return new PSObjectImpl(obj);
  }

  PSList<PSToken*> PSEngineImpl::getTokens() {
    check_runtime_error(m_started,"PSEngine has not been started");

    const TokenSet& tokens = m_planDatabase->getTokens();
    PSList<PSToken*> retval;

    for(TokenSet::const_iterator it = tokens.begin(); it != tokens.end(); ++it) {
      PSToken* tok = new PSTokenImpl(*it);
      retval.push_back(tok);
    }
    return retval;
  }

  PSToken* PSEngineImpl::getTokenByKey(PSEntityKey id) 
  {
    check_runtime_error(m_started,"PSEngine has not been started");

    EntityId entity = Entity::getEntity(id);
    check_runtime_error(entity.isValid());
    return new PSTokenImpl(entity);
  }

  PSList<PSVariable*>  PSEngineImpl::getGlobalVariables() {
    check_runtime_error(m_started,"PSEngine has not been started");

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
    check_runtime_error(m_started,"PSEngine has not been started");
    EntityId entity = Entity::getEntity(id);
    check_runtime_error(entity.isValid());
    return new PSVariableImpl(entity);
  }

  // TODO: this needs to be optimized
  PSVariable* PSEngineImpl::getVariableByName(const std::string& name)
  {
    check_runtime_error(m_started,"PSEngine has not been started");
    const ConstrainedVariableSet& vars = m_constraintEngine->getVariables();

    for(ConstrainedVariableSet::const_iterator it = vars.begin(); it != vars.end(); ++it) {
    	ConstrainedVariableId v = *it;
    	if (v->getName().toString() == name)
            return new PSVariableImpl(*it);
    }
    
    return NULL;
  }

  PSSolver* PSEngineImpl::createSolver(const std::string& configurationFile) 
  {
    check_runtime_error(m_started,"PSEngine has not been started");
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

   class BaseObjectWrapperGenerator : public ObjectWrapperGenerator 
   {
     public:
 	  PSObject* wrap(const ObjectId& obj) {
 		  return new PSObjectImpl(obj);
 	  }
   };

#ifndef NO_RESOURCES
   class ResourceWrapperGenerator : public ObjectWrapperGenerator 
   {
   public:
     PSObject* wrap(const ObjectId& obj) {
       checkRuntimeError(SAVH::ResourceId::convertable(obj),
 			"Object " << obj->toString() << " is not a resource.");
       return new PSResourceImpl(SAVH::ResourceId(obj));
     }
   }; 

   PSList<PSResource*> PSEngineImpl::getResourcesByType(const std::string& objectType) {
     check_runtime_error(m_started,"PSEngine has not been started");
     
     PSList<PSResource*> retval;
     
     const ObjectSet& objects = m_planDatabase->getObjects();
     for(ObjectSet::const_iterator it = objects.begin(); it != objects.end(); ++it){
       ObjectId object = *it;
       if(Schema::instance()->isA(object->getType(), objectType.c_str()))
 	    retval.push_back(dynamic_cast<PSResource*>(getObjectWrapperGenerator(object->getType())->wrap(object)));
     }
     
     return retval;
   }
   
   PSResource* PSEngineImpl::getResourceByKey(PSEntityKey id) {
     check_runtime_error(m_started,"PSEngine has not been started");

     EntityId entity = Entity::getEntity(id);
     check_runtime_error(entity.isValid());
     return new PSResourceImpl(entity);
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
   
   void PSEngineImpl::registerObjectWrappers()
   {
       addObjectWrapperGenerator("Object", new BaseObjectWrapperGenerator());	  
#ifndef NO_RESOURCES
       addObjectWrapperGenerator("Reservoir", new ResourceWrapperGenerator());
       addObjectWrapperGenerator("Reusable", new ResourceWrapperGenerator());
       addObjectWrapperGenerator("Unary", new ResourceWrapperGenerator());
#endif     
   }
   
  void PSEngineImpl::addObjectWrapperGenerator(const LabelStr& type,
					   ObjectWrapperGenerator* wrapper) {
    std::map<double, ObjectWrapperGenerator*>::iterator it =
      m_objectWrapperGenerators.find(type);
    if(it == m_objectWrapperGenerators.end())
      m_objectWrapperGenerators.insert(std::make_pair(type, wrapper));
    else {
      delete it->second;
      it->second = wrapper;
    }
  }

  ObjectWrapperGenerator* PSEngineImpl::getObjectWrapperGenerator(const LabelStr& type) {
    const std::vector<LabelStr>& types = Schema::instance()->getAllObjectTypes(type);
    for(std::vector<LabelStr>::const_iterator it = types.begin(); it != types.end(); ++it) {
      std::map<double, ObjectWrapperGenerator*>::iterator wrapper = m_objectWrapperGenerators.find(*it);
      if(wrapper != m_objectWrapperGenerators.end())
          return wrapper->second;
    }
    checkRuntimeError(ALWAYS_FAIL,"Don't know how to wrap objects of type " << type.toString());
    return NULL;
  }
  
}

