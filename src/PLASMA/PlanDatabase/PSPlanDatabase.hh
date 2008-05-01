#ifndef _H_PSPlanDatabase
#define _H_PSPlanDatabase

#include "PSConstraintEngine.hh"

namespace EUROPA {

  class PSObject;
  class PSToken;

  class ObjectWrapperGenerator 
  {
    public:
      virtual ~ObjectWrapperGenerator() {}
      virtual PSObject* wrap(const EntityId& obj) = 0;
  };

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

      virtual PSList<PSObject*> getObjectsByType(const std::string& objectType) = 0;
      virtual PSObject* getObjectByKey(PSEntityKey id) = 0;
      virtual PSObject* getObjectByName(const std::string& name) = 0;

      virtual PSList<PSToken*> getAllTokens() = 0;    	 
      virtual PSToken* getTokenByKey(PSEntityKey id) = 0;	

      virtual PSList<PSVariable*> getAllGlobalVariables() = 0;

      virtual void addObjectWrapperGenerator(const LabelStr& type,ObjectWrapperGenerator* wrapper) = 0;    
      
      virtual std::string toString() = 0;
  };
    
  class PSObject : public PSEntity
  {
    public:
      PSObject() {}	
      virtual ~PSObject() {}

      virtual const std::string& getEntityType() const = 0;

      virtual std::string getObjectType() const = 0; 

      virtual PSList<PSVariable*> getMemberVariables() = 0;
      virtual PSVariable* getMemberVariable(const std::string& name) = 0;

      virtual PSList<PSToken*> getTokens() = 0;

      virtual void addPrecedence(PSToken* pred,PSToken* succ) = 0;
      virtual void removePrecedence(PSToken* pred,PSToken* succ) = 0;
  };

  enum PSTokenState { INACTIVE,ACTIVE,MERGED,REJECTED };
  
  class PSToken : public PSEntity
  {
    public:
      
      PSToken() {}	
      virtual ~PSToken() {}

      virtual const std::string& getEntityType() const = 0;
      virtual std::string getTokenType() const = 0; 

      virtual bool isFact() = 0; 

      virtual PSTokenState getTokenState() const = 0;
      virtual PSVariable* getStart() = 0;
      virtual PSVariable* getEnd() = 0;
      virtual PSVariable* getDuration() = 0;
      
      virtual PSObject* getOwner() = 0; 
      virtual PSToken* getMaster() = 0;
      virtual PSList<PSToken*> getSlaves() = 0;

      virtual double getViolation() const = 0;
      virtual std::string getViolationExpl() const = 0;

      virtual PSList<PSVariable*> getParameters() = 0;
      virtual PSVariable* getParameter(const std::string& name) = 0;

      virtual void activate() = 0;      
      virtual void reject() = 0;      
      virtual void merge(PSToken* activeToken) = 0;            
      virtual void cancel() = 0; // Retracts merge, activate, reject    

      // returns active tokens that this token can be merged to
      virtual PSList<PSToken*> getCompatibleTokens(unsigned int limit, bool useExactTest) = 0;

      virtual std::string toString() = 0;
  };      
}

#endif
