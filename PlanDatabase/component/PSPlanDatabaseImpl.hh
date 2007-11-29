#ifndef _H_PSPlanDatabaseImpl
#define _H_PSPlanDatabaseImpl

#include "PSPlanDatabase.hh"
#include "PlanDatabaseDefs.hh"

namespace EUROPA {

  class PSObjectImpl : public virtual PSObject
  {
    public:
      virtual ~PSObjectImpl();
  
      virtual const std::string& getEntityType() const;

      virtual std::string getObjectType() const; 

      virtual PSList<PSVariable*> getMemberVariables();
      virtual PSVariable* getMemberVariable(const std::string& name);

      virtual PSList<PSToken*> getTokens();

      virtual void addPrecedence(PSToken* pred,PSToken* succ);
      virtual void removePrecedence(PSToken* pred,PSToken* succ);

    protected:
    	friend class PSEngineImpl;
    	friend class PSTokenImpl;
    	friend class PSVarValue;
    	friend class PSVariableImpl;
    	friend class BaseObjectWrapperGenerator;
    	PSObjectImpl(const ObjectId& obj);

    private:
    	ObjectId m_obj;
  };

  class PSTokenImpl : public PSToken
  {	    
    public:
      virtual ~PSTokenImpl() {}

      virtual const std::string& getEntityType() const;

      virtual std::string getTokenType() const; 

      virtual bool isFact(); 

      virtual PSObject* getOwner(); 

      virtual PSToken* getMaster();

      virtual PSList<PSToken*> getSlaves();

      virtual double getViolation() const;
      virtual std::string getViolationExpl() const;

      //Traditionally, the temporal variables and the object and state variables aren't 
      //considered "parameters".  I'm putting them in for the moment, but clearly the token
      //interface has the least thought put into it. ~MJI
      virtual PSList<PSVariable*> getParameters();
      virtual PSVariable* getParameter(const std::string& name);

      virtual void activate();      

      virtual std::string toString();

    protected:
    	friend class PSEngineImpl;
    	friend class PSObjectImpl;
    	friend class PSVariableImpl;
    	PSTokenImpl(const TokenId& tok);

    	TokenId m_tok;
  };      
  
}

#endif


