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

  void CESchema::purgeAll()
  {
	  purgeCFunctions();
      purgeConstraintTypes();
      purgeDataTypes();
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
    std::map<edouble, DataTypeId>::const_iterator it =  m_dataTypes.find(LabelStr(typeName).getKey());
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

    m_dataTypes.insert(std::pair<edouble, DataTypeId>(dt->getName().getKey(), dt));
    debugMsg("CESchema::registerDataType", "Registered data type " << dt->getName().toString());
  }

  const AbstractDomain & CESchema::baseDomain(const char* typeName)
  {
    DataTypeId factory = getDataType(typeName);
    check_error(factory.isValid(), "no DataType found for type '" + std::string(typeName) + "'");
    return factory->baseDomain();
  }
  
  void CESchema::purgeDataTypes()
  {
      std::map<edouble, DataTypeId >::iterator it = m_dataTypes.begin();
      while (it != m_dataTypes.end()) {
        DataTypeId dt = (it++)->second;
        debugMsg("DataType:purgeAll",
             "Removing data type " << dt->getName().toString());
        delete (DataType *) dt;
      }
      m_dataTypes.clear();
  }

  void CESchema::registerConstraintType(const ConstraintTypeId& factory) {
    const LabelStr& name = factory->getName();
    if(isConstraintType(name)){
      debugMsg("CESchema:registerConstraintType", "Over-riding prior registration for " << name.c_str());
      ConstraintTypeId oldFactory = getConstraintType(name);
      std::map<edouble, ConstraintTypeId>& factories = m_constraintTypes;
      factories.erase(name.getKey());
      oldFactory.release();
    }

    check_error(!isConstraintType(name), "Constraint Type '" + name.toString() + "' should not be registered, and yet it is....");
    m_constraintTypes.insert(std::pair<edouble, ConstraintTypeId>(name.getKey(),factory));
    debugMsg("CESchema:registerConstraintType", "Registered Constraint Type " << factory->getName().toString());
  }

  const ConstraintTypeId& CESchema::getConstraintType(const LabelStr& name) {
    std::map< edouble, ConstraintTypeId >::const_iterator it = m_constraintTypes.find(name.getKey());
    condDebugMsg(it ==  m_constraintTypes.end(), "europa:error", "Factory for constraint '" << name.toString() << "' is not registered.");
    check_error(it != m_constraintTypes.end(), "Factory for constraint '" + name.toString() + "' is not registered.");
    return(it->second);
  }

  bool CESchema::isConstraintType(const LabelStr& name, const bool& warn) {
    std::map<edouble, ConstraintTypeId >::const_iterator it = m_constraintTypes.find(name.getKey());
    if (it == m_constraintTypes.end()) {
      if (warn)
        std::cerr << "\nConstraint Type <" << name.toString() << "> has not been registered\n";
      return(false);
    }
    return(true);
  }

  void CESchema::purgeConstraintTypes()
  {
      std::map<edouble, ConstraintTypeId >::iterator it = m_constraintTypes.begin();
      while (it != m_constraintTypes.end()){
        ConstraintTypeId factory = it->second;
        check_error(factory.isValid());
        debugMsg("CESchema:purgeAll", "Removing constraint type " << factory->getName().toString());
        m_constraintTypes.erase(it++);
        factory.release();
      }
  }

  void CESchema::registerCFunction(const CFunctionId& cf)
  {
    check_error(cf.isValid());

    if(m_cfunctions.find(cf->getName().getKey()) != m_cfunctions.end()){
      debugMsg("CESchema::registerCFunction", "Over-writing prior registration for " << cf->getName().toString());
      CFunctionId old = m_cfunctions.find(cf->getName().getKey())->second;
      m_cfunctions.erase(cf->getName().getKey());
      delete (CFunction*) old;
    }

    checkError(m_cfunctions.find(cf->getName().getKey()) == m_cfunctions.end(), "Already have '" + cf->getName().toString() + "' registered.");

    m_cfunctions.insert(std::pair<edouble, CFunctionId>(cf->getName().getKey(), cf));
    debugMsg("CESchema::registerCFunction", "Registered CFunction " << cf->getName().toString());
  }

  CFunctionId CESchema::getCFunction(const LabelStr& name)
  {
    std::map<edouble, CFunctionId>::const_iterator it =  m_cfunctions.find(name.getKey());

    if (it != m_cfunctions.end())
    	return it->second;
    else
    	return CFunctionId::noId();
  }

  // TODO: write generic method to clean up maps instead
  void CESchema::purgeCFunctions()
  {
      std::map<edouble, CFunctionId >::iterator it = m_cfunctions.begin();
      while (it != m_cfunctions.end()) {
        CFunctionId cf = (it++)->second;
        debugMsg("CESchema:purgeAll",
             "Removing CFunction " << cf->getName().toString());
        delete (CFunction *) cf;
      }
      m_cfunctions.clear();
  }

} // namespace EUROPA
