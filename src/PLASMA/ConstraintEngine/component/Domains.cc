/*
 * Domains.cc
 *
 *  Created on: Apr 23, 2009
 *      Author: jbarreir
 */

#include "Domains.hh"

#include "LabelStr.hh"
#include "Entity.hh"
#include "DomainListener.hh"
#include <math.h>
#include <cmath>


namespace EUROPA {


  bool isAscending(const std::set<double>& values) {
    double greatest = *(values.begin());
    for (std::set<double>::const_iterator it = values.begin(); it != values.end(); ++it) {
    	double current = *it;
    	if (current < greatest)
    		return(false);
    	else
    		greatest = current;
    }
    return(true);
  }

  EnumeratedDomain::EnumeratedDomain(const DataTypeId& dt)
  : AbstractDomain(dt,true,false)
  {
  }

  EnumeratedDomain::EnumeratedDomain(const DataTypeId& dt, const std::list<double>& values)
  : AbstractDomain(dt,true,false)
  {
	  for (std::list<double>::const_iterator it = values.begin(); it != values.end(); ++it)
		  insert(*it);

	  close();
  }

  EnumeratedDomain::EnumeratedDomain(const DataTypeId& dt, double value)
  : AbstractDomain(dt,true,false)
  {
	  insert(value);
	  close();
  }

  EnumeratedDomain::EnumeratedDomain(const AbstractDomain& org)
  : AbstractDomain(org)
  {
	  check_error(org.isEnumerated(),
			  "Invalid source domain " + org.getTypeName().toString() + " for enumeration");
	  const EnumeratedDomain& enumOrg = static_cast<const EnumeratedDomain&>(org);
	  m_values = enumOrg.m_values;
  }

  bool EnumeratedDomain::isFinite() const {
	  return(true); // Always finite, even if bounds are infinite, since there are always a finite number of values to select.
  }

  bool EnumeratedDomain::isSingleton() const {
	  return(m_values.size() == 1);
  }

  bool EnumeratedDomain::isEmpty() const {
	  return(m_values.empty());
  }

  void EnumeratedDomain::empty() {
	  m_values.clear();
	  notifyChange(DomainListener::EMPTIED);
  }

  void EnumeratedDomain::close() {
	  AbstractDomain::close();
	  check_error(isEmpty() || isAscending(m_values));
  }

  unsigned int EnumeratedDomain::getSize() const {
	  return(m_values.size());
  }

  void EnumeratedDomain::insert(double value) {
	  check_error(check_value(value));
	  checkError(isOpen(), "Cannot insert into a closed domain." << toString());
	  std::set<double>::iterator it = m_values.begin();
	  for ( ; it != m_values.end(); it++) {
		  if (compareEqual(value, *it))
			  return; // Already a member.
		  if (value < *it) // Since members are sorted, value goes before *it.
			  break;
	  }
	  m_values.insert(it, value);

	  // CMG: Do not generate a relaxation for insertion into an open domain. The semantics of an open domain indicate that
	  // the set of values is unbound, and we are now simply adding in another explicit member.
	  // notifyChange(DomainListener::RELAXED);
  }

  void EnumeratedDomain::insert(const std::list<double>& values){
	  for(std::list<double>::const_iterator it = values.begin(); it != values.end(); ++it)
		  insert(*it);
  }

  void EnumeratedDomain::remove(double value) {
	  check_error(check_value(value));
	  std::set<double>::iterator it = m_values.begin();
	  for ( ; it != m_values.end(); it++)
		  if (compareEqual(value, *it))
			  break;
	  if (it == m_values.end())
		  return; // not present: no-op
	  m_values.erase(it);
	  if (!isEmpty() || isOpen())
		  notifyChange(DomainListener::VALUE_REMOVED);
	  else
		  notifyChange(DomainListener::EMPTIED);
  }

  void EnumeratedDomain::set(double value) {
	  if(isOpen())
		  close();

	  if(isMember(value)){
		  m_values.clear();
		  m_values.insert(value);
		  // Generate the notification, even if already a singleton. This is because setting a value to a singleton
		  // is different from restricting it.
		  notifyChange(DomainListener::SET_TO_SINGLETON);
	  }
	  else
		  empty();
  }

