#include "IntervalIntDomain.hh"
#include <cmath>

namespace PLASMA {


  IntervalIntDomain::IntervalIntDomain()
    : IntervalDomain(getDefaultTypeName().c_str()) {}

  IntervalIntDomain::IntervalIntDomain(const char* typeName)
    : IntervalDomain(typeName){}

  IntervalIntDomain::IntervalIntDomain(int lb, int ub)
    : IntervalDomain(lb, ub, getDefaultTypeName().c_str()) {
    check_error(check_value(m_lb), "Invalid lower bound");
    check_error(check_value(m_ub), "Invalid upper bound");
  }

  IntervalIntDomain::IntervalIntDomain(int value)
    : IntervalDomain(value, value, getDefaultTypeName().c_str()) {
    check_error(check_value(value), "Invalid value");
  }

  IntervalIntDomain::IntervalIntDomain(int lb, int ub, const char* typeName)
    : IntervalDomain(lb, ub, typeName) {
    check_error(check_value(m_lb), "Invalid lower bound");
    check_error(check_value(m_ub), "Invalid upper bound");
  }

  IntervalIntDomain::IntervalIntDomain(const AbstractDomain& org)
    : IntervalDomain(org) {
    check_error(check_value(m_lb), "Invalid lower bound");
    check_error(check_value(m_ub), "Invalid upper bound");
  }

  bool IntervalIntDomain::isFinite() const {
    check_error(!isOpen());
    return(m_lb > -MAX_INT && m_ub < MAX_INT);
  }

  void IntervalIntDomain::testPrecision(const double& value) const {
    int intValue = (int) value;
    double dblValue = (double) intValue;
    check_error(dblValue == value); // confirms no loss in precision
  }

  double IntervalIntDomain::convert(const double& value) const {
    return((int) value);
  }

  const AbstractDomain::DomainType& IntervalIntDomain::getType() const {
    static const AbstractDomain::DomainType s_type = INT_INTERVAL;
    return(s_type);
  }

  const LabelStr& IntervalIntDomain::getDefaultTypeName() {
    static const LabelStr sl_typeName("INT_INTERVAL");
    return(sl_typeName);
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
    
  void IntervalIntDomain::remove(double value) {
    check_error(check_value(value));
    if (!isMember(value))
      return; // Outside the interval.

    if (compareEqual(value, m_lb)) {
      m_lb += minDelta(); // Could make this 1 at this point!
      if (isEmpty())
        notifyChange(DomainListener::EMPTIED);
      else
        notifyChange(DomainListener::LOWER_BOUND_INCREASED);
      return;
    }

    if (compareEqual(value, m_ub)) {
      m_ub -= minDelta();
      check_error(!isEmpty()); // If it were empty, it should have been covered by prior EMPTIED call.
      notifyChange(DomainListener::UPPER_BOUND_DECREASED);
      return;
    }

    // Logic error above: the conditions should cover all possibilities.
    check_error( ALWAYS_FAILS, "Attempted to remove an element from within the interval. Wuuld require splitting.");
  }

  void IntervalIntDomain::getValues(std::list<double>& results) const {
    check_error(isFinite());
    int lb = (int) check(m_lb);
    int ub = (int) check(m_ub);
    for (int i = lb; i <= ub; i++)
      results.push_back(i);
  }

  double IntervalIntDomain::minDelta() const {
    return(1.0);
  }

  double IntervalIntDomain::translateNumber(double number, bool asMin) const {
    double result = IntervalDomain::translateNumber(int(number), asMin);

    // Incrementing result will round up.
    // Why the condition that number is positive? It breaks symmetry if nothing else. --wedgingt 2004 Mar 4
    if (result != number && asMin && number > 0)
      result = result + 1;
    return(result);
  }

  IntervalIntDomain *IntervalIntDomain::copy() const {
    IntervalIntDomain *ptr = new IntervalIntDomain(*this);
    check_error(ptr != 0);
    return(ptr);
  }
}
