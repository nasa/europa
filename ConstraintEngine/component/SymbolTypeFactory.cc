#include "SymbolTypeFactory.hh"
#include "SymbolDomain.hh"
#include "Variable.hh"

namespace Prototype {
  
  //
  // SymbolTypeFactory
  //

  SymbolTypeFactory::SymbolTypeFactory(const LabelStr& name)
   : ConcreteTypeFactory(name), m_baseDomain(name)
  {
  }

  ConstrainedVariableId
  SymbolTypeFactory::createVariable(const ConstraintEngineId& constraintEngine, 
                                    const AbstractDomain& baseDomain,
                                    bool canBeSpecified,
                                    const LabelStr& name,
                                    const EntityId& parent,
                                    int index) const
  {
    const SymbolDomain * stringDomain = dynamic_cast<const SymbolDomain*>(&baseDomain);
    check_error(stringDomain != NULL, "tried to create a SymbolDomain variable with a different kind of base domain");
    Variable<SymbolDomain> * variable
      = new Variable<SymbolDomain>(constraintEngine, *stringDomain, canBeSpecified, name, parent, index);
    check_error(variable != NULL,
                "failed to create Variable for SymbolDomain with name '" + name.toString() + "'");
    ConstrainedVariableId id = variable->getId();
    check_error(id.isValid());
    return id;
  }

  const AbstractDomain &
  SymbolTypeFactory::baseDomain() const
  {
    return m_baseDomain;
  }

  double SymbolTypeFactory::createValue(std::string value) const
  {
    return LabelStr(value);
  }

} // namespace Prototype
