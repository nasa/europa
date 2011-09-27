#ifndef _H_Token
#define _H_Token

/**
 * @author Conor McGann
 */

#include "PlanDatabaseDefs.hh"
#include "UnifyMemento.hh"
#include "StackMemento.hh"
#include "MergeMemento.hh"
#include "Schema.hh"
#include "Entity.hh"
#include "LabelStr.hh"
#include "Domains.hh"
#include "PlanDatabase.hh"

#include <vector>
#include <set>

namespace EUROPA {

  /**
   * @class Token
   * @brief A Token is a relation among variables that has some temporal scope, and describes state or action of an object.
   *
   * We make the following assumptions:
   * @li A Token's semantics are defined by its predicate.
   * @li All Tokens say something about an object.
   * @li One can speak of the start, end or duration of a Token when dealing with the temporal scope of a Token.
   * @li The following relationship holds among the temporal variables: start + duration == end.
   */
  class Token: public virtual PSToken, public Entity {
  public:
    DECLARE_ENTITY_TYPE(Token);

    /**
     * Begin Declaration of allowable states for a Token.
     */
    static const LabelStr INCOMPLETE; /*!< The Token has been created, but additional
				       parameter variables have yet to be added before use.*/

    static const LabelStr INACTIVE; /*!< The Token has been created and closed. Its variables
				      are accessible and may be constrained, and/or specified. */

    static const LabelStr ACTIVE; /*!< The Token has been activated. It may now support merged
				    tokens, and cannot itself be merged or rejected. It may also
				    be related to an object; i.e. can be added to the object.
				    Only active tokens are related to objects.*/

    static const LabelStr MERGED; /*!< The Token has been merged i.e. it is represented in the plan
				    by the active token it has been merged with. */

    static const LabelStr REJECTED; /*!< The Token has been rejected. Only master tokens can be rejected. */

    virtual ~Token();

    /**
     * @brief Get the internal Id.
     */
    const TokenId& getId() const;

    /**
     * @brief Get the name
     */
    const LabelStr& getName() const;

    /**
     * @brief Set the name
     */
    void setName(const LabelStr& name);

    /**
     * @brief Accessor for the master token for this token.
     * @return TokenId::noId() if this is a Master Token, otherwise returns the Token from
     * which this token was sub-goaled.
     */
    const TokenId& master() const;

    /**
     * @brief Accessor for the relation to the master token.
     * @return one of the supported allen relations (see documentation)
     */
    const LabelStr& getRelation() const;

    /**
     * @brief Obtain a slave token using a positional offset from this token
     * @return TokenId::noId() if no token is found at that location
     */
    const TokenId& getSlave(int slavePosition) const;

    /**
     * @brief Obtain the position of the given slave token in its ordered set of slaves.
     * @param slave The slave token to be found. It must be present.
     * @return Will return -1 if not found, otherwise will give the 0 based position.
     */
    int getSlavePosition(const TokenId& slave) const;

    /**
     * @brief All tokens are part of exactly one PlanDatabase
     */
    const PlanDatabaseId& getPlanDatabase() const;

    /**
     * @brief Access to the base type of the predicate.
     *
     * Note that since we have an inheritance mechanism, the token may actually be associated with an instance of a derived
     * class.
     */
    const LabelStr& getBaseObjectType() const;

    /**
     * @brief Access the predicate name for the token.
     *
     * This information is used for type checking with respect to the schema and is essential to the semantics of the token.
     * @see Schema::isPredicateDefined, Schema::canBeAssigned, Schema::canContain
     */
    const LabelStr& getPredicateName() const;

    /**
     * @brief Access to the unqualified predicate name (if it has delimiters they are stripped).
     */
    const LabelStr& getUnqualifiedPredicateName() const;

    /**
     * @brief Obtain the variable used to store reachable states. The full domain is INCOMPLETE, ACTIVE, MERGED and REJECTED.
     *
     * Operations controlling the lifecyle of the token control this variable. It cannot be specified directly.
     * @see close, activate, merge, reject, cancel.
     */
    const StateVarId& getState() const;

    /**
     * @brief In a fully grounded plan, a Token is assigned to a single object. The domain of this variable indicates
     * the possible assigments remaining.
     */
    const ObjectVarId& getObject() const;

