#ifndef INT_TYPE_HH
#define INT_TYPE_HH

#include "IntervalIntDomain.hh"
#include "TypeFactory.hh"

namespace Prototype {

  /**
   * @class intDomain
   * @brief same as IntervalIntDomain, except with the NDDL-specific "int" type name.
   */
  class intDomain : public IntervalIntDomain {
  public:
    intDomain();

    /**
     * @brief Get the name of the type of the domain.
     * @see AbstractDomain::getTypeName
     */
    virtual const LabelStr& getTypeName() const;
  };

  class intTypeFactory : public ConcreteTypeFactory {
  public:
    intTypeFactory();

    /**
     * @brief Create a variable
     */
    virtual ConstrainedVariableId createVariable(const ConstraintEngineId& constraintEngine, 
                                                 const LabelStr& variableName) const;

    /**
     * @brief Create a domain
     */
    virtual AbstractDomain * createDomain() const;

    /**
     * @brief Create a value for a string
     */
    virtual double createValue(std::string value) const;

  };

} // namespace Prototype

#endif // INT_TYPE_HH
