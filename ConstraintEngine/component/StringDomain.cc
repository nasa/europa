#include "StringDomain.hh"

namespace Prototype {

  StringDomain::StringDomain()
    :EnumeratedDomain(false, getDefaultTypeName().c_str()){}

  StringDomain::StringDomain(const char* typeName)
    :EnumeratedDomain(false, typeName){}

  StringDomain::StringDomain(double value, const char* typeName) 
    : EnumeratedDomain(value, false, typeName){}

  StringDomain::StringDomain(const std::list<double>& values, 
                             const char* typeName)
    : EnumeratedDomain(values, false, typeName)
  {
  }

  StringDomain::StringDomain(const AbstractDomain& org)
    : EnumeratedDomain(org)
  {
  }


  const AbstractDomain::DomainType&
  StringDomain::getType() const
  {
    static const AbstractDomain::DomainType s_type = STRING_ENUMERATION;
    return(s_type);
  }


  const LabelStr&
  StringDomain::getDefaultTypeName()
  {
    static const LabelStr sl_typeName("STRING_ENUMERATION");
    return(sl_typeName);
  }


  StringDomain *
  StringDomain::copy() const
  {
    StringDomain * ptr = new StringDomain(*this);
    check_error(ptr != NULL);
    return ptr;
  }
  
  /**
   * This appears to be necessary, though it should be sufficient to use the
   * base class method rather than having to delegate to it.
   */
  void StringDomain::set(const StringDomain& dom){
    EnumeratedDomain::set(dom);
  }
  
  void StringDomain::set(double value) {
    check_error(LabelStr::isString(value));
    EnumeratedDomain::set(value);
  }

} // namespace Prototype
