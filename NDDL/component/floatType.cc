#include "floatType.hh"
#include "Variable.hh"

namespace Prototype {
  
  //
  // floatDomain
  //

  floatDomain::floatDomain() : IntervalDomain() {}

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
                                   const LabelStr& variableName) const
  {
    floatDomain * baseDomain = static_cast<floatDomain*>(createDomain());
    Variable<floatDomain> * variable
      = new Variable<floatDomain>(constraintEngine, *baseDomain, true, variableName);
    check_error(variable != NULL,
                "failed to create Variable for 'float' with name '" + variableName.toString() + "'");
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
