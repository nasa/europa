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

  bool CESchema::isType(const char* typeName) const
  {
      return (m_typeFactories.find(LabelStr(typeName).getKey()) != m_typeFactories.end());
  }

  TypeFactoryId CESchema::getFactory(const char* typeName)
  {
    // Confirm it is present
    check_error(m_typeFactories.find(LabelStr(typeName).getKey()) != m_typeFactories.end(),
		"no TypeFactory found for type '" + std::string(typeName) + "'");

    TypeFactoryId factory = m_typeFactories.find(LabelStr(typeName).getKey())->second;
    check_error(factory.isValid());
    return factory;
  }

  void CESchema::registerFactory(const TypeFactoryId& factory)
  {
    check_error(factory.isValid());

    if(m_typeFactories.find(factory->getTypeName().getKey()) != m_typeFactories.end()){
      debugMsg("TypeFactory:registerFactory", "Over-writing prior registration for " << factory->getTypeName().toString());
      TypeFactoryId oldFactory = m_typeFactories.find(factory->getTypeName().getKey())->second;
      m_typeFactories.erase(factory->getTypeName().getKey());
      delete (TypeFactory*) oldFactory;
    }

    checkError(m_typeFactories.find(factory->getTypeName().getKey()) == m_typeFactories.end(), "Already have '" + factory->getTypeName().toString() + "' registered.");

    m_typeFactories.insert(std::pair<double, TypeFactoryId>(factory->getTypeName().getKey(), factory));
    debugMsg("TypeFactory:registerFactory", "Registered type factory " << factory->getTypeName().toString());
  }

  const AbstractDomain & CESchema::baseDomain(const char* typeName)
  {
    TypeFactoryId factory = getFactory(typeName);
    check_error(factory.isValid(), "no TypeFactory found for type '" + std::string(typeName) + "'");
    return factory->baseDomain();
  }

  void CESchema::purgeAll()
  {
      purgeConstraintFactories();
      purgeTypeFactories();
  }

  void CESchema::purgeTypeFactories()
  {
      std::map<double, TypeFactoryId >::iterator factories_iter = m_typeFactories.begin();
      while (factories_iter != m_typeFactories.end()) {
        TypeFactoryId factory = (factories_iter++)->second;
        debugMsg("TypeFactory:purgeAll",
             "Removing type factory for " << factory->getTypeName().toString());
        delete (TypeFactory *) factory;
      }
      m_typeFactories.clear();
  }

  void CESchema::purgeConstraintFactories()
  {
      std::map<double, ConstraintFactoryId >::iterator it = m_constraintFactories.begin();
      while (it != m_constraintFactories.end()){
        ConstraintFactoryId factory = it->second;
        check_error(factory.isValid());
        debugMsg("CESchema:purgeAll", "Removing constraint factory " << factory->getName().toString());
        m_constraintFactories.erase(it++);
        factory.release();
      }
  }

  void CESchema::registerConstraintFactory(ConstraintFactory* factory)
  {
    registerConstraintFactory(factory, factory->getName());
  }

  void CESchema::registerConstraintFactory(ConstraintFactory* factory, const LabelStr& name) {
    if(isConstraintFactoryRegistered(name)){
      debugMsg("CESchema:registerFactory", "Over-riding prior registration for " << name.c_str());
      ConstraintFactoryId oldFactory = getConstraintFactory(name);
      std::map<double, ConstraintFactoryId>& factories = m_constraintFactories;
      factories.erase(name.getKey());
      oldFactory.release();
    }

    check_error(isConstraintFactoryNotRegistered(name), "Constraint factory for '" + name.toString() + "' should not be registered, and yet it is....");
    m_constraintFactories.insert(std::pair<double, ConstraintFactoryId>(name.getKey(),
                                      factory->getId()));
    debugMsg("CESchema:registerFactory", "Registered factory " << factory->getName().toString());
  }

  const ConstraintFactoryId& CESchema::getConstraintFactory(const LabelStr& name) {
    check_error(isConstraintFactoryRegistered(name), "Factory for constraint '" + name.toString() + "' is not registered.");
    std::map< double, ConstraintFactoryId >::const_iterator it = m_constraintFactories.find(name.getKey());
    return(it->second);
  }

  bool CESchema::isConstraintFactoryRegistered(const LabelStr& name, const bool& warn) {
    std::map<double, ConstraintFactoryId >::const_iterator it = m_constraintFactories.find(name.getKey());
    if (it == m_constraintFactories.end()) {
      if (warn)
        std::cerr << "\nConstraint <" << name.toString() << "> has not been registered\n";
      return(false);
    }
    return(true);
  }

  bool CESchema::isConstraintFactoryNotRegistered(const LabelStr& name) {
    std::map< double, ConstraintFactoryId >::const_iterator it = m_constraintFactories.find(name.getKey());
    if (it != m_constraintFactories.end()) {
      std::cerr << "\nConstraint <" << name.toString() << "> has already been registered\n";
      return(false);
    }
    return(true);
  }
} // namespace EUROPA
