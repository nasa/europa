#include "IntervalIntDomain.hh"

namespace Prototype {

  IntervalIntDomain::IntervalIntDomain(int lb, int ub, bool closed, const DomainListenerId& listener)
    :IntervalDomain(lb, ub, closed, listener){}

  IntervalIntDomain::IntervalIntDomain(const DomainListenerId& listener)
    :IntervalDomain(-MAX_INT, MAX_INT, true, listener){}

  IntervalIntDomain::IntervalIntDomain(const IntervalIntDomain& org)
    :IntervalDomain(org){}

  bool IntervalIntDomain::isFinite() const {
    check_error(!isDynamic());
    return (m_lb > -MAX_INT && m_ub < MAX_INT);
  }

  void IntervalIntDomain::testPrecision(const double& value) const {
    int intValue = (int) value;
    double dblValue =(double) intValue;
    check_error(dblValue == value) // confirms no loss in precision
  }

  const AbstractDomain::DomainType& IntervalIntDomain::getType() const{
    static const AbstractDomain::DomainType s_type = INT_INTERVAL;
    return s_type;
  }
}
