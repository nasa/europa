#ifndef _H_DbClient
#define _H_DbClient

#include "PlanDatabaseDefs.hh"
#include "PSPlanDatabase.hh"
#include "Variable.hh"
#include <vector>

/**
 * @file Provides the interface for a client to the plan database to invoke operations to change
 * the plan database.
 *
 * Notes:
 * @li We do not expose operations that are conducted internally. For example, restriction of domains
 * through propagation, or insertion of sub-goal tokens.
 * @li We are not exposing the incremental mechanics for building objects and tokens, allowing the constructors to be
 * called, and then any additional parameters or variables to be added before closing the object. Instead, we require
 * that underlying factories deliver fully instantiated and closed instances.
 * @li We are not allowing construction of composed objects. It is expected that all compositions are defined via
 * the constructors of root objects.
 * @li The interfaces are key based, reflecting the expectation that the primary (i.e. only) intent is to play these
 * operations from a file of logged operations from a prior planning run. Retrieval of objects by key is supported
 * via Entity::getEntity().
 * @author Conor McGann, March, 2004.
 */
namespace EUROPA {

  /**
   * @brief Facade to create, update or delete data in the plan database from an external client.
   */
  class DbClient {
  public:
    const DbClientId& getId() const;

    /**
     * @brief Create a variable
     * @param type The type for the variable.
     * @param baseDomain The base domain of the new variable.
     * @param name The name for the variable. Must be unique.
     * @return The Id of the variable created. Will error out rather than return a noId.
     */
    ConstrainedVariableId createVariable(const char* typeName, const AbstractDomain& baseDomain, const char* name, bool isTmpVar = false, bool canBeSpecified=true);

    /**
     * @brief Create a variable
     * @param type The type for the variable.
     * @param name The name for the variable. Must be unique.
     * @return The Id of the variable created. Will error out rather than return a noId.
     */
    ConstrainedVariableId createVariable(const char* typeName, const char* name, bool isTmpVar = false);

    /**
     * @brief Delete a variable.  By way of symmetry with createVariable().
     */
    void deleteVariable(const ConstrainedVariableId& var);

    /**
     * @brief Create an object instance in the dabatase.
     * @param key The expected key value for the object. This is used as a check to ensure we are creating values
     * in the order we expect.
     * @param type The type of instance to create. Must match a name in the Schema. The daatabase must be open for
     * creation of instances of this type.
     * @param name The name for the instance. Must be unique.
     * @return The Id of the object created. Will error out rather than return a noId.
     */
    ObjectId createObject(const char* type, const char* name);

    /**
     * @brief Create an object instance in the dabatase, with a call to a specialized constructor
     * @param key The expected key value for the object. This is used as a check to ensure we are creating values
     * in the order we expect.
     * @param type The type of instance to create. Must match a name in the Schema. The database must be open for
     * creation of instances of this type.
     * @param name The name for the instance. Must be unique.
     * @param arguments A vector of name/value pairs used to invoke a particular constructor.
     * @return The Id of the object created. Will error out rather than return a noId.
     */
    ObjectId createObject(const char* type, const char* name, const std::vector<const AbstractDomain*>& arguments);

    /**
     * @brief Delete an object.  By way of symmetry with createObject().
     */
    void deleteObject(const ObjectId& obj);

    /**
     * @brief Close the database. This will prohibit any further insertion of objects.
     * @see close(const char* objectType)
     */
    void close();

    /**
     * @brief Close the database for further creation of any objects of a given type. Not supported yet. There is no
     * implementation for this yet, since we are not really supporting incremental closure of objects at this time.
     */
    void close(const char* objectType);

    /**
     * @brief Constructs a Token instance.
     * @param predicateName The name of the predicate for which this token is an instance. Must match a name in the
     * schema.
     * @return The Id of the token created. Will error out rather than return a noId.
     */
    TokenId createToken(const char* tokenType,
                        const char* tokenName = NULL,
                        bool rejectable = false,
                        bool isFact = false);

    /**
     * @brief Deletes a token instance.  By way of symmetry with createToken().
     */
    void deleteToken(const TokenId& token, const std::string& name = "");

    /**
     * @brief imposes a constraint such that token comes before successor, on the given object.
     * @param object
     * @param predecessor The token to be the predecessor
     * @param successor The token to be the successor. If 0, the Token is constrained to succeed all
     * other ordered tokens.
     * @return The resulting 'precedes' constraint
     */
    void constrain(const ObjectId& object, const TokenId& predecessor, const TokenId& successor);

