#ifndef _H_PSPlanDatabase
#define _H_PSPlanDatabase

#include "PSConstraintEngine.hh"

namespace EUROPA {

  class PSToken;
  
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
	  
	  virtual void addPrecedence(PSToken* pred,PSToken* succ) = 0;
	  virtual void removePrecedence(PSToken* pred,PSToken* succ) = 0;
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
}

#endif
