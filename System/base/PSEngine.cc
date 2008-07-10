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

#include <fstream>

namespace EUROPA {
  
  std::map<double, ObjectWrapperGenerator*>& PSEngine::getObjectWrapperGenerators()
  {
      static std::map<double, ObjectWrapperGenerator*> objectWrapperGenerators;
      return objectWrapperGenerators;
  }
  
  std::map<double, PSLanguageInterpreter*>& PSEngine::getLanguageInterpreters()
  {
      static std::map<double, PSLanguageInterpreter*> languageInterpreters;
      return languageInterpreters;
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

  PSObject::PSObject(const ObjectId& obj) : PSEntity(obj), m_obj(obj) {
  }

  PSObject::~PSObject() {
  }

  PSList<PSVariable*> PSObject::getMemberVariables() {
    PSList<PSVariable*> retval;
    const std::vector<ConstrainedVariableId>& vars = m_obj->getVariables();
    for(std::vector<ConstrainedVariableId>::const_iterator it = vars.begin(); it != vars.end();
	++it) {
      PSVariable* var = new PSVariable(*it); 
      check_runtime_error(var != NULL);
      retval.push_back(var);
    }

    return retval;
  }

  const std::string OBJECT_STR("OBJECT");
  const std::string& PSObject::getEntityType() const 
  {
  	return OBJECT_STR;
  }

  std::string PSObject::getObjectType() const 
  {
  	return m_obj->getType().toString();
  }

  PSVariable* PSObject::getMemberVariable(const std::string& name) {
    LabelStr realName(name);
    PSVariable* retval = NULL;
    const std::vector<ConstrainedVariableId>& vars = m_obj->getVariables();
    for(std::vector<ConstrainedVariableId>::const_iterator it = vars.begin(); it != vars.end();
	++it) {
      if((*it)->getName() == realName) {
	retval = new PSVariable(*it);
	break;
      }
    }
    return retval;
  }

  PSList<PSToken*> PSObject::getTokens() {
    PSList<PSToken*> retval;
    const TokenSet& tokens = m_obj->getTokens();
    for(TokenSet::const_iterator it = tokens.begin(); it != tokens.end(); ++it) {
      PSToken* tok = new PSToken(*it);
      check_runtime_error(tok != NULL);
      retval.push_back(tok);
    }
    return retval;
  }

  PSToken::PSToken(const TokenId& tok) : PSEntity(tok), m_tok(tok) {
  }

  const std::string TOKEN_STR("TOKEN");
  const std::string& PSToken::getEntityType() const 
  {
  	return TOKEN_STR;
  }

  std::string PSToken::getTokenType() const 
  {
  	return m_tok->getUnqualifiedPredicateName().toString();
  }

  PSObject* PSToken::getOwner() {
    if(!m_tok->isAssigned())
      return NULL;
      
    ObjectVarId objVar = m_tok->getObject();
    return new PSObject(ObjectId(objVar->lastDomain().getSingletonValue()));
  }
  
  PSToken* PSToken::getMaster() {
  	TokenId master = m_tok->getMaster();
  	if (master.isNoId())
  	    return NULL;
  	
  	return new PSToken(master);    
  }
  
  PSList<PSToken*> PSToken::getSlaves() {
    const TokenSet& tokens = m_tok->getSlaves();
    PSList<PSToken*> retval;

    for(TokenSet::const_iterator it = tokens.begin(); it != tokens.end(); ++it) {
      PSToken* tok = new PSToken(*it);
      retval.push_back(tok);
    }
    return retval;    	
  }

  PSToken* PSToken::getActiveToken() {
    return new PSToken(m_tok->getActiveToken());
  }

  PSList<PSToken*> PSToken::getMergedTokens() {
    const TokenSet& tokens = m_tok->getMergedTokens();
    PSList<PSToken*> retval;

    for(TokenSet::const_iterator it = tokens.begin(); it != tokens.end(); ++it) {
      PSToken* tok = new PSToken(*it);
      retval.push_back(tok);
    }
    return retval;
  }

  // TODO: Implement these
  double PSToken::getViolation() const 
  {
	  return m_tok->getViolation();
  }
  
  std::string PSToken::getViolationExpl() const 
  { 
	  return m_tok->getViolationExpl();
  }

  PSList<PSVariable*> PSToken::getParameters() {
    PSList<PSVariable*> retval;
    const std::vector<ConstrainedVariableId>& vars = m_tok->getVariables();
    for(std::vector<ConstrainedVariableId>::const_iterator it = vars.begin(); it != vars.end();
	++it) {
      PSVariable* var = new PSVariable(*it);
      check_runtime_error(var != NULL);
      retval.push_back(var);
    }
    return retval;
  }

  PSVariable* PSToken::getParameter(const std::string& name) {
    LabelStr realName(name);
    PSVariable* retval = NULL;
    const std::vector<ConstrainedVariableId>& vars = m_tok->getVariables();
    for(std::vector<ConstrainedVariableId>::const_iterator it = vars.begin(); it != vars.end();
	++it) {
      if((*it)->getName() == realName) {
	retval = new PSVariable(*it);
	break;
      }
    }
    return retval;
  }

  bool PSToken::isFact()
  {
  	return m_tok->isFact();
  }
  
  std::string PSToken::toString()
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


  PSVariable::PSVariable(const ConstrainedVariableId& var) : PSEntity(var), m_var(var) {
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
  const std::string& PSVariable::getEntityType() const 
  {
  	return VARIABLE_STR;
  }
  
  bool PSVariable::isEnumerated() {
    check_runtime_error(m_var.isValid());
    return m_var->baseDomain().isEnumerated();
  }

  bool PSVariable::isInterval() {
    check_runtime_error(m_var.isValid());
    return m_var->baseDomain().isInterval();
  }
  
  PSVarType PSVariable::getType() {
    check_runtime_error(m_var.isValid());
    return m_type;
  }

  bool PSVariable::isNull() {
    check_runtime_error(m_var.isValid());
    return m_var->lastDomain().isEmpty() && !m_var->isSpecified();
  }

  bool PSVariable::isSingleton() {
    check_runtime_error(m_var.isValid());
    return m_var->isSpecified() || m_var->lastDomain().isSingleton();
  }

  PSVarValue PSVariable::getSingletonValue() {
    check_runtime_error(m_var.isValid());
    check_runtime_error(isSingleton());
    
    if (m_var->isSpecified())
      return PSVarValue(m_var->getSpecifiedValue(), getType());
    else
      return PSVarValue(m_var->lastDomain().getSingletonValue(), getType());
  }

  PSList<PSVarValue> PSVariable::getValues() {
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


  double PSVariable::getLowerBound() {
    check_runtime_error(m_var.isValid());
    check_runtime_error(isInterval());
    return m_var->lastDomain().getLowerBound();
  }

  double PSVariable::getUpperBound() {
    check_runtime_error(m_var.isValid());
    check_runtime_error(isInterval());
    return m_var->lastDomain().getUpperBound();
  }

  void PSVariable::specifyValue(PSVarValue& v) {
    check_runtime_error(m_var.isValid());
    check_runtime_error(getType() == v.getType());

    debugMsg("PSVariable:specify","Specifying var:" << m_var->toString() << " to value:" << v.toString());
    
    // If specifying to the same value it already has, do nothing
    if (m_var->isSpecified()) {
      if(m_var->getSpecifiedValue() == v.asDouble()) {
        debugMsg("PSVariable:specify","Tried to specify to same value, so bailing out without doing any work");
        return;
      }
      m_var->reset();
      debugMsg("PSVariable:specify","After reset for var:" << m_var->toString());
      //only propagate if we aren't allowing violations
      //this can save us an extra call to reset() and propagate()
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

  void PSVariable::reset() {
    check_runtime_error(m_var.isValid());
    debugMsg("PSVariable:reset",
	     "Re-setting " << m_var->toString());
    m_var->reset();
    m_var->getConstraintEngine()->propagate();
  }

  double PSVariable::getViolation() const
  {
    check_runtime_error(m_var.isValid());
    return m_var->getViolation();
  }
  
  std::string PSVariable::getViolationExpl() const 
  { 
    check_runtime_error(m_var.isValid());
    return m_var->getViolationExpl();
  }
  
  PSEntity* PSVariable::getParent() {
    EntityId parent(m_var->getParent());
    if(parent.isNoId())
      return NULL;
    else if(TokenId::convertable(parent))
      return new PSToken((TokenId) parent);
    else if(ObjectId::convertable(parent))
      return new PSObject((ObjectId) parent);
    else if(RuleInstanceId::convertable(parent))
      return new PSToken(((RuleInstanceId)parent)->getToken());
    else {
      checkRuntimeError(ALWAYS_FAIL,
			"Variable " << toString() << " has a parent that isn't a token, " <<
			"object, or rule: " << m_var->getParent()->toString());
    }
    return NULL;
  }

  std::string PSVariable::toString() {
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
    return new PSObject(ObjectId(m_val));
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
   
  PSSolver::PSSolver(const SOLVERS::SolverId& solver, const std::string& configFilename,
		     SOLVERS::PlanWriter::PartialPlanWriter* ppw) 
      : m_solver(solver) 
      , m_configFile(configFilename),
	m_ppw(ppw)
  {
    m_ppw->setSolver(m_solver);
  }

  PSSolver::~PSSolver() {
    if(m_solver.isValid())
      destroy();
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
    m_ppw->clearSolver();
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
    check_runtime_error(horizonStart <= horizonEnd);
    SOLVERS::HorizonFilter::getHorizon().reset(IntervalIntDomain());
    SOLVERS::HorizonFilter::getHorizon().intersect(horizonStart, horizonEnd);
  }

  PSEngine::PSEngine() : m_ppw(NULL)
  {
  }

  PSEngine::~PSEngine() 
  {
  }

  //FIXME
  void PSEngine::start() {		
    Error::doThrowExceptions(); // throw exceptions!
    Error::doDisplayErrors();
    check_runtime_error(m_constraintEngine.isNoId());
    check_runtime_error(m_planDatabase.isNoId());
    check_runtime_error(m_rulesEngine.isNoId());
    
    m_constraintEngine = (new ConstraintEngine())->getId();
    new DefaultPropagator(LabelStr("Default"), m_constraintEngine);
    new TemporalPropagator(LabelStr("Temporal"), m_constraintEngine);

    initConstraintLibrary();
    initNDDL();
    
    //can't init plan database until we have the model loaded
  }

  //FIXME
  void PSEngine::shutdown() {
    if(m_ppw != NULL) {
      delete m_ppw;
      m_ppw = NULL;
    }
    Entity::purgeStarted();

    uninitConstraintLibrary();
    uninitNDDL();
    Schema::instance()->reset();
    ObjectFactory::purgeAll();
    TokenFactory::purgeAll();
    ConstraintLibrary::purgeAll();
    Rule::purgeAll();

    // TODO: deletes are causing a crash, fix it
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
    
  void PSEngine::initDatabase() 
  {
    m_planDatabase = (new PlanDatabase(m_constraintEngine, Schema::instance()))->getId();
    PropagatorId temporalPropagator =
      m_constraintEngine->getPropagatorByName(LabelStr("Temporal"));
    m_planDatabase->setTemporalAdvisor((new STNTemporalAdvisor(temporalPropagator))->getId());
    m_rulesEngine = (new RulesEngine(m_planDatabase))->getId();
    DbClientId client = m_planDatabase->getClient();
    m_interpTransactionPlayer = new InterpretedDbClientTransactionPlayer(client);
    m_transactionPlayer = new DbClientTransactionPlayer(client);
    m_ppw =
      new SOLVERS::PlanWriter::PartialPlanWriter(m_planDatabase, m_constraintEngine,
						 m_rulesEngine);
  }
   
  void PSEngine::loadModel(const std::string& modelFileName) {
    check_runtime_error(m_planDatabase.isNoId());
    check_runtime_error(m_rulesEngine.isNoId());

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
  std::string PSEngine::executeScript(const std::string& language, const std::string& script) {
    std::map<double, PSLanguageInterpreter*>::iterator it =
      getLanguageInterpreters().find(LabelStr(language));
    checkRuntimeError(it != getLanguageInterpreters().end(),
		      "Cannot execute script of unknown language \"" << language << "\"");
    return it->second->interpret(script);
  }

  PSList<PSObject*> PSEngine::getObjectsByType(const std::string& objectType) {
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

  PSObject* PSEngine::getObjectByKey(PSEntityKey id) {
    check_runtime_error(m_planDatabase.isValid());

    EntityId entity = Entity::getEntity(id);
    check_runtime_error(entity.isValid());
    return new PSObject(entity);
  }

  PSObject* PSEngine::getObjectByName(const std::string& name) {
    check_runtime_error(m_planDatabase.isValid());
    ObjectId obj = m_planDatabase->getObject(LabelStr(name));
    check_runtime_error(obj.isValid());
    return new PSObject(obj);
  }

  PSList<PSToken*> PSEngine::getTokens() {
    check_runtime_error(m_planDatabase.isValid());

    const TokenSet& tokens = m_planDatabase->getTokens();
    PSList<PSToken*> retval;

    for(TokenSet::const_iterator it = tokens.begin(); it != tokens.end(); ++it) {
      PSToken* tok = new PSToken(*it);
      retval.push_back(tok);
    }
    return retval;
  }

  PSToken* PSEngine::getTokenByKey(PSEntityKey id) {
    check_runtime_error(m_planDatabase.isValid());

    EntityId entity = Entity::getEntity(id);
    check_runtime_error(entity.isValid());
    return new PSToken(entity);
  }

  PSList<PSVariable*>  PSEngine::getGlobalVariables() {
    check_runtime_error(m_planDatabase.isValid());

    const ConstrainedVariableSet& vars = m_planDatabase->getGlobalVariables();
    PSList<PSVariable*> retval;

    for(ConstrainedVariableSet::const_iterator it = vars.begin(); it != vars.end(); ++it) {
      PSVariable* v = new PSVariable(*it);
      retval.push_back(v);
    }
    return retval;
  }  

  PSVariable* PSEngine::getVariableByKey(PSEntityKey id)
  {
    check_runtime_error(m_planDatabase.isValid());
    EntityId entity = Entity::getEntity(id);
    check_runtime_error(entity.isValid());
    return new PSVariable(entity);
  }

  // TODO: this needs to be optimized
  PSVariable* PSEngine::getVariableByName(const std::string& name)
  {
    check_runtime_error(m_planDatabase.isValid());
    const ConstrainedVariableSet& vars = m_constraintEngine->getVariables();

    for(ConstrainedVariableSet::const_iterator it = vars.begin(); it != vars.end(); ++it) {
    	ConstrainedVariableId v = *it;
    	if (v->getName().toString() == name)
            return new PSVariable(*it);
    }
    
    return NULL;
  }

  PSSolver* PSEngine::createSolver(const std::string& configurationFile) {
    TiXmlDocument* doc = new TiXmlDocument(configurationFile.c_str());
    doc->LoadFile();

    SOLVERS::SolverId solver =
      (new SOLVERS::Solver(m_planDatabase, *(doc->RootElement())))->getId();
    return new PSSolver(solver,configurationFile, m_ppw);
  }

  bool PSEngine::getAllowViolations() const
  {
  	return m_constraintEngine->getAllowViolations();
  }

  void PSEngine::setAllowViolations(bool v)
  {
    m_constraintEngine->setAllowViolations(v);
  }

  double PSEngine::getViolation() const
  {
  	return m_constraintEngine->getViolation();
  }
   
  std::string PSEngine::getViolationExpl() const
  {
  	return m_constraintEngine->getViolationExpl();
  }
   
   std::string PSEngine::planDatabaseToString() {
      PlanDatabaseWriter* pdw = new PlanDatabaseWriter();
      std::string planOutput = pdw->toString(m_planDatabase);
      delete pdw;
      return planOutput;
   }

  void PSEngine::addLanguageInterpreter(const LabelStr& language,
					PSLanguageInterpreter* interpreter) {
    std::map<double, PSLanguageInterpreter*>::iterator it =
      getLanguageInterpreters().find(language);
    if(it == getLanguageInterpreters().end())
      getLanguageInterpreters().insert(std::make_pair(language, interpreter));
    else {
      delete it->second;
      it->second = interpreter;
    }
  }

  void PSEngine::addObjectWrapperGenerator(const LabelStr& type,
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

  ObjectWrapperGenerator* PSEngine::getObjectWrapperGenerator(const LabelStr& type) {
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

  class BaseObjectWrapperGenerator : public ObjectWrapperGenerator {
  public:
    PSObject* wrap(const ObjectId& obj) {
      return new PSObject(obj);
    }
  };

  class NDDLInterpreter : public PSLanguageInterpreter {
  public:
    std::string interpret(const std::string& script) {
      return "";
    }
  };

  class PSEngineLocalStatic {
  public:
    PSEngineLocalStatic() {
      PSEngine::addObjectWrapperGenerator("Object", new BaseObjectWrapperGenerator());
      PSEngine::addLanguageInterpreter("nddl", new NDDLInterpreter());
    }
  };

  namespace PSEngine {
    PSEngineLocalStatic s_localStatic;
  }

}