  void EnumeratedDomain::reset(const AbstractDomain& dom) {
	  if (*this != dom) {
		  relax(dom);
		  notifyChange(DomainListener::RESET);
	  }
  }

  bool EnumeratedDomain::equate(AbstractDomain& dom) {
	  safeComparison(*this, dom);

	  // If both domains are closed enumerations we can use optimized method
	  if(!dom.isInterval() && dom.isClosed() && isClosed())
		  return equateClosedEnumerations(static_cast<EnumeratedDomain&>(dom));

	  bool changed = dom.intersect(*this);

	  if(changed && dom.isEmpty())
		  return true;

	  // Have to intersect again for the case of mixed types (enumeration and interval)
	  if(intersect(dom) && !isEmpty())
		  changed = dom.intersect(*this) || changed;

	  return changed;
  }

  bool EnumeratedDomain::equateClosedEnumerations(EnumeratedDomain& dom){
	  bool changed_a = false;
	  bool changed_b = false;
	  EnumeratedDomain& l_dom = static_cast<EnumeratedDomain&>(dom);

	  std::set<double>::iterator it_a = m_values.begin();
	  std::set<double>::iterator it_b = l_dom.m_values.begin();

	  while (it_a != m_values.end() && it_b != l_dom.m_values.end()) {
		  double val_a = *it_a;
		  double val_b = *it_b;

		  if (compareEqual(val_a, val_b)) {
			  ++it_a;
			  ++it_b;
		  } else
			  if (val_a < val_b) {
				  std::set<double>::iterator target = m_values.lower_bound(val_b);
				  m_values.erase(it_a, target);
				  it_a = target;
				  changed_a = true;
				  check_error(!isMember(val_a));
			  } else {
				  std::set<double>::iterator target = l_dom.m_values.lower_bound(val_a);
				  l_dom.m_values.erase(it_b, target);
				  it_b = target;
				  changed_b = true;
				  check_error(!l_dom.isMember(val_b));
			  }
	  }

	  if (it_a != m_values.end() && !l_dom.isEmpty()) {
		  m_values.erase(it_a, m_values.end());
		  changed_a = true;
		  check_error(it_b == l_dom.m_values.end());
	  } else
		  if (it_b != l_dom.m_values.end() && !isEmpty()) {
			  l_dom.m_values.erase(it_b, l_dom.m_values.end());
			  changed_b = true;
			  check_error(it_a == m_values.end());
		  }

	  if (changed_a) {
		  if (isEmpty())
			  notifyChange(DomainListener::EMPTIED);
		  else
			  if (isSingleton())
				  notifyChange(DomainListener::RESTRICT_TO_SINGLETON);
			  else
				  notifyChange(DomainListener::VALUE_REMOVED);
	  }

	  if (changed_b) {
		  if (l_dom.isEmpty())
			  l_dom.notifyChange(DomainListener::EMPTIED);
		  else
			  if (isSingleton())
				  l_dom.notifyChange(DomainListener::RESTRICT_TO_SINGLETON);
			  else
				  l_dom.notifyChange(DomainListener::VALUE_REMOVED);
	  }

	  check_error(!isEmpty() || ! dom.isEmpty());
	  check_error(isEmpty() || dom.isEmpty() || (l_dom.m_values == m_values));
	  return(changed_a || changed_b);
  }

  bool EnumeratedDomain::isMember(double value) const {
	  if (m_values.empty())
		  return false;
	  std::set<double>::const_iterator it = m_values.lower_bound(value);
	  // If we get a hit - the entry >= value
	  if (it != m_values.end()) {
		  double elem = *it;
		  // Try fast compare first, then epsilon safe version
		  if (value == elem || compareEqual(value, elem))
			  return true;
		  --it;
		  // Before giving up, see if prior position is within epsilon
		  return it != m_values.end() && compareEqual(value, *it);
	  }
	  return false;
  }


  bool EnumeratedDomain::convertToMemberValue(const std::string& strValue, double& dblValue) const {
	  double value = dblValue;

	  if(isNumeric())
		  value = atof(strValue.c_str());
	  else
		  value = LabelStr(strValue);
	  if(isMember(value)){
		  dblValue = value;
		  return true;
	  }

	  return false;
  }

