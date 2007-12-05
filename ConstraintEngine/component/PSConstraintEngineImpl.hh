#ifndef _H_PSConstraintEngineImpl
#define _H_PSConstraintEngineImpl

#include "PSConstraintEngine.hh"
#include "ConstraintEngineDefs.hh"
#include <string>

namespace EUROPA {
    
  class PSConstraintEngineImpl : public PSConstraintEngine
  {
    public:
    	PSConstraintEngineImpl(ConstraintEngineId ce);	    
    	virtual ~PSConstraintEngineImpl();

        virtual PSVariable* getVariableByKey(PSEntityKey id);
  	    virtual PSVariable* getVariableByName(const std::string& name);
  	    
    	virtual bool getAllowViolations() const;
    	virtual void setAllowViolations(bool v);

    	virtual double getViolation() const;
    	virtual std::string getViolationExpl() const;    

    protected:
    	ConstraintEngineId m_constraintEngine;
  };

  class PSVariableImpl : public PSVariable
  {
  public:
	PSVariableImpl(const ConstrainedVariableId& var);
    virtual ~PSVariableImpl(){}
	    
    virtual const std::string& getEntityType() const;

    virtual PSVarType getType(); // Data Type 

    virtual bool isEnumerated();
    virtual bool isInterval();

    virtual bool isNull();      // iif CurrentDomain is empty and the variable hasn't been specified    
    virtual bool isSingleton();
		   
    virtual PSVarValue getSingletonValue();    // Call to get value if isSingleton()==true 
	
    virtual PSList<PSVarValue> getValues();  // if isSingleton()==false && isEnumerated() == true
	
    virtual double getLowerBound();  // if isSingleton()==false && isInterval() == true
    virtual double getUpperBound();  // if isSingleton()==false && isInterval() == true
	    
    virtual void specifyValue(PSVarValue& v);
    virtual void reset();
	
    virtual double getViolation() const;
    virtual std::string getViolationExpl() const;

    virtual PSEntity* getParent();
	    
    virtual std::string toString();
  protected:
    ConstrainedVariableId m_var;
    PSVarType m_type;
  };   
  
}	

#endif 
