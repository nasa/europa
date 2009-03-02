#include "ObjectFactory.hh"
#include "Schema.hh"
#include "PlanDatabase.hh"
#include "TypeFactory.hh"
#include "Debug.hh"
#include "Object.hh"

namespace EUROPA {

  static const char* TYPE_DELIMITER = ":"; /*!< Used to delimit types in the factory signature*/

  ObjectFactory::ObjectFactory(const LabelStr& signature)
    : m_id(this), m_signature(signature){

    debugMsg("ObjectFactory:ObjectFactory", "Creating factory " << signature.toString());

    // Now we want to populate the signature types
    unsigned int count = signature.countElements(TYPE_DELIMITER);
    for(unsigned int i=0;i<count;i++){
      LabelStr labelStr = signature.getElement(i, TYPE_DELIMITER);
      m_signatureTypes.push_back(labelStr);
    }
  }

  ObjectFactory::~ObjectFactory()
  {
    m_id.remove();
  }

  const ObjectFactoryId& ObjectFactory::getId() const {return m_id;}

  const LabelStr& ObjectFactory::getSignature() const {return m_signature;}

  const std::vector<LabelStr>& ObjectFactory::getSignatureTypes() const {return m_signatureTypes;}

  ObjectTypeMgr::ObjectTypeMgr()
      : m_id(this)
  {
  }

  ObjectTypeMgr::~ObjectTypeMgr()
  {
      purgeAll();
      m_id.remove();
  }

  const ObjectTypeMgrId& ObjectTypeMgr::getId() const
  {
      return m_id;
  }

  void ObjectTypeMgr::purgeAll(){
    debugMsg("ObjectFactory:purgeAll", "Purging all");
    std::set<double> alreadyDeleted;
    for(std::map<double, ObjectFactoryId>::const_iterator it = m_factories.begin(); it != m_factories.end(); ++it) {
      if(alreadyDeleted.find(it->second) == alreadyDeleted.end()) {
          alreadyDeleted.insert(it->second);
          delete (ObjectFactory*) it->second;
      }
    }

    m_factories.clear();
  }

  LabelStr ObjectTypeMgr::makeFactoryName(const LabelStr& objectType, const std::vector<const AbstractDomain*>& arguments){
    std::string signature = objectType.toString();

    debugMsg("ObjectFactory:makeFactoryName", "Making factory name " << signature);
    // Iterate over the argument types and compose full signature
    for(std::vector<const AbstractDomain*>::const_iterator it = arguments.begin(); it != arguments.end(); ++it){
      signature = signature + TYPE_DELIMITER + (*it)->getTypeName().toString();
    }

    return signature;
  }

  /**
   * We must consider the possibility that the precise signature will not be registered, but a more abstracted signature may
   * work. For example. Suppose we have the following: Foo:int:float:Bar:Bing as a signature for a Foo factory. We are presented with
   * Foo:int:int:Bar:Bong where Bong extends Bing. We should permit a match under such circumstances.
   *
   * Matching rules for argument types:
   * matches(int, float)
   * matches(descendant, ancestor)
   * matches(x, x)
   */
  ObjectFactoryId ObjectTypeMgr::getFactory(const SchemaId& schema,
                                            const LabelStr& objectType,
                                            const std::vector<const AbstractDomain*>& arguments)
  {
    // Build the full signature for the factory
    LabelStr factoryName = makeFactoryName(objectType,arguments);

    debugMsg("ObjectFactory:getFactory", "looking for factory " << factoryName.toString());



    // Try to find a hit straight off
    std::map<double, ObjectFactoryId>::const_iterator it = m_factories.find(factoryName.getKey());

    // If we have a hit, return it
    if(it != m_factories.end())
      return it->second;

    // Otherwise, loop over all factories, and test for a match
    for(it = m_factories.begin(); it != m_factories.end(); ++it){
      ObjectFactoryId factory = it->second;
      const std::vector<LabelStr>& signatureTypes = factory->getSignatureTypes();

      // if there is no hit for the object type, move on immediately
      if(!schema->isA(objectType, signatureTypes[0]))
    continue;

      // If the argument length does not match the signature, which includes the extra for the class
      if(signatureTypes.size() - arguments.size() != 1)
    continue;

      // Now do a type by type comparison
      bool found = true;
      for (unsigned int j=1;j<signatureTypes.size();j++){
    if(schema->isType(arguments[j-1]->getTypeName()) &&
       schema->isType(signatureTypes[j])){
      if(!schema->isA(arguments[j-1]->getTypeName(), signatureTypes[j])){
        found = false;
        break;
      }
    }
    else if(arguments[j-1]->getTypeName() != signatureTypes[j]){
      found = false;
      break;
    }
      }

      if(found){
    // Cache for next time and return
    m_factories.insert(std::pair<double, ObjectFactoryId>(factoryName, factory));
    return factory;
      }
    }

    // At this point, we should have a hit
    check_error(ALWAYS_FAILS, "Factory '" + factoryName.toString() + "' is not registered.");
    return ObjectFactoryId::noId();
  }

