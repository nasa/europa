#include "IntervalIntDomain.hh"

namespace Prototype {

  IntervalIntDomain::IntervalIntDomain(const DomainListenerId& listener)
    : IntervalDomain(listener) {
  }

  IntervalIntDomain::IntervalIntDomain(int lb, int ub, const DomainListenerId& listener)
    : IntervalDomain(lb, ub, listener) {
    check_value(lb);
    check_value(ub);
  }

  IntervalIntDomain::IntervalIntDomain(int value, const DomainListenerId& listener)
    : IntervalDomain(value, listener) {
    check_value(value);
  }

  IntervalIntDomain::IntervalIntDomain(const IntervalIntDomain& org)
    : IntervalDomain(org) {
  }

  bool IntervalIntDomain::isFinite() const {
    check_error(!isDynamic());
    return (m_lb > -MAX_INT && m_ub < MAX_INT);
  }

  void IntervalIntDomain::testPrecision(const double& value) const {
    int intValue = (int) value;
    double dblValue = (double) intValue;
    check_error(dblValue == value); // confirms no loss in precision
  }

  double IntervalIntDomain::convert(const double& value) const{
    return((int) value);
  }

  const AbstractDomain::DomainType& IntervalIntDomain::getType() const {
    static const AbstractDomain::DomainType s_type = INT_INTERVAL;
    return(s_type);
  }

  void IntervalIntDomain::insert(double value) {
    check_error(check_value(value));
    if (isMember(value))
      return; // Already in the interval.
    if (isEmpty()) {
      m_lb = convert(value);
      m_ub = m_lb;
      if (!isDynamic())
        notifyChange(DomainListener::RELAXED);
      return;
    }
    // If just outside and low, reduce lower bound:
    if (m_lb - minDelta() <= value && value < m_lb) {
      m_lb = convert(value);
      if (!isDynamic())
        notifyChange(DomainListener::RELAXED);
      return;
    }
    // If just outside and high, increase upper bound:
    if (m_ub < value && value <= m_ub + minDelta()) {
      m_ub = convert(value);
      if (!isDynamic())
        notifyChange(DomainListener::RELAXED);
      return;
    }
    // Too far outside the interval to represent with a single interval.
    check_error(ALWAYS_FAILS);
  }
    
  void IntervalIntDomain::remove(double value) {
    check_error(check_value(value));
    if (!isMember(value))
      return; // Outside the interval.
    if (m_lb + minDelta() <= value && value <= m_ub - minDelta()) {
      // Too far "inside" interval; would cause split.
      check_error(ALWAYS_FAILS);
    }
    if (fabs(value - m_lb) < minDelta()) {
      m_lb += minDelta();
      if (isEmpty())
        notifyChange(DomainListener::EMPTIED);
      else
        notifyChange(DomainListener::LOWER_BOUND_INCREASED);
      return;
    }
    if (fabs(m_ub - value) < minDelta()) {
      m_ub -= minDelta();
      check_error(!isEmpty()); // If it were empty, it should have been covered by prior EMPTIED call.
      notifyChange(DomainListener::UPPER_BOUND_DECREASED);
      return;
    }
    // Logic error above: the conditions should cover all possibilities.
    check_error(ALWAYS_FAILS);
  }

  double IntervalIntDomain::minDelta() const {
    return(1);
  }

  double IntervalIntDomain::translateNumber(double number, bool asMin) const {
    double result = IntervalDomain::translateNumber(int(number), asMin);

    // Incrementing result will round up.
    // Why the condition that number is positive? It breaks symmetry if nothing else. --wedgingt 2004 Mar 4
    if (result != number && asMin && number > 0)
      result = result + 1;
    return(result);
  }
}
