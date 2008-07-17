#ifndef _H_StringTypeFactory
#define _H_StringTypeFactory

#include "TypeFactory.hh"
#include "StringDomain.hh"

namespace EUROPA {

  class StringTypeFactory : public TypeFactory {
  public:
    StringTypeFactory(const char* name = StringDomain::getDefaultTypeName().c_str());

    /**
     * @brief Create a variable
     */
    virtual ConstrainedVariableId createVariable(const ConstraintEngineId& constraintEngine, 
                                                 const AbstractDomain& baseDomain,
                                                 const bool internal = false,
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
    virtual double createValue(const std::string& value) const;

  private:
    StringDomain m_baseDomain;
  };

} // namespace EUROPA

#endif // _H_StringTypeFactory
