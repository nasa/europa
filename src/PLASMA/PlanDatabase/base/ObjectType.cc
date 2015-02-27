/*
 * ObjectType.cc
 *
 *  Created on: Jan 30, 2009
 *      Author: javier
 */

#include "ObjectType.hh"

#include <string.h>
#include "Debug.hh"
#include "PlanDatabase.hh"
#include "CESchema.hh"

#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/algorithm/string/split.hpp>

namespace EUROPA {

ObjectType::ObjectType(const std::string& name, const ObjectTypeId parent, bool _isNative)
    : m_id(this)
    , m_varType((new ObjectDT(name))->getId())
    , m_name(name)
    , m_parent(parent)
    , m_isNative(_isNative)
    , m_objectFactories(),
      m_tokenTypes(),
      m_members()
{
}

ObjectType::~ObjectType() {
  // TODO: enable this when Schema API is cleaned up to reflect the fact that object factories and token factories are owned by the object type
  // purgeAll();
  delete id_cast<ObjectDT>(m_varType);
  m_id.remove();
}

const DataTypeId ObjectType::getVarType() const
{
    return m_varType;
}

void ObjectType::purgeAll() {
  cleanup(m_objectFactories);
  cleanup(m_tokenTypes);
}


const ObjectTypeId ObjectType::getId() const
{
    return m_id;
}

const std::string& ObjectType::getName() const {
  return m_name;
}

const std::string& ObjectType::getNameString() const {
  return m_name;
}

const ObjectTypeId ObjectType::getParent() const
{
    return m_parent;
}

const std::string& ObjectType::getParentName() const {
  static std::string empty_string;
  if (m_parent.isNoId())
    return empty_string;
  return m_parent->getNameString();
}

const std::map<std::string,DataTypeId>& ObjectType::getMembers() const
{
    return m_members;
}


PSList<std::string> ObjectType::getMemberNames() const {
	  PSList<std::string> retval;
	  for (std::map<std::string,DataTypeId>::const_iterator it = m_members.begin(); it != m_members.end(); ++it)
		  retval.push_back(it->first);
	  return retval;
}

const std::map<std::string,ObjectFactoryId>& ObjectType::getObjectFactories() const {
  return m_objectFactories;
}

const std::map<std::string,TokenTypeId>& ObjectType::getTokenTypes() const
{
    return m_tokenTypes;
}

bool ObjectType::isNative() const
{
    return m_isNative;
}

void ObjectType::addMember(const DataTypeId type, const std::string& name)
{
    m_members[name] = type;
}

const DataTypeId ObjectType::getMemberType(const std::string& name) const
{
    std::map<std::string,DataTypeId>::const_iterator it = m_members.find(name);

    if (it != m_members.end())
        return it->second;

    if (std::string(name) == "this")
        return getVarType();

    if (m_parent.isId())
        return m_parent->getMemberType(name);

    return DataTypeId::noId();
}

PSDataType* ObjectType::getMemberTypeRef(const std::string& name) const {
	return getMemberType(name.c_str());
}

void ObjectType::addObjectFactory(const ObjectFactoryId factory) {
  // TODO: allow redefinition of old one
  m_objectFactories[factory->getSignature()] = factory;
}

void ObjectType::addTokenType(const TokenTypeId factory) {
  // TODO: allow redefinition of old one
  m_tokenTypes[factory->getSignature()] = factory;
}

const TokenTypeId ObjectType::getTokenType(const std::string& signature) const {
  check_error(boost::starts_with(signature, getName()),
              "Can't look for a token factory I don't own");

  std::map<std::string,TokenTypeId>::const_iterator it =
      m_tokenTypes.find(signature);
  if (it != m_tokenTypes.end())
    return it->second;
  
  if (m_parent.isId()) {
    std::string parentSignature = m_parent->getName()+"."+
        signature.substr(signature.find('.') + 1);
    return m_parent->getTokenType(parentSignature);
  }

  return TokenTypeId::noId();
}

PSList<PSTokenType*> ObjectType::getPredicates() const {
  PSList<PSTokenType*> retval;
  for (std::map<std::string,TokenTypeId>::const_iterator it = m_tokenTypes.begin();
       it != m_tokenTypes.end(); ++it) {
    retval.push_back(it->second);
  }
  return retval;
}

PSList<PSTokenType*> ObjectType::getPSTokenTypesByAttr( int attrMask ) const {
  PSList<PSTokenType*> retval;
  for (std::map<std::string,TokenTypeId>::const_iterator it = m_tokenTypes.begin();
       it != m_tokenTypes.end(); ++it) {
    if( it->second->hasAttributes( attrMask ) )
      retval.push_back(it->second);
  }
  return retval;
}

const TokenTypeId ObjectType::getParentType(const TokenTypeId type) const
{
    if (m_parent.isId()) {
        std::string parentSignature = m_parent->getName()+"."+type->getPredicateName();
        return m_parent->getTokenType(parentSignature);
    }

    return TokenTypeId::noId();
}

std::string ObjectType::toString() const {
  std::ostringstream os;
  std::string extends = (m_parent.isId() ? std::string("extends ")+m_parent->getName().c_str() : "");

  os << "class " << m_name.c_str() << " extends " << extends << " {" << std::endl;

  {
    std::map<std::string,DataTypeId>::const_iterator it = m_members.begin();
    for(;it != m_members.end(); ++it)
      os << "    "
         << it->second->getName() /*type*/ << " "
         <<  it->first/*name*/
         << std::endl;
  }

  os << std::endl;

  {
    std::map<std::string,ObjectFactoryId>::const_iterator it = m_objectFactories.begin();
    for(;it != m_objectFactories.end(); ++it)
      os << "    " << it->second->getSignature().c_str() << std::endl;
  }

  os << std::endl;

  {
    std::map<std::string,TokenTypeId>::const_iterator it = m_tokenTypes.begin();
    for(;it != m_tokenTypes.end(); ++it) {
      TokenTypeId tokenType = it->second;
      os << "    " << tokenType->getSignature().c_str();
      std::map<std::string,DataTypeId>::const_iterator paramIt = tokenType->getArgs().begin();
      for(;paramIt != tokenType->getArgs().end();++paramIt)
        os<< " " << paramIt->second->getName().c_str() /*type*/ << "->" << paramIt->first.c_str()/*name*/;
      os << std::endl;
    }
  }

  os << "}" << std::endl;

  return os.str();
}

