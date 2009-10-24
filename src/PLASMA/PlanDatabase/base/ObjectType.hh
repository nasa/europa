/*
 * ObjectType.hh
 *
 *  Created on: Jan 30, 2009
 *      Author: javier
 */

#ifndef OBJECTTYPE_HH_
#define OBJECTTYPE_HH_

#include <map>
#include <vector>

#include "LabelStr.hh"

#include "PDBInterpreter.hh"
#include "TokenType.hh"

namespace EUROPA {

class ObjectType;
typedef Id<ObjectType> ObjectTypeId;

class ObjectTypeMgr;
typedef Id<ObjectTypeMgr> ObjectTypeMgrId;

class ObjectFactory;
typedef Id<ObjectFactory> ObjectFactoryId;

class ObjectType
{
public:
    ObjectType(const char* name, const ObjectTypeId& parent, bool isNative=false);
    virtual ~ObjectType();

    const ObjectTypeId& getId() const;

    const DataTypeId& getVarType() const; // Data type for a variable that holds a reference to an object

    virtual const LabelStr& getName() const;
    virtual const ObjectTypeId& getParent() const;
    virtual bool isNative() const;

    virtual void addMember(const DataTypeId& type, const char* name); // TODO: use DataType instead
    virtual const std::map<std::string,DataTypeId>& getMembers() const;
    virtual const DataTypeId& getMemberType(const char* name) const;

    virtual void addObjectFactory(const ObjectFactoryId& factory);
    virtual const std::map<double,ObjectFactoryId>& getObjectFactories() const;

    virtual void addTokenType(const TokenTypeId& factory);
    virtual const std::map<double,TokenTypeId>& getTokenTypes() const;
    virtual const TokenTypeId& getTokenType(const LabelStr& signature) const;
    virtual const TokenTypeId& getParentType(const TokenTypeId& factory) const;

    virtual std::string toString() const;

    void purgeAll(); // TODO: make protected after Schema API is fixed

protected:
    ObjectTypeId m_id;
    DataTypeId m_varType;
    LabelStr m_name;
    ObjectTypeId m_parent;
    bool m_isNative;
    std::map<double,ObjectFactoryId> m_objectFactories;
    std::map<double,TokenTypeId> m_tokenTypes;
    std::map<std::string,DataTypeId> m_members;
};


/**
 * @brief Manages metadata on ObjectTypes
 *
 */
class ObjectTypeMgr {
public:

	ObjectTypeMgr();
	virtual ~ObjectTypeMgr();

	const ObjectTypeMgrId& getId() const;

    void registerObjectType(const ObjectTypeId& objType);
    const ObjectTypeId& getObjectType(const LabelStr& objType) const;
    std::vector<ObjectTypeId> getAllObjectTypes() const;

	/**
	 * @brief Helper method to compose full factory signature from type and arguments
	 * @param objectType The type of the object defined in a model.
	 * @param arguments The sequence of name/value pairs to be passed as arguments for construction of the object
	 * @return A ':' deliimited string of <objectType>:<arg0.type>:..:<argn.type>
	 */
	static LabelStr makeFactoryName(const LabelStr& objectType, const std::vector<const AbstractDomain*>& arguments);

	/**
	 * @brief Obtain the factory based on the type of object to create and the types of the arguments to the constructor
	 */
	ObjectFactoryId getFactory(const SchemaId& schema, const LabelStr& objectType, const std::vector<const AbstractDomain*>& arguments, const bool doCheckError = true);

	/**
	 * @brief Add a factory to provide instantiation of particular concrete types based on a label.
	 */
	void registerFactory(const ObjectFactoryId& factory);

	/**
	 * @brief Delete all meta-data stored.
	 */
	void purgeAll();

protected:
	ObjectTypeMgrId m_id;
    std::map<double, ObjectTypeId> m_objTypes;
	std::map<double, ObjectFactoryId> m_factories; // TODO: should delegate to object types instead
};

/**
 * @brief Each concrete class must provide an implementation for this.
 */
class ObjectFactory{
public:
  ObjectFactory(const LabelStr& signature);

  virtual ~ObjectFactory();

  const ObjectFactoryId& getId() const;

  /**
   * @brief Return the type for which this factory is registered.
   */
  const LabelStr& getSignature() const;

  /**
   * @brief Retreive the type signature as a vector of types
   */
  const std::vector<LabelStr>& getSignatureTypes() const;

