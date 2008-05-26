#ifndef _H_IntervalIntTypeFactory
#define _H_IntervalIntTypeFactory

#include "TypeFactory.hh"
#include "IntervalIntDomain.hh"

namespace EUROPA {

  class IntervalIntTypeFactory : public TypeFactory {
  public:
    /**
     * Permit registration by an external name
     */
    IntervalIntTypeFactory(const char* name = IntervalIntDomain::getDefaultTypeName().c_str());

    /**
     * Register with an external name and a base domain
     */
    IntervalIntTypeFactory(const char* name, const IntervalIntDomain& baseDomain);

    /**
     * @brief Create a variable
     */
    virtual ConstrainedVariableId createVariable(const ConstraintEngineId& constraintEngine, 
                                                 const AbstractDomain& baseDomain,
                                                 bool canBeSpecified = true,
                                                 const char* name = NO_VAR_NAME,
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
    const IntervalIntDomain m_baseDomain;
  };

} // namespace EUROPA

#endif // _H_IntervalIntTypeFactory
