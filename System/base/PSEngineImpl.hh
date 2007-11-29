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
    
}	

#endif // _H_PSEngineImpl
