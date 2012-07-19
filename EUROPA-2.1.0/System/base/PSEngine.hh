#ifndef _H_PSEngine
#define _H_PSEngine

#include "Entity.hh"

namespace EUROPA {

  typedef int TimePoint;
  typedef int PSEntityKey;
  
  template<class T>
  class PSList
  {
    public:
      int size() const { return m_elements.size(); }
      T& get(int idx) { return m_elements[idx]; }
      void remove(int idx) {m_elements.erase(std::advance(m_elements.begin(), idx));}
      void remove(const T& value) 
      {
        typename std::vector<T>::iterator it =
        std::find(m_elements.begin(), m_elements.end(), value);
        if(it != m_elements.end())
	    m_elements.erase(it);
      }
      void push_back(const T& value) {m_elements.push_back(value);}
    
    protected:
      std::vector<T> m_elements;    	
  };

  class PSObject;
  class PSToken;
  class PSSolver;
  class PSVariable;
  class PSVarValue;
    
  class PSEngine 
  {
    public:
	  virtual ~PSEngine() {}

	  virtual void start() = 0;
	  virtual void shutdown() = 0;

	  // Loads a planning model in binary format
	  virtual void loadModel(const std::string& modelFileName) = 0;

	  virtual void executeTxns(const std::string& xmlTxnSource,bool isFile,bool useInterpreter) = 0; // TODO: fold XML into executeScript?
	  virtual std::string executeScript(const std::string& language, const std::string& script) = 0;

	  virtual PSList<PSObject*> getObjectsByType(const std::string& objectType) = 0;
	  virtual PSObject* getObjectByKey(PSEntityKey id) = 0;
	  virtual PSObject* getObjectByName(const std::string& name) = 0;

	  virtual PSList<PSToken*> getTokens() = 0;    	 
	  virtual PSToken* getTokenByKey(PSEntityKey id) = 0;	

	  virtual PSList<PSVariable*> getGlobalVariables() = 0;
	  virtual PSVariable* getVariableByKey(PSEntityKey id) = 0;
	  virtual PSVariable* getVariableByName(const std::string& name) = 0;

	  virtual PSSolver* createSolver(const std::string& configurationFile) = 0;
	  virtual std::string planDatabaseToString() = 0;

	  virtual bool getAllowViolations() const = 0;
	  virtual void setAllowViolations(bool v) = 0;

	  virtual double getViolation() const = 0;
	  virtual std::string getViolationExpl() const = 0;    
	  
	  static PSEngine* makeInstance();
  };

  class PSEntity
  {
  public: 
    virtual ~PSEntity() {}
    
    virtual PSEntityKey getKey() const;
    virtual const std::string& getName() const;
    virtual const std::string& getEntityType() const;

    virtual std::string toString();

  protected:
    PSEntity(const EntityId& entity);

  private:
    EntityId m_entity;
  };	
    
  class PSObject : public PSEntity
  {
    public:
      PSObject(const EntityId& id) : PSEntity(id) {}	
	  virtual ~PSObject() {}

	  virtual const std::string& getEntityType() const = 0;

	  virtual std::string getObjectType() const = 0; 

	  virtual PSList<PSVariable*> getMemberVariables() = 0;
	  virtual PSVariable* getMemberVariable(const std::string& name) = 0;

	  virtual PSList<PSToken*> getTokens() = 0;
  };
    
  class PSSolver
  {
    public:
	  virtual ~PSSolver() {}

	  virtual void step() = 0;
	  virtual void solve(int maxSteps,int maxDepth) = 0;
	  virtual void reset() = 0;

	  virtual int getStepCount() = 0;
	  virtual int getDepth() = 0;		
	  virtual int getOpenDecisionCnt() = 0;	

	  virtual bool isExhausted() = 0;
	  virtual bool isTimedOut() = 0;	
	  virtual bool isConstraintConsistent() = 0;
	  virtual bool hasFlaws() = 0;	

	  virtual PSList<std::string> getFlaws() = 0;	
	  virtual std::string getLastExecutedDecision() = 0;	

	  // TODO: should horizon start and end be part of configuration?
	  virtual const std::string& getConfigFilename() = 0;	
	  virtual int getHorizonStart() = 0;
	  virtual int getHorizonEnd() = 0;

	  virtual void configure(int horizonStart, int horizonEnd) = 0;
  };

  class PSToken : public PSEntity
  {	    
    public:
      PSToken(const EntityId& id) : PSEntity(id) {}	
	  virtual ~PSToken() {}

	  virtual const std::string& getEntityType() const = 0;
	  virtual std::string getTokenType() const = 0; 

	  virtual bool isFact() = 0; 

	  virtual PSObject* getOwner() = 0; 
	  virtual PSToken* getMaster() = 0;
	  virtual PSList<PSToken*> getSlaves() = 0;

	  virtual double getViolation() const = 0;
	  virtual std::string getViolationExpl() const = 0;

	  virtual PSList<PSVariable*> getParameters() = 0;
	  virtual PSVariable* getParameter(const std::string& name) = 0;

	  virtual void activate() = 0;      
	  
	  virtual std::string toString() = 0;
  };
    
  enum PSVarType {OBJECT,STRING,INTEGER,DOUBLE,BOOLEAN};
  
  class PSVariable : public PSEntity
  {
    public:
      PSVariable(const EntityId& id) : PSEntity(id) {}	
	  virtual ~PSVariable() {}

	  virtual const std::string& getEntityType() const = 0;

	  virtual PSVarType getType() = 0; // Data Type 

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

	  virtual PSEntity* getParent() = 0;

	  virtual std::string toString() = 0;
  };
    
  class PSVarValue
  {
    public:
  	  PSVarValue(const double val, const PSVarType type);
	  PSVarType getType() const;

	  PSObject*           asObject() const;
	  int                 asInt() const;
	  double              asDouble() const;
	  bool                asBoolean() const;
	  const std::string&  asString() const; 

	  std::string toString() const;

	  static PSVarValue getInstance(std::string val) {return PSVarValue((double)LabelStr(val), STRING);}
	  static PSVarValue getInstance(int val) {return PSVarValue((double)val, INTEGER);}
	  static PSVarValue getInstance(double val) {return PSVarValue((double)val, DOUBLE);}
	  static PSVarValue getInstance(bool val) {return PSVarValue((double)val, BOOLEAN);}
	  static PSVarValue getObjectInstance(double obj) {return PSVarValue(obj, OBJECT);} // cast an ObjectId to double to call this
	  
    private:
	  double m_val;
	  PSVarType m_type;
  };                
}	

#endif // _H_PSEngine