  bool EnumeratedDomain::operator==(const AbstractDomain& dom) const {
	  safeComparison(*this, dom);
	  if (!dom.isEnumerated())
		  return(dom.isFinite() &&
				  getSize() == dom.getSize() &&
				  isSubsetOf(dom));
	  const EnumeratedDomain& l_dom = static_cast<const EnumeratedDomain&>(dom);
	  if (!AbstractDomain::operator==(dom))
		  return(false);
	  // If any member of either is not a member of the other, they're not equal.
	  // Since membership is not simple (due to minDelta()), this has to be done
	  // via a scan of both memberships, one member at a time.
	  std::set<double>::iterator it = m_values.begin();
	  for ( ; it != m_values.end(); it++)
		  if (!l_dom.isMember(*it))
			  return(false);
	  for (it = l_dom.m_values.begin(); it != l_dom.m_values.end(); it++)
		  if (!isMember(*it))
			  return(false);
	  return(true);
  }

  bool EnumeratedDomain::operator!=(const AbstractDomain& dom) const {
	  return(!operator==(dom));
  }

  void EnumeratedDomain::relax(const AbstractDomain& dom) {
	  check_error(dom.isEnumerated());

	  if(dom.isEmpty() && dom.isClosed())
		  return;

	  if (isEmpty() || this->isSubsetOf(dom)){
		  const EnumeratedDomain& l_dom = static_cast<const EnumeratedDomain&>(dom);
		  m_values = l_dom.m_values;
		  // Open up if we are closed and need be be relaxed to an open domain
		  if(dom.isOpen() && isClosed())
			  open();

		  notifyChange(DomainListener::RELAXED);
	  }
  }

  void EnumeratedDomain::relax(double value) {
	  checkError(isEmpty() || (isSingleton() && (getSingletonValue() == value)), toString());

	  if (isEmpty()){
		  m_values.insert(value);
		  notifyChange(DomainListener::RELAXED);
	  }
  }

  double EnumeratedDomain::getSingletonValue() const {
	  checkError(isSingleton(), toString());
	  return(*m_values.begin());
  }

  void EnumeratedDomain::getValues(std::list<double>& results) const {
	  check_error(results.empty());
	  check_error(isFinite());

	  for (std::set<double>::iterator it = m_values.begin(); it != m_values.end(); ++it)
		  results.push_back(*it);
  }

  const std::set<double>& EnumeratedDomain::getValues() const{
	  return m_values;
  }

  double EnumeratedDomain::getUpperBound() const {
	  double lb, ub;
	  getBounds(lb, ub);
	  return(ub);
  }

  double EnumeratedDomain::getLowerBound() const {
	  double lb, ub;
	  getBounds(lb, ub);
	  return(lb);
  }

  bool EnumeratedDomain::getBounds(double& lb, double& ub) const {
	  check_error(!isEmpty());
	  lb = *m_values.begin();
	  ub = *(--m_values.end());
	  check_error(lb <= ub);
	  return(!isNumeric() || lb == MINUS_INFINITY || ub == PLUS_INFINITY);
  }

