#ifndef _H_PSEngine
#define _H_PSEngine

#include "PSConstraintEngine.hh"
#include "PSPlanDatabase.hh"
#include "PSResource.hh"
#include "PSSolvers.hh"
#include "Module.hh"
#include "PSPlanDatabaseListener.hh"
#include "PSConstraintEngineListener.hh"

namespace EUROPA {

  class PSEngine
  {
    public:
      static PSEngine* makeInstance();

      virtual ~PSEngine() {}

      virtual void start() = 0;
      virtual void shutdown() = 0;

      virtual EngineConfig* getConfig() = 0;

      virtual void addModule(Module* module) = 0;
      virtual void removeModule(Module* module) = 0;
      virtual void loadModule(const std::string& moduleFileName) = 0;

      virtual std::string executeScript(const std::string& language, const std::string& script, bool isFile) = 0;

      // Constraint Engine methods
      virtual PSVariable* getVariableByKey(PSEntityKey id) = 0;
      virtual PSVariable* getVariableByName(const std::string& name) = 0;

      virtual bool getAutoPropagation() const = 0;
      virtual void setAutoPropagation(bool v) = 0;
      virtual bool propagate() = 0;

      virtual bool getAllowViolations() const = 0;
      virtual void setAllowViolations(bool v) = 0;
      virtual double getViolation() const = 0;
      virtual PSList<std::string> getViolationExpl() const = 0;
      virtual PSList<PSConstraint*> getAllViolations() const = 0;

      // Plan Database methods
    virtual PSList<PSObject*> getObjects() = 0;
      virtual PSList<PSObject*> getObjectsByType(const std::string& objectType) = 0;
      virtual PSObject* getObjectByKey(PSEntityKey id) = 0;
      virtual PSObject* getObjectByName(const std::string& name) = 0;

      virtual PSList<PSToken*> getTokens() = 0;
      virtual PSToken* getTokenByKey(PSEntityKey id) = 0;

      virtual PSList<PSVariable*> getGlobalVariables() = 0;

      virtual void addPlanDatabaseListener(PSPlanDatabaseListener& listener) = 0;
      virtual void addConstraintEngineListener(PSConstraintEngineListener& listener) = 0;

      virtual PSPlanDatabaseClient* getPlanDatabaseClient() = 0;

      virtual std::string planDatabaseToString() = 0;

      virtual PSSchema* getPSSchema() = 0;

      // Solver methods
      virtual PSSolver* createSolver(const std::string& configurationFile) = 0;

  };

}

#endif
