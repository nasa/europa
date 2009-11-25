#ifndef _H_Object
#define _H_Object

/**
 * @file   Object.hh
 * @author Conor McGann
 * @brief
 * @ingroup PlanDatabase
 */


#include "PlanDatabaseDefs.hh"
#include "LabelStr.hh"
#include "Variable.hh"
#include "Domains.hh"
#include "PSPlanDatabase.hh"

#include <set>
#include <vector>

namespace EUROPA {

  /**
   * @brief Used to represent instances of entities in a plan database. For example, a Rover in a domain model will be represented
   * as object with type Rover in the plan database.
   */

	// XXX:  CANNOT INHERIT VIRTUALLY FROM 'Entity' or pointers get messed up!

  class Object: public virtual PSObject, public Entity {
  public:
    DECLARE_ENTITY_TYPE(Object);

    enum State { INCOMPLETE = 0, /**< The object has been constructed but not all variables have been added yet. */
                 COMPLETE /**< All variables have been added. */
    };

    Object(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name, bool open = false);

    Object(const ObjectId& parent, const LabelStr& type, const LabelStr& localName, bool open = false);

    virtual ~Object();

    virtual void constructor(const std::vector<const AbstractDomain*>& arguments) {}

    /**
     * @brief Add a variable as a member to the object. This is used when building the object
     * and cannot be called once the specific object instance has been closed.
     * @param baseDomain The base domain to use to populate the variable
     * @param name The member name
     * @error If a variable of the given type with the given name cannot be added
     * @see Scheme::hasMember, Schema::canContain, Object::close()
     */
    virtual ConstrainedVariableId addVariable(const AbstractDomain& baseDomain, const char* name);

    const ObjectId& getId() const;

    const ObjectId& getParent() const;

    const PlanDatabaseId& getPlanDatabase() const;

    /**
     * @brief Access a variable which includes this object only.
     */
    const ConstrainedVariableId& getThis() const;

    /**
     * @brief Access the type name for which this object is an instance
     */
    const LabelStr& getType() const;

    /**
     * @brief Access the root type in the class hierarchy starting from this instance
     */
    const LabelStr getRootType() const;

    /**
     * @brief Access the instance name of this object.
     * Instance names include the full path, delimited by ':' in the composition hierarchy.
     */
    virtual const LabelStr& getName() const;



    /**
     * @brief Obtain the key ordered set of component objects i.e. objects constructed with this
     * object as their parent.
     */
    const ObjectSet& getComponents() const;

    /**
     * @brief Obtain the list of all objects in the database which are instances of an ancestor class
     * of this object.
     * @param results The output. Should be passed in empty.
     */
    void getAncestors(std::list<ObjectId>& results) const;

    /**
     * @brief Retrieves all active tokens whose object variable includes this object
     */
    const TokenSet& tokens() const;

    /**
     * @brief Test if the given token is actually assigned to this object.
     * @see constrain
     */
    virtual bool hasToken(const TokenId& token) const;

    /**
     * @brief Get all possible active tokens on this object which may be used to order the given token.
     * @param token The Token for which we want to evaluate possible choices
     * @param results Will be populated with the choices for constraining this token.
     * @see constrain
     */
    virtual void getOrderingChoices(const TokenId& token,
				    std::vector< std::pair< TokenId, TokenId> >& results,
				    unsigned int limit = std::numeric_limits<unsigned int>::max());

    /**
     * @brief Count the number of ordering choices for a token. Cut off if we hit the given limit
     * @param token The Token for which we want to count possible choices
     * @param limit The max we don't want to count past
     */
    unsigned int countOrderingChoices(const TokenId& token, unsigned int limit);

    /**
     * @brief Retrieve the last computed value when a call was made to countOrderingChoices.
     * @param token The token for which we want choice count.
     * @return The last value obtained from countOrderingChoices
     */
    unsigned int lastOrderingChoiceCount(const TokenId& token) const;

    /**
     * @brief Get all possible active tokens on this object which require ordering.
     * @see hasTokensToOrder
     */
    virtual void getTokensToOrder(std::vector<TokenId>& results);

    /**
     * @brief Tests if there are any active tokens on this object that must be ordered.
     * @see getTokensToOrder
     */
    virtual bool hasTokensToOrder() const;

