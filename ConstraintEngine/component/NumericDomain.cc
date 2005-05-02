#include "NumericDomain.hh"

namespace EUROPA {


  NumericDomain::NumericDomain()
    : EnumeratedDomain(true, getDefaultTypeName().c_str()){}

  NumericDomain::NumericDomain(const char* typeName)
    : EnumeratedDomain(true, typeName){}

  NumericDomain::NumericDomain(double value, const char* typeName) 
    : EnumeratedDomain(value, true, typeName){}

  NumericDomain::NumericDomain(const std::list<double>& values, 
                             const char* typeName)
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
  void NumericDomain::set(const NumericDomain& dom){
    EnumeratedDomain::set(dom);
  }
}
