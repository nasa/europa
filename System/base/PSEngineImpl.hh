#ifndef _H_PSEngineImpl
#define _H_PSEngineImpl

#include "PSEngine.hh"
#include "EngineBase.hh"
#include "Id.hh"
#include "Entity.hh"
#include "Module.hh"
#include "ConstraintEngineDefs.hh"
#include "PlanDatabaseDefs.hh"
#include "SolverDefs.hh"
#include "RulesEngineDefs.hh"
#include "DbClientTransactionPlayer.hh"
#include "TransactionInterpreter.hh"

namespace EUROPA {

  class PSObjectImpl;
  class PSTokenImpl;
  class PSSolverImpl;
  class PSVariableImpl;
  class PSVarValue;
    
  class ObjectWrapperGenerator {
  public:
    virtual ~ObjectWrapperGenerator() {}
    virtual PSObject* wrap(const ObjectId& obj) = 0;
  };

  class PSEngineImpl : public PSEngine, public EngineBase
  {
  public:
    PSEngineImpl();
    virtual ~PSEngineImpl();
	    
    virtual void start();
    virtual void shutdown();
	     
    virtual std::string executeScript(const std::string& language, const std::string& script);
    virtual void executeTxns(const std::string& xmlTxnSource,bool isFile,bool useInterpreter); 
    virtual void loadModel(const std::string& modelFileName);        		
	
    virtual PSList<PSObject*> getObjectsByType(const std::string& objectType);
    virtual PSObject* getObjectByKey(PSEntityKey id);
    virtual PSObject* getObjectByName(const std::string& name);
		
    virtual PSList<PSToken*> getTokens();    	 
    virtual PSToken* getTokenByKey(PSEntityKey id);	
		
    virtual std::string planDatabaseToString();
    
    virtual PSList<PSVariable*> getGlobalVariables();
    virtual PSVariable* getVariableByKey(PSEntityKey id);
    virtual PSVariable* getVariableByName(const std::string& name);
		
    virtual PSSolver* createSolver(const std::string& configurationFile);
    
    virtual bool getAllowViolations() const;
    virtual void setAllowViolations(bool v);
    
    virtual double getViolation() const;
    virtual std::string getViolationExpl() const;
    
  protected:
  	virtual void allocateComponents();
  	virtual void deallocateComponents();

  	virtual void registerObjectWrappers();
    void addObjectWrapperGenerator(const LabelStr& type,ObjectWrapperGenerator* wrapper);    
    ObjectWrapperGenerator* getObjectWrapperGenerator(const LabelStr& type);
    std::map<double, ObjectWrapperGenerator*>& getObjectWrapperGenerators();
    std::map<double, ObjectWrapperGenerator*> m_objectWrapperGenerators;
    
    DbClientTransactionPlayerId m_transactionPlayer;
    DbClientTransactionPlayerId m_interpTransactionPlayer;
    SOLVERS::PlanWriter::PartialPlanWriter* m_ppw;    
  };

  class PSObjectImpl : public PSObject
  {
  public:
    virtual ~PSObjectImpl();
    
    virtual const std::string& getEntityType() const;
    
    virtual std::string getObjectType() const; 

    virtual PSList<PSVariable*> getMemberVariables();
    virtual PSVariable* getMemberVariable(const std::string& name);

    virtual PSList<PSToken*> getTokens();
    
    virtual void addPrecedence(PSToken* pred,PSToken* succ);
	virtual void removePrecedence(PSToken* pred,PSToken* succ);
    
  protected:
    friend class PSEngineImpl;
    friend class PSTokenImpl;
    friend class PSVarValue;
    friend class PSVariableImpl;
    friend class BaseObjectWrapperGenerator;
    PSObjectImpl(const ObjectId& obj);
    
  private:
    ObjectId m_obj;
  };
    
  class PSSolverImpl : public PSSolver
  {
  public:
    virtual ~PSSolverImpl();
    
    virtual void step();
    virtual void solve(int maxSteps,int maxDepth);
    virtual void reset();
    virtual void destroy();
	    
    virtual int getStepCount();
    virtual int getDepth();
    virtual int getOpenDecisionCnt();	
			
    virtual bool isExhausted();
    virtual bool isTimedOut();	
    virtual bool isConstraintConsistent();
	
    virtual bool hasFlaws();	
    virtual PSList<std::string> getFlaws();	
    virtual std::string getLastExecutedDecision();	
		
    virtual const std::string& getConfigFilename();	
    virtual int getHorizonStart();
    virtual int getHorizonEnd();
	    
    virtual void configure(int horizonStart, int horizonEnd);
    
  protected:
    friend class PSEngineImpl;
    PSSolverImpl(const SOLVERS::SolverId& solver, const std::string& configFilename,
	     SOLVERS::PlanWriter::PartialPlanWriter* ppw);
  private:
    SOLVERS::SolverId m_solver;
    std::string m_configFile;
    SOLVERS::PlanWriter::PartialPlanWriter* m_ppw;
  };

  class PSTokenImpl : public PSToken
  {	    
  public:
    virtual ~PSTokenImpl() {}

    virtual const std::string& getEntityType() const;

    virtual std::string getTokenType() const; 

    virtual bool isFact(); 
    
    virtual PSObject* getOwner(); 
	    
    virtual PSToken* getMaster();
	
    virtual PSList<PSToken*> getSlaves();
	    
    virtual double getViolation() const;
    virtual std::string getViolationExpl() const;
	    
    //Traditionally, the temporal variables and the object and state variables aren't 
    //considered "parameters".  I'm putting them in for the moment, but clearly the token
    //interface has the least thought put into it. ~MJI
    virtual PSList<PSVariable*> getParameters();
    virtual PSVariable* getParameter(const std::string& name);

    virtual void activate() {if (m_tok->isInactive()) m_tok->activate();}      
	    
    virtual std::string toString();

  protected:
    friend class PSEngineImpl;
    friend class PSObjectImpl;
    friend class PSVariableImpl;
    PSTokenImpl(const TokenId& tok);

    TokenId m_tok;
  };
      
  class PSVariableImpl : public PSVariable
  {
  public:
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
    friend class PSEngineImpl;
    friend class PSObjectImpl;
    friend class PSTokenImpl;

    PSVariableImpl(const ConstrainedVariableId& var);
  private:
    ConstrainedVariableId m_var;
    PSVarType m_type;
  };    
}	

#endif // _H_PSEngineImpl
