#include "IntervalIntDomain.hh"

namespace Prototype {

  IntervalIntDomain::IntervalIntDomain(int lb, int ub, bool finite, bool closed, const DomainListenerId& listener)
    :IntervalDomain(lb, ub, finite, closed, listener){}

  IntervalIntDomain::IntervalIntDomain(const DomainListenerId& listener)
    :IntervalDomain(-MAX_INT, MAX_INT, true, true, listener){}

  IntervalIntDomain::IntervalIntDomain(const IntervalIntDomain& org)
    :IntervalDomain(org){}

  IntervalIntDomain::IntervalIntDomain(Europa::Domain& org)
    : IntervalDomain(org) {
    check_error(org.getSort() == Europa::intSort);
  }

  IntervalIntDomain& IntervalIntDomain::operator=(const IntervalIntDomain& org){
    IntervalDomain::operator=(org);
    return(*this);
  }

  bool IntervalIntDomain::operator==(const IntervalIntDomain& dom) const{
    return IntervalDomain::operator==(dom);
  }

  bool IntervalIntDomain::intersect(const IntervalIntDomain& dom){
    return IntervalDomain::intersect(dom);
  }

  bool IntervalIntDomain::isSubsetOf(const IntervalIntDomain& dom) const{
    return IntervalDomain::isSubsetOf(dom);
  }

  void IntervalIntDomain::testPrecision(const double& value) const {
    int intValue = (int) value;
    double dblValue =(double) intValue;
    check_error(dblValue == value) // confirms no loss in precision
  }

  const Europa::Domain IntervalIntDomain::makeDomain() const {
    return Europa::Domain(Europa::intSort, (int) m_lb, (int) m_ub);
  }

  const AbstractDomain::DomainType& IntervalIntDomain::getType() const{
    static const AbstractDomain::DomainType s_type = INT_INTERVAL;
    return s_type;
  }
}
