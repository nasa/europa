#include "BoolDomain.hh"

namespace Prototype {

  BoolDomain::BoolDomain(bool singletonValue, const DomainListenerId& listener)
    : IntervalDomain(singletonValue, singletonValue, listener) { }

  BoolDomain::BoolDomain(const DomainListenerId& listener)
    : IntervalDomain(0, 1, listener) { }

  BoolDomain::BoolDomain(const BoolDomain& org)
    : IntervalDomain(org) { }

  void BoolDomain::testPrecision(const double& value) const {
    check_error(value == 0 || value == 1);
  }

  double BoolDomain::convert(const double& value) const {
    return(value);
  }

  bool BoolDomain::isFinite() const{
    return(true);
  }

  bool BoolDomain::isFalse() const {
    return(m_ub == 0 && m_lb == 0);
  }

  bool BoolDomain::isTrue() const {
    return(m_ub == 1 && m_lb == 1);
  }

  const AbstractDomain::DomainType& BoolDomain::getType() const {
    static const AbstractDomain::DomainType s_type = BOOL;
    return s_type;
  }
}