    /**
     * @brief The start variable is a pure virtual function since we derived classes with no duration
     * may have an optimized implementation that shares start and end variable as the same variable.
     */
    virtual const TempVarId& start() const = 0;

    /**
     * @brief The end variable is a pure virtual function since we derived classes with no duration
     * may have an optimized implementation that shares start and end variable as the same variable.
     */
    virtual const TempVarId& end() const = 0;

    /**
     * @brief All Tokens will have a temporal duration.
     */
    const TempVarId& duration() const;

    /**
     * @brief Access all Parameter variables for the token. May be empty.
     */
    const std::vector<ConstrainedVariableId>& parameters() const;

    /**
     * @brief Access all variables (state, object, start, end, duration, parameters).
     */
    const std::vector<ConstrainedVariableId>& getVariables() const;

    /**
     * @brief Access all variables (state, object, start, end, duration, parameters).
     */
    const ConstrainedVariableId getVariable(const LabelStr& name, bool checkGlobalContext=true) const;

    /**
     * @brief Access all tokens generated as sub-goals of this token.
     */
    const TokenSet& slaves() const;

    /**
     * @brief Access all tokens supported by this token. A Token must be active to support merged tokens.
     */
    const TokenSet& getMergedTokens() const;

    /**
     * @brief Access the active token supporting this token, if this is a merged token.
     */
    const TokenId& getActiveToken() const;

    /**
     * @brief add a built in constraint for the token
     */
    void addStandardConstraint(const ConstraintId& constraint);

    /**
     * @brief Test if a given constraint is a built in constraint for the token
     */
    bool isStandardConstraint(const ConstraintId& constraint) const;

    /**
     * @brief Internally generated constraints that are standard across Token instances of the same type.
     */
    const std::set<ConstraintId>& getStandardConstraints() const;

    /**
     * @brief Sum of violation value for all the constraints attached to this token
     */
    virtual double getViolation() const;

    /**
     * @brief Concatenation of violation expl for all the constraints attached to this token
     */
    virtual std::string getViolationExpl() const;

    /**< State checks */
    bool isIncomplete() const;

    /**
     * @brief Test if a token is pending commitment. Typically, all tokens in planning are in a
     * pending state. We should not plan with committed tokens.
     * @todo Evaluate enforcing this rigorously in flaw management.
     * @see canBeCommitted
     */
    bool isPending() const;

    /**
     * @brief Test if a token has been committed.
     * @see commit
     */
    bool isCommitted() const;

    /**
     * @brief Test if a token can be committted. It must be active, and currently uncommitted.
     * @see pending
     */
    bool canBeCommitted() const;


    /**
     * @brief True if the token was established as a fact
     */
    virtual bool isFact() const { return m_isFact; }

    /**
     * @brief Token becomes a fact, it's ok to call if token is already a fact
     */
    void makeFact();

    /**
     * Substates of Pending
     */
    bool isActive() const;
    bool isInactive() const;
    bool isMerged() const;
    bool isRejected() const;

    /**< State change operations */

    /**
     * @brief Close the token, indicating no more parameters are to be added. It will become INACTIVE
     * as a result of this operation.
     */
    virtual void close();

    bool isClosed() const;

    /**
     * @brief Commits a token, the token will not be deleted when all the merged tokens are un-merged.
     *
     * The semantics of commitment are to state that a token is fully justified in the plan independently
     * of any other pre-existing justification. A temporally scoped fact in the initial conditions of a plan
     * might be considered already commited. Such a token would not transition into the active state, but would begin
     * with the base domain of the state being active.
     */
    void commit();

    /**
     * @brief Restrict base domains to current derived domains. Applies to all token and extended token variables.
     */
    void restrictBaseDomains();

    /**
     * @brief Test if the token has been terminated.
     */
    bool isTerminated() const;

    /**
     * @brief Test if the token can be terminated. Must not have active constraints.
     * @param tick The current clock value
     */
    bool canBeTerminated(eint tick) const;

    /**
     * @brief Terminate a token, disconnects it from propagation.
     */
    void terminate();

