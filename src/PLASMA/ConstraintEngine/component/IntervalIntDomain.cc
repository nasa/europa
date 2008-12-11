#include "IntervalIntDomain.hh"
#include <cmath>

namespace EUROPA {


  IntervalIntDomain::IntervalIntDomain()
    : IntervalDomain(getDefaultTypeName().toString()) {m_minDelta = 1.0;}

  IntervalIntDomain::IntervalIntDomain(const std::string& typeName)
    : IntervalDomain(typeName) {m_minDelta = 1.0;}

  IntervalIntDomain::IntervalIntDomain(eint lb, eint ub)
    : IntervalDomain(lb, ub, getDefaultTypeName().toString()) {
    check_error(check_value(m_lb), "Invalid lower bound");
    check_error(check_value(m_ub), "Invalid upper bound");
    m_minDelta = 1.0;
  }

  IntervalIntDomain::IntervalIntDomain(eint value)
    : IntervalDomain(value, value, getDefaultTypeName().toString()) {
    check_error(check_value(value), "Invalid value");
    m_minDelta = 1.0;
  }

  IntervalIntDomain::IntervalIntDomain(eint lb, eint ub, const std::string& typeName)
    : IntervalDomain(lb, ub, typeName) {
    check_error(check_value(m_lb), "Invalid lower bound");
    check_error(check_value(m_ub), "Invalid upper bound");
    m_minDelta = 1.0;
  }

  IntervalIntDomain::IntervalIntDomain(const AbstractDomain& org)
    : IntervalDomain(org) {
    check_error(check_value(m_lb), "Invalid lower bound");
    check_error(check_value(m_ub), "Invalid upper bound");
    m_minDelta = 1.0;
  }

  bool IntervalIntDomain::isFinite() const {
    check_error(!isOpen());
    return(m_lb > -MAX_INT && m_ub < MAX_INT);
  }

  bool IntervalIntDomain::isSingleton() const {
    return(m_lb == m_ub);
  }

  void IntervalIntDomain::testPrecision(const edouble& value) const {
#ifndef EUROPA_FAST
    eint intValue = (eint) value;
    edouble dblValue = (edouble) intValue;
    checkError(dblValue == value,
	       value << " must be an integer."); // confirms no loss in precision
#endif
  }

  edouble IntervalIntDomain::convert(const edouble& value) const {
    return((eint) value);
  }

  const LabelStr& IntervalIntDomain::getDefaultTypeName() {
    static const LabelStr sl_typeName("INT_INTERVAL");
    return(sl_typeName);
  }

  void IntervalIntDomain::insert(edouble value) {
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

  void IntervalIntDomain::getValues(std::list<edouble>& results) const {
    check_error(isFinite());
    eint lb = (eint) check(m_lb);
    eint ub = (eint) check(m_ub);
    for (eint i = lb; i <= ub; i++)
      results.push_back(i);
  }

//   edouble IntervalIntDomain::minDelta() const {
//     return(1.0);
//   }

  edouble IntervalIntDomain::translateNumber(edouble number, bool asMin) const {
    edouble result = IntervalDomain::translateNumber(eint(number), asMin);

    // Incrementing result will round up.
    // Why the condition that number is positive? It breaks symmetry if nothing else. --wedgingt 2004 Mar 4
    if ((std::abs(result - number) >= EPSILON) && asMin && number > 0)
      result = result + 1;
    return(result);
  }

  IntervalIntDomain *IntervalIntDomain::copy() const {
    IntervalIntDomain *ptr = new IntervalIntDomain(*this);
    check_error(ptr != 0);
    return(ptr);
  }

  bool IntervalIntDomain::intersect(edouble lb, edouble ub) {    
    return IntervalDomain::intersect(std::ceil(lb), std::floor(ub));
  }

  bool IntervalIntDomain::intersect(const AbstractDomain& dom) {
    return intersect(dom.getLowerBound(), dom.getUpperBound());
  }
}
