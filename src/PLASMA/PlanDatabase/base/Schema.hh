/**
 * @file Schema.hh
 * @brief Introduces the interface for a Schema, which identifies the types and
 * rules of a plan database,
 * @author Conor McGann, Andrew Bachmann
 */

#ifndef H_Schema
#define H_Schema

#include "PlanDatabaseDefs.hh"
#include "PSPlanDatabase.hh"
#include "Domain.hh"
#include "ObjectType.hh"
#include "TokenTypeMgr.hh"
#include "Method.hh"

#include <vector>

namespace EUROPA {
class LabelStr;

  /**
   * @class Schema
   * @brief Defines an interface for type checking information for the PlanDatabase.
   *
   * Schema is a singleton class since it inherits from DomainComparator, which is a singleton.
   * This singleton behavior is enforced in the constructor of the base class.
   *
   * @note Often, accessors which might appear to be immutable, are permitted to have non-const
   * interfaces to allow for the possibility of making incremental changes to cache for speed.
   * @see PlanDatabase
   */
  class Schema: public DomainComparator, public PSSchema {
  public:

    typedef std::pair<std::string, std::string> NameValuePair;
    typedef std::vector<NameValuePair> NameValueVector;
    typedef std::set<edouble> ValueSet;

    Schema(const std::string& name, const CESchemaId cesch);
    ~Schema();

    /**
     * @brief Retrieve the delimiter for separating elements in schema element names.
     */
    static const char getDelimiter();

    /**
     * @brief Creates a fully qualifiedName for a predicate
     * @see getDelimiter()
     */
    static const std::string makeQualifiedName(const std::string& objectType,
                                               const std::string& unqualifiedPredicateName);

    /**
     * @brief Accessor for the root object
     */
    static const std::string& rootObject();


    const SchemaId getId() const;

    const std::string& getName() const;

    /**
     * @brief Implements the DomainComparator interface function
     * to over-ride the base type relationships to include the use
     * of classes and inheritance.
     */
    bool canCompare(const Domain& domx, const Domain& domy) const;

    /**
     * @brief Utility method to remove all schema details, excluding the model name
     */
    void reset();

    /**
     * @brief Tests if the given name is a defined objectType or predciate
     */
    bool isType(const std::string& type) const;


    /**
     * @brief Tests if the given name is a recognized primitove type
     */
    bool isPrimitive(const std::string& str) const;

    /**
     * @brief Test if the given string is an enumerated type
     */
    bool isEnum(const std::string& str) const;

    /**
     * @brief Test if the given value is a member of the given enum.  Calling this 
     *        function with a name that isn't an enum is an error.
     * @param enumName The name of the enumeration
     * @param value The value to be tested.
     */
    bool isEnumValue(const std::string& enumName, const edouble value) const;

    /**
     * @brief Test if the given value is a member of any enum
     * @param value The value to be tested.
     */
    bool isEnumValue(const edouble value) const;

    /**
     * @brief Determine of a given predicate is part of the schema
     */
    bool isPredicate(const std::string& type) const;

    /**
     * @brief Test if the given string is a class name
     */
    bool isObjectType(const std::string& type) const;

    /**
     * @brief Determine if a given predicate is compatible with a given object type.
     */
    bool canBeAssigned(const std::string& objectType,
		       const std::string& predicate) const;

    /**
     * @brief Determine if a given composition of objects is allowed.
     * @param parentType the type of the composing object or predicate
     * @param memberType the type of the composed object
     * @param memberName the member name for the composed field.
     */
    bool canContain(const std::string& parentType,
		    const std::string& memberType,
		    const std::string& memberName) const;

    /**
     *
     * @brief returns a vector with the members of an object type
     * each element in the vector is a pair:
     * - the first element is the member's type
     * - the second elemnt is the member's name
     */
    const NameValueVector& getMembers(const std::string& objectType) const;

    /**
     * @brief Determines if the given member is contained in the parent
     * @param parentType the type of the composing object or predicate
     * @param memberName the member name for the composed field.
     */
    bool hasMember(const std::string& parentType, const std::string& memberName) const;

    /**
     * @brief Gets the type of a parents member.
     * @param parentType The parentType.
     * @param parameterName The parameter name.
     */
    const std::string getMemberType(const std::string& parentType,
					 const std::string& parameterName) const;

