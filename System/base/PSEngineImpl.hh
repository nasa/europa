#ifndef _H_PSEngineImpl
#define _H_PSEngineImpl

#include "PSEngine.hh"
#include "EngineBase.hh"
#include "SolverDefs.hh"

namespace EUROPA {

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
	    
    static void initialize();
    static void terminate();
    
    virtual void start();
    virtual void shutdown();
	     
    virtual void loadModel(const std::string& modelFileName);        		
    virtual std::string executeScript(const std::string& language, const std::string& script, bool isFile);
	
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
    
    virtual PSList<PSResource*> getResourcesByType(const std::string& objectType);
    virtual PSResource* getResourceByKey(PSEntityKey id);	  
    
  protected:
  	virtual void allocateComponents();
  	virtual void deallocateComponents();

  	virtual void registerObjectWrappers();
    void addObjectWrapperGenerator(const LabelStr& type,ObjectWrapperGenerator* wrapper);    
    ObjectWrapperGenerator* getObjectWrapperGenerator(const LabelStr& type);
    std::map<double, ObjectWrapperGenerator*> m_objectWrapperGenerators;
    
    bool m_started;
    SOLVERS::PlanWriter::PartialPlanWriter* m_ppw;    
  };
    
}	

#endif // _H_PSEngineImpl
