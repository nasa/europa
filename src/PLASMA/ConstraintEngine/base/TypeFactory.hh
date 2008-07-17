#ifndef _H_TypeFactory
#define _H_TypeFactory

#include "ConstraintEngineDefs.hh"
#include "AbstractDomain.hh"
#include "ConstrainedVariable.hh"
#include "LabelStr.hh"
#include <map>
#include <string>

/**
 * @file Factory class for allocation of variables, domains, and values.
 * @author Andrew Bachmann, July, 2004
 */

namespace EUROPA {

  class TypeFactory;
  typedef Id<TypeFactory> TypeFactoryId;
  
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
                                                   const bool internal = false,
                                                   bool canBeSpecified = true,
                                                   const char* name = NO_VAR_NAME,
                                                   const EntityId& parent = EntityId::noId(),
                                                   int index = ConstrainedVariable::NO_INDEX) const = 0;

      /**
       * @brief Create a value for a string
       */
      virtual double createValue(const std::string& value) const = 0;

      /**
       * @brief Return the base domain
       */
      virtual const AbstractDomain& baseDomain() const = 0;

    protected:
      TypeFactoryId m_id;
      LabelStr m_typeName;
  };  
} // namespace EUROPA

#endif // _H_TypeFactory
