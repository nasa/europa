
#include "PSPlanDatabaseImpl.hh"
#include "PSConstraintEngineImpl.hh"
#include "PlanDatabaseWriter.hh"
#include "Object.hh"
#include "Token.hh"
#include "TokenVariable.hh"
#include <sstream>

namespace EUROPA 
{
  class BaseObjectWrapperGenerator : public ObjectWrapperGenerator 
  {
    public:
    	PSObject* wrap(const EntityId& obj) {
    		return new PSObjectImpl(obj);
    	}
  };

  PSPlanDatabaseImpl::PSPlanDatabaseImpl(PlanDatabaseId& pdb) 
    : m_planDatabase(pdb) 
  {
      addObjectWrapperGenerator("Object", new BaseObjectWrapperGenerator());	  	  
  }	
  
  PSPlanDatabaseImpl::~PSPlanDatabaseImpl() {}

  PSList<PSObject*> PSPlanDatabaseImpl::getObjectsByType(const std::string& objectType) 
  {
    PSList<PSObject*> retval;

    const ObjectSet& objects = m_planDatabase->getObjects();
    for(ObjectSet::const_iterator it = objects.begin(); it != objects.end(); ++it){
    	ObjectId object = *it;
    	if(Schema::instance()->isA(object->getType(), objectType.c_str()))
    		retval.push_back(getObjectWrapperGenerator(object->getType())->wrap(object));
    }

    return retval;
  }
  PSObject* PSPlanDatabaseImpl::getObjectByKey(PSEntityKey id) 
  {
    EntityId entity = Entity::getEntity(id);
    check_runtime_error(entity.isValid());
    return new PSObjectImpl(entity);
  }

  PSObject* PSPlanDatabaseImpl::getObjectByName(const std::string& name) {
    ObjectId obj = m_planDatabase->getObject(LabelStr(name));
    check_runtime_error(obj.isValid());
    return new PSObjectImpl(obj);
  }

  PSList<PSToken*> PSPlanDatabaseImpl::getTokens() {
    const TokenSet& tokens = m_planDatabase->getTokens();
    PSList<PSToken*> retval;

    for(TokenSet::const_iterator it = tokens.begin(); it != tokens.end(); ++it) {
      PSToken* tok = new PSTokenImpl(*it);
      retval.push_back(tok);
    }
    
    return retval;
  }

  PSToken* PSPlanDatabaseImpl::getTokenByKey(PSEntityKey id) 
  {

    EntityId entity = Entity::getEntity(id);
    check_runtime_error(entity.isValid());
    return new PSTokenImpl(entity);
  }

  PSList<PSVariable*>  PSPlanDatabaseImpl::getGlobalVariables() {

    const ConstrainedVariableSet& vars = m_planDatabase->getGlobalVariables();
    PSList<PSVariable*> retval;

    for(ConstrainedVariableSet::const_iterator it = vars.begin(); it != vars.end(); ++it) {
      PSVariable* v = new PSVariableImpl(*it);
      retval.push_back(v);
    }
    return retval;
  }  

  std::string PSPlanDatabaseImpl::toString() 
  {
      PlanDatabaseWriter* pdw = new PlanDatabaseWriter();
      std::string planOutput = pdw->toString(m_planDatabase);
      delete pdw;
      return planOutput;
  }

  void PSPlanDatabaseImpl::addObjectWrapperGenerator(const LabelStr& type,
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

  ObjectWrapperGenerator* PSPlanDatabaseImpl::getObjectWrapperGenerator(const LabelStr& type) {
    const std::vector<LabelStr>& types = Schema::instance()->getAllObjectTypes(type);
    for(std::vector<LabelStr>::const_iterator it = types.begin(); it != types.end(); ++it) {
      std::map<double, ObjectWrapperGenerator*>::iterator wrapper = m_objectWrapperGenerators.find(*it);
      if(wrapper != m_objectWrapperGenerators.end())
          return wrapper->second;
    }
    checkRuntimeError(ALWAYS_FAIL,"Don't know how to wrap objects of type " << type.toString());
    return NULL;
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

  const std::string& PSObjectImpl::getEntityType() const 
  {
	static const std::string OBJECT_STR("OBJECT");
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
  
  void PSTokenImpl::activate() 
  {
	  if (m_tok->isInactive()) m_tok->activate();
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

}



