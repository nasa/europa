#ifndef H_CESchema
#define H_CESchema

#include "ConstraintEngineDefs.hh"
#include "Engine.hh"

#include <map>

/**
 * @file Class to manage all metadata for Constraint engine (variable data types, constraint types, etc).
 * @author Javier Barreiro, May 2008
 */

namespace EUROPA {
class Domain;
class CESchema;
typedef Id<CESchema> CESchemaId;

class CESchema : public EngineComponent
{
 public:
  CESchema();
  virtual ~CESchema();

  const CESchemaId getId() const;

  // Methods to Manage Data Types
  void registerDataType(const DataTypeId dt);
  DataTypeId getDataType(const std::string& typeName);
  bool isDataType(const std::string& typeName) const;
  const Domain & baseDomain(const std::string& typeName);
  void purgeDataTypes();

  // Methods to Manage Constraint Factories
  void registerConstraintType(const ConstraintTypeId ct);
  const ConstraintTypeId getConstraintType(const std::string& name);
  bool isConstraintType(const std::string& name, const bool& warn = false);
  void purgeConstraintTypes();

  // Methods to manage CFunctions
  void registerCFunction(const CFunctionId cf);
  CFunctionId getCFunction(const std::string& name);
  void purgeCFunctions();

  /**
   * @brief Delete all meta data stored.
   */
  void purgeAll();

 protected:
  CESchemaId m_id;
  std::map<std::string, DataTypeId> m_dataTypes;
  std::map<std::string, ConstraintTypeId > m_constraintTypes;
  std::map<std::string, CFunctionId> m_cfunctions;
};

} // namespace EUROPA

#endif // H_CESchema
