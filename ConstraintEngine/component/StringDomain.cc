#include "StringDomain.hh"

namespace Prototype {

  StringDomain::StringDomain(const LabelStr& typeName) 
    : EnumeratedDomain(false, typeName)
  {
  }


  StringDomain::StringDomain(const std::list<double>& values, 
                             bool closed,
                             const DomainListenerId& listener,
                             const LabelStr& typeName)
    : EnumeratedDomain(values, closed, listener, false, typeName)
  {
  }


  StringDomain::StringDomain(double value,
                             const DomainListenerId& listener,
                             const LabelStr& typeName)
    : EnumeratedDomain(value, listener, false, typeName)
  {
  }


  StringDomain::StringDomain(const StringDomain& org)
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


} // namespace Prototype
