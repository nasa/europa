#include "IntervalDomain.hh"
#include "DomainListener.hh"

namespace Prototype {
  IntervalDomain::IntervalDomain(double lb, double ub, bool closed, const DomainListenerId& listener)
    : AbstractDomain(closed, false, listener), m_ub(ub), m_lb(lb){
    check_error(ub >= lb);
    check_error(ub <= PLUS_INFINITY);
    check_error(lb >= MINUS_INFINITY);
  }

  IntervalDomain::~IntervalDomain(){}

  IntervalDomain::IntervalDomain(const IntervalDomain& org)
    : AbstractDomain(org.m_closed, false, DomainListenerId::noId()), m_ub(org.m_ub), m_lb(org.m_lb){}

  bool IntervalDomain::intersect(const AbstractDomain& dom) {
    check_error(dom.isInterval());
    check_error(dom.isDynamic() || !dom.isEmpty());
    check_error(isDynamic() || !isEmpty());
    return intersect(dom.getLowerBound(), dom.getUpperBound());
  }

  void IntervalDomain::relax(const AbstractDomain& dom){
    check_error(dom.isInterval());
    relax(dom.getLowerBound(), dom.getUpperBound());
  }

  bool IntervalDomain::operator==(const AbstractDomain& dom) const {
    return (AbstractDomain::operator==(dom) &&
	    m_lb == dom.getLowerBound() &&
	    m_ub == dom.getUpperBound());
  }

  bool IntervalDomain::operator!=(const AbstractDomain& dom) const {
    return (! operator==(dom));
  }

  bool IntervalDomain::isSubsetOf(const AbstractDomain& dom) const{
    check_error(dom.isDynamic() || !dom.isEmpty());
    check_error(!isDynamic());
    check_error(dom.isInterval());
    bool result = ((isFinite() || dom.isInfinite()) && 
		   dom.getUpperBound() >= m_ub && dom.getLowerBound() <= m_lb);
    return result;
  }

  bool IntervalDomain::equate(AbstractDomain& dom){
    bool result = intersect(dom);
    if(!isEmpty() && dom.intersect(*this))
      result = true;
    return result;
  }

  double IntervalDomain::getUpperBound() const {return m_ub;}

  double IntervalDomain::getLowerBound() const {return m_lb;}

  bool IntervalDomain::getBounds(double& lb, double& ub) const{
    lb = m_lb;
    ub = m_ub;
    return isInfinite();
  }

  double IntervalDomain::getSingletonValue() const {
    check_error(isSingleton());
    return m_ub;
  }
 
  void IntervalDomain::set(const AbstractDomain& dom){
    intersect(dom);
    notifyChange(DomainListener::SET);
  }
 
  void IntervalDomain::set(double value){
    if(!isMember(value)){
      empty();
      return;
    }

    m_lb = value;
    m_ub = value;
    notifyChange(DomainListener::SET_TO_SINGLETON);
  }

  void IntervalDomain::reset(const AbstractDomain& dom){
    check_error(dom.isInterval());
    if(*this != dom){
      relax(dom);
      notifyChange(DomainListener::RESET);
    }
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
      notifyChange(DomainListener::RESTRICT_TO_SINGLETON);
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
    check_error(isEmpty() || (lb <= m_lb && ub >= m_ub));

    // Test if really causes a change
    bool relaxed = (ub > m_ub) || ( lb < m_lb);

    if(relaxed){
      m_lb = safeConversion(lb);
      m_ub = safeConversion(ub);
      notifyChange(DomainListener::RELAXED);
    }

    return relaxed;
  }

  bool IntervalDomain::isMember(double value) const {
    double converted = convert(value);
    return ((converted == value) && converted >= m_lb && converted <= m_ub);
  }

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
      return (int)(m_ub - m_lb + 1);
  }

  void IntervalDomain::getValues(std::list<double>& results) const{
    check_error(isFinite());
    int lb = (int) check(m_lb);
    int ub = (int) check(m_ub);
    for(int i = lb; i <= ub; i++)
      results.push_back(i);
  }

  double IntervalDomain::check(const double& value) const {
    testPrecision(value);
    return value;
  }

  void IntervalDomain::operator>>(ostream& os) const {
    AbstractDomain::operator>>(os);
    os << "[" << m_lb << ", " << m_ub << "]";
  }
}