    /**
     * @brief imposes a constraint such that token comes before successor. May additinally impose
     * the constraint that the object variable of each token is restricted to this object. We do not restrict
     * the case where predecessor == successor. This will simply constrain the token to this object.
     * @param token The token to be the predecessor
     * @param successor The token to be the successor.
     * @see free
     */
    virtual void constrain(const TokenId& predecessor, const TokenId& successor);

    /**
     * @brief Removes the specific constraint which must have been created by calling 'constrain'. May
     * additionally remove implied constraints to constrain tokens to an object.
     * @param token The token that is the predecessor
     * @param successor The token that is the successor
     * @see constrain
     */
    virtual void free(const TokenId& predecessor, const TokenId& successor);

    /**
     * @brief Get the collection of all member variables of this object in the order in which they
     * were added.
     */
    const std::vector<ConstrainedVariableId>& getVariables() const;

    /**
     * @brief Obtain a variable by name. Variable names must be unique within the context of an object.
     * @param name The name for the requested variable.
     * @error Will fail if a variable of the given name cannot be found
     */
    const ConstrainedVariableId& getVariable(const LabelStr& name) const;

    /**
     * @brief Obtain the variable by traversing a path. Requires that all contained members along the way
     * are singletons.
     * @param path The index based path for traversing objects structures.
     * @error Will fail if there are non-singleton fields or the path is not correct.
     */
    ConstrainedVariableId getVariable(const std::vector<unsigned int>& path) const;

    /**
     * @brief Indicates that the object construction has been completed. Prerequisitie
     * is that !isComplete(). Afterwards, isComplete().
     * @see isComplete, addVariable
     */
    virtual void close();

    /**
     * @brief Tests if the object has been closed
     * @return true if close() has been called, otherwise false.
     */
    bool isComplete() const;

    /**
     * @brief Tests if the given token has been constrained to associate exclusively with this
     * object.
     * @param token The token of interest.
     * @return true if constrain(token) has been called. Othwerwise false.
     */
    bool isConstrainedToThisObject(const TokenId& token) const;


    /**
     * @brief tests if the given tokens have been ordered explicitly for this object.
     * @param predecessor Candidate token constrained to precede successor
     * @param successor Candidate token constrained to succeed predecessor
     */
    bool isConstrainedToPrecede(const TokenId& predecessor, const TokenId& successor) const;

    /**
     * @brief Tests if the given entity can be compared to an object
     */
     bool canBeCompared(const EntityId& entity) const;

    /**
     * @brief Notify the object that a token that might have been added to this object has been merged.
     */
    virtual void notifyMerged(const TokenId& token);

    /**
     * @brief Notify the object that a token that might have been added to this object has been rejected.
     */
    virtual void notifyRejected(const TokenId& token);

    /**
     * @brief Notify the object that a token that might have been added to this object has been deleted
     */
    virtual void notifyDeleted(const TokenId& token);

  /*
   * Hack! Code generation currently skips the factories and directly calls the constructor that specifies the parent,
   * so this is necessary for the interpreter to provide the same behavior
   * Everybody should be going through the factories
   */
    void setParent(const ObjectId& parent);


    /**
     * @brief Retrieve verbose a String description.
     */
    static std::string toString(ObjectVarId objVar);
    std::string toString() const;
    std::string toLongString() const;

    // Looking for pairwise precedence constraints
    ConstraintId getPrecedenceConstraint(const TokenId& predecessor, const TokenId& successor) const;

    void getPrecedenceConstraints(const TokenId& token,  std::vector<ConstraintId>& results) const;

    // PS Methods:
    virtual const std::string& getEntityType() const;

    virtual std::string getObjectType() const;

    virtual PSList<PSVariable*> getMemberVariables();
    virtual PSVariable* getMemberVariable(const std::string& name);

    virtual PSList<PSToken*> getTokens() const;

    virtual void addPrecedence(PSToken* pred,PSToken* succ);
    virtual void removePrecedence(PSToken* pred,PSToken* succ);

    virtual PSVarValue asPSVarValue() const;

  protected:

    /**
     * @brief Handle deallocation
     */
    void handleDiscard();

    Object(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name, bool hasVariables, bool notify);

    // Calls for managing object - token connections
    friend class ObjectTokenRelation;
    virtual void add(const TokenId& token);
    virtual void remove(const TokenId& token);