    /**
     * @brief Frees any constraints imposed on a Token arising from calls to constrain.
     * @param object The object to which the token has been constrained.
     * @param predecessor The token that is the predecessor
     * @param successor The token that is the successor.
     * @param constraint The constraint to be removed.
     */
    void free(const ObjectId& object, const TokenId& predecessor, const TokenId& successor);

    /**
     * @brief Activate the given token
     * @param token The token to be activated. It must be inactive.
     */
    void activate(const TokenId& token);

    /**
     * @brief Merge the given token
     * @param token The token to be merged. It must be inactive.
     * @param activeTokenKey The token to be merged onto.
     */
    void merge(const TokenId& token, const TokenId& activeToken);

    /**
     * @brief Reject the given token
     * @param token The token to be rejected. It must be inactive.
     */
    void reject(const TokenId& token);

    /**
     * @brief Cancel restriction to Token Variables state through activate, merge, or reject
     * @param token The target token
     */
    void cancel(const TokenId& token);

    /**
     * @brief The initial state may include constraints, even if the planner does not express any decisions
     * as constraints. Must be at least a binary constraint.
     * @param name The name of the constraint to be created
     * @param scope The variables to provide the scope of the constraint.
     */
    ConstraintId createConstraint(const char* name,
				  const std::vector<ConstrainedVariableId>& scope);

    /**
     * @brief Construction of a unary constraint.
     * @param name The name of the constraint to be created
     * &param var the target variable.
     * @param domain The domain to restrict against.
     */
    ConstraintId createConstraint(const char* name,
				  const ConstrainedVariableId& variable,
				  const AbstractDomain& domain);

    /**
     * @brief Delete a constraint.  By way of symmetry with createConstraint().
     */
    void deleteConstraint(const ConstraintId& constr);

    /**
     * @brief Restricts the base domain of a variable
     * @param variable The variable to be restricted
     * @param value The new base domain of the variable.
     * @see getEntityByKey
     */
    void restrict(const ConstrainedVariableId& variable, const AbstractDomain& domain);

    /**
     * @brief Binds the value of a variable
     * @param variable The variable to be bound
     * @param value The value of the variable. All variable values can be cast to a double. If the variable
     * is an object variable, care must be taken to translate the object key to the id and from there obtain the
     * value to specify.
     * @see getEntityByKey
     */
    void specify(const ConstrainedVariableId& variable, edouble value);

    /**
     * @brief Close the domains of a dynamic variable.
     * @param variable The dynamic variable to be closed.=
     */
    void close(const ConstrainedVariableId& variable);

    /**
     * @brief resets the specified domain of the target variable to its base domain
     * @param variable The variable to be reset
     */
    void reset(const ConstrainedVariableId& variable);

    /*!< Support for interaction with ConsistencyManagement */

    /**
     * @brief Force propagation of any pending changes in the system to check consistency.
     * @return true if not proven inconsistent and all data propagated.. Otherwise false.
     * @see ConstraintEngine::propagate()
     */
    bool propagate();

    /**
     * @brief Lookup an object by name. It is an error if the object is not present.
     * @return The requested object
     */
    ObjectId getObject(const char* name) const;

    /**
     * @brief Lookup a global variable by name. It is an error if not present
     * @retrun The requested variable.
     */
    const ConstrainedVariableId getGlobalVariable(const LabelStr& varName) const;

    /**
     * @brief Test if a global exists for a given name
     * @return true if present, otherwise false
     */
    bool isGlobalVariable(const LabelStr& varName) const;

    /**
     * @brief Lookup a global token by name. It is an error if not present
     * @retrun The requested token.
     */
    const TokenId getGlobalToken(const LabelStr& name) const;

    /**
     * @brief Test if a global exists for a given name
     * @return true if present, otherwise false
     */
    bool isGlobalToken(const LabelStr& name) const;

    /**
     * @brief Retrieve token defined by a particular path from a root token. Transaction Logging must be enabled.
     * For example: [1265,2,3,1,0,9] will return (Token 1265).slave(2).slave(3).slave(1).slave(0).slave(9)
     * @param relativePath The relative path to find the target token where each vector position reflects the position in the ordered
     * set of slaves for the parent. The first location in the path is a token we expect to find directly by key.
     * @return The token given by the path. If no token found, return TokenId::noId()
     * @see Token::getChild(int slavePosition)
     * @see enableTransactionLogging
     */
    TokenId getTokenByPath(const std::vector<int>& relativePath) const;

    /**
     * @brief Retrieve the relative path for obtaining the target token from a given root token.
     * Transaction Logging must be enabled.
     * @param targetToken The token to find a path to from its root in the ancestor tree
     * @return The vector giving its relative path. The first element is the root token key. Subsequent elements represent slave positions.
     * @see getTokenByPath
     * @see enableTransactionLogging
     */
    std::vector<int> getPathByToken(const TokenId& targetToken) const;

