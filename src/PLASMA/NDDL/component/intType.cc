#include "intType.hh"
#include "Variable.hh"
#include "Utils.hh"

namespace EUROPA {
  
  //
  // intTypeFactory
  //

  intTypeFactory::intTypeFactory()
  //how did this compile in the first place?
//     : IntervalIntTypeFactory(getDefaultTypeName().c_str(), getDefaultTypeName().c_str()){}
    : IntervalIntTypeFactory(getDefaultTypeName().toString()){}

  edouble intTypeFactory::createValue(const std::string& value) const
  {
    if (value == "-inf") {
      return MINUS_INFINITY;
    }
    if (value == "+inf") {
      return PLUS_INFINITY;
    }
    return toValue<eint>(value);
  }

  const LabelStr& intTypeFactory::getDefaultTypeName() {
    static const LabelStr sl_typeName("int");
    return(sl_typeName);
  }

} // namespace EUROPA
