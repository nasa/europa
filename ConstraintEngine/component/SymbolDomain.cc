#include "SymbolDomain.hh"

namespace Prototype {

  SymbolDomain::SymbolDomain(const LabelStr& typeName) 
    : EnumeratedDomain(false, typeName)
  {
  }


  SymbolDomain::SymbolDomain(const std::list<double>& values, 
                             bool closed,
                             const DomainListenerId& listener,
                             const LabelStr& typeName)
    : EnumeratedDomain(values, closed, listener, false, typeName)
  {
  }


  SymbolDomain::SymbolDomain(double value,
                             const DomainListenerId& listener,
                             const LabelStr& typeName)
    : EnumeratedDomain(value, listener, false, typeName)
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
  SymbolDomain::getDefaultTypeName()
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