    // Calls for managing object composition
    virtual void add(const ObjectId& component);
    virtual void remove(const ObjectId& component);
    void cascadeDelete();

    /**
     * @brief Determine if this token participates in any explicit constraints on the timeline
     */
    bool hasExplicitConstraint(const TokenId& token) const;

    /**
     * @brief Remove constraints added implicitly which include this token.
     * @see free, constrain
     */
    void freeImplicitConstraints(const TokenId& token);

    // Other utilities
    bool isValid() const;
    void notifyOrderingRequired(const TokenId& token);
    void notifyOrderingNoLongerRequired(const TokenId& token);

    /**
     * @brief Provides the meat of the constrain behaviour. Allows for useage internally
     * and externally.
     * @param predecessor The token to be ordered first
     * @param successor The token to be ordered second
     * @param isExplicit true if the source is outside the elements of the plan database, false.
     */
    void constrain(const TokenId& predecessor, const TokenId& successor, bool isExplicit);

    /**
     * @brief Provides the meat of the free behaviour. Allows for useage internally
     * and externally.
     * @param predecessor The token to be ordered first
     * @param successor The token to be ordered second
     * @param isExplicit true if the source is outside the elements of the plan database, false.
     */
    void free(const TokenId& predecessor, const TokenId& successor, bool isExplicit);

    void removePrecedenceConstraint(const ConstraintId& constraint);

    /**
     * @brief Utility to generate a hashkey for a token pair
     */
    static int makeKey(const TokenId& a, const TokenId& b);

    ObjectId m_id;
    ObjectId m_parent;
    LabelStr m_type;
    LabelStr m_name;
    const PlanDatabaseId m_planDatabase;
    State m_state;
    ObjectSet m_components;
    TokenSet m_tokens;
    std::vector<ConstrainedVariableId> m_variables;
    std::set<eint> m_explicitConstraints; /*!< Stores list of explicitly posted constraints to order tokens. Either the key of the constraint
					   is stored, or in cases where it is a straight assignment of a token, the key of the token is stored. */
    unsigned int m_lastOrderingChoiceCount; /*!< The last computed count of ordering choices */
    std::multimap<eint, ConstraintId> m_constraintsByTokenKey; /**< All Precedence Constraints by Token Key */
    std::multimap<int, ConstraintId> m_constraintsByKeyPair; /**< Precedence Constraints by  encoded key pair */
    std::map<eint, int> m_keyPairsByConstraintKey; /**< Reverse lookup to obtain the key pair */
    ConstrainedVariableId m_thisVar; /**< Used to constrain against */

  private:

    void clean(const TokenId& token);
    void clean(const ConstraintId& constraint, eint tokenKey);
    void constrainToThisObjectAsNeeded(const TokenId& token);

    Object(const Object&); /**< NO IMPL - Prevent use of copy constructor. */
  };


  class ObjectDT : public DataType
  {
  public:
      ObjectDT(const char* name);
      virtual ~ObjectDT();

      virtual bool isNumeric() const;
      virtual bool isBool() const;
      virtual bool isString() const;
      virtual bool isEntity() const;

      virtual edouble createValue(const std::string& value) const;
      virtual std::string toString(edouble value) const;
  };

  class ObjectDomain: public EnumeratedDomain {
  public:
    ObjectDomain(const DataTypeId& dt);
    ObjectDomain(const DataTypeId& dt, const std::list<ObjectId>& initialValues);
    ObjectDomain(const DataTypeId& dt, const ObjectId& initialValue);
    ObjectDomain(const AbstractDomain& org);

    /**
     * @brief Generate a list of object id's from a list of doubles
     */
    static std::list<ObjectId> makeObjectList(const std::list<edouble>& inputs);

    /**
     * @brief Generate a list of object id's internal member data
     */
    std::list<ObjectId> makeObjectList() const;

    ObjectId getObject(const eint key) const;

    /**
     * @brief Obtain the double encoded value from the string if it is a member.
     */
    bool convertToMemberValue(const std::string& strValue, edouble& dblValue) const;

    virtual ObjectDomain *copy() const;

    virtual std::string toString() const;
    std::string toString(edouble value) const;

    void remove(const ObjectId& obj);
    void remove(edouble value);

    bool isMember(const ObjectId& obj) const;
    bool isMember(edouble value) const; //because of the reference-casting rules...
  };
}

#endif
