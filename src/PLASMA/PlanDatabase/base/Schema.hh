/**
 * @file Schema.hh
 * @brief Introduces the interface for a Schema, which identifes the types and
 * rules of a plan database,
 * @author Conor McGann, Andrew Bachmann
 */

#ifndef _H_Schema
#define _H_Schema

#include "PlanDatabaseDefs.hh"
#include "PSPlanDatabase.hh"
#include "LabelStr.hh"
#include "AbstractDomain.hh"
#include "ObjectType.hh"
#include "ObjectFactory.hh"
#include "TokenTypeMgr.hh"
#include "Method.hh"

#include <vector>

namespace EUROPA {

  typedef std::set<edouble> LabelStrSet;

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

    typedef std::pair<LabelStr, LabelStr> NameValuePair;
    typedef std::vector<NameValuePair> NameValueVector;
    typedef std::set<edouble> ValueSet;
    typedef std::map<edouble, LabelStr> LabelStr_LabelStr_Map;
    typedef std::map<edouble, LabelStrSet > LabelStr_LabelStrSet_Map;
    typedef std::map<edouble, ValueSet > LabelStr_ValueSet_Map;
    typedef std::map<edouble,LabelStr_LabelStr_Map> LabelStr_LabelStrLabelStrMap_Map;

    Schema(const LabelStr& name, const CESchemaId& cesch);
    ~Schema();

    /**
     * @brief Retrieve the delimiter for separating elements in schema element names.
     */
    static const char* getDelimiter();

    /**
     * @brief Creates a fully qualifiedName for a predicate
     * @see getDelimiter()
     */
    static const LabelStr makeQualifiedName(const LabelStr& objectType,
					    const LabelStr& unqualifiedPredicateName);

    /**
     * @brief Accessor for the root object
     */
    static const LabelStr& rootObject();


    const SchemaId& getId() const;

    const LabelStr& getName() const;

    /**
     * @brief Implements the DomainComparator interface function
     * to over-ride the base type relationships to include the use
     * of classes and inheritance.
     */
    bool canCompare(const AbstractDomain& domx, const AbstractDomain& domy) const;

    /**
     * @brief Utility method to remove all schema details, excluding the model name
     */
    void reset();

    /**
     * @brief Tests if the given name is a defined objectType or predciate
     */
    bool isType(const LabelStr& type) const;


    /**
     * @brief Tests if the given name is a recognized primitove type
     */
    bool isPrimitive(const LabelStr& str) const;

    /**
     * @brief Test if the given string is an enumerated type
     */
    bool isEnum(const LabelStr& str) const;

    /**
     * @brief Test if the given value is a member of the given enum
     * @param enumName The name of the enumeration
     * @param value The value to be tested.
     * @error !isEnum(enum)
     */
    bool isEnumValue(const LabelStr& enumName, edouble value) const;

    /**
     * @brief Test if the given value is a member of any enum
     * @param value The value to be tested.
     */
    bool isEnumValue(double value) const;

    /**
     * @brief Determine of a given predicate is part of the schema
     */
    bool isPredicate(const LabelStr& type) const;

    /**
     * @brief Test if the given string is a class name
     */
    bool isObjectType(const LabelStr& type) const;

    /**
     * @brief Determine if a given predicate is compatible with a given object type.
     */
    bool canBeAssigned(const LabelStr& objectType,
		       const LabelStr& predicate) const;

    /**
     * @brief Determine if a given composition of objects is allowed.
     * @param parentType the type of the composing object or predicate
     * @param memberType the type of the composed object
     * @param memberName the member name for the composed field.
     */
    bool canContain(const LabelStr& parentType,
		    const LabelStr& memberType,
		    const LabelStr& memberName) const;

    /**
     *
     * @brief returns a vector with the members of an object type
     * each element in the vector is a pair:
     * - the first element is the member's type
     * - the second elemnt is the member's name
     */
    const NameValueVector& getMembers(const LabelStr& objectType) const;

    /**
     * @brief Determines if the given member is contained in the parent
     * @param parentType the type of the composing object or predicate
     * @param memberName the member name for the composed field.
     */
    bool hasMember(const LabelStr& parentType, const LabelStr& memberName) const;

    /**
     * @brief Gets the type of a parents member.
     * @param parentType The parentType.
     * @param parameterName The parameter name.
     */
    const LabelStr getMemberType(const LabelStr& parentType,
					 const LabelStr& parameterName) const;

