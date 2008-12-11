#ifndef _H_IntervalTypeFactory
#define _H_IntervalTypeFactory

#include "TypeFactory.hh"
#include "IntervalDomain.hh"

namespace EUROPA {

  class IntervalTypeFactory : public TypeFactory {
  public:
    /**
     * Permit registration by an external name
     */
    IntervalTypeFactory(const std::string& name = IntervalDomain::getDefaultTypeName().toString());

    /**
     * Register with an external name and a base domain
     */
    IntervalTypeFactory(const std::string& name, const IntervalDomain& baseDomain);

    /**
     * @brief Create a variable
     */
    virtual ConstrainedVariableId createVariable(const ConstraintEngineId& constraintEngine, 
                                                 const AbstractDomain& baseDomain,
                                                 const bool internal = false,
                                                 bool canBeSpecified = true,
                                                 const std::string& name = NO_VAR_NAME,
                                                 const EntityId& parent = EntityId::noId(),
                                                 int index = ConstrainedVariable::NO_INDEX) const;

    /**
     * @brief Return the base domain
     */
    virtual const AbstractDomain & baseDomain() const;

    /**
     * @brief Create a value for a string
     */
    virtual edouble createValue(const std::string& value) const;

  private:
    IntervalDomain m_baseDomain;
  };

} // namespace EUROPA

#endif // _H_IntervalTypeFactory
