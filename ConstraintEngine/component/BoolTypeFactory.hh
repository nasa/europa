#ifndef _H_BoolTypeFactory
#define _H_BoolTypeFactory

#include "TypeFactory.hh"
#include "BoolDomain.hh"

namespace PLASMA {

  class BoolTypeFactory : public ConcreteTypeFactory {
  public:
    BoolTypeFactory(const char* name = BoolDomain::getDefaultTypeName().toString().c_str());

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
    BoolDomain m_baseDomain;
  };

} // namespace PLASMA

#endif // _H_BoolTypeFactory
