#include "IntervalDomain.hh"
#include "DomainListener.hh"

namespace Prototype {
  IntervalDomain::IntervalDomain(double lb, double ub, bool finite, bool closed, const DomainListenerId& listener)
    : AbstractDomain(finite, closed, listener), m_ub(ub), m_lb(lb){
    check_error(ub >= lb)
  }

  bool IntervalDomain::intersect(const IntervalDomain& dom) {
    check_error(dom.isDynamic() || !dom.isEmpty());
    check_error(isDynamic() || !isEmpty());
    return intersect(dom.m_lb, dom.m_ub);
  }

  IntervalDomain::~IntervalDomain(){}

  IntervalDomain::IntervalDomain(const IntervalDomain& org)
    : AbstractDomain(org.m_finite, org.m_closed, org.m_listener), m_ub(org.m_ub), m_lb(org.m_lb){}


  IntervalDomain::IntervalDomain(Europa::Domain& org)
    : AbstractDomain(org){
    check_error(org.isInterval());
    m_ub = org.getUpperBound().getRealValue();
    m_lb = org.getLowerBound().getRealValue();
  }

  IntervalDomain& IntervalDomain::operator=(const IntervalDomain& org){
    AbstractDomain::operator=(org);
    relax(org.m_lb, org.m_ub);
    return (*this);
  }

  bool IntervalDomain::operator==(const IntervalDomain& dom) const {
    check_error(dom.isDynamic() || !dom.isEmpty());
    check_error(isDynamic() || !isEmpty());

    return (AbstractDomain::operator==(dom) &&
	    m_lb == dom.getLowerBound() &&
	    m_ub == dom.getUpperBound());
  }

  bool IntervalDomain::isSubsetOf(const IntervalDomain& dom) const{
    check_error(dom.isDynamic() || !dom.isEmpty());
    check_error(!isDynamic());

    return (dom.getUpperBound() >= m_ub && dom.getLowerBound() <= m_lb);
  }

  double IntervalDomain::getUpperBound() const {return m_ub;}

  double IntervalDomain::getLowerBound() const {return m_lb;}

  bool IntervalDomain::getBounds(double& lb, double& ub){
    lb = m_lb;
    ub = m_ub;
    return isInfinite();
  }

  double IntervalDomain::getSingletonValue() const {
    check_error(isSingleton());
    return m_ub;
  }
 
  void IntervalDomain::setToSingleton(double value){
    if(!isMember(value)){
      empty();
      return;
    }

    m_lb = value;
    m_ub = value;
    notifyChange(DomainListener::SET_TO_SINGLETON);
  }

  bool IntervalDomain::intersect(double lb, double ub){
    check_error(lb <= ub)

    // test case for empty intersection
    if(ub < m_lb || lb > m_ub){
      empty();
      return true;
    }

    bool ub_decreased(false);

    if(ub < m_ub){
      m_ub = safeConversion(ub);
      ub_decreased = true;
    }

    bool lb_increased(false);
    if(lb > m_lb){
      m_lb = safeConversion(lb);
      lb_increased = true;
    }

    // Select the strongest message available
    if(m_ub == m_lb && (lb_increased || ub_decreased))
      notifyChange(DomainListener::SET_TO_SINGLETON);
    else if(lb_increased && ub_decreased)
      notifyChange(DomainListener::BOUNDS_RESTRICTED);
    else if(lb_increased)
      notifyChange(DomainListener::LOWER_BOUND_INCREASED);
    else if(ub_decreased)
      notifyChange(DomainListener::LOWER_BOUND_INCREASED);

    return (lb_increased || ub_decreased);
  }

  bool IntervalDomain::relax(double lb, double ub){
    // Ensure given bounds are not empty
    check_error(lb <= ub);

    // Ensure this domain is a subset of the new bounds for relaxation
    check_error(lb <= m_lb && ub >= m_ub);

    // Test of really causes a change
    bool relaxed = (ub > m_ub) || ( lb < m_lb);

    if(relaxed){
      m_lb = safeConversion(lb);
      m_ub = safeConversion(ub);
      notifyChange(DomainListener::RELAXED);
    }

    return relaxed;
  }

  bool IntervalDomain::isMember(double value) const {
    checkPrecision(value);
    return (value > m_lb && value <= m_ub);
  }

  bool IntervalDomain::isEnumerated() const { return false;}

  bool IntervalDomain::isSingleton() const {
    check_error(!isDynamic());
    return m_ub == m_lb;
  }

  bool IntervalDomain::isEmpty() const {
    check_error(!isDynamic());
    return (m_ub < m_lb);
  }

  void IntervalDomain::empty() {
    m_ub = 1;
    m_lb = 2;
    notifyChange(DomainListener::EMPTIED);
  }

  int IntervalDomain::getSize() const {
    check_error(!isDynamic() && isFinite());

    if(isEmpty())
      return 0;
    else
      return (int)(m_ub - m_lb);
  }

  double IntervalDomain::check(const double& value) const {
    testPrecision(value);
    return value;
  }
}
