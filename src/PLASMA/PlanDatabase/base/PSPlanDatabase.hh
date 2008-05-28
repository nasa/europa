#ifndef _H_PSPlanDatabase
#define _H_PSPlanDatabase

#include "PSConstraintEngine.hh"

namespace EUROPA {

  class PSObject;
  class PSToken;

  class PSSchema : public EngineComponent
  {
    public:
        // TODO: flesh this interface out
      virtual ~PSSchema() {}
  };
  
  class PSPlanDatabase : public EngineComponent
  {
    public:
      virtual ~PSPlanDatabase() {}

      virtual PSList<PSObject*> getObjectsByType(const std::string& objectType) const = 0;
      virtual PSObject* getObjectByKey(PSEntityKey id) const = 0;
      virtual PSObject* getObjectByName(const std::string& name) const = 0;

      virtual PSList<PSToken*> getAllTokens() const = 0;
      virtual PSToken* getTokenByKey(PSEntityKey id) const = 0;	

      virtual PSList<PSVariable*> getAllGlobalVariables() const = 0;
  };
    
  class PSObject : public virtual PSEntity
  {
    public:
      PSObject() {}	
      virtual ~PSObject() {}

      virtual const std::string& getEntityType() const = 0;
      virtual std::string getObjectType() const = 0; 

      virtual PSList<PSVariable*> getMemberVariables() = 0;
      virtual PSVariable* getMemberVariable(const std::string& name) = 0;

      virtual PSList<PSToken*> getPSTokens() const = 0;

      virtual void addPrecedence(PSToken* pred,PSToken* succ) = 0;
      virtual void removePrecedence(PSToken* pred,PSToken* succ) = 0;
  };

  
  class PSToken : public virtual PSEntity
  {
  public:
	  enum PSTokenState { INACTIVE,ACTIVE,MERGED,REJECTED };
      
      PSToken() {}	
      virtual ~PSToken() {}

      virtual const std::string& getEntityType() const = 0;
      virtual std::string getTokenType() const = 0; 

      virtual bool isFact() const = 0; 

      virtual PSTokenState getTokenState() const = 0;
      virtual PSVariable* getPSStart() const = 0;
      virtual PSVariable* getPSEnd() const = 0;
      virtual PSVariable* getPSDuration() const = 0;
      
      virtual PSObject* getOwner() const = 0; 
      virtual PSToken* getPSMaster() const = 0;
      virtual PSList<PSToken*> getPSSlaves() const = 0;

      virtual double getViolation() const = 0;
      virtual std::string getViolationExpl() const = 0;

      virtual PSList<PSVariable*> getPSParameters() const = 0;
      virtual PSVariable* getParameter(const std::string& name) const = 0;

      virtual void activate() = 0;      
      virtual void reject() = 0;      
      virtual void mergePS(PSToken* activeToken) = 0;            
      virtual void cancel() = 0; // Retracts merge, activate, reject    

      // returns active tokens that this token can be merged to
      virtual PSList<PSToken*> getCompatibleTokens(unsigned int limit, bool useExactTest) = 0;

      virtual std::string toPSString() const = 0;
  };      
}

#endif