    /**
     * @brief Determine if one type is a sub type of another
     * @param descendant The candidate derived type. Must be a defined objectType.
     * @param ancestor The candidate ancestor type. Must be a defined objectType.
     * @see isObjectType
     */
    bool isA(const LabelStr& descendant, const LabelStr& ancestor) const;

    /**
     * @brief Tests if the given type has a parent.
     * @param objectType The objectType to test. Must be a valid type.
     * @return true if it does, otherwise false.
     * @see getParent, isA
     */
    bool hasParent(const LabelStr& objectType) const;

    /**
     * @brief Obtains the parent of a given type, if there is one. Will error out if it has no parent.
     * @param objectType the input type for which to obtain the parent
     * @see hasParent, isA
     */
    const LabelStr getParent(const LabelStr& objectType) const;

    /**
     * @brief Obtains all the Object Types in the Schema.
     * @return a const ref to a set of LabelStr (each of which is the name of an  ObjectType)
     */
    const LabelStrSet& getAllObjectTypes() const;

    /**
     * @brief Obtains the set of all ObjectTypes that can be matched with the given object type.
     * @param objectType The objectType for which we want all matchable classes.
     * @return results All matchable classes in the schema. Includes objectType and all super classes.
     * @see hasParent, getParent
     */
    const std::vector<LabelStr>&  getAllObjectTypes(const LabelStr& objectType);

     /**
     * @brief Obtains the set of values for an enumeration.
     * @param enumName enumeration type name.
     * @return a const ref to the set of values for enumName.
     * @error !isEnum(enumName).
     */
    const std::set<edouble>& getEnumValues(const LabelStr& enumName) const;

    /**
     * @brief Obtains the enum name for an enum value
     * @param value enum value to be looked up
     */
    const LabelStr& getEnumForValue(double value) const;

    /**
     * @brief Obtain the set of predicates for a given object type.
     * @param objectType The ObjectType to use
     * @return a const ref to a set of LabelStr (each of which is the name of a Predicate)
     * @see addObjectType, isType
     * @error !isType(objecttype)
     */
    void getPredicates(const LabelStr& objectType, std::set<LabelStr>& results) const;

    /**
     * @brief Obtain the entire set of predicates.
     * @param results The set into which the predicates are put.
     */

    void getPredicates(std::set<LabelStr>& results) const;

    /**
     * @brief Test if an object type has any predicates
     */
    bool hasPredicates(const LabelStr& objectType);

    /**
     * @brief Helper method to compose a parent string
     * @param predicate The input predicate
     * @param predStr The result. Must be initially empty
     * @return true if it has a parent, othwerise false
     */
    bool makeParentPredicateString(const LabelStr& predicate, std::string& predStr) const;

    /**
     * @brief Obtain the object type for the given predicate.
     * @param predicate The predicate to use
     * @see addPredicate, isPredicateDefined
     * @error !isPredicateDefined(predicate)
     */
    const LabelStr getObjectTypeForPredicate(const LabelStr& predicate) const;

    /**
     * @brief Gets the index of a named member in a types member list.
     * @param type the type to search
     * @param memberName the name of the member
     */
    unsigned int getIndexFromName(const LabelStr& type, const LabelStr& memberName) const;

    /**
     * @brief Gets the name of a member from a types member list.
     * @param type the type to search
     * @param index the index of the member
     */
    const LabelStr getNameFromIndex(const LabelStr& type, unsigned int index) const;

    /**
     * @brief Gets the type of the enumeration to which the given member belongs
     * @param member the member
     */
    const LabelStr getEnumFromMember(const LabelStr& member) const;

    /**
     * @brief Gets the number of parameters in a predicate
     * @param predicate the name of the predicate
     * @error !isPredicateDefined(predicate)
     */
    unsigned int getParameterCount(const LabelStr& predicate) const;

    /**
     * @brief Gets the type of parameter at a particular index location in a predicate
     * @param predicate the name of the predicate
     * @param paramIndex the index of the parameter
     * @error !isPredicateDefined(predicate)
     */
    const LabelStr getParameterType(const LabelStr& predicate, unsigned int paramIndex) const;

    /**
     * @brief Introduce a primitive type name to be used
     */
    void addPrimitive(const LabelStr& primitiveName);