  static const char* TYPE_DELIMITER = ":"; /*!< Used to delimit types in the factory signature*/

ObjectFactory::ObjectFactory(const std::string& signature)
    : m_id(this), m_signature(signature), m_signatureTypes() {

  debugMsg("ObjectFactory:ObjectFactory", "Creating factory " << signature);

  // Now we want to populate the signature types
  boost::split(m_signatureTypes, signature, boost::is_any_of(TYPE_DELIMITER));
}

  ObjectFactory::~ObjectFactory()
  {
    m_id.remove();
  }

  const ObjectFactoryId ObjectFactory::getId() const {return m_id;}

  const std::string& ObjectFactory::getSignature() const {return m_signature;}

const std::vector<std::string>& ObjectFactory::getSignatureTypes() const {return m_signatureTypes;}

ObjectId ObjectFactory::makeNewObject(const PlanDatabaseId,
                                      const std::string&,
                                      const std::string&,
                                      const std::vector<const Domain*>&) const {
  return ObjectId::noId();
}

void ObjectFactory::evalConstructorBody(ObjectId,
                                        const std::vector<const Domain*>&) const {
}


  ObjectTypeMgr::ObjectTypeMgr()
      : m_id(this), m_objTypes(), m_factories()
  {
  }

  ObjectTypeMgr::~ObjectTypeMgr()
  {
      purgeAll();
      m_id.remove();
  }

