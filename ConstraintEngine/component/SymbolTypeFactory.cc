#include "SymbolTypeFactory.hh"
#include "SymbolDomain.hh"
#include "Variable.hh"

namespace Prototype {
  
  //
  // SymbolTypeFactory
  //

  SymbolTypeFactory::SymbolTypeFactory() : ConcreteTypeFactory(SymbolDomain().getTypeName()) {}

  ConstrainedVariableId
  SymbolTypeFactory::createVariable(const ConstraintEngineId& constraintEngine, 
                                    const AbstractDomain& baseDomain,
                                    bool canBeSpecified = true,
                                    const LabelStr& name = ConstrainedVariable::NO_NAME(),
                                    const EntityId& parent = EntityId::noId(),
                                    int index = ConstrainedVariable::NO_INDEX) const
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

  AbstractDomain *
  SymbolTypeFactory::createDomain() const
  {
    SymbolDomain * domain = new SymbolDomain();
    check_error(domain != NULL, "failed to create SymbolDomain");
    return domain;
  }

  double SymbolTypeFactory::createValue(std::string value) const
  {
    return LabelStr(value);
  }

} // namespace Prototype
