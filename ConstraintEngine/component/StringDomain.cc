#include "StringDomain.hh"

namespace Prototype {

  StringDomain::StringDomain() 
    : EnumeratedDomain(false)
  {
  }


  StringDomain::StringDomain(const std::list<double>& values, 
                             bool closed = true,
                             const DomainListenerId& listener = DomainListenerId::noId())
    : EnumeratedDomain(values, closed, listener, false)
  {
  }


  StringDomain::StringDomain(double value,
                             const DomainListenerId& listener = DomainListenerId::noId())
    : EnumeratedDomain(value, listener, false)
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
  StringDomain::getTypeName() const
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
