#include "boolType.hh"
#include "Variable.hh"

namespace Prototype {
  
  //
  // boolDomain
  //

  boolDomain::boolDomain(const DomainListenerId& listener)
   : BoolDomain(listener)
  {
  }

  boolDomain::boolDomain(bool value, 
                         const DomainListenerId& listener)
   : BoolDomain(value, listener)
  {
  }

  boolDomain::boolDomain(const boolDomain& org)
   : BoolDomain(org)
  {
  }

  const LabelStr& boolDomain::getTypeName() const
  {
    static const LabelStr sl_typeName("bool");
    return(sl_typeName);
  }

  //
  // boolTypeFactory
  //

  boolTypeFactory::boolTypeFactory() : ConcreteTypeFactory(LabelStr("bool")) {}

  ConstrainedVariableId
  boolTypeFactory::createVariable(const ConstraintEngineId& constraintEngine, 
                                  const LabelStr& typeName,
                                  bool canBeSpecified,
                                  const LabelStr& name,
                                  const EntityId& parent,
                                  int index) const
  {
    boolDomain * baseDomain = static_cast<boolDomain*>(createDomain());
    Variable<boolDomain> * variable
      = new Variable<boolDomain>(constraintEngine, *baseDomain, canBeSpecified, name, parent, index);
    check_error(variable != NULL,
                "failed to create Variable for 'bool' with name '" + name.toString() + "'");
    ConstrainedVariableId id = variable->getId();
    check_error(id.isValid());
    return id;
  }

  AbstractDomain *
  boolTypeFactory::createDomain() const
  {
    boolDomain * domain = new boolDomain();
    check_error(domain != NULL, "failed to create 'bool' domain");
    return domain;
  }

  double boolTypeFactory::createValue(std::string value) const
  {
    if (value == "-inf") {
      return MINUS_INFINITY;
    }
    if (value == "+inf") {
      return PLUS_INFINITY;
    }
    return atof(value.c_str());
  }

} // namespace Prototype
