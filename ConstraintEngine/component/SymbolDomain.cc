#include "SymbolDomain.hh"

namespace Prototype {

  SymbolDomain::SymbolDomain() 
    : EnumeratedDomain(false)
  {
  }


  SymbolDomain::SymbolDomain(const std::list<double>& values, 
                             bool closed = true,
                             const DomainListenerId& listener = DomainListenerId::noId())
    : EnumeratedDomain(values, closed, listener, false)
  {
  }


  SymbolDomain::SymbolDomain(double value,
                             const DomainListenerId& listener = DomainListenerId::noId())
    : EnumeratedDomain(value, listener, false)
  {
  }


  SymbolDomain::SymbolDomain(const SymbolDomain& org)
    : EnumeratedDomain(org)
  {
  }


  const AbstractDomain::DomainType&
  SymbolDomain::getType() const
  {
    static const AbstractDomain::DomainType s_type = SYMBOL_ENUMERATION;
    return(s_type);
  }


  const LabelStr&
  SymbolDomain::getTypeName() const
  {
    static const LabelStr sl_typeName("SYMBOL_ENUMERATION");
    return(sl_typeName);
  }


  SymbolDomain *
  SymbolDomain::copy() const
  {
    SymbolDomain * ptr = new SymbolDomain(*this);
    check_error(ptr != NULL);
    return ptr;
  }


} // namespace Prototype
