#include "IntervalDomain.hh"
#include "DomainListener.hh"

namespace EUROPA {

  IntervalDomain::IntervalDomain()
    : AbstractDomain(true, false, getDefaultTypeName().c_str()), 
      m_ub(PLUS_INFINITY), m_lb(MINUS_INFINITY){}

  IntervalDomain::IntervalDomain(const char* typeName)
    : AbstractDomain(true, false, typeName), 
      m_ub(PLUS_INFINITY), m_lb(MINUS_INFINITY){}

  IntervalDomain::IntervalDomain(double lb, double ub, const char* typeName)
    : AbstractDomain(true, false, typeName), m_ub(ub), m_lb(lb) {
    check_error(ub >= lb);
    check_error(ub <= PLUS_INFINITY);
    check_error(lb >= MINUS_INFINITY);
  }

  IntervalDomain::IntervalDomain(double lb, double ub)
    : AbstractDomain(true, false, getDefaultTypeName().c_str()), m_ub(ub), m_lb(lb) {
    checkError(ub >= lb, "Tried to construct a domain with bounds [" << lb << " " << ub << "]");
    checkError(ub <= PLUS_INFINITY, "Upper bound " << ub << " is too large.");
    checkError(lb >= MINUS_INFINITY, "Lower bound " << lb << " is too small.");
  }

  IntervalDomain::IntervalDomain(double value)
    : AbstractDomain(true, false, getDefaultTypeName().c_str()), m_ub(value), m_lb(value) {
    check_error(value <= PLUS_INFINITY);
    check_error(value >= MINUS_INFINITY);
  }

  IntervalDomain::~IntervalDomain() {}

  IntervalDomain::IntervalDomain(const AbstractDomain& org)
    : AbstractDomain(org), m_ub(org.getUpperBound()), m_lb(org.getLowerBound()){
    check_error(org.isInterval(), 
		"Attempted to create an Interval domain from " + org.getTypeName().toString());
  }

  bool IntervalDomain::intersect(const AbstractDomain& dom) {
    safeComparison(*this, dom);
    check_error(dom.isOpen() || !dom.isEmpty());
    check_error(isOpen() || !isEmpty());
    return(intersect(dom.getLowerBound(), dom.getUpperBound()));
  }

  bool IntervalDomain::difference(const AbstractDomain& dom) {
    safeComparison(*this, dom);
    check_error(dom.isOpen() || !dom.isEmpty());
    check_error(isOpen() || !isEmpty());

    // Check for case where difference could cause a split in the interval;
    //   don't handle it in this implementation.
    check_error(! (dom.getLowerBound() > m_lb && dom.getUpperBound() < m_ub));

    // Nothing to be done if the lower bound > dom.upper bound, or the upper bound < dom.lowerbound
    if (m_lb > dom.getUpperBound() || m_ub < dom.getLowerBound())
      return(false);

    // If it is contained by dom then it will be emptied
    if (m_lb >= dom.getLowerBound() && m_ub <= dom.getUpperBound()) {
      empty();
      return(true);
    }

    // If lower bound > dom.lower bound then we must increment it to exceed the upper bound
    if (m_lb >= dom.getLowerBound()) {
      m_lb = dom.getUpperBound() + minDelta();
      notifyChange(DomainListener::LOWER_BOUND_INCREASED);
    }

    // Similarly for the upper bound
    if (m_ub <= dom.getUpperBound()) {
      m_ub = dom.getLowerBound() - minDelta();
      notifyChange(DomainListener::UPPER_BOUND_DECREASED);
    }

    return(true);
  }

  AbstractDomain& IntervalDomain::operator=(const AbstractDomain& dom) {
    safeComparison(*this, dom);
    check_error(m_listener.isNoId());
    m_lb = dom.getUpperBound();
    m_ub = dom.getUpperBound();
    m_closed = dom.isFinite();
    return(*this);
  }

  void IntervalDomain::relax(const AbstractDomain& dom) {
    safeComparison(*this, dom);
    relax(dom.getLowerBound(), dom.getUpperBound());
  }

  void IntervalDomain::insert(double value) {
    check_error(ALWAYS_FAILS, "Cannot insert to an interval domain");
  }

