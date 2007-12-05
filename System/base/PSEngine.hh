#ifndef _H_PSEngine
#define _H_PSEngine

#include "PSConstraintEngine.hh"
#include "PSPlanDatabase.hh"
#include "PSResource.hh"
#include "PSSolvers.hh"

namespace EUROPA {

  class PSEngine 
  {
    public:
      static void initialize();
      static void terminate();
      
  	  static PSEngine* makeInstance();
  	  
	  virtual ~PSEngine() {}

	  virtual void start() = 0;
	  virtual void shutdown() = 0;

	  virtual void loadModel(const std::string& modelFileName) = 0; // Loads a planning model in binary format
	  virtual std::string executeScript(const std::string& language, const std::string& script, bool isFile) = 0;

	  // Constraint Engine methods
	  virtual PSVariable* getVariableByKey(PSEntityKey id) = 0;
	  virtual PSVariable* getVariableByName(const std::string& name) = 0;
	  
	  virtual bool getAllowViolations() const = 0;
	  virtual void setAllowViolations(bool v) = 0;
	  virtual double getViolation() const = 0;
	  virtual std::string getViolationExpl() const = 0;    

	  // Plan Database methods
	  virtual PSList<PSObject*> getObjectsByType(const std::string& objectType) = 0;
	  virtual PSObject* getObjectByKey(PSEntityKey id) = 0;
	  virtual PSObject* getObjectByName(const std::string& name) = 0;

	  virtual PSList<PSToken*> getTokens() = 0;    	 
	  virtual PSToken* getTokenByKey(PSEntityKey id) = 0;	

	  virtual PSList<PSVariable*> getGlobalVariables() = 0;

	  virtual std::string planDatabaseToString() = 0;

	  // Solver methods
	  virtual PSSolver* createSolver(const std::string& configurationFile) = 0;
  };
  
}	

#endif 
