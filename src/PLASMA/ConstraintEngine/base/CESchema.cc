#include "CESchema.hh"
#include "Debug.hh"

namespace EUROPA 
{
  CESchema::CESchema() 
    : m_id(this)
  {      
  }
  
  CESchema::~CESchema()
  {
      purgeAll();
      m_id.remove();      
  }

  const CESchemaId& CESchema::getId() const
  {
      return m_id;
  }

  TypeFactoryId CESchema::getFactory(const char* typeName)
  {
    // Confirm it is present
    check_error(m_factories.find(LabelStr(typeName).getKey()) != m_factories.end(),
		"no TypeFactory found for type '" + std::string(typeName) + "'");

    TypeFactoryId factory = m_factories.find(LabelStr(typeName).getKey())->second;
    check_error(factory.isValid());
    return factory;
  }
			      
  void CESchema::registerFactory(const TypeFactoryId& factory)
  {
    check_error(factory.isValid());

    if(m_factories.find(factory->getTypeName().getKey()) != m_factories.end()){
      debugMsg("TypeFactory:registerFactory", "Over-writing prior registration for " << factory->getTypeName().toString());
      TypeFactoryId oldFactory = m_factories.find(factory->getTypeName().getKey())->second;
      m_factories.erase(factory->getTypeName().getKey());
      delete (TypeFactory*) oldFactory;
    }

    checkError(m_factories.find(factory->getTypeName().getKey()) == m_factories.end(), "Already have '" + factory->getTypeName().toString() + "' registered.");

    m_factories.insert(std::pair<double, TypeFactoryId>(factory->getTypeName().getKey(), factory));
    debugMsg("TypeFactory:registerFactory", "Registered type factory " << factory->getTypeName().toString());
  }

  ConstrainedVariableId
  CESchema::createVariable(const char* typeName,
                              const ConstraintEngineId& constraintEngine, 
                              bool canBeSpecified,
                              const char* name,
                              const EntityId& parent,
                              int index)
  {
    TypeFactoryId factory = getFactory(typeName);
    check_error(factory.isValid(), "no TypeFactory found for type '" + std::string(typeName) + "'");
    return createVariable(typeName, constraintEngine, factory->baseDomain(), canBeSpecified, name, parent, index);
  }

  ConstrainedVariableId
  CESchema::createVariable(const char* typeName,
                              const ConstraintEngineId& constraintEngine, 
                              const AbstractDomain& baseDomain,
                              bool canBeSpecified,
                              const char* name,
                              const EntityId& parent,
                              int index)
  {
    check_error(constraintEngine.isValid());
    TypeFactoryId factory = getFactory(typeName);
    check_error(factory.isValid(), "no TypeFactory found for type '" + std::string(typeName) + "'");
    ConstrainedVariableId variable = factory->createVariable(constraintEngine, baseDomain, canBeSpecified, name, parent, index);
    check_error(variable.isValid());
    return variable;
  }


  const AbstractDomain & CESchema::baseDomain(const char* typeName)
  {
    TypeFactoryId factory = getFactory(typeName);
    check_error(factory.isValid(), "no TypeFactory found for type '" + std::string(typeName) + "'");
    return factory->baseDomain();
  }

  double CESchema::createValue(const char* typeName,
                                  std::string value)
  {
    TypeFactoryId factory = getFactory(typeName);
    check_error(factory.isValid(), "no TypeFactory found for type '" + std::string(typeName) + "'");
    return factory->createValue(value);
  }

  void CESchema::purgeAll()
  {
    std::map<double, TypeFactoryId >::iterator factories_iter = m_factories.begin();
    while (factories_iter != m_factories.end()) {
      TypeFactoryId factory = (factories_iter++)->second;
      debugMsg("TypeFactory:purgeAll",
	       "Removing factory for " << factory->getTypeName().toString());
      delete (TypeFactory *) factory;
    }
    m_factories.clear();
  }

} // namespace EUROPA
