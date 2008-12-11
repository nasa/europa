#ifndef _H_SymbolTypeFactory
#define _H_SymbolTypeFactory

#include "TypeFactory.hh"
#include "SymbolDomain.hh"

namespace EUROPA {

  class SymbolTypeFactory : public TypeFactory {
  public:
    SymbolTypeFactory(const std::string& name = SymbolDomain::getDefaultTypeName().toString());
    SymbolTypeFactory(const std::string& name, const SymbolDomain& baseDomain);

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
    const SymbolDomain m_baseDomain;
  };

} // namespace EUROPA

#endif // _H_SymbolTypeFactory