    /**
     * @brief Merge this token onto the given active token, thereby satisfying its requirements by unification
     * with a pre existing token in the plan.
     *
     * @pre isInactive
     * @post isMerged
     */
    void doMerge(const TokenId& activeToken);

    /**
     * @brief Activate a token, thereby inserting it into the plan.
     *
     * @pre isInactive
     * @post isActive
     */
    virtual void activate();

    /**
     * @brief Reject a token, thereby removing it from consideration in the plan.
     *
     * @pre isInactive
     * @post isRejected
     */
    virtual void reject();

    /**
     * @brief Retracts :merge, activate, reeject
     *
     * @pre isClosed() && !isInactive()
     * @post isInactive
     *
     * @see split
     * @see deactivate
     * @see reinstate
     */
    virtual void cancel();

    /**
     * @brief Invoked when a constraint is added to a merged token
     */
    void handleAdditionOfInactiveConstraint(const ConstraintId& constraint);

    /**
     * @brief Invoked when a constaint is removed from a merged token
     */
    void handleRemovalOfInactiveConstraint(const ConstraintId& constraint);

    /**
     * @brief Test of the token is assigned to an object.
     *
     * Will only be true if the token is active, and the
     * derived domain of the object variable is a singleton, and the inderlying singleton object says that
     * it is in fact assigned.
     */
    bool isAssigned() const;

    /**
     * @brief Test if the token is deleted.
     */
    bool isDeleted() const;

    // See PSTokenType::TokenAttribute
    virtual int getAttributes() const;
    virtual void setAttributes(int attrs);
    virtual void addAttributes(int attrMask);
    virtual bool hasAttributes( int attrMask ) const;

    /**
     * @brief Test if the variable is a token state variable
     */
    static bool isStateVariable(const ConstrainedVariableId& var);

    void addLocalVariable(const ConstrainedVariableId& var);
    void removeLocalVariable(const ConstrainedVariableId& var);
    const ConstrainedVariableSet& getLocalVariables();

    static const LabelStr& noObject();

    /**
     * @brief Utility for allocating pseudo variable names such that there are no duplicates. Duplicates can be dangerous
     * since associative maps look up variables by name and can lead to mix-ups.
     */
    static LabelStr makePseudoVarName();

    /**
     * @brief Add a parameter as a member to the object. This is used when building the instance
     * and cannot be called once the specific token instance has been closed.
     * @param baseDomain The base domain to use to populate the variable
     * @param name The member name
     * @error If a parameter of the given type with the given name cannot be added
     * @see Scheme::hasMember, Schema::canContain, Token::close()
     */
    template<class DomainType>
    ConstrainedVariableId addParameter(const DomainType& baseDomain, const LabelStr& name){
      check_error(isIncomplete(),
		  "Cannot add parameter " + name.toString() +
		  " after completing token construction.");

      check_error(m_planDatabase->getSchema()->canContain(m_predicateName, baseDomain.getTypeName(), name),
		  "Predicate '" + m_predicateName.toString() +
		  "' cannot contain parameter '" + name.toString() + "'");

      ConstrainedVariableId id = (new TokenVariable<DomainType>(m_id,
								m_allVariables.size(),
								m_planDatabase->getConstraintEngine(),
								baseDomain,
								false,
								true,
								name))->getId();
      m_parameters.push_back(id);
      m_allVariables.push_back(id);
      return id;
    }

    /**
     * @brief Tests if the given entity can be compared to this token
     */
    bool canBeCompared(const EntityId& entity) const;

    /**
     * @brief Indicates the master token is being removed. Token may self-destruct.
     * @param token The token just unmerged
     * @return true if the token self-destructed
     */
    bool removeMaster(const TokenId& token);

    virtual std::string toLongString() const;

    // PS Methods:
    virtual const std::string& getEntityType() const;
    virtual std::string getTokenType() const;
    virtual std::string getFullTokenType() const;

    virtual PSTokenState getTokenState() const;
    virtual PSVariable* getStart() const;
    virtual PSVariable* getEnd() const;
    virtual PSVariable* getDuration() const;

    virtual PSObject* getOwner() const;
    virtual PSToken* getMaster() const;
    virtual PSList<PSToken*> getSlaves() const;

