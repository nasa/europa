#include "EnumeratedDomain.hh"
#include <algorithm>

namespace Prototype {

  const AbstractDomain::DomainType& EnumeratedDomain::getType() const {
    static const AbstractDomain::DomainType s_type = AbstractDomain::REAL_ENUMERATION;
    return s_type;
  }

  EnumeratedDomain::EnumeratedDomain(const std::list<double>& values, bool closed, const DomainListenerId& listener)
    :AbstractDomain(false, true, listener){
    for(std::list<double>::const_iterator it = values.begin(); it != values.end(); ++it)
      insert(*it);
    if(closed)
      close();
  }

  EnumeratedDomain::EnumeratedDomain(const EnumeratedDomain& org)
    : AbstractDomain(org.m_closed, true, DomainListenerId::noId()) {
    m_values = org.m_values;
    m_membership = org.m_membership;
  }

  EnumeratedDomain::EnumeratedDomain()
    : AbstractDomain(false, true, DomainListenerId::noId()){}

  bool EnumeratedDomain::isFinite() const {
    check_error(!isDynamic());
    return true;
  }

  bool EnumeratedDomain::isSingleton() const {return m_membership.count() == 1;}

  bool EnumeratedDomain::isEmpty() const {
    return m_membership.none();
  }

  void EnumeratedDomain::empty() {
    m_membership.reset();
    notifyChange(DomainListener::EMPTIED);
  }
  void EnumeratedDomain::close() {

    // Ok, now initialize all the membership bits
    m_membership.reset(); // Negates everything

    int i = 0;
    for (std::set<double>::iterator it = m_values.begin(); it != m_values.end(); ++ it){
      m_membership.set(i++);
    }

    AbstractDomain::close();
  }

  int EnumeratedDomain::getSize() const {
    check_error(!isDynamic());
    return m_membership.count();
  }

  void EnumeratedDomain::insert(double value){
    check_error(value <= PLUS_INFINITY && value >= MINUS_INFINITY);

    if(isDynamic()){
      std::pair<std::set<double>::iterator, bool> result = m_values.insert(value);
      check_error(result.second); // Ensure it has been added - i.e. was not present.
    }
    else {
      int index = getIndex(value);
      check_error(index > -1);
      m_membership.set(index);
    }
    notifyChange(DomainListener::RELAXED);
  }

  void EnumeratedDomain::remove(double value){
    int index = getIndex(value);
    check_error(index > -1);
    m_membership.reset(index);

    if(!isEmpty())
      notifyChange(DomainListener::VALUE_REMOVED);
    else
      notifyChange(DomainListener::EMPTIED);
  }

  void EnumeratedDomain::set(const AbstractDomain& dom){
    intersect(dom);
    notifyChange(DomainListener::SET);
  }

  void EnumeratedDomain::set(double value){
    // Find the value in the current set.
    int index = getIndex(value);

    if (index < 0){
      empty();
      return;
    }

    if(!isSingleton()){ // Implying not currently a singleton
      m_membership.reset();
      m_membership.set(index);
    }

    notifyChange(DomainListener::SET_TO_SINGLETON);
  }

  void EnumeratedDomain::reset(const AbstractDomain& dom){
    if(*this != dom){
      relax(dom);
      notifyChange(DomainListener::RESET);
    }
  }

  bool EnumeratedDomain::equate(AbstractDomain& dom){
    check_error(isDynamic() || dom.isDynamic() || (!isEmpty() && !dom.isEmpty()));
    check_error(dom.isEnumerated());

    EnumeratedDomain& l_dom = static_cast<EnumeratedDomain&>(dom);
    std::bitset<MAX_SIZE> intersection = m_membership & l_dom.m_membership;

    if(intersection.none()){
      empty();
      return false;
    }

    bool changed_a = intersection.count() < m_membership.count();
    bool changed_b = intersection.count() < l_dom.m_membership.count();

    if (changed_a){
      m_membership = intersection;
      if(isSingleton())
	notifyChange(DomainListener::RESTRICT_TO_SINGLETON);
      else
	notifyChange(DomainListener::VALUE_REMOVED);
    }

    if (changed_b){
      l_dom.m_membership = intersection;
      if(l_dom.isSingleton())
	l_dom.notifyChange(DomainListener::RESTRICT_TO_SINGLETON);
      else
	l_dom.notifyChange(DomainListener::VALUE_REMOVED);
    }

    return (changed_a || changed_b);
  }
 
  bool EnumeratedDomain::isMember(double value) const {
    check_error(!isDynamic());
    int index = getIndex(value);
    if(index > -1)
      return m_membership.test(index);
    else
      return false;
  }

  bool EnumeratedDomain::operator==(const AbstractDomain& dom) const{
    bool equal;

    if(dom.isEnumerated()){
      const EnumeratedDomain& l_dom = static_cast<const EnumeratedDomain&>(dom);
      check_error(sameBaseSet(l_dom));
      equal = (AbstractDomain::operator==(dom) && m_membership == l_dom.m_membership);
    }
    else 
      equal = (dom.isFinite() &&
	       getSize() == dom.getSize() &&
	       isSubsetOf(dom));

    return equal;
  }

