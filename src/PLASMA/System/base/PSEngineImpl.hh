#ifndef _H_PSEngineImpl
#define _H_PSEngineImpl

#include "PSEngine.hh"
#include "EuropaEngineBase.hh"

namespace EUROPA {

  class PSEngineImpl : public PSEngine, public EuropaEngineBase
  {
  public:
    PSEngineImpl();
    virtual ~PSEngineImpl();
	    
    static void initialize();
    static void terminate();
    
    virtual void start();
    virtual void shutdown();
	     
    virtual void addModule(ModuleId module);
    virtual void removeModule(ModuleId module);
    virtual void loadModule(const std::string& moduleFileName);

    virtual void loadModel(const std::string& modelFileName);        		
    virtual std::string executeScript(const std::string& language, const std::string& script, bool isFile);
	
	  // Constraint Engine methods
    virtual PSVariable* getVariableByKey(PSEntityKey id);
    virtual PSVariable* getVariableByName(const std::string& name);
    
    virtual PSConstraint* addConstraint(const std::string& type, PSList<PSVariable*> args);
    virtual void removeConstraint(PSEntityKey id);

    virtual bool getAutoPropagation() const;
    virtual void setAutoPropagation(bool v);      
    virtual bool propagate(); 
    
    virtual bool getAllowViolations() const;
    virtual void setAllowViolations(bool v);    
    virtual double getViolation() const;
    virtual std::string getViolationExpl() const;
    
    // Plan Database methods
    virtual PSList<PSObject*> getObjectsByType(const std::string& objectType);
    virtual PSObject* getObjectByKey(PSEntityKey id);
    virtual PSObject* getObjectByName(const std::string& name);
		
    virtual PSList<PSToken*> getTokens();    	 
    virtual PSToken* getTokenByKey(PSEntityKey id);	
		
    virtual PSList<PSVariable*> getGlobalVariables();
		
    virtual std::string planDatabaseToString();    
    
    // Solver methods
    virtual PSSolver* createSolver(const std::string& configurationFile);
    
  protected:
    bool m_started;
    PSSolverManager*    m_psSolverManager;
        
  private:	
  	// Is there a way to avoid these:
    virtual PlanDatabaseId& getPlanDatabase() { return EuropaEngineBase::getPlanDatabase(); }      
  	friend class PSPlanDatabaseListener;
  };
    
}	

#endif // _H_PSEngineImpl
