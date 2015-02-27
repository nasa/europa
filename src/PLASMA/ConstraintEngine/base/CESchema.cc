#include "CESchema.hh"
#include "Debug.hh"
#include "ConstraintType.hh"
#include "CFunction.hh"

namespace EUROPA
{
CESchema::CESchema() : m_id(this), m_dataTypes(), m_constraintTypes(), m_cfunctions() {}

  CESchema::~CESchema()
  {
      purgeAll();
      m_id.remove();
  }

  void CESchema::purgeAll()
  {
	  purgeCFunctions();
      purgeConstraintTypes();
      purgeDataTypes();
  }

  const CESchemaId CESchema::getId() const
  {
      return m_id;
  }

bool CESchema::isDataType(const std::string& typeName) const {
  return (m_dataTypes.find(typeName) != m_dataTypes.end());
}

DataTypeId CESchema::getDataType(const std::string& typeName) {
  std::map<std::string, DataTypeId>::const_iterator it =  m_dataTypes.find(typeName);
  condDebugMsg(it == m_dataTypes.end(),
               "europa:error", "no DataType found for type '" << typeName << "'");
  checkError(it != m_dataTypes.end(),
             "no DataType found for type '" << typeName << "'");

  DataTypeId dt = it->second;
  check_error(dt.isValid());
  return dt;
}

void CESchema::registerDataType(const DataTypeId dt) {
  check_error(dt.isValid());

  if(m_dataTypes.find(dt->getName()) != m_dataTypes.end()){
    debugMsg("CESchema::registerDataType", "Over-writing prior registration for " << dt->getName());
    DataTypeId oldFactory = m_dataTypes.find(dt->getName())->second;
    m_dataTypes.erase(dt->getName());
    delete static_cast<DataType*>(oldFactory);
  }

  checkError(m_dataTypes.find(dt->getName()) == m_dataTypes.end(),
             "Already have '" + dt->getName() + "' registered.");

  m_dataTypes.insert(std::make_pair(dt->getName(), dt));
  debugMsg("CESchema::registerDataType", "Registered data type " << dt->getName());
}

const Domain & CESchema::baseDomain(const std::string& typeName) {
  DataTypeId factory = getDataType(typeName);
  check_error(factory.isValid(), "no DataType found for type '" + typeName + "'");
  return factory->baseDomain();
}
  
void CESchema::purgeDataTypes() {
  std::map<std::string, DataTypeId >::iterator it = m_dataTypes.begin();
  while (it != m_dataTypes.end()) {
    DataTypeId dt = (it++)->second;
    debugMsg("DataType:purgeAll",
             "Removing data type " << dt->getName());
    delete static_cast<DataType *>(dt);
  }
  m_dataTypes.clear();
}

void CESchema::registerConstraintType(const ConstraintTypeId factory) {
  const std::string& name = factory->getName();
  if(isConstraintType(name)){
    debugMsg("CESchema:registerConstraintType", "Over-riding prior registration for " << name.c_str());
    ConstraintTypeId oldFactory = getConstraintType(name);
    std::map<std::string, ConstraintTypeId>& factories = m_constraintTypes;
    factories.erase(name);
    oldFactory.release();
  }

  check_error(!isConstraintType(name), "Constraint Type '" + name + "' should not be registered, and yet it is....");
  m_constraintTypes.insert(std::make_pair(name,factory));
  debugMsg("CESchema:registerConstraintType", "Registered Constraint Type " << factory->getName());
}

const ConstraintTypeId CESchema::getConstraintType(const std::string& name) {
  std::map< std::string, ConstraintTypeId >::const_iterator it = m_constraintTypes.find(name);
  condDebugMsg(it ==  m_constraintTypes.end(),
               "europa:error", "Factory for constraint '" << name << "' is not registered.");
  checkError(it != m_constraintTypes.end(),
             "Factory for constraint '" << name << "' is not registered.");
  return(it->second);
}

bool CESchema::isConstraintType(const std::string& name, const bool& warn) {
  std::map<std::string, ConstraintTypeId >::const_iterator it = m_constraintTypes.find(name);
  if (it == m_constraintTypes.end()) {
    if (warn)
      std::cerr << "\nConstraint Type <" << name << "> has not been registered\n";
    return(false);
  }
  return(true);
}

void CESchema::purgeConstraintTypes() {
  std::map<std::string, ConstraintTypeId >::iterator it = m_constraintTypes.begin();
  while (it != m_constraintTypes.end()){
    ConstraintTypeId factory = it->second;
    check_error(factory.isValid());
    debugMsg("CESchema:purgeAll", "Removing constraint type " << factory->getName());
    m_constraintTypes.erase(it++);
    factory.release();
  }
}

void CESchema::registerCFunction(const CFunctionId cf) {
  check_error(cf.isValid());

  if(m_cfunctions.find(cf->getName()) != m_cfunctions.end()){
    debugMsg("CESchema::registerCFunction", "Over-writing prior registration for " << cf->getName());
    CFunctionId old = m_cfunctions.find(cf->getName())->second;
    m_cfunctions.erase(cf->getName());
    delete static_cast<CFunction*>(old);
  }

  checkError(m_cfunctions.find(cf->getName()) == m_cfunctions.end(), "Already have '" + cf->getName() + "' registered.");

  m_cfunctions.insert(std::make_pair(cf->getName(), cf));
  debugMsg("CESchema::registerCFunction", "Registered CFunction " << cf->getName());
}

CFunctionId CESchema::getCFunction(const std::string& name) {
  std::map<std::string, CFunctionId>::const_iterator it =  m_cfunctions.find(name);

  if (it != m_cfunctions.end())
    return it->second;
  else
    return CFunctionId::noId();
}

// TODO: write generic method to clean up maps instead
void CESchema::purgeCFunctions() {
  std::map<std::string, CFunctionId >::iterator it = m_cfunctions.begin();
  while (it != m_cfunctions.end()) {
    CFunctionId cf = (it++)->second;
    debugMsg("CESchema:purgeAll",
             "Removing CFunction " << cf->getName());
    delete static_cast<CFunction *>(cf);
  }
  m_cfunctions.clear();
}

} // namespace EUROPA
