
#include "PSPlanDatabaseImpl.hh"
#include "PSConstraintEngineImpl.hh"
#include "Object.hh"
#include "Token.hh"
#include "TokenVariable.hh"

namespace EUROPA 
{
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