  bool EnumeratedDomain::intersect(const AbstractDomain& dom) {
	  safeComparison(*this, dom);

	  // If this domain is open, and the new domain is closed, then assign all
	  // values in the new domain to this domain.
	  if(isOpen() && dom.isClosed()){
		  checkError(!dom.isInterval(), "Cannot intersect a closed interval and and open enumeration.");
		  const EnumeratedDomain& l_dom = static_cast<const EnumeratedDomain&>(dom);
		  m_values = l_dom.m_values;

		  // Only close when values are added as it will otherwise generate an empty domain event
		  close();
		  return true;
	  }

	  // If the given domain is open, then there is no meaningful intersection to apply
	  if(dom.isOpen())
		  return false;

	  bool changed = false;

	  if (dom.isInterval()) {
		  std::set<double>::iterator it = m_values.begin();
		  while (it != m_values.end()) {
			  double value = *it;
			  if (!dom.isMember(value)) {
				  changed = true;
				  if (value > dom.getUpperBound()) {
					  m_values.erase(it, m_values.end());
					  break;
				  } else
					  m_values.erase(it++);
			  } else {
				  ++it;
			  }
		  }
	  } else if (dom.isOpen())
		  return false;
	  else {
		  const EnumeratedDomain& l_dom = static_cast<const EnumeratedDomain&>(dom);
		  std::set<double>::iterator it_a = m_values.begin();
		  std::set<double>::const_iterator it_b = l_dom.m_values.begin();

		  while (it_a != m_values.end() && it_b != l_dom.m_values.end()) {
			  double val_a = *it_a;
			  double val_b = *it_b;

			  if (compareEqual(val_a, val_b)) { // If they are equal, advance both
				  ++it_a;
				  ++it_b;
			  } else
				  if (val_a < val_b) { // A < B, so remove A and advance
					  m_values.erase(it_a++);
					  changed = true;
					  check_error(!isMember(val_a));
				  } else
					  ++it_b; // So just advance B
		  }

		  if (it_a != m_values.end()) {
			  m_values.erase(it_a, m_values.end());
			  changed = true;
		  }
	  }

	  if (!changed)
		  return(false);

	  if (isEmpty())
		  notifyChange(DomainListener::EMPTIED);
	  else
		  if (isSingleton())
			  notifyChange(DomainListener::RESTRICT_TO_SINGLETON);
		  else
			  notifyChange(DomainListener::VALUE_REMOVED);

	  return(true);
  }

  bool EnumeratedDomain::intersect(double lb, double ub){
	  checkError(!isSymbolic(), "Cannot do bounds based intersection on symbolic domain " << toString());
	  if(lb > ub){
		  empty();
		  return true;
	  }

	  // Allocate as an interval and delegate to existing method
	  IntervalDomain intervalDomain(lb, ub, getDataType());

	  return intersect(intervalDomain);
  }

  bool EnumeratedDomain::difference(const AbstractDomain& dom) {
	  safeComparison(*this, dom);

	  // Trivial implementation, for all members of this domain that
	  // are present in dom, remove them.
	  bool value_removed = false;

	  for (std::set<double>::iterator it = m_values.begin(); it != m_values.end();) {
		  double value = *it;
		  if (dom.isMember(value)) {
			  m_values.erase(it++);
			  value_removed = true;
		  } else
			  ++it;
	  }

	  if (m_values.empty())
		  notifyChange(DomainListener::EMPTIED);
	  else
		  if (value_removed)
			  notifyChange(DomainListener::VALUE_REMOVED);

	  return(value_removed);
  }

  AbstractDomain& EnumeratedDomain::operator=(const AbstractDomain& dom) {
	  safeComparison(*this, dom);
	  check_error(m_listener.isNoId(), "Can only do direct assigment if not registered with a listener");
	  const EnumeratedDomain& e_dom = static_cast<const EnumeratedDomain&>(dom);
	  m_values = e_dom.m_values;
	  return(*this);
  }

  bool EnumeratedDomain::isSubsetOf(const AbstractDomain& dom) const {
	  safeComparison(*this, dom);

	  // Always true if the given domain is open. Also never true if the given domain is closed
	  // but this domain is open
	  if(dom.isOpen())
		  return true;
	  else if(isOpen())
		  return false;

	  for (std::set<double>::const_iterator it = m_values.begin(); it != m_values.end(); ++it)
		  if (!dom.isMember(*it))
			  return(false);

	  return(true);
  }

  bool EnumeratedDomain::intersects(const AbstractDomain& dom) const {
	  if(dom.isOpen() || this->isOpen())
		  return true;

	  safeComparison(*this, dom);
	  for (std::set<double>::const_iterator it = m_values.begin(); it != m_values.end(); ++it)
		  if (dom.isMember(*it))
			  return(true);
	  return(false);
  }

  void EnumeratedDomain::operator>>(ostream&os) const {
	  // Now commence output
	  AbstractDomain::operator>>(os);
	  os << "{";

	  // First construct a lexicographic ordering for the set of values.
	  std::set<std::string> orderedSet;

	  std::string comma = "";
	  for (std::set<double>::const_iterator it = m_values.begin(); it != m_values.end(); ++it) {
		  double valueAsDouble = *it;
		  std::string valueAsStr = getDataType()->toString(valueAsDouble);

		  if (isNumeric()) {
			  os << comma << valueAsStr;
			  comma = ", ";
		  }
		  else
			  orderedSet.insert(valueAsStr);
	  }

	  for (std::set<std::string>::const_iterator it = orderedSet.begin(); it != orderedSet.end(); ++it) {
		  check_error(!isNumeric());
		  os << comma
		  << *it;
		  comma = ",";
	  }

	  os << "}";
  }

