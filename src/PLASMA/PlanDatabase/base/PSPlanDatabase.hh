#ifndef _H_PSPlanDatabase
#define _H_PSPlanDatabase

#include "PSConstraintEngine.hh"

namespace EUROPA {
	enum PSTokenState { INACTIVE,ACTIVE,MERGED,REJECTED };

  class PSObject;
  class PSToken;
  class PSPlanDatabaseClient;

  class PSSchema : public EngineComponent
  {
    public:
        // TODO: flesh this interface out
      virtual ~PSSchema() {}
      virtual PSList<std::string> getAllPredicates() const = 0;
      virtual PSList<std::string> getMembers(const std::string& objectType) const = 0;
      virtual bool hasMember(const std::string& parentType, const std::string& memberName) const = 0;
  };

  class PSPlanDatabase : public EngineComponent
  {
    public:
      virtual ~PSPlanDatabase() {}

    virtual PSList<PSObject*> getAllObjects() const = 0;
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

      virtual PSList<PSToken*> getTokens() const = 0;

      virtual void addPrecedence(PSToken* pred,PSToken* succ) = 0;
      virtual void removePrecedence(PSToken* pred,PSToken* succ) = 0;

      virtual PSVarValue asPSVarValue() const = 0;

      static PSObject* asPSObject(PSEntity* entity);
  };


  class PSToken : public virtual PSEntity
  {
  public:

      PSToken() {}
      virtual ~PSToken() {}

      virtual const std::string& getEntityType() const = 0;
      virtual std::string getTokenType() const = 0;
    virtual std::string getFullTokenType() const = 0;

      virtual bool isFact() const = 0;
      virtual bool isIncomplete() const = 0;

      virtual PSTokenState getTokenState() const = 0;
      virtual PSVariable* getStart() const = 0;
      virtual PSVariable* getEnd() const = 0;
      virtual PSVariable* getDuration() const = 0;

      virtual PSObject* getOwner() const = 0;
      virtual PSToken* getMaster() const = 0;
      virtual PSList<PSToken*> getSlaves() const = 0;

      virtual PSToken* getActive() const = 0;
      virtual PSList<PSToken*> getMerged() const = 0;

      virtual double getViolation() const = 0;
      virtual std::string getViolationExpl() const = 0;

      virtual PSList<PSVariable*> getParameters() const = 0;
      virtual PSVariable* getParameter(const std::string& name) const = 0;
    virtual PSList<PSVariable*> getPredicateParameters() const = 0;

      virtual void activate() = 0;
      virtual void reject() = 0;
      virtual void merge(PSToken* activeToken) = 0;
      virtual void cancel() = 0; // Retracts merge, activate, reject

      // returns active tokens that this token can be merged to
      virtual PSList<PSToken*> getCompatibleTokens(unsigned int limit, bool useExactTest) = 0;

  };

  class PSPlanDatabaseClient
  {
    public:
      PSPlanDatabaseClient() {}
      virtual ~PSPlanDatabaseClient() {}

      virtual PSVariable* createVariable(const std::string& typeName, const std::string& name, bool isTmpVar) = 0;
      virtual void deleteVariable(PSVariable* var) = 0;

      virtual PSObject* createObject(const std::string& type, const std::string& name) = 0;
      //virtual PSObject* createObject(const std::string& type, const std::string& name, const PSList<PSVariable*>& arguments) = 0;
      virtual void deleteObject(PSObject* obj) = 0;

      virtual PSToken* createToken(const std::string& predicateName, bool rejectable, bool isFact) = 0;
      virtual void deleteToken(PSToken* token) = 0;

      virtual void constrain(PSObject* object, PSToken* predecessor, PSToken* successor) = 0;
      virtual void free(PSObject* object, PSToken* predecessor, PSToken* successor) = 0;
      virtual void activate(PSToken* token) = 0;
      virtual void merge(PSToken* token, PSToken* activeToken) = 0;
      virtual void reject(PSToken* token) = 0;
      virtual void cancel(PSToken* token) = 0;

      virtual PSConstraint* createConstraint(const std::string& name, PSList<PSVariable*>& scope) = 0;
      virtual void deleteConstraint(PSConstraint* constr) = 0;

      virtual void specify(PSVariable* variable, double value) = 0;
      virtual void reset(PSVariable* variable) = 0;

      virtual void close(PSVariable* variable) = 0;
      virtual void close(const std::string& objectType) = 0;
      virtual void close() = 0;
  };
}

#endif