    virtual PSToken* getActive() const;
    virtual PSList<PSToken*> getMerged() const;

    virtual PSList<PSVariable*> getParameters() const;
    virtual PSList<PSVariable*> getPredicateParameters() const;
    virtual PSVariable* getParameter(const std::string& name) const;

    virtual void merge(PSToken* activeToken);

    // returns active tokens that this token can be merged to
    virtual PSList<PSToken*> getCompatibleTokens(unsigned int limit, bool useExactTest);


  protected:

    /**
     * @brief Constructor for master token creation.
     */
    Token(const PlanDatabaseId& planDatabase,
          const LabelStr& predicateName,
          bool rejectable,
          bool isFact,
          const IntervalIntDomain& durationBaseDomain,
          const LabelStr& objectName,
          bool closed);

    /**
     * @brief Constructor for slave token creation.
     */
    Token(const TokenId& master,
	  const LabelStr& relation,
	  const LabelStr& predicateName,
          const IntervalIntDomain& durationBaseDomain,
          const LabelStr& objectName,
          bool closed);

    /**
     * Concrete methods to reveres particular states
     */
    void split();
    void deactivate();
    void reinstate();

    void add(const TokenId& slave);
    void remove(const TokenId& slave);

    /**
     * Used to allow derived classes to restrict state on creation.
     */
    void activateInternal();

    /**
     * @brief Will insert token in set of merged tokens.
     * @param token The merged token
     */
    void addMergedToken(const TokenId& token);

    /**
     * @brief Will remove the token from the set of merged tokens. Token may self-destruct.
     * @param token The token just unmerged
     * @return true if the token self-destructed
     */
    bool removeMergedToken(const TokenId& token);

    TokenId m_id;
    LabelStr m_name;
    TokenId m_master;
    LabelStr m_relation;
    LabelStr m_baseObjectType;
    LabelStr m_predicateName;
    StateVarId m_state; /*!< state variable for token.*/
    ObjectVarId m_object; /*!< object variable for token. The set of objects it may be assigned to. */
    TempVarId m_duration; /*!< The duration of the token. [0 +inf]. */
    bool m_isFact;
    int m_attributes;
    std::vector<ConstrainedVariableId> m_parameters; /*!< The parameters of the token specification. May be empty */
    std::vector<ConstrainedVariableId> m_allVariables; /*!< The set of all variables of a token specification. Includes built in variables
							 such as object, state, start, end, duration. Also includes all parameters (m_parameters). */
    TokenSet m_slaves;
    std::set<ConstraintId> m_standardConstraints; /**< Indicates internally generated constraints that are standard
                                                     across Token instances of the same type. */
    std::vector<ConstrainedVariableId> m_pseudoVariables; /**< Indicates internally generated variables that are standard
                                                             across token instances of the same type. Pseudo variables cannot be specified
                                                             externally. */
    const PlanDatabaseId m_planDatabase;

    /**
     * @brief Handle deallocation
     */
    void handleDiscard();

  private:


    bool isValid() const;

    /**
     * @brief Shared initialization code across master and slave constructors
     * @see Token::Token
     */
    void commonInit(const LabelStr& predicateName,
                    bool rejectable,
                    bool isFact,
                    const IntervalIntDomain& durationBaseDomain,
                    const LabelStr& objectName,
                    bool closed);

    /**
     * @brief Used for validation purposes only. Tests if there are any constraints on variables of this token
     * that involve variables from another source.
     */
    bool noExternalConstraints() const;

    TokenSet m_mergedTokens;
    TokenId m_activeToken;
    UnifyMementoId m_unifyMemento;
    bool m_committed;
    unsigned int m_refCount; /*!< The number of sources requiring existence of the token */
    bool m_deleted;
    bool m_terminated;
    ConstrainedVariableSet m_localVariables; /*!< Variables created external to the token but related to it. They are
					       not part of the predicate definition but may be derived from the model elsewhere
					       such as via local rule variables.*/
    LabelStr m_unqualifiedPredicateName;
  };

  class StateDomain : public EnumeratedDomain {
  public:
    StateDomain();
    StateDomain(const Domain& org);
    virtual void operator>>(ostream& os) const;
  };
}

#endif
