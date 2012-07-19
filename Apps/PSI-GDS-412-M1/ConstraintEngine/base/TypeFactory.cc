#include "TypeFactory.hh"
#include "Debug.hh"

namespace EUROPA {

  //
  // TypeFactory
  //

  ConcreteTypeFactoryId TypeFactory::getFactory(const char* typeName)
  {
    std::map<double, ConcreteTypeFactoryId>& factories = getInstance().m_factories;

    // Confirm it is present
    check_error(factories.find(LabelStr(typeName).getKey()) != factories.end(),
		"no TypeFactory found for type '" + std::string(typeName) + "'");

    ConcreteTypeFactoryId factory = factories.find(LabelStr(typeName).getKey())->second;
    check_error(factory.isValid());
    return factory;
  }

  TypeFactory::TypeFactory() {}
			      
  TypeFactory& TypeFactory::getInstance()
  {
    static TypeFactory sl_instance;
    return sl_instance;
  }

  TypeFactory::~TypeFactory()
  {
    std::map<double, ConcreteTypeFactoryId>& factories = getInstance().m_factories;
    std::map<double, ConcreteTypeFactoryId>::const_iterator it = factories.begin();
    while (it != factories.end()){
      std::pair<double, ConcreteTypeFactoryId> pair = *(it++);
      ConcreteTypeFactoryId factory = pair.second;
      check_error(factory.isValid());
      delete (ConcreteTypeFactory*) factory;
    }
  }

  void TypeFactory::registerFactory(const ConcreteTypeFactoryId& factory)
  {
    std::map<double, ConcreteTypeFactoryId>& factories = getInstance().m_factories;
    check_error(factory.isValid());

    if(factories.find(factory->getTypeName().getKey()) != factories.end()){
      debugMsg("TypeFactory:registerFactory", "Over-writing prior registration for " << factory->getTypeName().toString());
      ConcreteTypeFactoryId oldFactory = factories.find(factory->getTypeName().getKey())->second;
      factories.erase(factory->getTypeName().getKey());
      delete (ConcreteTypeFactory*) oldFactory;
    }

    checkError(factories.find(factory->getTypeName().getKey()) == factories.end(), "Already have '" + factory->getTypeName().toString() + "' registered.");

    factories.insert(std::pair<double, ConcreteTypeFactoryId>(factory->getTypeName().getKey(), factory));
    debugMsg("TypeFactory:registerFactory", "Registered type factory " << factory->getTypeName().toString());
  }

  ConstrainedVariableId
  TypeFactory::createVariable(const char* typeName,
                              const ConstraintEngineId& constraintEngine, 
                              bool canBeSpecified,
                              const char* name,
                              const EntityId& parent,
                              int index)
  {
    ConcreteTypeFactoryId factory = getFactory(typeName);
    check_error(factory.isValid(), "no TypeFactory found for type '" + std::string(typeName) + "'");
    return createVariable(typeName, constraintEngine, factory->baseDomain(), canBeSpecified, name, parent, index);
  }

  ConstrainedVariableId
  TypeFactory::createVariable(const char* typeName,
                              const ConstraintEngineId& constraintEngine, 
                              const AbstractDomain& baseDomain,
                              bool canBeSpecified,
                              const char* name,
                              const EntityId& parent,
                              int index)
  {
    check_error(constraintEngine.isValid());
    ConcreteTypeFactoryId factory = getFactory(typeName);
    check_error(factory.isValid(), "no TypeFactory found for type '" + std::string(typeName) + "'");
    ConstrainedVariableId variable = factory->createVariable(constraintEngine, baseDomain, canBeSpecified, name, parent, index);
    check_error(variable.isValid());
    return variable;
  }


  const AbstractDomain & TypeFactory::baseDomain(const char* typeName)
  {
    ConcreteTypeFactoryId factory = getFactory(typeName);
    check_error(factory.isValid(), "no TypeFactory found for type '" + std::string(typeName) + "'");
    return factory->baseDomain();
  }

  double TypeFactory::createValue(const char* typeName,
                                  std::string value)
  {
    ConcreteTypeFactoryId factory = getFactory(typeName);
    check_error(factory.isValid(), "no TypeFactory found for type '" + std::string(typeName) + "'");
    return factory->createValue(value);
  }

  void TypeFactory::purgeAll()
  {
    std::map<double, ConcreteTypeFactoryId >& factories = getInstance().m_factories;
    std::map<double, ConcreteTypeFactoryId >::iterator factories_iter = factories.begin();
    while (factories_iter != factories.end()) {
      ConcreteTypeFactoryId factory = (factories_iter++)->second;
      debugMsg("TypeFactory:purgeAll",
	       "Removing factory for " << factory->getTypeName().toString());
      delete (ConcreteTypeFactory *) factory;
    }
    factories.clear();
  }

  //
  // ConcreteTypeFactory
  //

  ConcreteTypeFactory::ConcreteTypeFactory(const char* typeName)
    : m_id(this), m_typeName(typeName)
  {
    TypeFactory::registerFactory(m_id);
  }

  ConcreteTypeFactory::~ConcreteTypeFactory()
  {
    m_id.remove();
  }

  const ConcreteTypeFactoryId& ConcreteTypeFactory::getId() const
  {
    return m_id;
  }

  const LabelStr& ConcreteTypeFactory::getTypeName() const
  {
    return m_typeName;
  }

} // namespace EUROPA
