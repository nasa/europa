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
//    	PSVariable();
    	virtual ~PSVariable() {}

    	virtual const std::string& getEntityType() const;

    	virtual PSVarType getType() = 0; // Data Type 

    	virtual std::string toString();

    	
    	virtual bool isEnumerated() = 0;
    	virtual bool isInterval() = 0;

    	virtual bool isNull() = 0;      // iif CurrentDomain is empty and the variable hasn't been specified    
    	virtual bool isSingleton() = 0;

    	virtual PSVarValue getSingletonValue() = 0;    // Call to get value if isSingleton()==true 

    	virtual PSList<PSVarValue> getValues() = 0;  // if isSingleton()==false && isEnumerated() == true

    	virtual double getLowerBound() = 0;  // if isSingleton()==false && isInterval() == true
    	virtual double getUpperBound() = 0;  // if isSingleton()==false && isInterval() == true

    	virtual void specifyValue(PSVarValue& v) = 0;
    	virtual void reset() = 0;

    	virtual double getViolation() const = 0;
    	virtual std::string getViolationExpl() const = 0;

    	virtual PSEntity* getPSParent() const = 0;
    	
  };

  class PSVarValue
  {
    public:
	  PSVarValue(const double val, const PSVarType type);
	  PSVarType getType() const;

	  // Doesn't work?  Conversion to PSEntity by Id class returns something bogus, I think?
	  // (at least trying to then call simple methods on this failed ... needs further investigation)
//	  PSEntity*           asObject() const;
	  int                 asInt() const;
	  double              asDouble() const;
	  bool                asBoolean() const;
	  const std::string&  asString() const; 

	  std::string toString() const;

	  static PSVarValue getInstance(std::string val) {return PSVarValue((double)LabelStr(val), STRING);}
	  static PSVarValue getInstance(int val) {return PSVarValue((double)val, INTEGER);}
	  static PSVarValue getInstance(double val) {return PSVarValue((double)val, DOUBLE);}
	  static PSVarValue getInstance(bool val) {return PSVarValue((double)val, BOOLEAN);}
	  static PSVarValue getObjectInstance(double obj) {return PSVarValue(obj, OBJECT);} // cast an EntityId to double to call this

    private:
	  double m_val;
	  PSVarType m_type;
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
  };

}

#endif

