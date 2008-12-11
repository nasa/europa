#include "NumericDomain.hh"

namespace EUROPA {


  NumericDomain::NumericDomain()
    : EnumeratedDomain(true, getDefaultTypeName().toString()){}

  NumericDomain::NumericDomain(const std::string& typeName)
    : EnumeratedDomain(true, typeName){}

  NumericDomain::NumericDomain(edouble value, const std::string& typeName) 
    : EnumeratedDomain(value, true, typeName){}

  NumericDomain::NumericDomain(const std::list<edouble>& values, 
                               const std::string& typeName)
    : EnumeratedDomain(values, true, typeName) {}

  NumericDomain::NumericDomain(const AbstractDomain& org)
    : EnumeratedDomain(org) {}

  const LabelStr&
  NumericDomain::getDefaultTypeName()
  {
    static const LabelStr sl_typeName("REAL_ENUMERATION");
    return(sl_typeName);
  }

  NumericDomain *
  NumericDomain::copy() const
  {
    NumericDomain * ptr = new NumericDomain(*this);
    check_error(ptr != NULL);
    return ptr;
  }
  
  /**
   * This appears to be necessary, though it should be sufficient to use the
   * base class method rather than having to delegate to it.
   */
  void NumericDomain::set(edouble value){
    EnumeratedDomain::set(value);
  }
}