    /**
     * @brief Determine if one type is a sub type of another
     * @param descendant The candidate derived type. Must be a defined objectType.
     * @param ancestor The candidate ancestor type. Must be a defined objectType.
     * @see isObjectType
     */
    bool isA(const std::string& descendant, const std::string& ancestor) const;

    /**
     * @brief Tests if the given type has a parent.
     * @param objectType The objectType to test. Must be a valid type.
     * @return true if it does, otherwise false.
     * @see getParent, isA
     */
    bool hasParent(const std::string& objectType) const;

    /**
     * @brief Obtains the parent of a given type, if there is one. Will error out if it has no parent.
     * @param objectType the input type for which to obtain the parent
     * @see hasParent, isA
     */
    const std::string getParent(const std::string& objectType) const;

    /**
     * @brief Obtains all the Object Types in the Schema.
     * @return a const ref to a set of std::string (each of which is the name of an  ObjectType)
     */
    const std::set<std::string>& getAllObjectTypes() const;

    /**
     * @brief Obtains the set of all ObjectTypes that can be matched with the given object type.
     * @param objectType The objectType for which we want all matchable classes.
     * @return results All matchable classes in the schema. Includes objectType and all super classes.
     * @see hasParent, getParent
     */
    const std::vector<std::string>&  getAllObjectTypes(const std::string& objectType);

     /**
     * @brief Obtains the set of values for an enumeration.  Calling this function with a
     *        name not of an enumeration is an error.
     * @param enumName enumeration type name.
     * @return a const ref to the set of values for enumName.
     */
    const std::set<edouble>& getEnumValues(const std::string& enumName) const;

    /**
     * @brief Obtains the enum name for an enum value
     * @param value enum value to be looked up
     */
    const std::string& getEnumForValue(edouble value) const;

    /**
     * @brief Obtain the set of predicates for a given object type.  Errors if objectType
     *        doesn't name an object type.
     * @param objectType The ObjectType to use
     * @see addObjectType, isType
     */
    void getPredicates(const std::string& objectType, std::set<std::string>& results) const;

    /**
     * @brief Obtain the entire set of predicates.
     * @param results The set into which the predicates are put.
     */

    void getPredicates(std::set<std::string>& results) const;

    /**
     * @brief Test if an object type has any predicates
     */
    bool hasPredicates(const std::string& objectType);

    /**
     * @brief Helper method to compose a parent string
     * @param predicate The input predicate
     * @param predStr The result. Must be initially empty
     * @return true if it has a parent, othwerise false
     */
    bool makeParentPredicateString(const std::string& predicate, std::string& predStr) const;

    /**
     * @brief Obtain the object type for the given predicate.  Errors if predicate doesn't
     *        name an existing predicate.
     * @param predicate The predicate to use
     * @see addPredicate, isPredicateDefined
     */
    const std::string getObjectTypeForPredicate(const std::string& predicate) const;

    /**
     * @brief Gets the index of a named member in a types member list.
     * @param type the type to search
     * @param memberName the name of the member
     */
    unsigned int getIndexFromName(const std::string& type, const std::string& memberName) const;

    /**
     * @brief Gets the name of a member from a types member list.
     * @param type the type to search
     * @param index the index of the member
     */
    const std::string getNameFromIndex(const std::string& type, unsigned int index) const;

    /**
     * @brief Gets the type of the enumeration to which the given member belongs
     * @param member the member
     */
    const std::string getEnumFromMember(const LabelStr& member) const;

    /**
     * @brief Gets the number of parameters in a predicate.  Error: !isPredicateDefined(predicate)
     * @param predicate the name of the predicate
     */
    unsigned long getParameterCount(const std::string& predicate) const;

    /**
     * @brief Gets the type of parameter at a particular index location in a predicate.
     *        Errors: !isPredicateDefined(predicate)
     * @param predicate the name of the predicate
     * @param paramIndex the index of the parameter
     */
    const std::string getParameterType(const std::string& predicate, unsigned int paramIndex) const;

    /**
     * @brief Introduce a primitive type name to be used
     */
    void addPrimitive(const std::string& primitiveName);

    /**
     * @brief Declare an object type. The type can be referenced from now on.
     * I'ts the responsibility of the client to make sure that addObjectType will be called for this type eventually
     *
     */
    void declareObjectType(const std::string& objectType);

