#ifndef _H_TypeFactory
#define _H_TypeFactory

#include "ConstraintEngineDefs.hh"
#include "AbstractDomain.hh"
#include "ConstrainedVariable.hh"
#include "LabelStr.hh"
#include "Engine.hh"
#include <map>
#include <string>

/**
 * @file Factory class for allocation of variables, domains, and values.
 * @author Andrew Bachmann, July, 2004
 */

namespace EUROPA {

  class TypeFactory;
  typedef Id<TypeFactory> TypeFactoryId;
  
  class TypeFactoryMgr;
  typedef Id<TypeFactoryMgr> TypeFactoryMgrId;

  /**
   * @brief Each concrete class must provide an implementation for this.
   */
  class TypeFactory
  {
    public:
      TypeFactory(const char* typeName);

      virtual ~TypeFactory();

      const TypeFactoryId& getId() const;

      /**
       * @brief Return the type for which this factory is registered.
       */
      const LabelStr& getTypeName() const;

      /**
       * @brief Create a variable
       */
      virtual ConstrainedVariableId createVariable(const ConstraintEngineId& constraintEngine, 
                                                   const AbstractDomain& baseDomain,
                                                   bool canBeSpecified = true,
                                                   const char* name = NO_VAR_NAME,
                                                   const EntityId& parent = EntityId::noId(),
                                                   int index = ConstrainedVariable::NO_INDEX) const = 0;

      /**
       * @brief Create a value for a string
       */
      virtual double createValue(std::string value) const = 0;

      /**
       * @brief Return the base domain
       */
      virtual const AbstractDomain& baseDomain() const = 0;

    protected:
      TypeFactoryId m_id;
      LabelStr m_typeName;
  };
  
  class TypeFactoryMgr : public EngineComponent
  {
    public:
      TypeFactoryMgr();
      virtual ~TypeFactoryMgr();
  
      const TypeFactoryMgrId& getId() const;
      
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
      TypeFactoryMgrId m_id;
      std::map<double, TypeFactoryId> m_factories;                    
  };
  
} // namespace EUROPA

#endif // _H_TypeFactory
