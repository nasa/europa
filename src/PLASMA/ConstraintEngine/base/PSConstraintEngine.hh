#ifndef _H_PSConstraintEngine
#define _H_PSConstraintEngine

#include "Engine.hh"
#include "PSUtils.hh"
#include "LabelStr.hh"

namespace EUROPA {

  enum PSVarType {INTEGER,DOUBLE,BOOLEAN,STRING,OBJECT};

  class PSVariable;
  class PSVarValue;
  class PSConstraint;
  class PSObject;
  
  class PSConstraintEngine : public EngineComponent
  {
    public:
      virtual ~PSConstraintEngine() {}
    	
	  virtual PSVariable* getVariableByKey(PSEntityKey id) = 0;
	  virtual PSVariable* getVariableByName(const std::string& name) = 0;
	  
      virtual bool getAutoPropagation() const = 0;
	  virtual void setAutoPropagation(bool v) = 0;	  
	  virtual bool propagate() = 0; 
	  
  	  virtual bool getAllowViolations() const = 0;
  	  virtual void setAllowViolations(bool v) = 0;

  	  virtual double getViolation() const = 0;
  	  virtual std::string getViolationExpl() const = 0;      	  
  };
  
  class PSVariable : public virtual PSEntity
  {
    public:
    	virtual ~PSVariable() {}

    	virtual const std::string& getEntityType() const = 0;

    	virtual PSVarType getType() const = 0; // Data Type 

    	virtual std::string toString() const = 0;
    	virtual std::string toLongString() const = 0;
    	
    	virtual bool isEnumerated() const = 0;
    	virtual bool isInterval() const = 0;

    	virtual bool isNull() const = 0;      // iif CurrentDomain is empty and the variable hasn't been specified    
    	virtual bool isSingleton() const = 0;

    	virtual PSVarValue getSingletonValue() const = 0;    // Call to get value if isSingleton()==true 

    	virtual PSList<PSVarValue> getValues() const = 0;  // if isSingleton()==false && isEnumerated() == true
    	virtual PSList<PSConstraint*> getConstraints() const = 0;

    	virtual double getLowerBound() const = 0;  // if isSingleton()==false && isInterval() == true
    	virtual double getUpperBound() const = 0;  // if isSingleton()==false && isInterval() == true

    	virtual void specifyValue(PSVarValue& v) = 0;
    	virtual void reset() = 0;

    	virtual double getViolation() const = 0;
    	virtual std::string getViolationExpl() const = 0;

    	virtual PSEntity* getParent() const = 0;
    	
  };
  
  class PSConstraint : public virtual PSEntity
  {
    public:    
      PSConstraint() {}    
      virtual ~PSConstraint() {}

      virtual const std::string& getEntityType() const = 0;
      
      virtual bool isActive() const = 0;
      virtual void deactivate() = 0;
      virtual void undoDeactivation() = 0;
      
      virtual double getViolation() const = 0;
      virtual std::string getViolationExpl() const = 0;
      
      virtual PSList<PSVariable*> getVariables() const = 0;
  };

}

#endif

