#ifndef _H_CESchema
#define _H_CESchema

#include "ConstraintEngineDefs.hh"
#include "AbstractDomain.hh"
#include "ConstrainedVariable.hh"
#include "LabelStr.hh"
#include "Engine.hh"
#include "TypeFactory.hh"
#include <map>
#include <string>

/**
 * @file Class to manage all metadata for Constraint engine (variable data types, constraint types, etc).
 * @author Javier Barreiro, May 2008
 */

namespace EUROPA {

  class CESchema;
  typedef Id<CESchema> CESchemaId;

  class CESchema : public EngineComponent
  {
    public:
      CESchema();
      virtual ~CESchema();
  
      const CESchemaId& getId() const;
      
      /**
       * @brief Add a factory to provide instantiation of particular concrete types based on a label.
       */
      void registerFactory(const TypeFactoryId& factory);

      /**
       * @brief Obtain the factory based on the type name
       */ 
      TypeFactoryId getFactory(const char* typeName);

      /**
       * @brief Delete all factory instances stored. 
       */
      void purgeAll();

      /**
       * @brief Create a variable
       */
      ConstrainedVariableId createVariable(const char* typeName,
                                                  const ConstraintEngineId& constraintEngine, 
                                                  bool canBeSpecified = true,
                                                  const char* name = NO_VAR_NAME,
                                                  const EntityId& parent = EntityId::noId(),
                                                  int index = ConstrainedVariable::NO_INDEX);

      /**
       * @brief Create a variable
       */
      ConstrainedVariableId createVariable(const char* typeName,
                                                  const ConstraintEngineId& constraintEngine, 
                                                  const AbstractDomain& baseDomain,
                                                  bool canBeSpecified = true,
                                                  const char* name = NO_VAR_NAME,
                                                  const EntityId& parent = EntityId::noId(),
                                                  int index = ConstrainedVariable::NO_INDEX);

      /**
       * @brief Create a value for a string
       */
      double createValue(const char* typeName, 
                         std::string value);

      /**
       * @brief Return the base domain
       */
      const AbstractDomain & baseDomain(const char* typeName);

    protected:
      CESchemaId m_id;
      std::map<double, TypeFactoryId> m_factories;                    
  };
  
} // namespace EUROPA

#endif // _H_CESchema
