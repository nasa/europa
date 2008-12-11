#ifndef _H_BoolTypeFactory
#define _H_BoolTypeFactory

#include "TypeFactory.hh"
#include "BoolDomain.hh"

namespace EUROPA {

  class BoolTypeFactory : public TypeFactory {
  public:
    BoolTypeFactory(const std::string& name = BoolDomain::getDefaultTypeName().toString());

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
    BoolDomain m_baseDomain;
  };

} // namespace EUROPA

#endif // _H_BoolTypeFactory