    /**
     * @brief Declare an object type. The type can be referenced from now on.
     * I'ts the responsibility of the client to make sure that addObjectType will be called for this type eventually
     *
     */
    void declareObjectType(const LabelStr& objectType);

    /**
     * @brief Add an object type. It must not be present already.
     */
    void addObjectType(const LabelStr& objectType);

    /**
     * @brief Add an object type as a derived type from the parent.
     */
    void addObjectType(const LabelStr& objectType,
                       const LabelStr& parent);

    /**
     * @brief Adds a predicate.
     * @param predicate The fully qualified name of the predicate. Must be of the form <prefix>.<suffix>
     * @error !isObjectType(prefix), isPredicateDefined(suffix)
     */
    void addPredicate(const LabelStr& predicate);

    /**
     * @brief Indicates a composition of members
     * @param parentObjectType The type for the composing object
     * @param memberType The type for the composed member
     * @param memberName The name of the composed member
     * @return The indexed position of the parameter in the predicate list of parameters.
     */
    unsigned int addMember(const LabelStr& parentObjectType,
				   const LabelStr& memberType,
				   const LabelStr& memberName);

    /**
     * @brief Introduces a user defined enumeration type.
     * @param enumName The name of the enumeration.
     */
    void addEnum(const LabelStr& enumName);

    /**
     * @brief Add a member to a custom defined enumeration
     * @param enumName The name of the enumeration
     * @param enumValue The member to be added
     */
    void addValue(const LabelStr& enumName, edouble enumValue);

    void registerEnum(const char* enumName, const EnumeratedDomain& domain);

    /**
     * @brief Obtain a list of names of enumerations
     * @param results a list of enumeration names
     */
    void getEnumerations(std::list<LabelStr>& results) const;

    /**
     * @brief Output contents to the given stream
     */
    void write (ostream& os) const;

    const CESchemaId& getCESchema() const { return m_ceSchema; }

    // TODO: ObjectType is replacing ObjectFactory
    void registerObjectType(const ObjectTypeId& objType);
    const ObjectTypeId& getObjectType(const LabelStr& objType);

    void registerObjectFactory(const ObjectFactoryId& obj_fact);
    ObjectFactoryId getObjectFactory(const LabelStr& objectType, const std::vector<const AbstractDomain*>& arguments, const bool doCheckError = true);

    void registerTokenType(const TokenTypeId& tokenType);
    TokenTypeId getTokenType(const LabelStr& tokenType);
    TokenTypeId getParentTokenType( const LabelStr& tokenType, const LabelStr& parentObjType);
    bool hasTokenTypes() const;

    void registerMethod(const MethodId& m);
    MethodId getMethod(const LabelStr& methodName, const DataTypeId& targetType, const std::vector<DataTypeId>& argTypes);

    //PSSchema methods:
    PSList<std::string> getAllPredicates() const;
    PSList<std::string> getMembers(const std::string& objectType) const;
    bool hasMember(const std::string& parentType, const std::string& memberName) const;

    PSList<PSObjectType> getAllPSObjectTypes() const;

  private:

    static const std::set<LabelStr>& getBuiltInVariableNames();

    SchemaId m_id;
    const CESchemaId& m_ceSchema;
    std::map<double, ObjectTypeId> m_objTypes; // TODO: this must be subsumed by ObjectTypeMgr, get rid of
    const ObjectTypeMgrId m_objectTypeMgr;
    const TokenTypeMgrId m_tokenTypeMgr;
    std::map<double, MethodId> m_methods; // TODO: define methodMgr instead of keeping a map here

    const LabelStr m_name;
    LabelStr_ValueSet_Map enumValues;
    std::map<double, LabelStr> enumValuesToEnums;

    LabelStrSet objectTypes;  //get rid of
    LabelStrSet predicates;
    LabelStrSet primitives;

    std::map<edouble, NameValueVector> membershipRelation; /*! All type compositions */
    std::map<edouble, LabelStr> childOfRelation; /*! Required to answer the getParent query */
    LabelStr_LabelStrSet_Map objectPredicates; /*! All predicates by object type */
    LabelStrSet typesWithNoPredicates; /*! Cache for lookup efficiently */
    std::map<edouble, std::vector<LabelStr> > allObjectTypes; /*! Cache to retrieve allObjectTypes by sub-class */
    Schema(const Schema&); /**< NO IMPL */

  };

}
#endif