    /**
     * @brief Retrieve the relative path for obtaining the target token from a given root token.
     * Transaction Logging must be enabled.
     * @param targetToken The token to find a path to from its root in the ancestor tree
     * @return A string of the vector giving its relative path. The first element is the root token key
     * @see getPathByToken
     * @see enableTransactionLogging
     */
    std::string getPathAsString(const TokenId& targetToken) const;

    /**
     * @brief Retrieve a constrained variable of any type based on its 'index'
     */
    ConstrainedVariableId getVariableByIndex(unsigned int index);

    /**
     * @brief Retrieve an index for a variable. Required for logging. Tricks will have to be done to make
     * this fast!
     */
    unsigned int getIndexByVariable(const ConstrainedVariableId& var);

    ConstraintId getConstraintByIndex(unsigned int index);

    unsigned int getIndexByConstraint(const ConstraintId& constr);

    /**
     * @brief Adds a listener to operations invoked on the client
     */
    void notifyAdded(const DbClientListenerId& listener);

    /**
     * @brief Removes a listener
     */
    void notifyRemoved(const DbClientListenerId& listener);


    /**
     * @brief Check for consistency
     */
    bool constraintConsistent() const;


    /**
     * @brief Used to determine if system has the necessary components in place to support
     * automatic token allocation in the Plan Database.
     */
    bool supportsAutomaticAllocation() const;

    /**
     * @brief Enables transaction logging. This is necessary if you wisk to replay or log transactions.
     * Must be disabled.
     */
    void enableTransactionLogging();

    /**
     * @brief Disables transaction logging. Must be enabled.
     */
    void disableTransactionLogging();

    /**
     * @brief Test of Transaction logging is enabled.
     * @see enableTransactionLoggng
     */
    bool isTransactionLoggingEnabled() const;

    /**
     * @brief Create a value for a string
     */
    edouble createValue(const char* typeName, const std::string& value);
        
    // Temporarily exposing these to remove singletons, need to review DbClient concept in general
    const CESchemaId& getCESchema() const;
    const SchemaId& getSchema() const;

  private:
    friend class PlanDatabase;

    DbClient(const PlanDatabaseId& db);
    ~DbClient();
    DbClient(); /* NO IMPL */
    DbClient(const DbClient&); /* NO IMPL */

    /*!< Helper methods */
    TokenId allocateToken(const char* tokenType,
                          const char* tokenName,
                          bool rejectable,
                          bool isFact=false);

    DbClientId m_id;
    PlanDatabaseId m_planDb;
    std::vector<eint> m_keysOfTokensCreated; /*!< Used for managing instance independent paths */
    std::set<DbClientListenerId> m_listeners; /*! Stores current DbClientListeners */
    bool m_deleted; /*!< Used to indicate a deletion and this ignore synchronization of listeners on removal */
    bool m_transactionLoggingEnabled; /*!< Used to configure transaction loggng services required for Key Matching */
  };

  class PSPlanDatabaseClientImpl : public PSPlanDatabaseClient
  {
    public:
      PSPlanDatabaseClientImpl(const DbClientId& c);

      virtual PSVariable* createVariable(const std::string& typeName, const std::string& name, bool isTmpVar);
      virtual void deleteVariable( PSVariable* var);

      virtual PSObject* createObject(const std::string& type, const std::string& name);
      //virtual PSObject* createObject(const std::string& type, const std::string& name, PSList<PSVariable*>& arguments);
      virtual void deleteObject(PSObject* obj);

      virtual PSToken* createToken(const std::string& predicateName, bool rejectable, bool isFact);
      virtual void deleteToken(PSToken* token);

      virtual void constrain(PSObject* object, PSToken* predecessor, PSToken* successor);
      virtual void free(PSObject* object, PSToken* predecessor, PSToken* successor);
      virtual void activate(PSToken* token);
      virtual void merge(PSToken* token, PSToken* activeToken);
      virtual void reject(PSToken* token);
      virtual void cancel(PSToken* token);

      virtual PSConstraint* createConstraint(const std::string& name, PSList<PSVariable*>& scope);
      virtual void deleteConstraint(PSConstraint* constr);

      virtual void specify(PSVariable* variable, double value);
      virtual void reset(PSVariable* variable);

      virtual void close(PSVariable* variable);
      virtual void close(const std::string& objectType);
      virtual void close();

    protected:
      DbClientId m_client;

      ConstrainedVariableId toId(PSVariable* v);
      ConstraintId          toId(PSConstraint* c);
      ObjectId              toId(PSObject* o);
      TokenId               toId(PSToken* t);
  };

}

#endif
