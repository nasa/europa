#include "intType.hh"
#include "Variable.hh"

namespace Prototype {
  
  //
  // intTypeFactory
  //

  intTypeFactory::intTypeFactory()
    : IntervalIntTypeFactory(getDefaultTypeName().c_str(), getDefaultTypeName().c_str()){}

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

  const LabelStr& intTypeFactory::getDefaultTypeName() {
    static const LabelStr sl_typeName("int");
    return(sl_typeName);
  }

} // namespace Prototype
