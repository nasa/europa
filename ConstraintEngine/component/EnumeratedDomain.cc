#include "EnumeratedDomain.hh"
#include <algorithm>

namespace Prototype {

  const AbstractDomain::DomainType& EnumeratedDomain::getType() const {
    static const AbstractDomain::DomainType s_type = AbstractDomain::REAL_ENUMERATION;
    return s_type;
  }

  EnumeratedDomain::EnumeratedDomain(const std::list<double>& values, bool closed, const DomainListenerId& listener)
    :AbstractDomain(false, listener){
    for(std::list<double>::const_iterator it = values.begin(); it != values.end(); ++it)
      insert(*it);
    if(closed)
      close();
  }

  EnumeratedDomain::EnumeratedDomain(const EnumeratedDomain& org)
    : AbstractDomain(org.m_closed, DomainListenerId::noId()) {
    m_values = org.m_values;
    m_membership = org.m_membership;
  }

  EnumeratedDomain::EnumeratedDomain()
    : AbstractDomain(true, DomainListenerId::noId()){}

  bool EnumeratedDomain::isFinite() const {
    check_error(!isDynamic());
    return true;
  }

  bool EnumeratedDomain::isEnumerated() const {return true;}

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
    check_error(isDynamic());
    std::pair<std::set<double>::iterator, bool> result = m_values.insert(value);
    check_error(result.second); // Ensure it has been added - i.e. was not present.
    notifyChange(DomainListener::RELAXED);
  }

  void EnumeratedDomain::remove(double value){
    int index = getIndex(value);
    m_membership.reset(index);
    notifyChange(DomainListener::VALUE_REMOVED);
  }

  void EnumeratedDomain::set(const EnumeratedDomain& dom){
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

  void EnumeratedDomain::reset(const EnumeratedDomain& dom){
    if(*this != dom){
      *this = dom;
      notifyChange(DomainListener::RESET);
    }
  }

  bool EnumeratedDomain::equate(EnumeratedDomain& dom){
    check_error(isDynamic() || dom.isDynamic() || (!isEmpty() && !dom.isEmpty()));

    std::bitset<MAX_SIZE> intersection = m_membership & dom.m_membership;

    if(intersection.none()){
      empty();
      return false;
    }

    bool changed_a = intersection.count() < m_membership.count();
    bool changed_b = intersection.count() < dom.m_membership.count();

    if (changed_a){
      m_membership = intersection;
      if(isSingleton())
	notifyChange(DomainListener::RESTRICT_TO_SINGLETON);
      else
	notifyChange(DomainListener::VALUE_REMOVED);
    }

    if (changed_b){
      dom.m_membership = intersection;
      if(dom.isSingleton())
	dom.notifyChange(DomainListener::RESTRICT_TO_SINGLETON);
      else
	dom.notifyChange(DomainListener::VALUE_REMOVED);
    }

    return (changed_a || changed_b);
  }
 
  bool EnumeratedDomain::isMember(double value) const {
    check_error(!isDynamic());
    int index = getIndex(value);
    check_error(getIndex(value) > -1);
    return m_membership.test(index);
  }

  bool EnumeratedDomain::operator==(const EnumeratedDomain& dom) const{
    check_error(sameBaseSet(dom));
    return (AbstractDomain::operator==(dom) && m_membership == dom.m_membership);
  }

  bool EnumeratedDomain::operator!=(const EnumeratedDomain& dom) const{
    check_error(sameBaseSet(dom));
    return (!operator==(dom));
  }

  EnumeratedDomain& EnumeratedDomain::operator=(const EnumeratedDomain& dom) {
    check_error(dom.isDynamic() || !dom.isEmpty());
    check_error(isSubsetOf(dom));

    if(!((*this) == dom)){
      m_values = dom.m_values;
      m_membership = dom.m_membership;
      notifyChange(DomainListener::RELAXED);
    }

    return *this;
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

  bool EnumeratedDomain::intersect(const EnumeratedDomain& dom){
    check_error(isDynamic() || dom.isDynamic() || (!isEmpty() && !dom.isEmpty()));
    check_error(sameBaseSet(dom));

    int lastCount = m_membership.count();
    m_membership = m_membership & dom.m_membership;

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

  bool EnumeratedDomain::isSubsetOf(const EnumeratedDomain& dom) const{
    check_error(dom.isDynamic() || !dom.isEmpty());
    check_error(sameBaseSet(dom));
    std::bitset<MAX_SIZE> intersection = dom.m_membership & m_membership;
    return m_membership == intersection;
  }

  int EnumeratedDomain::getIndex(double value) const{
    int i = 0;
    for(std::set<double>::const_iterator it = m_values.begin(); it != m_values.end(); ++it){
      if(*it == value)
	return i;
      else
	i++;
    }
    return -1;
  }

  bool EnumeratedDomain::sameBaseSet(const EnumeratedDomain& dom) const{
    /*!< @todo Put this back in */
    //return (dom.m_values == m_values);
    return true;
  }
}