  const ObjectTypeMgrId ObjectTypeMgr::getId() const
  {
      return m_id;
  }

void ObjectTypeMgr::purgeAll(){
  debugMsg("ObjectFactory:purgeAll", "Purging all");
  
  // TODO: this should be done by the object types
  std::set<ObjectFactory*> alreadyDeleted;
  for(std::map<std::string, ObjectFactoryId>::const_iterator it = m_factories.begin(); it != m_factories.end(); ++it) {
    if(alreadyDeleted.find(it->second) == alreadyDeleted.end()) {
      alreadyDeleted.insert(static_cast<ObjectFactory*>(it->second));
      delete static_cast<ObjectFactory*>(it->second);
    }
  }
  m_factories.clear();
  
  cleanup(m_objTypes);
}


void ObjectTypeMgr::registerObjectType(const ObjectTypeId objType) {
  // TODO: instead of keeping separate map, we should probably just delegate to the ObjectType
  {
    std::map<std::string,ObjectFactoryId>::const_iterator it = objType->getObjectFactories().begin();
    for(;it != objType->getObjectFactories().end(); ++it)
      registerFactory(it->second);
  }

  m_objTypes[objType->getName()] = objType;

  debugMsg("Schema:registerObjectType",
           "Registered object type:" << std::endl << objType->toString());
}

const ObjectTypeId ObjectTypeMgr::getObjectType(const std::string& objType) const {
  std::map<std::string,ObjectTypeId>::const_iterator it =
      m_objTypes.find(objType);

  return (it == m_objTypes.end() ? ObjectTypeId::noId() : it->second);
}

std::vector<ObjectTypeId> ObjectTypeMgr::getAllObjectTypes() const {
  std::vector<ObjectTypeId> retval;
  for(std::map<std::string, ObjectTypeId>::const_iterator it = m_objTypes.begin();
      it != m_objTypes.end(); ++it) {
    retval.push_back(it->second);
  }

  return retval;
}