  void IntervalDomain::insert(const std::list<double>& values){
    check_error(ALWAYS_FAILS, "Cannot insert to an interval domain");
  }

  void IntervalDomain::remove(double value) {
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

  bool IntervalDomain::operator==(const AbstractDomain& dom) const {
    safeComparison(*this, dom);
    return(compareEqual(m_lb, dom.getLowerBound()) &&
           compareEqual(m_ub, dom.getUpperBound()) &&
           AbstractDomain::operator==(dom));
  }

  bool IntervalDomain::operator!=(const AbstractDomain& dom) const {
    return(! operator==(dom));
  }

  bool IntervalDomain::isSubsetOf(const AbstractDomain& dom) const {
    safeComparison(*this, dom);
    check_error(!isOpen());
    check_error(!dom.isEmpty());
    bool result = ((isFinite() || dom.isInfinite()) && 
                   (dom.getUpperBound() + minDelta()) >= m_ub && 
                   (dom.getLowerBound() - minDelta()) <= m_lb);
    return result;
  }

  bool IntervalDomain::intersects(const AbstractDomain& dom) const {
    safeComparison(*this, dom);
    check_error(!isOpen());
    check_error(!dom.isEmpty());

    double ub = dom.getUpperBound();
    if(ub < m_lb)
      return false;

    double lb = dom.getLowerBound();
    if(lb > m_ub)
      return false;

    return true;
  }

  bool IntervalDomain::equate(AbstractDomain& dom) {
    safeComparison(*this, dom);

    // Avoid duplicating part of EnumeratedDomain::equate().  Needed for
    // full propagation in cases like equate([0.0 10.0] {5.0 20.0}),
    // which would otherwise only trim to [5.0 10.0] {5.0} unless two
    // passes are made.  --wedgingt@ptolemy.arc.nasa.gov 2004 Apr 22
    if (dom.isEnumerated())
      return(dom.equate(*this));

    bool result = intersect(dom);
    if (!isEmpty() && dom.intersect(*this))
      result = true;
    return(result);
  }

  double IntervalDomain::getUpperBound() const {
    return(m_ub);
  }

  double IntervalDomain::getLowerBound() const {
    return(m_lb);
  }

  double IntervalDomain::getSingletonValue() const {
    check_error(isSingleton());
    return(m_ub);
  }
 
  void IntervalDomain::set(const AbstractDomain& dom) {
    // David Rijsman filed GNAT 2963
    //checkError(dom.isSubsetOf(*this), 
    //"Attempt to set to a new domain " << dom.toString() << 
    //" that is not a subset of the current domain " << toString());

    checkError(!dom.isSingleton(), "You must use set(double value) to set a domain to a singleton.");
    
    safeComparison(*this, dom);
    
    intersect(dom);
    notifyChange(DomainListener::SET);
  }
 
  void IntervalDomain::set(double value) {
    checkError(isMember(value), "Attempt to set to " << value << " which is not a member of the " << toString());
    
    m_lb = value;
    m_ub = value;
 
    notifyChange(DomainListener::SET_TO_SINGLETON);
 
  }

  bool IntervalDomain::convertToMemberValue(const std::string& strValue, double& dblValue) const {
    double value = atof(strValue.c_str());
    if(isMember(value)){
      dblValue = value;
      return true;
    }

    return false;
  }

  void IntervalDomain::reset(const AbstractDomain& dom) {
    safeComparison(*this, dom);
    if (*this != dom) {
      relax(dom);
      notifyChange(DomainListener::RESET);
    }
  }

  bool IntervalDomain::intersect(double lb, double ub) {
    // Test for empty intersection while accounting for precision/rounding errors.
    if ((lb > ub && (lb-ub > EPSILON)) || m_lb - ub >= minDelta() || lb - m_ub >= minDelta()) {
      empty();
      return(true);
    }

    bool ub_decreased(false);
    if (ub < m_ub) {
      m_ub = safeConversion(ub);
      ub_decreased = true;
    }

    bool lb_increased(false);
    if (lb > m_lb) {
      m_lb = safeConversion(lb);
      lb_increased = true;
    }

    // Select the strongest message applicable.
    if (isSingleton() && (lb_increased || ub_decreased))
      notifyChange(DomainListener::RESTRICT_TO_SINGLETON);
    else
      if (lb_increased && ub_decreased) {
        if (isEmpty())
          notifyChange(DomainListener::EMPTIED);
        else
          notifyChange(DomainListener::BOUNDS_RESTRICTED);
      } else
        if (lb_increased)
          notifyChange(DomainListener::LOWER_BOUND_INCREASED);
        else
          if (ub_decreased)
            notifyChange(DomainListener::UPPER_BOUND_DECREASED);
    return(lb_increased || ub_decreased);
  }

  bool IntervalDomain::relax(double lb, double ub) {
    // Ensure given bounds are not empty
    check_error(lb <= ub);

    // Ensure this domain is a subset of the new bounds for relaxation.
    check_error(isEmpty() || (lb - minDelta() <= m_lb  && ub + minDelta() >= m_ub));

    // Test if really causes a change.
    bool relaxed = (ub > m_ub) || (lb < m_lb);

    if (relaxed) {
      m_lb = safeConversion(lb);
      m_ub = safeConversion(ub);
      notifyChange(DomainListener::RELAXED);
    }

    return(relaxed);
  }

  bool IntervalDomain::isMember(double value) const {
    double converted = convert(value);
    return(converted == value && 
           converted + minDelta() > m_lb && 
           converted - minDelta() < m_ub);
  }

  bool IntervalDomain::isSingleton() const {
    check_error(!isOpen());
    return(compareEqual(m_lb, m_ub));
  }

  bool IntervalDomain::isEmpty() const {
    check_error(!isOpen());
    return(m_ub - m_lb < -EPSILON);
  }

  void IntervalDomain::empty() {
    m_ub = -2;
    m_lb = 2 + minDelta();
    notifyChange(DomainListener::EMPTIED);
  }

  int IntervalDomain::getSize() const {
    checkError(!isOpen(), "Cannot test for the size of an open domain.");

    if (isEmpty())
      return(0);
    else if (isSingleton()) // Need to test separately in case of rounding errors
        return(1);
    else if(isFinite())
      return((int)(m_ub - m_lb + 1));
    else
      return PLUS_INFINITY;
  }

  void IntervalDomain::getValues(std::list<double>& results) const {
    check_error(isSingleton());
    if (!isEmpty()) {
      results.push_back(m_lb); // consider averaging m_lb and m_ub
    }
  }

  double IntervalDomain::check(const double& value) const {
    testPrecision(value);
    return(value);
  }

  void IntervalDomain::operator>>(ostream& os) const {
    AbstractDomain::operator>>(os);
    os << "[";

    if (m_lb == MINUS_INFINITY)
      os << "-inf";
    else
      if (m_lb == PLUS_INFINITY)
        os << "+inf";
      else
        os << m_lb;

    os << ", ";
    if (m_ub == MINUS_INFINITY)
      os << "-inf";
    else
      if (m_ub == PLUS_INFINITY)
        os << "+inf";
      else
        os << m_ub;

    os << "]";
  }

  /**
   * @brief Convert the value appropriately for the particular Domain class.
   * @note A no-op for reals.
   */
  double IntervalDomain::convert(const double& value) const {return value;}

  bool IntervalDomain::isFinite() const {
    check_error(!isOpen());
    // Real domains are only finite if they are singleton or empty.
    return(isSingleton() || isEmpty());
  }

  const LabelStr& IntervalDomain::getDefaultTypeName() {
    static const LabelStr sl_typeName("REAL_INTERVAL");
    return(sl_typeName);
  }

  double IntervalDomain::translateNumber(double number, bool) const {
    if (number < 0.0)
      return(number < MINUS_INFINITY ? MINUS_INFINITY : number);
    if (number > 0.0)
      return(number > PLUS_INFINITY ? PLUS_INFINITY : number);
    // This has to be explicitly 0.0 for hardware/OS/compiler
    // combinations that have a '-0.0' (negative zero) that
    // equals 0.0 but is also negative.
    return(0.0);
  }

  IntervalDomain *IntervalDomain::copy() const {
    IntervalDomain *ptr = new IntervalDomain(*this);
    check_error(ptr != 0);
    return(ptr);
  }
}
