#include "floatType.hh"
#include "Variable.hh"
#include "Utils.hh"

namespace EUROPA {
  
  //
  // floatTypeFactory
  //

  floatTypeFactory::floatTypeFactory()
    //how did this compile in the first place??
//     : IntervalTypeFactory(getDefaultTypeName().c_str(), getDefaultTypeName().c_str()){}
    : IntervalTypeFactory(getDefaultTypeName().toString()) {}

  edouble floatTypeFactory::createValue(const std::string& value) const
  {
    if (value == "-inf") {
      return MINUS_INFINITY;
    }
    if (value == "+inf") {
      return PLUS_INFINITY;
    }
    return toValue<edouble>(value);
  }

  const LabelStr& floatTypeFactory::getDefaultTypeName() {
    static const LabelStr sl_typeName("float");
    return(sl_typeName);
  }

} // namespace EUROPA
