#include "EnumeratedTypeFactory.hh"
#include "EnumeratedDomain.hh"
#include "Variable.hh"

namespace Prototype {
  
  //
  // EnumeratedTypeFactory
  //

  EnumeratedTypeFactory::EnumeratedTypeFactory(const char* typeName, const char* elementName)
   : ConcreteTypeFactory(typeName), m_elementName(elementName), m_baseDomain(false, typeName)
  {
  }

  EnumeratedTypeFactory::EnumeratedTypeFactory(const char* typeName, const char* elementName, const EnumeratedDomain& baseDomain)
   : ConcreteTypeFactory(typeName), m_elementName(elementName), m_baseDomain(baseDomain)
  {
  }

  ConstrainedVariableId
  EnumeratedTypeFactory::createVariable(const ConstraintEngineId& constraintEngine, 
                                  const AbstractDomain& baseDomain,
                                  bool canBeSpecified,
                                  const char* name,
                                  const EntityId& parent,
                                  int index) const
  {
    const EnumeratedDomain * enumeratedDomain = dynamic_cast<const EnumeratedDomain*>(&baseDomain);
    check_error(enumeratedDomain != NULL, "tried to create a EnumeratedDomain variable with a different kind of base domain");
    Variable<EnumeratedDomain> * variable
      = new Variable<EnumeratedDomain>(constraintEngine, *enumeratedDomain, canBeSpecified, name, parent, index);
    check_error(variable != NULL,
                "failed to create Variable for EnumeratedDomain with name '" + std::string(name) + "'");
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
    if(m_baseDomain.isNumeric())
      return atof(value.c_str());
    else
      return LabelStr(value);
  }

} // namespace Prototype
