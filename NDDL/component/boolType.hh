#ifndef BOOL_TYPE_HH
#define BOOL_TYPE_HH

#include "BoolDomain.hh"
#include "TypeFactory.hh"

namespace Prototype {

  /**
   * @class boolDomain
   * @brief same as BoolDomain, except with the NDDL-specific "bool" type name.
   */
  class boolDomain : public BoolDomain {
  public:
    boolDomain(const DomainListenerId& listener = DomainListenerId::noId());

    boolDomain(bool value, 
    	       const DomainListenerId& listener = DomainListenerId::noId());

    boolDomain(const boolDomain& org);

    /**
     * @brief Get the name of the type of the domain.
     * @see AbstractDomain::getTypeName
     */
    virtual const LabelStr& getTypeName() const;
  };

  class boolTypeFactory : public ConcreteTypeFactory {
  public:
    boolTypeFactory();

    /**
     * @brief Create a variable
     */
    virtual ConstrainedVariableId createVariable(const ConstraintEngineId& constraintEngine, 
                                                 const AbstractDomain& baseDomain,
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

#endif // BOOL_TYPE_HH
