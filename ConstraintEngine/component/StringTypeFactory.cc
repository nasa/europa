#include "StringTypeFactory.hh"
#include "StringDomain.hh"
#include "Variable.hh"

namespace Prototype {
  
  //
  // StringTypeFactory
  //

  StringTypeFactory::StringTypeFactory() : ConcreteTypeFactory(StringDomain().getTypeName()) {}

  ConstrainedVariableId
  StringTypeFactory::createVariable(const ConstraintEngineId& constraintEngine, 
                                    const AbstractDomain& baseDomain,
                                    bool canBeSpecified = true,
                                    const LabelStr& name = ConstrainedVariable::NO_NAME(),
                                    const EntityId& parent = EntityId::noId(),
                                    int index = ConstrainedVariable::NO_INDEX) const
  {
    const StringDomain * stringDomain = dynamic_cast<const StringDomain*>(&baseDomain);
    check_error(stringDomain != NULL, "tried to create a StringDomain variable with a different kind of base domain");
    Variable<StringDomain> * variable
      = new Variable<StringDomain>(constraintEngine, *stringDomain, canBeSpecified, name, parent, index);
    check_error(variable != NULL,
                "failed to create Variable for StringDomain with name '" + name.toString() + "'");
    ConstrainedVariableId id = variable->getId();
    check_error(id.isValid());
    return id;
  }

  AbstractDomain *
  StringTypeFactory::createDomain() const
  {
    StringDomain * domain = new StringDomain();
    check_error(domain != NULL, "failed to create StringDomain");
    return domain;
  }

  double StringTypeFactory::createValue(std::string value) const
  {
    return LabelStr(value);
  }

} // namespace Prototype
