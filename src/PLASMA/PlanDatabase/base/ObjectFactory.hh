#ifndef _H_ObjectFactory
#define _H_ObjectFactory

#include "PlanDatabaseDefs.hh"
#include "PDBInterpreter.hh"
#include "LabelStr.hh"
#include <map>
#include <vector>

/**
 * @file Factory class for allocation of objects.
 * @author Conor McGann, March, 2004
 * @see DbClient
 */
namespace EUROPA {

  class ObjectTypeMgr;
  typedef Id<ObjectTypeMgr> ObjectTypeMgrId;

  class ObjectFactory;
  typedef Id<ObjectFactory> ObjectFactoryId;

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

  /**
   * @brief Manages metadata on ObjectTypes
   *
   */
  class ObjectTypeMgr {
  public:

    ObjectTypeMgr();
    virtual ~ObjectTypeMgr();

    const ObjectTypeMgrId& getId() const;
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
     * @brief Delete all factory instances stored. Should only be used to support testing, since
     * factories should remain for all instances of the plan database in the same process.
     */
    void purgeAll();

  protected:
    ObjectTypeMgrId m_id;
    std::map<edouble, ObjectFactoryId> m_factories;
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

  class ObjectType;
  typedef Id<ObjectType> ObjectTypeId;

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

  /**
   * Macro for registering object factories with a signature for lookup
   */
#define REGISTER_OBJECT_FACTORY(schema_id, className, signature) schema_id->registerObjectFactory((new className(LabelStr(#signature)))->getId());

}

#endif