  void ObjectTypeMgr::registerFactory(const ObjectFactoryId& factory){
    check_error(factory.isValid());

    debugMsg("ObjectFactory:registerFactory", "Registering factory with signature " << factory->getSignature().toString());

    if(m_factories.find(factory->getSignature().getKey()) != m_factories.end()){
      ObjectFactoryId oldFactory = m_factories.find(factory->getSignature().getKey())->second;
      m_factories.erase(factory->getSignature().getKey());
      delete (ObjectFactory*) oldFactory;
      debugMsg("ObjectFactory:registerFactory", "Over-riding registeration for factory with signature " << factory->getSignature().toString());
    }

    // Ensure it is not present already
    check_error(m_factories.find(factory->getSignature().getKey()) == m_factories.end());
    m_factories.insert(std::pair<double, ObjectFactoryId>(factory->getSignature().getKey(), factory));
  }


  /*
   * InterpretedObjectFactory
   */

  /*
   * ObjectEvalContext
   * Puts Object member variables in context
   */
  class ObjectEvalContext : public EvalContext
  {
    public:
        ObjectEvalContext(EvalContext* parent, const ObjectId& objInstance);
        virtual ~ObjectEvalContext();

        virtual ConstrainedVariableId getVar(const char* name);

    protected:
        ObjectId m_obj;
  };

  ObjectEvalContext::ObjectEvalContext(EvalContext* parent, const ObjectId& objInstance)
    : EvalContext(parent)
    , m_obj(objInstance)
  {
  }

  ObjectEvalContext::~ObjectEvalContext()
  {
  }

  ConstrainedVariableId ObjectEvalContext::getVar(const char* name)
  {
    if (std::strcmp(name,"this") == 0)
        return m_obj->getThis();

    ConstrainedVariableId var = m_obj->getVariable(m_obj->getName().toString()+"."+name);

    if (!var.isNoId()) {
      debugMsg("Interpreter:EvalContext:Object","Found var in object instance:" << name);
      return var;
    }
    else {
      debugMsg("Interpreter:EvalContext:Object","Didn't find var in object instance:" << name);
      return EvalContext::getVar(name);
    }
  }

  InterpretedObjectFactory::InterpretedObjectFactory(
                             const ObjectTypeId& objType,
                             const LabelStr& signature,
                             const std::vector<std::string>& constructorArgNames,
                             const std::vector<std::string>& constructorArgTypes,
                             ExprConstructorSuperCall* superCallExpr,
                             const std::vector<Expr*>& constructorBody,
                             bool canMakeNewObject)
    : ObjectFactory(signature)
    , m_className(objType->getName())
    , m_constructorArgNames(constructorArgNames)
    , m_constructorArgTypes(constructorArgTypes)
    , m_superCallExpr(superCallExpr)
    , m_constructorBody(constructorBody)
    , m_canMakeNewObject(canMakeNewObject)
  {
      if (!m_canMakeNewObject && m_superCallExpr==NULL) {
          m_superCallExpr = new ExprConstructorSuperCall(objType->getParent(),std::vector<Expr*>());
          debugMsg("InterpretedObjectFactory:InterpretedObjectFactory","created default super call for object factory:" << signature.c_str());
      }
  }

