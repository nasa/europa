#include "intType.hh"
#include "Variable.hh"

namespace Prototype {
  
  //
  // intDomain
  //

  intDomain::intDomain() : IntervalIntDomain() {}

  const LabelStr& intDomain::getTypeName() const
  {
    static const LabelStr sl_typeName("int");
    return(sl_typeName);
  }

  //
  // intTypeFactory
  //

  intTypeFactory::intTypeFactory() : ConcreteTypeFactory(LabelStr("int")) {}

  ConstrainedVariableId
  intTypeFactory::createVariable(const ConstraintEngineId& constraintEngine, 
                                 const LabelStr& variableName) const
  {
    intDomain * baseDomain = static_cast<intDomain*>(createDomain());
    Variable<intDomain> * variable
      = new Variable<intDomain>(constraintEngine, *baseDomain, true, variableName);
    check_error(variable != NULL,
                "failed to create Variable for 'int' with name '" + variableName.toString() + "'");
    ConstrainedVariableId id = variable->getId();
    check_error(id.isValid());
    return id;
  }

  AbstractDomain *
  intTypeFactory::createDomain() const
  {
    intDomain * domain = new intDomain();
    check_error(domain != NULL, "failed to create 'int' domain");
    return domain;
  }

  double intTypeFactory::createValue(std::string value) const
  {
    if (value == "-inf") {
      return MINUS_INFINITY;
    }
    if (value == "+inf") {
      return PLUS_INFINITY;
    }
    return atoi(value.c_str());
  }

} // namespace Prototype