  std::string EnumeratedDomain::toString() const
  {
	  return AbstractDomain::toString();
  }

  EnumeratedDomain *EnumeratedDomain::copy() const {
	  EnumeratedDomain *ptr = new EnumeratedDomain(*this);
	  check_error(ptr != 0);
	  return(ptr);
  }

  IntervalDomain::IntervalDomain(const DataTypeId& dt)
    : AbstractDomain(dt,false,true)
    , m_ub(PLUS_INFINITY)
    , m_lb(MINUS_INFINITY)
  {
    commonInit();
  }

  IntervalDomain::IntervalDomain(double lb, double ub, const DataTypeId& dt)
    : AbstractDomain(dt,false,true)
    , m_ub(ub)
    , m_lb(lb)
  {
    commonInit();
  }

  IntervalDomain::IntervalDomain(double value, const DataTypeId& dt)
    : AbstractDomain(dt,false,true)
    , m_ub(value)
    , m_lb(value)
  {
    commonInit();
  }

  IntervalDomain::~IntervalDomain()
  {
  }

  IntervalDomain::IntervalDomain(const AbstractDomain& org)
    : AbstractDomain(org)
    , m_ub(org.getUpperBound())
    , m_lb(org.getLowerBound())
  {
    check_error(org.isInterval(),"Attempted to create an Interval domain from " + org.getTypeName().toString());
    commonInit();
  }

  bool IntervalDomain::intersect(const AbstractDomain& dom) {
    safeComparison(*this, dom);
    check_error(dom.isOpen() || !dom.isEmpty(), dom.toString());
    checkError(isOpen() || !isEmpty(), toString());
    double lb, ub;
    dom.getBounds(lb, ub);
    return(intersect(lb, ub));
  }

