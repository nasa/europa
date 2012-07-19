#include "ObjectFactory.hh"
#include "Schema.hh"
#include "TypeFactory.hh"
#include "Debug.hh"

namespace EUROPA {

  static const char* TYPE_DELIMITER = ":"; /*!< Used to delimit types in the factory signature*/

  LabelStr ObjectFactory::makeFactoryName(const LabelStr& objectType, const std::vector<ConstructorArgument>& arguments){
    std::string signature = objectType.toString();

    debugMsg("ObjectFactory:makeFactoryName", "Making factory name " << signature);
    // Iterate over the argument types and compose full signature
    for(std::vector<ConstructorArgument>::const_iterator it = arguments.begin(); it != arguments.end(); ++it){
      signature = signature + TYPE_DELIMITER + it->first.toString();
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
  ConcreteObjectFactoryId ObjectFactory::getFactory(const LabelStr& objectType, const std::vector<ConstructorArgument>& arguments){
    std::map<double, ConcreteObjectFactoryId>& factories = getInstance().m_factories;

    // Build the full signature for the factory
    LabelStr factoryName = makeFactoryName(objectType,arguments);

    debugMsg("ObjectFactory:getFactory", "looking for factory " << factoryName.toString());



    // Try to find a hit straight off
    std::map<double, ConcreteObjectFactoryId>::const_iterator it = factories.find(factoryName.getKey());

    // If we have a hit, return it
    if(it != factories.end())
      return it->second;

    SchemaId schema = Schema::instance();

    // Otherwise, loop over all factories, and test for a match
    for(it = factories.begin(); it != factories.end(); ++it){
      ConcreteObjectFactoryId factory = it->second;
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
	if(schema->isType(arguments[j-1].first) &&
	   schema->isType(signatureTypes[j])){
	  if(!schema->isA(arguments[j-1].first, signatureTypes[j])){
	    found = false;
	    break;
	  }
	}
	else if(arguments[j-1].first != signatureTypes[j]){
	  found = false;
	  break;
	}
      }

      if(found){
	// Cache for next time and return
	factories.insert(std::pair<double, ConcreteObjectFactoryId>(factoryName, factory));
	return factory;
      }
    }

    // At this point, we should have a hit
    check_error(ALWAYS_FAILS, "Factory '" + factoryName.toString() + "' is not registered.");
    return ConcreteObjectFactoryId::noId();
  }

  ObjectFactory::ObjectFactory(){}

  ObjectFactory& ObjectFactory::getInstance(){
    static ObjectFactory sl_instance;
    return sl_instance;
  }

  ObjectFactory::~ObjectFactory(){
    std::map<double, ConcreteObjectFactoryId>& factories = getInstance().m_factories;
    for(std::map<double, ConcreteObjectFactoryId>::const_iterator it = factories.begin(); it != factories.end(); ++it){
      ConcreteObjectFactoryId factory = it->second;
      check_error(factory.isValid());
      debugMsg("ObjectFactory:~ObjectFactory", "Deleting factory with signature "  << factory->getSignature().toString());
      delete (ConcreteObjectFactory*) factory;
    }
  }

  void ObjectFactory::registerFactory(const ConcreteObjectFactoryId& factory){
    std::map<double, ConcreteObjectFactoryId>& factories = getInstance().m_factories;
    check_error(factory.isValid());

    debugMsg("ObjectFactory:registerFactory", "Registering factory with signature " << factory->getSignature().toString());

    // Ensure it is not present already
    check_error(factories.find(factory->getSignature().getKey()) == factories.end());
    factories.insert(std::pair<double, ConcreteObjectFactoryId>(factory->getSignature().getKey(), factory));
  }

  ObjectId ObjectFactory::createInstance(const PlanDatabaseId& planDb, 
					 const LabelStr& objectType, 
					 const LabelStr& objectName,
					 const std::vector<ConstructorArgument>& arguments){
    check_error(planDb.isValid());

    debugMsg("ObjectFactory:createInstance", "objectType " << objectType.toString() << " objectName " << objectName.toString());

    // Obtain the factory 
    ConcreteObjectFactoryId factory = getFactory(objectType, arguments);

    ObjectId object = factory->createInstance(planDb, objectType, objectName, arguments);

    check_error(object.isValid());
    return object;
  }

  void ObjectFactory::purgeAll(){
    debugMsg("ObjectFactory:purgeAll", "Purging all");
    std::map<double,ConcreteObjectFactoryId >& factories = getInstance().m_factories;
    std::set<double> alreadyDeleted;
    for(std::map<double, ConcreteObjectFactoryId>::const_iterator it = factories.begin(); it != factories.end(); ++it)
      if(alreadyDeleted.find(it->second) == alreadyDeleted.end()){
	alreadyDeleted.insert(it->second);
	delete (ConcreteObjectFactory*) it->second;
      }
    factories.clear();
  }

  ConcreteObjectFactory::ConcreteObjectFactory(const LabelStr& signature)
    : m_id(this), m_signature(signature){
    ObjectFactory::registerFactory(m_id);

    debugMsg("ConcreteObjectFactory:ConcreteObjectFactory", "Creating factory " << signature.toString());

    // Now we want to populate the signature types
    unsigned int count = signature.countElements(TYPE_DELIMITER);
    for(unsigned int i=0;i<count;i++){
      LabelStr labelStr = signature.getElement(i, TYPE_DELIMITER);
      m_signatureTypes.push_back(labelStr);
    }
  }

  ConcreteObjectFactory::~ConcreteObjectFactory(){
    m_id.remove();
  }

  const ConcreteObjectFactoryId& ConcreteObjectFactory::getId() const {return m_id;}

  const LabelStr& ConcreteObjectFactory::getSignature() const {return m_signature;}

  const std::vector<LabelStr>& ConcreteObjectFactory::getSignatureTypes() const {return m_signatureTypes;}
}
