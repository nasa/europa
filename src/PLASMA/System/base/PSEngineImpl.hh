#ifndef _H_PSEngineImpl
#define _H_PSEngineImpl

#include "PSEngine.hh"
#include "EuropaEngine.hh"

namespace EUROPA {

  class PSEngineImpl : public PSEngine, public EuropaEngine
  {
  public:
    PSEngineImpl();
    virtual ~PSEngineImpl();

    virtual void start();
    virtual void shutdown();

    virtual EngineConfig* getConfig();

    virtual void addModule(Module* module);
    virtual void removeModule(Module* module);
    virtual void loadModule(const std::string& moduleFileName);

    virtual void loadModel(const std::string& modelFileName);
    virtual std::string executeScript(const std::string& language, const std::string& script, bool isFile);

	  // Constraint Engine methods
    virtual PSVariable* getVariableByKey(PSEntityKey id);
    virtual PSVariable* getVariableByName(const std::string& name);

    virtual bool getAutoPropagation() const;
    virtual void setAutoPropagation(bool v);
    virtual bool propagate();

    virtual bool getAllowViolations() const;
    virtual void setAllowViolations(bool v);
    virtual double getViolation() const;
    virtual PSList<std::string> getViolationExpl() const;
    virtual PSList<PSConstraint*> getAllViolations() const;

    // Plan Database methods
    virtual PSList<PSObject*> getObjects();
    virtual PSList<PSObject*> getObjectsByType(const std::string& objectType);
    virtual PSObject* getObjectByKey(PSEntityKey id);
    virtual PSObject* getObjectByName(const std::string& name);

    virtual PSList<PSToken*> getTokens();
    virtual PSToken* getTokenByKey(PSEntityKey id);

    virtual PSList<PSVariable*> getGlobalVariables();

    virtual void addPlanDatabaseListener(PSPlanDatabaseListener& listener);
    virtual void addConstraintEngineListener(PSConstraintEngineListener& listener);

    virtual PSPlanDatabaseClient* getPlanDatabaseClient();

    virtual std::string planDatabaseToString();
    virtual PSSchema* getPSSchema();


    // Solver methods
    virtual PSSolver* createSolver(const std::string& configurationFile);

  protected:
    bool m_started;
  };

}

#endif // _H_PSEngineImpl
