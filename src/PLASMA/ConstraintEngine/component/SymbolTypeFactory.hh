#ifndef _H_SymbolTypeFactory
#define _H_SymbolTypeFactory

#include "TypeFactory.hh"
#include "SymbolDomain.hh"

namespace EUROPA {

  class SymbolTypeFactory : public TypeFactory {
  public:
    SymbolTypeFactory(const char* name = SymbolDomain::getDefaultTypeName().c_str());
    SymbolTypeFactory(const char* name, const SymbolDomain& baseDomain);

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
    virtual double createValue(const std::string& value) const;

  private:
    const SymbolDomain m_baseDomain;
  };

} // namespace EUROPA

#endif // _H_SymbolTypeFactory
