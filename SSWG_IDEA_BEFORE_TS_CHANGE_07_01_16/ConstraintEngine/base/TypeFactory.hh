#ifndef _H_TypeFactory
#define _H_TypeFactory

#include "ConstraintEngineDefs.hh"
#include "AbstractDomain.hh"
#include "ConstrainedVariable.hh"
#include "LabelStr.hh"
#include <list>
#include <string>

/**
 * @file Factory class for allocation of variables, domains, and values.
 * @author Andrew Bachmann, July, 2004
 */

namespace EUROPA {

  class ConcreteTypeFactory;
  typedef Id<ConcreteTypeFactory> ConcreteTypeFactoryId;

  /**
   * @brief Singleton, abstract factory which provides main point for 
   * variable, domain, value allocation for types.  It relies on binding
   * to concrete factories for each distinct type.
   * @see ConcreteTypeFactory
   */
  class TypeFactory {
  public:
    /**
     * @brief Should be private, but breaks with Andrews compiler if it is.
     */
    ~TypeFactory();

    /**
     * @brief Create a variable
     */
    static ConstrainedVariableId createVariable(const char* typeName,
                                                const ConstraintEngineId& constraintEngine, 
                                                bool canBeSpecified = true,
                                                const char* name = NO_VAR_NAME,
                                                const EntityId& parent = EntityId::noId(),
                                                int index = ConstrainedVariable::NO_INDEX);

    /**
     * @brief Create a variable
     */
    static ConstrainedVariableId createVariable(const char* typeName,
                                                const ConstraintEngineId& constraintEngine, 
                                                const AbstractDomain& baseDomain,
                                                bool canBeSpecified = true,
                                                const char* name = NO_VAR_NAME,
                                                const EntityId& parent = EntityId::noId(),
                                                int index = ConstrainedVariable::NO_INDEX);

    /**
     * @brief Return the base domain
     */
    static const AbstractDomain & baseDomain(const char* typeName);

    /**
     * @brief Create a value for a string
     */
    static double createValue(const char* typeName, 
                              std::string value);

    /**
     * @brief Delete all factory instances stored. Should only be used to support testing, since
     * factories should remain for all instances of the constraint engine in the same process.
     */
    static void purgeAll();

  private:
    friend class ConcreteTypeFactory; /*!< Requires access to registerFactory */

    /**
     * @brief Add a factory to provide instantiation of particular concrete types based on a label.
     */
    static void registerFactory(const ConcreteTypeFactoryId& factory);

    /**
     * @brief Obtain the factory based on the type name
     */ 
    static ConcreteTypeFactoryId getFactory(const char* typeName);

    static TypeFactory& getInstance();

    TypeFactory();

    std::map<double, ConcreteTypeFactoryId> m_factories;
  };

  /**
   * @brief Each concrete class must provide an implementation for this.
   */
  class ConcreteTypeFactory{
  protected:
    friend class TypeFactory;

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
     * @brief Return the base domain
     */
    virtual const AbstractDomain & baseDomain() const = 0;

    /**
     * @brief Create a value for a string
     */
    virtual double createValue(std::string value) const = 0;

    const ConcreteTypeFactoryId& getId() const;

    /**
     * @brief Return the type for which this factory is registered.
     */
    const LabelStr& getTypeName() const;

    ConcreteTypeFactory(const char* typeName);

    virtual ~ConcreteTypeFactory();

  private:
    ConcreteTypeFactoryId m_id;
    LabelStr m_typeName;
  };

} // namespace EUROPA

#endif // _H_TypeFactory
