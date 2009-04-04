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

  bool CESchema::isDataType(const char* typeName) const
  {
      return (m_dataTypes.find(LabelStr(typeName).getKey()) != m_dataTypes.end());
  }

  DataTypeId CESchema::getDataType(const char* typeName)
  {
    std::map<double, DataTypeId>::const_iterator it =  m_dataTypes.find(LabelStr(typeName).getKey());
    condDebugMsg(it == m_dataTypes.end(), "europa:error", "no DataType found for type '" << std::string(typeName) << "'");
    check_error(it != m_dataTypes.end(), "no DataType found for type '" + std::string(typeName) + "'");

    DataTypeId dt = it->second;
    check_error(dt.isValid());
    return dt;
  }

  void CESchema::registerDataType(const DataTypeId& dt)
  {
    check_error(dt.isValid());

    if(m_dataTypes.find(dt->getName().getKey()) != m_dataTypes.end()){
      debugMsg("CESchema::registerDataType", "Over-writing prior registration for " << dt->getName().toString());
      DataTypeId oldFactory = m_dataTypes.find(dt->getName().getKey())->second;
      m_dataTypes.erase(dt->getName().getKey());
      delete (DataType*) oldFactory;
    }

    checkError(m_dataTypes.find(dt->getName().getKey()) == m_dataTypes.end(), "Already have '" + dt->getName().toString() + "' registered.");

    m_dataTypes.insert(std::pair<double, DataTypeId>(dt->getName().getKey(), dt));
    debugMsg("CESchema::registerDataType", "Registered data type " << dt->getName().toString());
  }

  const AbstractDomain & CESchema::baseDomain(const char* typeName)
  {
    DataTypeId factory = getDataType(typeName);
    check_error(factory.isValid(), "no DataType found for type '" + std::string(typeName) + "'");
    return factory->baseDomain();
  }

  void CESchema::purgeAll()
  {
      purgeConstraintFactories();
      purgeDataTypes();
  }

  void CESchema::purgeDataTypes()
  {
      std::map<double, DataTypeId >::iterator it = m_dataTypes.begin();
      while (it != m_dataTypes.end()) {
        DataTypeId dt = (it++)->second;
        debugMsg("DataType:purgeAll",
             "Removing type factory for " << dt->getName().toString());
        delete (DataType *) dt;
      }
      m_dataTypes.clear();
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
      debugMsg("CESchema:registerConstraintFactory", "Over-riding prior registration for " << name.c_str());
      ConstraintFactoryId oldFactory = getConstraintFactory(name);
      std::map<double, ConstraintFactoryId>& factories = m_constraintFactories;
      factories.erase(name.getKey());
      oldFactory.release();
    }

    check_error(isConstraintFactoryNotRegistered(name), "Constraint factory for '" + name.toString() + "' should not be registered, and yet it is....");
    m_constraintFactories.insert(std::pair<double, ConstraintFactoryId>(name.getKey(),
                                      factory->getId()));
    debugMsg("CESchema:registerConstraintFactory", "Registered factory " << factory->getName().toString());
  }

  const ConstraintFactoryId& CESchema::getConstraintFactory(const LabelStr& name) {
    std::map< double, ConstraintFactoryId >::const_iterator it = m_constraintFactories.find(name.getKey());
    condDebugMsg(it ==  m_constraintFactories.end(), "europa:error", "Factory for constraint '" << name.toString() << "' is not registered.");
    check_error(it != m_constraintFactories.end(), "Factory for constraint '" + name.toString() + "' is not registered.");
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
