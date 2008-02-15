
#include "PSPlanDatabaseImpl.hh"
#include "PSConstraintEngineImpl.hh"
#include "Object.hh"
#include "Token.hh"
#include "TokenVariable.hh"
#include "Error.hh"
#include <sstream>

namespace EUROPA 
{
  PSObjectImpl::PSObjectImpl(const ObjectId& obj) 
      : m_obj(obj) 
  {
      m_entity = m_obj;
  }

  PSObjectImpl::~PSObjectImpl() 
  {
  }

  PSList<PSVariable*> PSObjectImpl::getMemberVariables() 
  {
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
	  TokenId p = m_obj->getPlanDatabase()->getEntityByKey(pred->getEntityKey());
	  TokenId s = m_obj->getPlanDatabase()->getEntityByKey(succ->getEntityKey());
	  m_obj->constrain(p,s);
	  // TODO: move this to Object::constrain()
      if (m_obj->getPlanDatabase()->getConstraintEngine()->getAutoPropagation())
          m_obj->getPlanDatabase()->getConstraintEngine()->propagate();	  
  }
  
  void PSObjectImpl::removePrecedence(PSToken* pred,PSToken* succ)
  {
	  TokenId p = m_obj->getPlanDatabase()->getEntityByKey(pred->getEntityKey());
	  TokenId s = m_obj->getPlanDatabase()->getEntityByKey(succ->getEntityKey());
	  m_obj->free(p,s);	  
      // TODO: move this to Object::free()
      if (m_obj->getPlanDatabase()->getConstraintEngine()->getAutoPropagation())
          m_obj->getPlanDatabase()->getConstraintEngine()->propagate();   
  }

  
  PSTokenImpl::PSTokenImpl(const TokenId& tok) 
      : m_tok(tok) 
  {
      m_entity = m_tok;
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

  PSTokenState PSTokenImpl::getTokenState() const
  {
      if (m_tok->isActive())
          return ACTIVE;
      
      if (m_tok->isInactive())
          return INACTIVE;
      
      if (m_tok->isMerged())
          return MERGED;
      
      if (m_tok->isRejected())
          return REJECTED;
      
      check_error(ALWAYS_FAIL,"Unknown token state");
      return INACTIVE;
  }
  
  PSVariable* PSTokenImpl::getStart()
  {
      return new PSVariableImpl(m_tok->getStart());
  }
  
  PSVariable* PSTokenImpl::getEnd()
  {
      return new PSVariableImpl(m_tok->getEnd());      
  }
  
  PSVariable* PSTokenImpl::getDuration()
  {
      return new PSVariableImpl(m_tok->getDuration());            
  }

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
    for(std::vector<ConstrainedVariableId>::const_iterator it = vars.begin(); it != vars.end();++it) {
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
	  m_tok->activate();
  }        

  void PSTokenImpl::reject() 
  {
      m_tok->reject();
  }        

  void PSTokenImpl::merge(PSToken* activeToken) 
  {
      check_error(activeToken != NULL, "Can't merge on NULL token");
      TokenId tok = m_tok->getPlanDatabase()->getEntityByKey(activeToken->getEntityKey());
      m_tok->merge(tok);
  }        

  void PSTokenImpl::cancel() 
  {
      m_tok->cancel();
  }        

  PSList<PSToken*> PSTokenImpl::getCompatibleTokens(unsigned int limit, bool useExactTest)
  {
      std::vector<TokenId> tokens; 
      m_tok->getPlanDatabase()->getCompatibleTokens(m_tok,tokens,limit,useExactTest);
      PSList<PSToken*> retval;

      for(unsigned int i=0;i<tokens.size();i++) {
        PSToken* tok = new PSTokenImpl(tokens[i]);
        retval.push_back(tok);
      }
      
      return retval;      
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
  	    os << "    " << vars.get(i)->getEntityName() << " : " << vars.get(i)->toString() << std::endl;
	    delete vars.get(i);
  	}
  	
  	os << "}" << std::endl;
  	
  	return os.str();
  }

}



