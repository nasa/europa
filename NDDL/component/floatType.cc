#include "floatType.hh"
#include "Variable.hh"

namespace Prototype {
  
  //
  // floatTypeFactory
  //

  floatTypeFactory::floatTypeFactory() : IntervalTypeFactory(getDefaultTypeName()) {}

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

  const LabelStr& floatTypeFactory::getDefaultTypeName() {
    static const LabelStr sl_typeName("float");
    return(sl_typeName);
  }

} // namespace Prototype
