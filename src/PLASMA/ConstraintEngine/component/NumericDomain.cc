#include "NumericDomain.hh"

namespace EUROPA {

  NumericDomain::NumericDomain(const DataTypeId& dt) : EnumeratedDomain(dt) {}
  NumericDomain::NumericDomain(double value, const DataTypeId& dt) : EnumeratedDomain(dt,value) {}
  NumericDomain::NumericDomain(const std::list<double>& values, const DataTypeId& dt) : EnumeratedDomain(dt,values) {}

  NumericDomain::NumericDomain(const AbstractDomain& org) : EnumeratedDomain(org) {}

  NumericDomain* NumericDomain::copy() const
  {
    NumericDomain * ptr = new NumericDomain(*this);
    check_error(ptr != NULL);
    return ptr;
  }

}