  /**
   * @brief Create a root object instance
   * @see DbClient::createObject(const LabelStr& type, const LabelStr& name)
   * for the interpreted version createInstance = makeObject + evalConstructorBody
   */
  virtual ObjectId createInstance(const PlanDatabaseId& planDb,
                  const LabelStr& objectType,
                  const LabelStr& objectName,
                  const std::vector<const AbstractDomain*>& arguments) const = 0;


 // TODO: when code generation goes away, InterpretedObjectFactory will be the base implementation
 // makeNewObject will still be the function to be overriden by native classes
 // evalConstructorBody will become protected and will probably never be overriden.
 // dummy implementation provided for now to be compatible with code generation.
 /**
   * @brief makes an instance of a new object, this is purely construction, initialization happens in evalConstructorBody
   */
 virtual ObjectId makeNewObject(
                          const PlanDatabaseId& planDb,
                          const LabelStr& objectType,
                          const LabelStr& objectName,
                          const std::vector<const AbstractDomain*>& arguments) const { return ObjectId::noId(); };
  /**
   * @brief The body of the constructor after the object is created
   * any operations done by createInstance to the object after it is created must be done by this method
   * so that calls to "super()" in subclasses can be supported correctly
   */
  virtual void evalConstructorBody(ObjectId& instance, const std::vector<const AbstractDomain*>& arguments) const {};

private:
  ObjectFactoryId m_id;
  LabelStr m_signature;
  std::vector<LabelStr> m_signatureTypes;
};

// Call to super inside a constructor
class ExprConstructorSuperCall : public Expr
{
  public:
      ExprConstructorSuperCall(const LabelStr& superClassName,
                               const std::vector<Expr*>& argExprs);
      virtual ~ExprConstructorSuperCall();

      virtual DataRef eval(EvalContext& context) const;

      const LabelStr& getSuperClassName() const { return m_superClassName; }

      void evalArgs(EvalContext& context, std::vector<const AbstractDomain*>& arguments) const;

  protected:
      LabelStr m_superClassName;
      std::vector<Expr*> m_argExprs;
};

class InterpretedObjectFactory : public ObjectFactory
{
  public:
      InterpretedObjectFactory(
          const ObjectTypeId& objType,
          const LabelStr& signature,
          const std::vector<std::string>& constructorArgNames,
          const std::vector<std::string>& constructorArgTypes,
          ExprConstructorSuperCall* superCallExpr,
          const std::vector<Expr*>& constructorBody,
          bool canMakeNewObject = false
      );

      virtual ~InterpretedObjectFactory();

  protected:
      // createInstance = makeNewObject + evalConstructorBody
      virtual ObjectId createInstance(
                              const PlanDatabaseId& planDb,
                              const LabelStr& objectType,
                              const LabelStr& objectName,
                              const std::vector<const AbstractDomain*>& arguments) const;

      // Any exported C++ classes must register a factory for each C++ constructor
      // and override this method to call the C++ constructor
      virtual ObjectId makeNewObject(
                          const PlanDatabaseId& planDb,
                          const LabelStr& objectType,
                          const LabelStr& objectName,
                          const std::vector<const AbstractDomain*>& arguments) const;

      virtual void evalConstructorBody(
                         ObjectId& instance,
                         const std::vector<const AbstractDomain*>& arguments) const;

      bool checkArgs(const std::vector<const AbstractDomain*>& arguments) const;

      LabelStr                  m_className;
      std::vector<std::string>  m_constructorArgNames;
      std::vector<std::string>  m_constructorArgTypes;
      ExprConstructorSuperCall* m_superCallExpr;
      std::vector<Expr*>        m_constructorBody;
      bool                      m_canMakeNewObject;
      mutable EvalContext*      m_evalContext;
};

class NativeObjectFactory : public InterpretedObjectFactory
{
  public:
      NativeObjectFactory(const ObjectTypeId& objType, const LabelStr& signature)
          : InterpretedObjectFactory(
                objType,                    // objType
                signature,                  // signature
                std::vector<std::string>(), // ConstructorArgNames
                std::vector<std::string>(), // constructorArgTypes
                NULL,                       // SuperCallExpr
                std::vector<Expr*>(),       // constructorBody
                true                        // canCreateObjects
            )
      {
      }

      virtual ~NativeObjectFactory() {}

  protected:
      virtual ObjectId makeNewObject(
                          const PlanDatabaseId& planDb,
                          const LabelStr& objectType,
                          const LabelStr& objectName,
                          const std::vector<const AbstractDomain*>& arguments) const = 0;
};

}


#endif /* OBJECTTYPE_HH_ */
