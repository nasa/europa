#ifndef _H_IntervalTypeFactory
#define _H_IntervalTypeFactory

#include "TypeFactory.hh"
#include "IntervalDomain.hh"

namespace Prototype {

  class IntervalTypeFactory : public ConcreteTypeFactory {
  public:
    /**
     * Permit registration by an external name
     */
    IntervalTypeFactory(const LabelStr& name = IntervalDomain::getDefaultTypeName());

    /**
     * Register with an external name and a base domain
     */
    IntervalTypeFactory(const LabelStr& name, const IntervalDomain& baseDomain);

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
     * @brief Return the base domain
     */
    virtual const AbstractDomain & baseDomain() const;

    /**
     * @brief Create a value for a string
     */
    virtual double createValue(std::string value) const;

  private:
    IntervalDomain m_baseDomain;
  };

} // namespace Prototype

#endif // _H_IntervalTypeFactory
