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
    std::set<long> alreadyDeleted;
    for(std::map<edouble, ObjectFactoryId>::const_iterator it = m_factories.begin(); it != m_factories.end(); ++it) {
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
    std::map<edouble, ObjectFactoryId>::const_iterator it = m_factories.find(factoryName.getKey());

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
    m_factories.insert(std::pair<edouble, ObjectFactoryId>(factoryName, factory));
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
    m_factories.insert(std::make_pair(factory->getSignature().getKey(), factory));
  }
  
}