  InterpretedObjectFactory::~InterpretedObjectFactory()
  {
    if (m_superCallExpr != NULL)
      delete m_superCallExpr;

    for (unsigned int i=0; i < m_constructorBody.size(); i++)
      delete m_constructorBody[i];
  }

  class ObjectFactoryEvalContext : public EvalContext
  {
  public:
    ObjectFactoryEvalContext(const PlanDatabaseId& planDb,
                 const std::vector<std::string>& argNames,
                 const std::vector<std::string>& argTypes,
                 const std::vector<const AbstractDomain*>& args)
      : EvalContext(NULL) // TODO: should pass in eval context from outside to have access to globals
    {
            debugMsg("ObjectFactoryEvalContext", ">> ");
      // Add arguments to eval context
      for (unsigned int i=0;i<argNames.size();i++) {
                debugMsg("ObjectFactoryEvalContext:createVariable", argTypes[i] << " " << argNames[i] << " = " << *(args[i]));
          ConstrainedVariableId arg = planDb->getClient()->createVariable(
                                    argTypes[i].c_str(),
                                    *(args[i]),
                                    argNames[i].c_str(),
                                    true
                                    );
           m_tmpVars.push_back(arg);
           addVar(argNames[i].c_str(),arg);
      }
            debugMsg("ObjectFactoryEvalContext", "<< ");
    }

    virtual ~ObjectFactoryEvalContext()
    {
      for (unsigned int i=0;i<m_tmpVars.size();i++) {
          // TODO: must release temporary vars, causing crash?
          //m_tmpVars[i].release();
      }
    }

  protected:
    std::vector<ConstrainedVariableId> m_tmpVars;
  };

  ObjectId InterpretedObjectFactory::createInstance(
                            const PlanDatabaseId& planDb,
                            const LabelStr& objectType,
                            const LabelStr& objectName,
                            const std::vector<const AbstractDomain*>& arguments) const
  {
    check_runtime_error(checkArgs(arguments));

    debugMsg("InterpretedObjectFactory:createInstance", "Creating instance for type " << objectType.toString() << " with name " << objectName.toString());

    ObjectId instance = makeNewObject(planDb, objectType, objectName,arguments);
    evalConstructorBody(instance,arguments);
    instance->close();

    debugMsg("InterpretedObjectFactory:createInstance", "Created instance " << instance->toString() << " for type " << objectType.toString() << " with name " << objectName.toString());

    return instance;
  }

  bool InterpretedObjectFactory::checkArgs(const std::vector<const AbstractDomain*>& arguments) const
  {
    // TODO: implement this. is this even necessary?, parser should take care of it
    return true;
  }

