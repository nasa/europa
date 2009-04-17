#include "SymbolDomain.hh"
#include "DataTypes.hh"

namespace EUROPA {

  SymbolDomain::SymbolDomain(const DataTypeId& dt) : EnumeratedDomain(dt) {}
  SymbolDomain::SymbolDomain(double value, const DataTypeId& dt) : EnumeratedDomain(dt,value) {}
  SymbolDomain::SymbolDomain(const std::list<double>& values, const DataTypeId& dt) : EnumeratedDomain(dt,values) {}

  SymbolDomain::SymbolDomain(const AbstractDomain& org) : EnumeratedDomain(org) {}

  SymbolDomain* SymbolDomain::copy() const
  {
    SymbolDomain * ptr = new SymbolDomain(*this);
    check_error(ptr != NULL);
    return ptr;
  }

} // namespace EUROPA
