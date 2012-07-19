#ifndef _H_PSPlanDatabaseImpl
#define _H_PSPlanDatabaseImpl

#include "PSPlanDatabase.hh"
#include "ConstraintEngineDefs.hh"
#include "PlanDatabaseDefs.hh"
#include <map>

namespace EUROPA {

  class PSPlanDatabaseImpl : public PSPlanDatabase
  {
    public:
      PSPlanDatabaseImpl(PlanDatabaseId& pdb);	
      virtual ~PSPlanDatabaseImpl();

      virtual PSList<PSObject*> getObjectsByType(const std::string& objectType);
      virtual PSObject* getObjectByKey(PSEntityKey id);
      virtual PSObject* getObjectByName(const std::string& name);

      virtual PSList<PSToken*> getTokens();    	 
      virtual PSToken* getTokenByKey(PSEntityKey id);	

      virtual PSList<PSVariable*> getGlobalVariables();

      virtual std::string toString();

      virtual void addObjectWrapperGenerator(const LabelStr& type,ObjectWrapperGenerator* wrapper);    
      
    protected:
      PlanDatabaseId m_planDatabase;	
  };

  class PSObjectImpl : public virtual PSObject
  {
    public:
      PSObjectImpl(const ObjectId& obj);
      virtual ~PSObjectImpl();
  
      virtual const std::string& getEntityType() const;

      virtual std::string getObjectType() const; 

      virtual PSList<PSVariable*> getMemberVariables();
      virtual PSVariable* getMemberVariable(const std::string& name);

      virtual PSList<PSToken*> getTokens();

      virtual void addPrecedence(PSToken* pred,PSToken* succ);
      virtual void removePrecedence(PSToken* pred,PSToken* succ);

    protected:
    	ObjectId m_obj;    	
  };

  class PSTokenImpl : public PSToken
  {	    
    public:
      PSTokenImpl(const TokenId& tok);
      virtual ~PSTokenImpl() {}

      virtual const std::string& getEntityType() const;

      virtual std::string getTokenType() const; 

      virtual bool isFact(); 

      virtual PSObject* getOwner(); 
      virtual PSToken* getMaster();
      virtual PSList<PSToken*> getSlaves();

      virtual PSTokenState getTokenState() const;
      virtual PSVariable* getStart();
      virtual PSVariable* getEnd();
      virtual PSVariable* getDuration();
            
      virtual double getViolation() const;
      virtual std::string getViolationExpl() const;

      virtual PSList<PSVariable*> getParameters();
      virtual PSVariable* getParameter(const std::string& name);

      virtual void activate();      
      virtual void reject();      
      virtual void merge(PSToken* activeToken);            
      virtual void cancel(); // Retracts merge, activate, reject      

      virtual PSList<PSToken*> getCompatibleTokens(unsigned int limit, bool useExactTest);

      virtual std::string toString();

    protected:
    	TokenId m_tok;
  };      
  
}

#endif