    /**
     * @brief Add an object type. It must not be present already.
     */
    void addObjectType(const std::string& objectType);

    /**
     * @brief Add an object type as a derived type from the parent.
     */
    void addObjectType(const std::string& objectType,
                       const std::string& parent);

    /**
     * @brief Adds a predicate.  Errors: !isObjectType(prefix), isPredicateDefined(suffix)
     * @param predicate The fully qualified name of the predicate. Must be of the form <prefix>.<suffix>
     */
    void addPredicate(const std::string& predicate);

    /**
     * @brief Indicates a composition of members
     * @param parentObjectType The type for the composing object
     * @param memberType The type for the composed member
     * @param memberName The name of the composed member
     * @return The indexed position of the parameter in the predicate list of parameters.
     */
    unsigned long addMember(const std::string& parentObjectType,
                            const std::string& memberType,
                            const std::string& memberName);

    /**
     * @brief Introduces a user defined enumeration type.
     * @param enumName The name of the enumeration.
     */
    void addEnum(const std::string& enumName);

    /**
     * @brief Add a member to a custom defined enumeration
     * @param enumName The name of the enumeration
     * @param enumValue The member to be added
     */
    void addValue(const std::string& enumName, edouble enumValue);

    void registerEnum(const std::string& enumName, const EnumeratedDomain& domain);

    /**
     * @brief Obtain a list of names of enumerations
     * @param results a list of enumeration names
     */
    void getEnumerations(std::list<std::string>& results) const;

    /**
     * @brief Output contents to the given stream
     */
    void write (ostream& os) const;

    const CESchemaId getCESchema() const { return m_ceSchema; }

    // TODO: ObjectType is replacing ObjectFactory
    void registerObjectType(const ObjectTypeId objType);
    const ObjectTypeId getObjectType(const std::string& objType);
    ObjectFactoryId getObjectFactory(const std::string& objectType, const std::vector<const Domain*>& arguments, const bool doCheckError = true);

    void registerTokenType(const TokenTypeId tokenType);
    TokenTypeId getTokenType(const std::string& tokenType);
    TokenTypeId getParentTokenType( const std::string& tokenType, const std::string& parentObjType);

    bool hasTokenTypes() const;

    void registerMethod(const MethodId m);
    MethodId getMethod(const std::string& methodName, const DataTypeId targetType, const std::vector<DataTypeId>& argTypes);

    std::vector<TokenTypeId> getTypeSupporters( TokenTypeId type );

    //PSSchema methods:
    PSList<std::string> getAllPredicates() const;
    PSList<std::string> getObjectMembers(const std::string& objectType) const;

    PSList<PSObjectType*> getAllPSObjectTypes() const;
    PSList<PSTokenType*>  getPSTokenTypesByAttr( int attrMask ) const;

  protected:
    SchemaId m_id;
    const std::string m_name;
    const CESchemaId m_ceSchema;
    const ObjectTypeMgrId m_objectTypeMgr;
    const TokenTypeMgrId m_tokenTypeMgr;
    std::map<std::string, MethodId> m_methods; // TODO: define methodMgr instead of keeping a map here

    // TODO: Drop these. Enums have been deprecated
    std::map<std::string, std::set<edouble> > enumValues;
    std::map<edouble, std::string> enumValuesToEnums;

    // TODO: get rid of all data members from this point on. There are object-oriented abstractions in place now
    // that hold the same information
    std::set<std::string> objectTypes;
    std::set<std::string> predicates;
    std::set<std::string> primitives;

    std::map<std::string, NameValueVector> membershipRelation; /*! All type compositions */
    std::map<std::string, std::string> childOfRelation; /*! Required to answer the getParent query */
    std::map<std::string, std::set<std::string> > objectPredicates; /*! All predicates by object type */
    std::set<std::string> typesWithNoPredicates; /*! Cache for lookup efficiently */
    std::map<std::string, std::vector<std::string> > allObjectTypes; /*! Cache to retrieve allObjectTypes by sub-class */

    mutable std::set<std::string> m_predTrueCache, m_predFalseCache; /**< Caches from isPredicate, now useful and not static . */
    mutable std::set<std::string> m_hasParentCache; /**< Cache from hasParent, now useful and not static */

    Schema(const Schema&); /**< NO IMPL */
    static const std::set<std::string>& getBuiltInVariableNames();

  };

}
#endif
