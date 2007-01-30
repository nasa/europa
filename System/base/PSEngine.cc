/*
 * PSEngine.cc
 *
 */
 
#include "PSEngine.hh"
#include "ConstrainedVariable.hh"
#include "Entity.hh"
#include "LabelStr.hh"
#include "Object.hh"
#include "Token.hh"
#include "TokenVariable.hh"
#include "SAVH_Resource.hh"
#include "SAVH_Profile.hh"
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
#include "DbClientTransactionPlayer.hh"
#include "TransactionInterpreter.hh"
#include "Filters.hh"
#include "DefaultPropagator.hh"
#include "TemporalPropagator.hh"
#include "SAVH_ProfilePropagator.hh"
#include "STNTemporalAdvisor.hh"
#include "ResourcePropagator.hh"

// TODO: registration for these needs to happen somewhere else
#include "SAVH_ReusableFVDetector.hh"
#include "SAVH_IncrementalFlowProfile.hh" 
#include "ResourceThreatDecisionPoint.hh"

#include <fstream>

namespace EUROPA {
  const std::string UNKNOWN("UNKNOWN");

  PSEntity::PSEntity(const EntityId& entity) : m_entity(entity) {}

  PSEntityKey PSEntity::getKey() const {return PSEntityKey(m_entity->getKey());}
  
  const std::string& PSEntity::getName() const {return m_entity->getName().toString();}

  //FIXME
  const std::string& PSEntity::getType() const {return UNKNOWN;}

  PSObject::PSObject(const ObjectId& obj) : PSEntity(obj), m_obj(obj) {
    const std::vector<ConstrainedVariableId>& vars = m_obj->getVariables();
    for(std::vector<ConstrainedVariableId>::const_iterator it = vars.begin(); it != vars.end();
	++it) {
      PSVariable* var = new PSVariable(*it); 
      check_error(var != NULL);
      m_vars.push_back(var);
    }
  }

  PSObject::~PSObject() {
    for(int i = 0; i < m_vars.size(); ++i)
      delete m_vars.get(i);
  }

  const PSList<PSVariable*>& PSObject::getMemberVariables() {
    return m_vars;
  }

  PSVariable* PSObject::getMemberVariable(const std::string& name) {
    PSVariable* retval = NULL;

    for(int i = 0; i < m_vars.size(); ++i)
      if(m_vars.get(i)->getName() == name) {
	retval = m_vars.get(i);
	break;
      }
    return retval;
  }

  PSList<PSToken*> PSObject::getTokens() {
    PSList<PSToken*> retval;
    const TokenSet& tokens = m_obj->getTokens();
    for(TokenSet::const_iterator it = tokens.begin(); it != tokens.end(); ++it) {
      PSToken* tok = new PSToken(*it);
      check_error(tok != NULL);
      retval.push_back(tok);
    }
    return retval;
  }

  PSResource::PSResource(const SAVH::ResourceId& res) : PSEntity(res), m_res(res) {}

  PSResourceProfile* PSResource::getLimits() {
    return new PSResourceProfile(m_res->getLowerLimit(), m_res->getUpperLimit());
  }

  PSResourceProfile* PSResource::getLevels() {
    return new PSResourceProfile(m_res->getProfile());
  }

  PSResourceProfile::PSResourceProfile(const double lb, const double ub)
    : m_isConst(true), m_lb(lb), m_ub(ub) {
    TimePoint inst = (TimePoint) MINUS_INFINITY;
    m_times.push_back(inst);
  }

  PSResourceProfile::PSResourceProfile(const SAVH::ProfileId& profile)
    : m_isConst(false), m_profile(profile) {
    SAVH::ProfileIterator it(m_profile);
    while(!it.done()) {
      TimePoint inst = (TimePoint) it.getTime();
      m_times.push_back(inst);
      it.next();
    }
  }

  double PSResourceProfile::getLowerBound(TimePoint time) {
    if(m_isConst)
      return m_lb;

    IntervalDomain dom;
    m_profile->getLevel((int) time, dom);
    return dom.getLowerBound();
  }

  double PSResourceProfile::getUpperBound(TimePoint time) {
    if(m_isConst)
      return m_ub;
    IntervalDomain dom;
    m_profile->getLevel((int) time, dom);
    return dom.getUpperBound();
  }

  const PSList<TimePoint>& PSResourceProfile::getTimes() {return m_times;}

