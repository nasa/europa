#ifndef _H_CESchema
#define _H_CESchema

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
  DataTypeId getDataType(const char* typeName);
  bool isDataType(const char* typeName) const;
  const Domain & baseDomain(const char* typeName);
  void purgeDataTypes();

  // Methods to Manage Constraint Factories
  void registerConstraintType(const ConstraintTypeId ct);
  const ConstraintTypeId getConstraintType(const LabelStr& name);
  bool isConstraintType(const LabelStr& name, const bool& warn = false);
  void purgeConstraintTypes();

  // Methods to manage CFunctions
  void registerCFunction(const CFunctionId cf);
  CFunctionId getCFunction(const LabelStr& name);
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

#endif // _H_CESchema
