 
#include "PSConstraintEngineImpl.hh"
#include "ConstraintEngine.hh"
#include "ConstrainedVariable.hh"
#include "Debug.hh"


namespace EUROPA {
  
  PSVariableImpl::PSVariableImpl(const ConstrainedVariableId& var) 
      : m_var(var) 
  {
      //m_entityId = m_var;
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

  const std::string& PSVariableImpl::getEntityType() const 
  {
	static const std::string VARIABLE_STR("VARIABLE");
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
    
    m_var->specify(v.asDouble());
    debugMsg("PSVariable:specify","After specify for var:" << m_var->toString() << " to value:" << v.toString());
    // TODO: move this to ConstrainedVariable::specify()
    if (m_var->getConstraintEngine()->getAutoPropagation())
       m_var->getConstraintEngine()->propagate();
    debugMsg("PSVariable:specify","After propagate for var:" << m_var->toString());
  }

  void PSVariableImpl::reset() {
    check_runtime_error(m_var.isValid());
    debugMsg("PSVariable:reset",
	     "Re-setting " << m_var->toString());
    m_var->reset();
    // TODO: move this to ConstrainedVariable::reset()
    if (m_var->getConstraintEngine()->getAutoPropagation())
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
    
    /* TODO: fix this
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
    */
    //return new PSEntity(parent);
    return parent;
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
  
  int PSVarValue::asInt() const {check_runtime_error(m_type == INTEGER); return (int) m_val;}
  
  double PSVarValue::asDouble() const {return m_val;}

  bool PSVarValue::asBoolean() const {check_runtime_error(m_type == BOOLEAN); return (bool) m_val;}

  const std::string& PSVarValue::asString() const {
    check_runtime_error(m_type == STRING);
    return LabelStr(m_val).toString();
  }
  
  PSEntity* PSVarValue::asObject() const 
  {
    check_runtime_error(m_type == OBJECT);
    /* TODO: provide hooks to return PSObject or other objects */
    //return new PSEntity(EntityId(m_val));
    return EntityId(m_val);
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
  		        PSEntity* obj = asObject();
                os << "OBJECT:" << obj->getEntityName() << "(" << obj->getKey() << ")";
                delete obj;
  		    }
  		    break;
  		
  		default:
  		    check_error(ALWAYS_FAILS, "Unknown type");    
  	}
  	  	
  	return os.str();
  }      
}