  PSToken::PSToken(const TokenId& tok) : PSEntity(tok), m_tok(tok) {
    const std::vector<ConstrainedVariableId>& vars = m_tok->getVariables();
    for(std::vector<ConstrainedVariableId>::const_iterator it = vars.begin(); it != vars.end();
	++it) {
      PSVariable* var = new PSVariable(*it);
      check_error(var != NULL);
      m_vars.push_back(var);
    }
  }

  PSObject* PSToken::getOwner() {
    ObjectVarId objVar = m_tok->getObject();
    if(!objVar->lastDomain().isSingleton())
      return NULL;
    else
      return new PSObject(ObjectId(objVar->lastDomain().getSingletonValue()));
    check_error(ALWAYS_FAIL);
    return NULL;
  }

  double PSToken::getViolation() {return 0.0;}

  const std::string& PSToken::getViolationExpl() {return UNKNOWN;}

  const PSList<PSVariable*>& PSToken::getParameters() {return m_vars;}

  PSVariable* PSToken::getParameter(const std::string& name) {
    PSVariable* retval = NULL;
    for(int i = 0; i < m_vars.size(); ++i)
      if(m_vars.get(i)->getName() == name) {
	retval = m_vars.get(i);
	break;
      }
    return retval;
  }

  PSVariable::PSVariable(const ConstrainedVariableId& var) : m_var(var) {
    check_error(m_var.isValid());
    if(m_var->baseDomain().isString())
      m_type =  STRING;
    else if(m_var->baseDomain().isSymbolic()) {
      if(LabelStr::isString(m_var->baseDomain().getLowerBound()))
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
  
  const std::string& PSVariable::getName() {
    check_error(m_var.isValid());
    return m_var->getName().toString();
  }

  bool PSVariable::isEnumerated() {
    check_error(m_var.isValid());
    return m_var->baseDomain().isEnumerated();
  }

  bool PSVariable::isInterval() {
    check_error(m_var.isValid());
    return m_var->baseDomain().isInterval();
  }
  
  PSVarType PSVariable::getType() {
    check_error(m_var.isValid());
    return m_type;
  }

  bool PSVariable::isSingleton() {
    check_error(m_var.isValid());
    return m_var->lastDomain().isSingleton();
  }

  PSVarValue PSVariable::getSingletonValue() {
    check_error(m_var.isValid());
    check_error(isSingleton());
    return PSVarValue(m_var->lastDomain().getSingletonValue(), getType());
  }

  PSList<PSVarValue> PSVariable::getValues() {
    check_error(m_var.isValid());
    check_error(!isSingleton() && isEnumerated());
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


  double PSVariable::getLowerBound() {
    check_error(m_var.isValid());
    check_error(isInterval());
    return m_var->lastDomain().getLowerBound();
  }

  double PSVariable::getUpperBound() {
    check_error(m_var.isValid());
    check_error(isInterval());
    return m_var->lastDomain().getLowerBound();
  }

  void PSVariable::specifyValue(PSVarValue& v) {
    check_error(m_var.isValid());
    check_error(getType() == v.getType());
    m_var->specify(v.asDouble());
  }

  std::string PSVariable::toString() {
    check_error(m_var.isValid());
    return m_var->toString();
  }

  PSVarValue::PSVarValue(const double val, const PSVarType type) : m_val(val), m_type(type) {}

  PSVarType PSVarValue::getType() const {return m_type;}
  
  PSObject* PSVarValue::asObject() {
    check_error(m_type == OBJECT);
    return new PSObject(ObjectId(m_val));
  }

  int PSVarValue::asInt() {check_error(m_type == INTEGER); return (int) m_val;}
  
  double PSVarValue::asDouble() {return m_val;}

  bool PSVarValue::asBoolean() {check_error(m_type == BOOLEAN); return (bool) m_val;}

  const std::string& PSVarValue::asString() {
    check_error(m_type == STRING);
    return LabelStr(m_val).toString();
  }

  PSSolver::PSSolver(const SOLVERS::SolverId& solver, const std::string& configFilename) 
      : m_solver(solver) 
      , m_configFile(configFilename)
  {
  }

  void PSSolver::step() {
    m_solver->step();
  }

  void PSSolver::solve(int maxSteps, int maxDepth) {
    m_solver->solve(maxSteps, maxDepth);
  }

  void PSSolver::reset() {
    m_solver->reset();
  }

  void PSSolver::destroy() {
    delete (SOLVERS::Solver*) m_solver;
    m_solver = SOLVERS::SolverId::noId();
  }

  int PSSolver::getStepCount() {
    return (int) m_solver->getStepCount();
  }

  int PSSolver::getDepth() {
    return (int) m_solver->getDepth();
  }

  bool PSSolver::isExhausted() {
    return m_solver->isExhausted();
  }

  bool PSSolver::isTimedOut() {
    return m_solver->isTimedOut();
  }

  bool PSSolver::isConstraintConsistent() {
    return m_solver->isConstraintConsistent();
  }

  bool PSSolver::hasFlaws() {
    return !m_solver->noMoreFlaws();
  }

  int PSSolver::getOpenDecisionCnt() {
    int count = 0;
    IteratorId flawIt = m_solver->createIterator();
    while(!flawIt->done()) {
      count++;
      flawIt->next();
    }
    delete (Iterator*) flawIt;
    return count;
  }

  PSList<std::string> PSSolver::getFlaws() {
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

  std::string PSSolver::getLastExecutedDecision() {
    return m_solver->getLastExecutedDecision();
  }

  const std::string& PSSolver::getConfigFilename() {return m_configFile;}

  int PSSolver::getHorizonStart() {
    return (int) SOLVERS::HorizonFilter::getHorizon().getLowerBound();
  }

  int PSSolver::getHorizonEnd() {
    return (int) SOLVERS::HorizonFilter::getHorizon().getUpperBound();
  }

  void PSSolver::configure(int horizonStart, int horizonEnd) {
    check_error(horizonStart <= horizonEnd);
    SOLVERS::HorizonFilter::getHorizon().reset(IntervalIntDomain());
    SOLVERS::HorizonFilter::getHorizon().intersect(horizonStart, horizonEnd);
  }

  PSEngine::PSEngine() 
  {
  	// TODO: this needs to happen somewhere else
	 REGISTER_FVDETECTOR(EUROPA::SAVH::ReusableFVDetector, ReusableFVDetector);
	 REGISTER_PROFILE(EUROPA::SAVH::IncrementalFlowProfile, IncrementalFlowProfile);
	 REGISTER_FLAW_HANDLER(EUROPA::SOLVERS::ResourceThreatDecisionPoint, ResourceThreatDecisionPoint);
  }

  PSEngine::~PSEngine() 
  {  	
  }

  //FIXME
  void PSEngine::start() {
    check_error(m_constraintEngine.isNoId());
    check_error(m_planDatabase.isNoId());
    check_error(m_rulesEngine.isNoId());
    
    m_constraintEngine = (new ConstraintEngine())->getId();
    new DefaultPropagator(LabelStr("Default"), m_constraintEngine);
    new TemporalPropagator(LabelStr("Temporal"), m_constraintEngine);
    new SAVH::ProfilePropagator(LabelStr("SAVH_Resource"), m_constraintEngine);

    initConstraintLibrary();
    initNDDL();
    
    //can't init plan database until we have the model loaded
  }

  //FIXME
  void PSEngine::shutdown() {
    Entity::purgeStarted();

    uninitConstraintLibrary();
    uninitNDDL();
    ObjectFactory::purgeAll();
    TokenFactory::purgeAll();
    ConstraintLibrary::purgeAll();
    Rule::purgeAll();

    // TODO: deletes are causing a crash, fix it
    delete (RulesEngine*) m_rulesEngine;
    m_rulesEngine = RulesEngineId::noId();    
    //delete (PlanDatabase*) m_planDatabase;
    m_planDatabase = PlanDatabaseId::noId();
    //delete (ConstraintEngine*) m_constraintEngine;
    m_constraintEngine = ConstraintEngineId::noId();

    Entity::purgeEnded();
  }
    
  void PSEngine::initDatabase() 
  {
    m_planDatabase = (new PlanDatabase(m_constraintEngine, Schema::instance()))->getId();
    new ResourcePropagator(LabelStr("Resource"), m_constraintEngine, m_planDatabase);
    PropagatorId temporalPropagator =
      m_constraintEngine->getPropagatorByName(LabelStr("Temporal"));
    m_planDatabase->setTemporalAdvisor((new STNTemporalAdvisor(temporalPropagator))->getId());
    m_rulesEngine = (new RulesEngine(m_planDatabase))->getId();
  }
   
  void PSEngine::loadModel(const std::string& modelFileName) {
    check_error(m_planDatabase.isNoId());
    check_error(m_rulesEngine.isNoId());

    void* libHandle = p_dlopen(modelFileName.c_str(), RTLD_NOW);
    checkError(libHandle != NULL,
	       "Error opening model " << modelFileName << ": " << p_dlerror());

    SchemaId (*fcn_schema)();
    fcn_schema = (SchemaId (*)()) p_dlsym(libHandle, "loadSchema");
    checkError(fcn_schema != NULL,
	       "Error locating symbol 'loadSchema' in " << modelFileName << ": " <<
	       p_dlerror());

    SchemaId schema = (*fcn_schema)();
    initDatabase();    
  }

  void PSEngine::executeTxns(const std::string& xmlTxnSource,bool isFile,bool useInterpreter) {
  	// if we're using the TransactionInterpreter, we'll be starting from scratch
    if(m_planDatabase.isNoId())
          initDatabase();

    check_error(m_planDatabase.isValid());
    
    DbClientId client = m_planDatabase->getClient();

    DbClientTransactionPlayer* player;      
    if (useInterpreter)
        player = new InterpretedDbClientTransactionPlayer(client);
    else
        player = new DbClientTransactionPlayer(client);

    std::istream* in;    
    if (isFile)
      in = new std::ifstream(xmlTxnSource.c_str());
    else
      in = new std::istringstream(xmlTxnSource);

    //check_error(*in);
    player->play(*in);

    delete in;    
    delete player;
      
  }

  //FIXME
  void PSEngine::executeScript(const std::string& language, const std::string& script) {
  }

  PSList<PSObject*> PSEngine::getObjectsByType(const std::string& objectType) {
    check_error(m_planDatabase.isValid());

    PSList<PSObject*> retval;

    const ObjectSet& objects = m_planDatabase->getObjects();
    for(ObjectSet::const_iterator it = objects.begin(); it != objects.end(); ++it){
        ObjectId object = *it;
	    if(Schema::instance()->isA(object->getType(), objectType.c_str())) {
            PSObject* obj = new PSObject(*it);
            retval.push_back(obj);
	    }
    }

    return retval;
  }

  PSObject* PSEngine::getObjectByKey(PSEntityKey id) {
    check_error(m_planDatabase.isValid());

    EntityId entity = Entity::getEntity(id);
    check_error(entity.isValid());
    return new PSObject(entity);
  }

  PSList<PSResource*> PSEngine::getResourcesByType(const std::string& objectType) {
    check_error(m_planDatabase.isValid());
    
    std::list<SAVH::ResourceId> objs;
    m_planDatabase->getObjectsByType(objectType, objs);

    PSList<PSResource*> retval;
    
    for(std::list<SAVH::ResourceId>::const_iterator it = objs.begin(); it != objs.end();
	++it) {
      PSResource* res = new PSResource(*it);
      retval.push_back(res);
    }
    return retval;
  }
  
  PSResource* PSEngine::getResourceByKey(PSEntityKey id) {
    check_error(m_planDatabase.isValid());

    EntityId entity = Entity::getEntity(id);
    check_error(entity.isValid());
    return new PSResource(entity);
  }

  PSList<PSToken*> PSEngine::getTokens() {
    check_error(m_planDatabase.isValid());

    const TokenSet& tokens = m_planDatabase->getTokens();
    PSList<PSToken*> retval;

    for(TokenSet::const_iterator it = tokens.begin(); it != tokens.end(); ++it) {
      PSToken* tok = new PSToken(*it);
      retval.push_back(tok);
    }
    return retval;
  }

  PSToken* PSEngine::getTokenByKey(PSEntityKey id) {
    check_error(m_planDatabase.isValid());

    EntityId entity = Entity::getEntity(id);
    check_error(entity.isValid());
    return new PSToken(entity);
  }

  PSSolver* PSEngine::createSolver(const std::string& configurationFile) {
    TiXmlDocument* doc = new TiXmlDocument(configurationFile.c_str());
    doc->LoadFile();

    SOLVERS::SolverId solver =
      (new SOLVERS::Solver(m_planDatabase, *(doc->RootElement())))->getId();

    return new PSSolver(solver,configurationFile);
  }

}
