#include "floatType.hh"
#include "Variable.hh"

namespace EUROPA {
  
  //
  // floatTypeFactory
  //

  floatTypeFactory::floatTypeFactory()
    : IntervalTypeFactory(getDefaultTypeName().c_str(), getDefaultTypeName().c_str()){}

  double floatTypeFactory::createValue(const std::string& value) const
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

} // namespace EUROPA
