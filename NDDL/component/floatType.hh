#ifndef FLOAT_TYPE_HH
#define FLOAT_TYPE_HH

#include "IntervalDomain.hh"
#include "TypeFactory.hh"

namespace Prototype {

  /**
   * @class floatDomain
   * @brief same as IntervalDomain, except with the NDDL-specific "float" type name.
   */
  class floatDomain : public IntervalDomain {
  public:
    floatDomain(const DomainListenerId& listener = DomainListenerId::noId());

    floatDomain(double lb, double ub, 
    	        const DomainListenerId& listener = DomainListenerId::noId());

    floatDomain(double value, 
    	        const DomainListenerId& listener = DomainListenerId::noId());

    floatDomain(const floatDomain& org);

    /**
     * @brief Get the name of the type of the domain.
     * @see AbstractDomain::getTypeName
     */
    virtual const LabelStr& getTypeName() const;
  };

  class floatTypeFactory : public ConcreteTypeFactory {
  public:
    floatTypeFactory();

    /**
     * @brief Create a variable
     */
    virtual ConstrainedVariableId createVariable(const ConstraintEngineId& constraintEngine, 
                                                 const LabelStr& typeName,
                                                 bool canBeSpecified = true,
                                                 const LabelStr& name = ConstrainedVariable::NO_NAME(),
                                                 const EntityId& parent = EntityId::noId(),
                                                 int index = ConstrainedVariable::NO_INDEX) const;

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

#endif // FLOAT_TYPE_HH