  bool IntervalDomain::difference(const AbstractDomain& dom) {
    safeComparison(*this, dom);
    check_error(dom.isOpen() || !dom.isEmpty());
    check_error(isOpen() || !isEmpty());

    // Check for case where difference could cause a split in the interval;
    //   don't handle it in this implementation.
    check_error(! (lt(m_lb, dom.getLowerBound()) && lt(dom.getUpperBound(), m_ub)));

    // Nothing to be done if the lower bound > dom.upper bound, or the upper bound < dom.lowerbound
    if (lt(dom.getUpperBound(), m_lb) || lt(m_ub, dom.getLowerBound()))
      return(false);

    // If it is contained by dom then it will be emptied
    if (leq(dom.getLowerBound(), m_lb) && leq(m_ub, dom.getUpperBound())) {
      empty();
      return(true);
    }

    // If lower bound > dom.lower bound then we must increment it to exceed the upper bound
    if (leq(dom.getLowerBound(), m_lb)) {
      m_lb = dom.getUpperBound() + minDelta();
      notifyChange(DomainListener::LOWER_BOUND_INCREASED);
    }

    // Similarly for the upper bound
    if (leq(m_ub, dom.getUpperBound())) {
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
    check_error( ALWAYS_FAILS, "Attempted to remove an element from within the interval. W0uld require splitting.");
  }

  bool IntervalDomain::operator==(const AbstractDomain& dom) const {
    safeComparison(*this, dom);

    return(eq(m_lb, dom.getLowerBound()) &&
           eq(m_ub, dom.getUpperBound()) &&
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
                   leq(m_ub, dom.getUpperBound()) &&
                   leq(dom.getLowerBound(), m_lb));
    return result;
  }

  bool IntervalDomain::intersects(const AbstractDomain& dom) const {
    safeComparison(*this, dom);
    check_error(!isOpen());
    check_error(!dom.isEmpty());

    double ub = dom.getUpperBound();

    if( lt(ub, m_lb) )
      return false;

    double lb = dom.getLowerBound();

    if( lt(m_ub, lb))
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

  void IntervalDomain::set(double value) {
    if(!isMember(value))
      empty();
    else {
      bool changed = (m_lb != value || m_ub != value);
      if(changed == true){
	m_lb = value;
	m_ub = value;

	notifyChange(DomainListener::SET_TO_SINGLETON);
      }
    }
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
    if (lt(ub, lb) || lt(ub, m_lb) || lt(m_ub, lb)){
      empty();
      return(true);
    }

    bool ub_decreased(false);

    if (lt(ub,m_ub)) {
      m_ub = safeConversion(ub);
      ub_decreased = true;
    }

    bool lb_increased(false);
    if (lt(m_lb, lb)){
      m_lb = safeConversion(lb);
      lb_increased = true;
    }

    // Select the strongest message applicable.
    if ((lb_increased || ub_decreased) && isSingleton())
      notifyChange(DomainListener::RESTRICT_TO_SINGLETON);
    else if (lb_increased && ub_decreased) {
      if (isEmpty())
	notifyChange(DomainListener::EMPTIED);
      else
	notifyChange(DomainListener::BOUNDS_RESTRICTED);
      }
    else
      if (lb_increased)
	notifyChange(DomainListener::LOWER_BOUND_INCREASED);
      else
	if (ub_decreased)
	  notifyChange(DomainListener::UPPER_BOUND_DECREASED);

    return(lb_increased || ub_decreased);

  }

  void IntervalDomain::relax(double value) {
    bool wasEmpty = isEmpty();
    m_ub = value;
    m_lb = value;
    if (wasEmpty) {
      notifyChange(DomainListener::RELAXED);
    }
  }

  bool IntervalDomain::relax(double lb, double ub) {
    // Ensure given bounds are not empty
    check_error(leq(lb, ub));

    // Ensure this domain is a subset of the new bounds for relaxation.
    //checkError(isEmpty() || (leq(lb, m_lb) && leq(m_ub, ub)), lb << " >=" << m_lb << " OR " << m_ub << " >= " << ub);

    // Test if really causes a change.
    bool relaxed = (lt(m_ub, ub) || lt(lb, m_lb));

    if (relaxed) {
      m_lb = safeConversion(lb);
      m_ub = safeConversion(ub);
      notifyChange(DomainListener::RELAXED);
    }

    return(relaxed);
  }

  bool IntervalDomain::isMember(double value) const {
    double converted = convert(value);
    return(converted == value && leq(m_lb, converted) && leq(converted, m_ub));
  }

  bool IntervalDomain::isSingleton() const {
    check_error(!isOpen());
    return(compareEqual(m_lb, m_ub));
  }

  bool IntervalDomain::isEmpty() const {
    check_error(!isOpen());
    return(lt(m_ub, m_lb));
  }

  void IntervalDomain::empty() {
    m_ub = -2;
    m_lb = 2 + minDelta();
    notifyChange(DomainListener::EMPTIED);
  }

  unsigned int IntervalDomain::getSize() const {
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


  void IntervalDomain::testPrecision(const double& value) const {}

  void IntervalDomain::operator>>(ostream& os) const {
    AbstractDomain::operator>>(os);
    os << "[";

    std::streamsize prec = os.precision();
    os.precision(15);
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
    os.precision(prec);
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
    return((isSingleton() && areBoundsFinite()) || isEmpty());
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

  void IntervalDomain::commonInit(){
    checkError(leq(m_ub,PLUS_INFINITY), "Upper bound " << m_ub << " is too large.");
    checkError(leq(MINUS_INFINITY, m_lb), "Lower bound " << m_lb << " is too small.");

    if(isEmpty())
      notifyChange(DomainListener::EMPTIED);
  }

  StringDomain::StringDomain(const DataTypeId& dt) : EnumeratedDomain(dt) {}
  StringDomain::StringDomain(double value, const DataTypeId& dt) : EnumeratedDomain(dt,value) {}
  StringDomain::StringDomain(const std::string& value, const DataTypeId& dt) : EnumeratedDomain(dt,LabelStr(value)) {}
  StringDomain::StringDomain(const std::list<double>& values, const DataTypeId& dt) : EnumeratedDomain(dt,values) {}

  StringDomain::StringDomain(const AbstractDomain& org) : EnumeratedDomain(org) {}

  StringDomain* StringDomain::copy() const
  {
    StringDomain * ptr = new StringDomain(*this);
    check_error(ptr != NULL);
    return ptr;
  }

  void StringDomain::set(double value) {
    check_error(LabelStr::isString(value));
    checkError(isEmpty() || isMember(value), value << " is not a member of the domain :" << toString());

    // Insert the value into the set as a special behavior for strings
    m_values.insert(value);
    EnumeratedDomain::set(value);
  }

  bool StringDomain::isMember(double value) const {
      // This is a hack so that specify() will work
      // string domain needs to be able to handle all situations that involve literal string gracefully
      // for example :
      // string str; str.specify("foo"); eq(str,"bar");
      if (isOpen())
          return true;

      return EnumeratedDomain::isMember(value);
  }

  void StringDomain::set(const std::string& value){
    LabelStr lbl(value);
    set((double) lbl);
  }

  bool StringDomain::isMember(const std::string& value) const{
    LabelStr lbl(value);
    return isMember((double) lbl);
  }

  void StringDomain::insert(const std::string& value){
    LabelStr lbl(value);
    EnumeratedDomain::insert((double) lbl);
  }

  void StringDomain::insert(double value){
    EnumeratedDomain::insert(value);
  }

  SymbolDomain::SymbolDomain(const DataTypeId& dt) : EnumeratedDomain(dt) {}
  SymbolDomain::SymbolDomain(double value, const DataTypeId& dt) : EnumeratedDomain(dt,value) {}
  SymbolDomain::SymbolDomain(const std::list<double>& values, const DataTypeId& dt) : EnumeratedDomain(dt,values) {}

  SymbolDomain::SymbolDomain(const AbstractDomain& org) : EnumeratedDomain(org) {}

  SymbolDomain* SymbolDomain::copy() const
  {
    SymbolDomain * ptr = new SymbolDomain(*this);
    check_error(ptr != NULL);
    return ptr;
  }

  NumericDomain::NumericDomain(const DataTypeId& dt) : EnumeratedDomain(dt) {}
  NumericDomain::NumericDomain(double value, const DataTypeId& dt) : EnumeratedDomain(dt,value) {}
  NumericDomain::NumericDomain(const std::list<double>& values, const DataTypeId& dt) : EnumeratedDomain(dt,values) {}

  NumericDomain::NumericDomain(const AbstractDomain& org) : EnumeratedDomain(org) {}

  NumericDomain* NumericDomain::copy() const
  {
    NumericDomain * ptr = new NumericDomain(*this);
    check_error(ptr != NULL);
    return ptr;
  }

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

  BoolDomain::BoolDomain(const DataTypeId& dt)
    : IntervalIntDomain(0,1,dt)
  {
  }

  BoolDomain::BoolDomain(bool value, const DataTypeId& dt)
    : IntervalIntDomain(value, dt)
  {
  }

  BoolDomain::BoolDomain(const AbstractDomain& org)
    : IntervalIntDomain(org)
  {
  }

  void BoolDomain::testPrecision(const double& value) const
  {
    check_error(value == 0 || value == 1);
  }

  // convert(), insert(), and remove() are inherited from IntervalIntDomain.

  bool BoolDomain::isFinite() const{
    return(true);
  }

  bool BoolDomain::isFalse() const {
    return(m_ub == 0 && m_lb == 0);
  }

  bool BoolDomain::isTrue() const {
    return(m_ub == 1 && m_lb == 1);
  }

  BoolDomain *BoolDomain::copy() const {
    BoolDomain *ptr = new BoolDomain(*this);
    check_error(ptr != 0);
    return(ptr);
  }

  bool BoolDomain::intersect(const AbstractDomain& dom) {
    return intersect(dom.getLowerBound(), dom.getUpperBound());
  }

  bool BoolDomain::intersect(double lb, double ub) {
    double boolLb = lb;
    double boolUb = ub;
    if(boolLb > boolUb) {
      boolLb = boolUb;
      boolUb = lb;
    }
    if(boolLb != 0.0)
      boolLb = 1.0;
    if(boolUb != 0.0)
      boolUb = 1.0;
    return IntervalIntDomain::intersect(boolLb, boolUb);
  }
}
