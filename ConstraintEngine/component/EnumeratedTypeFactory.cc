#include "EnumeratedTypeFactory.hh"
#include "EnumeratedDomain.hh"
#include "Variable.hh"

namespace Prototype {
  
  //
  // EnumeratedTypeFactory
  //

  EnumeratedTypeFactory::EnumeratedTypeFactory(const LabelStr& typeName, const LabelStr& elementName)
   : ConcreteTypeFactory(typeName), m_elementName(elementName), m_baseDomain(false, typeName)
  {
  }

  EnumeratedTypeFactory::EnumeratedTypeFactory(const LabelStr& typeName, const LabelStr& elementName, const EnumeratedDomain& baseDomain)
   : ConcreteTypeFactory(typeName), m_elementName(elementName), m_baseDomain(baseDomain)
  {
  }

  ConstrainedVariableId
  EnumeratedTypeFactory::createVariable(const ConstraintEngineId& constraintEngine, 
                                  const AbstractDomain& baseDomain,
                                  bool canBeSpecified,
                                  const LabelStr& name,
                                  const EntityId& parent,
                                  int index) const
  {
    const EnumeratedDomain * enumeratedDomain = dynamic_cast<const EnumeratedDomain*>(&baseDomain);
    check_error(enumeratedDomain != NULL, "tried to create a EnumeratedDomain variable with a different kind of base domain");
    Variable<EnumeratedDomain> * variable
      = new Variable<EnumeratedDomain>(constraintEngine, *enumeratedDomain, canBeSpecified, name, parent, index);
    check_error(variable != NULL,
                "failed to create Variable for EnumeratedDomain with name '" + name.toString() + "'");
    ConstrainedVariableId id = variable->getId();
    check_error(id.isValid());
    return id;
  }

  const AbstractDomain &
  EnumeratedTypeFactory::baseDomain() const
  {
    return m_baseDomain;
  }

  double EnumeratedTypeFactory::createValue(std::string value) const
  {
    return TypeFactory::createValue(m_elementName, value);
  }

} // namespace Prototype
