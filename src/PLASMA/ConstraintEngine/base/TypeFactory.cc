#include "TypeFactory.hh"
#include "Debug.hh"

namespace EUROPA 
{
  /*
   * TypeFactory
   */
  TypeFactory::TypeFactory(const char* typeName)
    : m_id(this)
    , m_typeName(typeName)
  {
  }

  TypeFactory::~TypeFactory()
  {
      m_id.remove();
  }

  const TypeFactoryId& TypeFactory::getId() const
  {
      return m_id;
  }

  const LabelStr& TypeFactory::getTypeName() const
  {
      return m_typeName;
  }


  /*
   * TypeFactoryMgr
   */
  TypeFactoryMgr::TypeFactoryMgr() 
    : m_id(this)
  {      
  }
  
  TypeFactoryMgr::~TypeFactoryMgr()
  {
      purgeAll();
      m_id.remove();      
  }

  const TypeFactoryMgrId& TypeFactoryMgr::getId() const
  {
      return m_id;
  }

  TypeFactoryId TypeFactoryMgr::getFactory(const char* typeName)
  {
    // Confirm it is present
    check_error(m_factories.find(LabelStr(typeName).getKey()) != m_factories.end(),
		"no TypeFactory found for type '" + std::string(typeName) + "'");

    TypeFactoryId factory = m_factories.find(LabelStr(typeName).getKey())->second;
    check_error(factory.isValid());
    return factory;
  }
			      
  void TypeFactoryMgr::registerFactory(const TypeFactoryId& factory)
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
  TypeFactoryMgr::createVariable(const char* typeName,
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
  TypeFactoryMgr::createVariable(const char* typeName,
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


  const AbstractDomain & TypeFactoryMgr::baseDomain(const char* typeName)
  {
    TypeFactoryId factory = getFactory(typeName);
    check_error(factory.isValid(), "no TypeFactory found for type '" + std::string(typeName) + "'");
    return factory->baseDomain();
  }

  double TypeFactoryMgr::createValue(const char* typeName,
                                  std::string value)
  {
    TypeFactoryId factory = getFactory(typeName);
    check_error(factory.isValid(), "no TypeFactory found for type '" + std::string(typeName) + "'");
    return factory->createValue(value);
  }

  void TypeFactoryMgr::purgeAll()
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