  /*
   * If a class has a native C++ ancestor the ancestor instance must be instanciated instead
   * - Every exported C++ class (like Timeline) must extend Object
   * - Every exported C++ class must register an InterpretedObjectFactory for each one of its constructors
   *   that overrides this method and calls the appropriate constructor
   *
   */
  ObjectId InterpretedObjectFactory::makeNewObject(
                           const PlanDatabaseId& planDb,
                           const LabelStr& objectType,
                           const LabelStr& objectName,
                           const std::vector<const AbstractDomain*>& arguments) const
  {
    // go up the hierarchy and give the parents a chance to create the object, this allows native classes to be exported
    // TODO: some effort can be saved by keeping track of whether a class has a native ancestor different from Object.
    // If it doesn't, the object can be created right away and this traversal up the hierarchy can be skipped
    if (m_canMakeNewObject) {
      debugMsg("Interpreter:InterpretedObject","Created Object:" << objectName.toString() << " type:" << objectType.toString());
      return (new Object(planDb, objectType, objectName,true))->getId();
    }
    else {
      check_error(m_superCallExpr != NULL, std::string("Failed to find factory for object ") + objectName.toString() + " of type "+objectType.toString());

      ObjectFactoryEvalContext evalContext(
          planDb,
          m_constructorArgNames,
          m_constructorArgTypes,
          arguments
      );

      // TODO: argumentsForSuper are eval'd twice, once here and once when m_superCallExpr->eval(evalContext) is called
      //  figure out how to do it only once
      std::vector<const AbstractDomain*> argumentsForSuper;
      m_superCallExpr->evalArgs(evalContext,argumentsForSuper);

      ObjectFactoryId objFactory = planDb->getSchema()->getObjectFactory(m_superCallExpr->getSuperClassName(),argumentsForSuper);
      ObjectId retval = objFactory->makeNewObject(
          planDb,
          objectType,
          objectName,
          argumentsForSuper
      );

      return retval;
    }
  }

    void InterpretedObjectFactory::evalConstructorBody(
                                               ObjectId& instance,
                                               const std::vector<const AbstractDomain*>& arguments) const
    {
        // TODO: should pass in eval context from outside to have access to globals
        ObjectFactoryEvalContext factoryEvalContext(
            instance->getPlanDatabase(),
            m_constructorArgNames,
            m_constructorArgTypes,
            arguments
        );
        ObjectEvalContext evalContext(&factoryEvalContext,instance);

        if (m_superCallExpr != NULL)
            m_superCallExpr->eval(evalContext);

        for (unsigned int i=0; i < m_constructorBody.size(); i++)
            m_constructorBody[i]->eval(evalContext);

        // Initialize any variables that were not explicitly initialized
        const Schema::NameValueVector& members = instance->getPlanDatabase()->getSchema()->getMembers(m_className);
        for (unsigned int i=0; i < members.size(); i++) {
            std::string varName = instance->getName().toString() + "." + members[i].second.toString();
            if (instance->getVariable(varName) == ConstrainedVariableId::noId()) {
                const AbstractDomain& baseDomain =
                    instance->getPlanDatabase()->getConstraintEngine()->getCESchema()->baseDomain(members[i].first.c_str());
                instance->addVariable(
                  baseDomain,
                  members[i].second.c_str()
                );
                debugMsg("Interpreter:InterpretedObject","Used default initializer for " << m_className.toString() << "." << members[i].second.toString());
            }
        }

        debugMsg("Interpreter:evalConstructorBody",
                 "Evaluated constructor for " << instance->toString());
    }

    /*
     * ExprConstructorSuperCall
     */
    ExprConstructorSuperCall::ExprConstructorSuperCall(const LabelStr& superClassName, const std::vector<Expr*>& argExprs)
      : m_superClassName(superClassName)
      , m_argExprs(argExprs)
    {
    }

    ExprConstructorSuperCall::~ExprConstructorSuperCall()
    {
    }

    DataRef ExprConstructorSuperCall::eval(EvalContext& context) const
    {
      ObjectId object = context.getVar("this")->derivedDomain().getSingletonValue();

      std::vector<const AbstractDomain*> arguments;

      evalArgs(context,arguments);
      ObjectFactoryId objFactory = object->getPlanDatabase()->getSchema()->getObjectFactory(m_superClassName,arguments);
      objFactory->evalConstructorBody(object,arguments);

      return DataRef::null;
    }

    void ExprConstructorSuperCall::evalArgs(EvalContext& context, std::vector<const AbstractDomain*>& arguments) const
    {
      for (unsigned int i=0; i < m_argExprs.size(); i++) {
        DataRef arg = m_argExprs[i]->eval(context);
        arguments.push_back(&(arg.getValue()->derivedDomain()));
      }
    }
}
