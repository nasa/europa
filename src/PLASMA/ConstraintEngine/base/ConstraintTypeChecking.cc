#include "ConstraintTypeChecking.hh"
#include "Domain.hh"

namespace EUROPA {

bool CanBePositive::operator()(const std::string&, DataTypeId dt, std::ostream&) const {
  return dt->baseDomain().getUpperBound() >= 0.0;
}

}
