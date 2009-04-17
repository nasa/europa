#include "IntervalIntDomain.hh"
#include <cmath>

namespace EUROPA {


  IntervalIntDomain::IntervalIntDomain(const DataTypeId& dt) : IntervalDomain(dt)  { m_minDelta = 1.0; }
  IntervalIntDomain::IntervalIntDomain(int lb, int ub, const DataTypeId& dt) : IntervalDomain(lb,ub,dt) { m_minDelta = 1.0; }
  IntervalIntDomain::IntervalIntDomain(int value, const DataTypeId& dt) : IntervalDomain(value,dt) { m_minDelta = 1.0; }
  IntervalIntDomain::IntervalIntDomain(const AbstractDomain& org) : IntervalDomain(org) { m_minDelta = 1.0; }

  IntervalIntDomain::~IntervalIntDomain()
  {
  }

  bool IntervalIntDomain::isFinite() const {
    check_error(!isOpen());
    return(m_lb > -MAX_INT && m_ub < MAX_INT);
  }

  bool IntervalIntDomain::isSingleton() const {
    return(m_lb == m_ub);
  }

  void IntervalIntDomain::testPrecision(const double& value) const {
#ifndef EUROPA_FAST
    int intValue = (int) value;
    double dblValue = (double) intValue;
    checkError(dblValue == value,
	       value << " must be an integer."); // confirms no loss in precision
#endif
  }

  double IntervalIntDomain::convert(const double& value) const {
    return((int) value);
  }

  void IntervalIntDomain::insert(double value) {
    check_error(check_value(value));
    if (isMember(value))
      return; // Already in the interval.
    if (isEmpty()) {
      m_lb = convert(value);
      m_ub = m_lb;
      if (!isOpen())
        notifyChange(DomainListener::RELAXED);
      return;
    }
    // If just outside and low, reduce lower bound:
    if (m_lb - minDelta() <= value && value < m_lb) {
      m_lb = convert(value);
      if (!isOpen())
        notifyChange(DomainListener::RELAXED);
      return;
    }
    // If just outside and high, increase upper bound:
    if (m_ub < value && value <= m_ub + minDelta()) {
      m_ub = convert(value);
      if (!isOpen())
        notifyChange(DomainListener::RELAXED);
      return;
    }
    // Too far outside the interval to represent with a single interval.
    check_error(ALWAYS_FAILS);
  }

  void IntervalIntDomain::getValues(std::list<double>& results) const {
    check_error(isFinite());
    int lb = (int) check(m_lb);
    int ub = (int) check(m_ub);
    for (int i = lb; i <= ub; i++)
      results.push_back(i);
  }

  double IntervalIntDomain::translateNumber(double number, bool asMin) const {
    double result = IntervalDomain::translateNumber(int(number), asMin);

    // Incrementing result will round up.
    // Why the condition that number is positive? It breaks symmetry if nothing else. --wedgingt 2004 Mar 4
    if ((fabs(result - number) >= EPSILON) && asMin && number > 0)
      result = result + 1;
    return(result);
  }

  IntervalIntDomain *IntervalIntDomain::copy() const {
    IntervalIntDomain *ptr = new IntervalIntDomain(*this);
    check_error(ptr != 0);
    return(ptr);
  }

  bool IntervalIntDomain::intersect(double lb, double ub) {
    return IntervalDomain::intersect(ceil(lb), floor(ub));
  }

  bool IntervalIntDomain::intersect(const AbstractDomain& dom) {
    return intersect(dom.getLowerBound(), dom.getUpperBound());
  }
}
