#ifndef _H_ObjectFactory
#define _H_ObjectFactory

#include "PlanDatabaseDefs.hh"
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

  protected:
    ObjectFactory(const LabelStr& signature);

    /**
     * @brief Protected destructor ensures control so that only the object factory can
     * delete factories.
     */
    virtual ~ObjectFactory();

  private:
    ObjectFactoryId m_id;
    LabelStr m_signature;
    std::vector<LabelStr> m_signatureTypes;
    
    friend class ObjectTypeMgr;
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
    ObjectFactoryId getFactory(const SchemaId& schema, const LabelStr& objectType, const std::vector<const AbstractDomain*>& arguments);

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
    std::map<double, ObjectFactoryId> m_factories; 
  };

  /**
   * Macro for registering object factories with a signature for lookup
   */
#define REGISTER_OBJECT_FACTORY(schema_id, className, signature) schema_id->registerObjectFactory((new className(LabelStr(#signature)))->getId());

}

#endif