  std::string ObjectTypeMgr::makeFactoryName(const std::string& objectType, const std::vector<const Domain*>& arguments){
    std::string signature = objectType;

    debugMsg("ObjectFactory:makeFactoryName", "Making factory name " << signature);
    // Iterate over the argument types and compose full signature
    for(std::vector<const Domain*>::const_iterator it = arguments.begin(); it != arguments.end(); ++it){
      signature = signature + TYPE_DELIMITER + (*it)->getTypeName();
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
ObjectFactoryId ObjectTypeMgr::getFactory(const SchemaId schema,
                                          const std::string& objectType,
                                          const std::vector<const Domain*>& arguments,
                                          const bool doCheckError) {
  // Build the full signature for the factory
  std::string factoryName = makeFactoryName(objectType,arguments);

  debugMsg("ObjectFactory:getFactory", "looking for factory " << factoryName);



  // Try to find a hit straight off
  std::map<std::string, ObjectFactoryId>::const_iterator it = m_factories.find(factoryName);

  // If we have a hit, return it
  if(it != m_factories.end())
    return it->second;

  // Otherwise, loop over all factories, and test for a match
  for(it = m_factories.begin(); it != m_factories.end(); ++it){
    ObjectFactoryId factory = it->second;
    const std::vector<std::string>& signatureTypes = factory->getSignatureTypes();

    // if there is no hit for the object type, move on immediately
    if(!schema->isA(objectType, signatureTypes[0]))
      continue;

    if (signatureTypes[0].c_str() != objectType.c_str())
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
      m_factories.insert(std::make_pair(factoryName, factory));
      return factory;
    }
  }

  // At this point, we should have a hit
  if (doCheckError)
    check_error(ALWAYS_FAILS, "Factory '" + factoryName + "' is not registered.");
  return ObjectFactoryId::noId();
}

void ObjectTypeMgr::registerFactory(const ObjectFactoryId factory) {
  check_error(factory.isValid());

  debugMsg("ObjectFactory:registerFactory",
           "Registering factory with signature " << factory->getSignature());
  if(m_factories.find(factory->getSignature()) != m_factories.end()){
    ObjectFactoryId oldFactory = m_factories.find(factory->getSignature())->second;
    m_factories.erase(factory->getSignature());
    delete static_cast<ObjectFactory*>(oldFactory);
    debugMsg("ObjectFactory:registerFactory", "Over-riding registeration for factory with signature " << factory->getSignature());
  }

  // Ensure it is not present already
  check_error(m_factories.find(factory->getSignature()) == m_factories.end());
  m_factories.insert(std::make_pair(factory->getSignature(), factory));
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
        ObjectEvalContext(EvalContext* parent, const ObjectId objInstance);
        virtual ~ObjectEvalContext();

        virtual ConstrainedVariableId getVar(const std::string& name);

        virtual void* getElement(const std::string& name) const;

    protected:
        ObjectId m_obj;
  };

  ObjectEvalContext::ObjectEvalContext(EvalContext* parent, const ObjectId objInstance)
    : EvalContext(parent)
    , m_obj(objInstance)
  {
  }

  ObjectEvalContext::~ObjectEvalContext()
  {
  }

  ConstrainedVariableId ObjectEvalContext::getVar(const std::string& name)
  {
    if (name == "this")
        return m_obj->getThis();

    ConstrainedVariableId var = m_obj->getVariable(m_obj->getName()+"."+name);

    if (!var.isNoId()) {
      debugMsg("Interpreter:EvalContext:Object","Found var in object instance:" << name);
      return var;
    }
    else {
      debugMsg("Interpreter:EvalContext:Object","Didn't find var in object instance:" << name);
      return EvalContext::getVar(name);
    }
  }

void* ObjectEvalContext::getElement(const std::string& name) const {
  if (std::string(name)=="PlanDatabase")
    return static_cast<PlanDatabase*>(m_obj->getPlanDatabase());
  
  return EvalContext::getElement(name);
}

  InterpretedObjectFactory::InterpretedObjectFactory(
                             const ObjectTypeId objType,
                             const std::string& signature,
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
    , m_canMakeNewObject(canMakeNewObject),
  m_evalContext(NULL)
  {
      if (!m_canMakeNewObject && m_superCallExpr==NULL) {
          m_superCallExpr = new ExprConstructorSuperCall(objType->getParent()->getName(),std::vector<Expr*>());
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
    ObjectFactoryEvalContext(const PlanDatabaseId planDb,
                 const std::vector<std::string>& argNames,
                 const std::vector<std::string>& argTypes,
                 const std::vector<const Domain*>& args);

    virtual ~ObjectFactoryEvalContext();

    virtual void* getElement(const std::string& name) const;

  protected:
    PlanDatabaseId m_planDb;
    std::vector<ConstrainedVariableId> m_tmpVars;
  };

  ObjectFactoryEvalContext::ObjectFactoryEvalContext(const PlanDatabaseId planDb,
               const std::vector<std::string>& argNames,
               const std::vector<std::string>& argTypes,
               const std::vector<const Domain*>& args)
    : EvalContext(NULL) // TODO: should pass in eval context from outside to have access to globals
    , m_planDb(planDb)
    , m_tmpVars()
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

  ObjectFactoryEvalContext::~ObjectFactoryEvalContext()
  {
    for (unsigned int i=0;i<m_tmpVars.size();i++) {
        // TODO: must release temporary vars, causing crash?
        //m_tmpVars[i].release();
    }
  }

void* ObjectFactoryEvalContext::getElement(const std::string& name) const {
  if (std::string(name)=="PlanDatabase")
    return static_cast<PlanDatabase*>(m_planDb);
  return EvalContext::getElement(name);
}

  ObjectId InterpretedObjectFactory::createInstance(
                            const PlanDatabaseId planDb,
                            const std::string& objectType,
                            const std::string& objectName,
                            const std::vector<const Domain*>& arguments) const
  {
    check_runtime_error(checkArgs(arguments));

    debugMsg("InterpretedObjectFactory:createInstance", "Creating instance for type " << objectType << " with name " << objectName);

    ObjectId instance = makeNewObject(planDb, objectType, objectName,arguments);
    evalConstructorBody(instance,arguments);
    instance->close();

    debugMsg("InterpretedObjectFactory:createInstance", "Created instance " << instance->toString() << " for type " << objectType << " with name " << objectName);

    return instance;
  }

  bool InterpretedObjectFactory::checkArgs(const std::vector<const Domain*>&) const
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
                           const PlanDatabaseId planDb,
                           const std::string& objectType,
                           const std::string& objectName,
                           const std::vector<const Domain*>& arguments) const
  {
    // go up the hierarchy and give the parents a chance to create the object, this allows native classes to be exported
    // TODO: some effort can be saved by keeping track of whether a class has a native ancestor different from Object.
    // If it doesn't, the object can be created right away and this traversal up the hierarchy can be skipped
    if (m_canMakeNewObject) {
      debugMsg("Interpreter:InterpretedObject","Created Object:" << objectName << " type:" << objectType);
      return (new Object(planDb, objectType, objectName,true))->getId();
    }
    else {
      check_error(m_superCallExpr != NULL, std::string("Failed to find factory for object ") + objectName + " of type "+objectType);

      ObjectFactoryEvalContext evalContext(
          planDb,
          m_constructorArgNames,
          m_constructorArgTypes,
          arguments
      );

      // TODO: argumentsForSuper are eval'd twice, once here and once when m_superCallExpr->eval(evalContext) is called
      //  figure out how to do it only once
      std::vector<const Domain*> argumentsForSuper;
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
                                               ObjectId instance,
                                               const std::vector<const Domain*>& arguments) const
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
            std::string varName = instance->getName() + "." + members[i].second;
            if (instance->getVariable(varName) == ConstrainedVariableId::noId()) {
                const Domain& baseDomain =
                    instance->getPlanDatabase()->getConstraintEngine()->getCESchema()->baseDomain(members[i].first.c_str());
                instance->addVariable(
                  baseDomain,
                  members[i].second.c_str()
                );
                debugMsg("Interpreter:InterpretedObject","Used default initializer for " << m_className << "." << members[i].second);
            }
        }

        debugMsg("Interpreter:evalConstructorBody",
                 "Evaluated constructor for " << instance->toString());
    }

    /*
     * ExprConstructorSuperCall
     */
    ExprConstructorSuperCall::ExprConstructorSuperCall(const std::string& superClassName, const std::vector<Expr*>& argExprs)
      : m_superClassName(superClassName)
      , m_argExprs(argExprs)
    {
    }

    ExprConstructorSuperCall::~ExprConstructorSuperCall()
    {
        for (unsigned int i=0; i < m_argExprs.size(); i++)
            delete m_argExprs[i];
        m_argExprs.clear();
    }

    DataRef ExprConstructorSuperCall::eval(EvalContext& context) const
    {
      ObjectId object = Entity::getTypedEntity<Object>(context.getVar("this")->derivedDomain().getSingletonValue());

      std::vector<const Domain*> arguments;

      evalArgs(context,arguments);
      ObjectFactoryId objFactory = object->getPlanDatabase()->getSchema()->getObjectFactory(m_superClassName,arguments);
      objFactory->evalConstructorBody(object,arguments);

      return DataRef::null;
    }

    void ExprConstructorSuperCall::evalArgs(EvalContext& context, std::vector<const Domain*>& arguments) const
    {
      for (unsigned int i=0; i < m_argExprs.size(); i++) {
        DataRef arg = m_argExprs[i]->eval(context);
        arguments.push_back(&(arg.getValue()->derivedDomain()));
      }
    }
}