  bool EnumeratedDomain::operator!=(const AbstractDomain& dom) const{
    return (!operator==(dom));
  }

  void EnumeratedDomain::relax(const AbstractDomain& dom) {
    check_error(dom.isDynamic() || !dom.isEmpty());
    check_error(isSubsetOf(dom));
    check_error(dom.isEnumerated());

    if(!((*this) == dom)){
      const EnumeratedDomain& l_dom = static_cast<const EnumeratedDomain&>(dom);
      m_values = l_dom.m_values;
      m_membership = l_dom.m_membership;
      notifyChange(DomainListener::RELAXED);
    }
  }

  double EnumeratedDomain::getSingletonValue() const {
    check_error(isSingleton());
    int i = 0;
    for(std::set<double>::const_iterator it = m_values.begin(); it != m_values.end(); ++it){
      if(m_membership.test(i))
	return (*it);
      else
	i++;
    }

    // Should never get to here!
    check_error(ALWAYS_FAILS);
    return 0;
  }

  void EnumeratedDomain::getValues(std::list<double>& results) const{
    check_error(results.empty());
    check_error(isFinite());
    int i=0;
    for(std::set<double>::iterator it = m_values.begin(); it != m_values.end(); ++it){
      if(m_membership.test(i++))
	results.push_back(*it);
    }
  }

  double EnumeratedDomain::getUpperBound() const{
    double lb, ub;
    getBounds(lb, ub);
    return ub;
  }

  double EnumeratedDomain::getLowerBound() const{
    double lb, ub;
    getBounds(lb, ub);
    return lb;

  }

  bool EnumeratedDomain::getBounds(double& lb, double& ub) const{
    check_error(!isEmpty());
    lb = PLUS_INFINITY;
    ub = MINUS_INFINITY;
    int index = 0;
    for (std::set<double>::const_iterator it = m_values.begin(); it != m_values.end(); ++it){
      if(m_membership.test(index++)){
	double value = *it;
	if(value < lb)
	  lb = value;
	if(value > ub)
	  ub = value;
      }
    }
    check_error(lb <= ub);
    return false; // ALWAYS FINITE
  }

  bool EnumeratedDomain::intersect(const AbstractDomain& dom){
    check_error(isDynamic() || dom.isDynamic() || (!isEmpty() && !dom.isEmpty()));

    if(dom.isEnumerated()){
      const EnumeratedDomain& l_dom = static_cast<const EnumeratedDomain&>(dom);
      check_error(sameBaseSet(l_dom));

      int lastCount = m_membership.count();
      m_membership = m_membership & l_dom.m_membership;

      bool changed = m_membership.count() < lastCount;

      if(changed && m_membership.none())
	empty();
      else if (changed){
	if(isSingleton())
	  notifyChange(DomainListener::RESTRICT_TO_SINGLETON);
	else
	  notifyChange(DomainListener::VALUE_REMOVED);
      }
      return changed;
    }
    else {
      bool changed = false;

      // Remove all values present that are not within the bounds
      int index = 0;
      for (std::set<double>::const_iterator it = m_values.begin(); it != m_values.end(); ++it){
	double value = *it;
	if(m_membership.test(index) && !dom.isMember(value)){
	  changed = true;
	  m_membership.reset(index);
	  if(!isEmpty())
	    notifyChange(DomainListener::VALUE_REMOVED);
	  else {
	    notifyChange(DomainListener::EMPTIED);
	    break;
	  }
	}
	index++;
      }

      return changed;
    }
  }

  bool EnumeratedDomain::isSubsetOf(const AbstractDomain& dom) const{
    check_error(dom.isDynamic() || !dom.isEmpty());

    if(dom.isEnumerated()){
      const EnumeratedDomain& l_dom = static_cast<const EnumeratedDomain&>(dom);
      check_error(sameBaseSet(l_dom));
      std::bitset<MAX_SIZE> intersection = l_dom.m_membership & m_membership;
      return m_membership == intersection;
    }
    else {
      // Remove all values present that are not within the bounds
      int index = 0;
      for (std::set<double>::const_iterator it = m_values.begin(); it != m_values.end(); ++it){
	if(m_membership.test(index++) && !dom.isMember(*it))
	  return false;
      }
      return true;
    }
  }

  int EnumeratedDomain::getIndex(double value) const{
    int i = 0;
    double previous = *(m_values.begin());
    for(std::set<double>::const_iterator it = m_values.begin(); it != m_values.end(); ++it){
      check_error(previous <= *it);
      previous = *it;
      if(*it == value)
	return i;
      else
	i++;
    }
    return -1;
  }

  bool EnumeratedDomain::sameBaseSet(const EnumeratedDomain& dom) const{
    return (dom.m_values == m_values);
  }
}
