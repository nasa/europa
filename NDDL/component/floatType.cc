#include "floatType.hh"
#include "Variable.hh"

namespace Prototype {
  
  //
  // floatDomain
  //

  floatDomain::floatDomain(const DomainListenerId& listener)
   : IntervalDomain(listener)
  {
  }

  floatDomain::floatDomain(double lb, double ub, 
    	                   const DomainListenerId& listener)
   : IntervalDomain(lb, ub, listener)
  {
  }

  floatDomain::floatDomain(double value, 
    	                   const DomainListenerId& listener)
   : IntervalDomain(value, listener)
  {
  }

  floatDomain::floatDomain(const floatDomain& org)
   : IntervalDomain(org)
  {
  }

  const LabelStr& floatDomain::getTypeName() const
  {
    static const LabelStr sl_typeName("float");
    return(sl_typeName);
  }

  //
  // floatTypeFactory
  //

  floatTypeFactory::floatTypeFactory() : ConcreteTypeFactory(LabelStr("float")) {}

  ConstrainedVariableId
  floatTypeFactory::createVariable(const ConstraintEngineId& constraintEngine, 
                                   const LabelStr& typeName,
                                   bool canBeSpecified,
                                   const LabelStr& name,
                                   const EntityId& parent,
                                   int index) const
  {
    floatDomain * baseDomain = static_cast<floatDomain*>(createDomain());
    Variable<floatDomain> * variable
      = new Variable<floatDomain>(constraintEngine, *baseDomain, canBeSpecified, name, parent, index);
    check_error(variable != NULL,
                "failed to create Variable for 'float' with name '" + name.toString() + "'");
    ConstrainedVariableId id = variable->getId();
    check_error(id.isValid());
    return id;
  }

  AbstractDomain *
  floatTypeFactory::createDomain() const
  {
    floatDomain * domain = new floatDomain();
    check_error(domain != NULL, "failed to create 'float' domain");
    return domain;
  }

  double floatTypeFactory::createValue(std::string value) const
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
