#include "EnumeratedDomain.hh"
#include <algorithm>

namespace Prototype {

  const AbstractDomain::DomainType& EnumeratedDomain::getType() const {
    static const AbstractDomain::DomainType s_type = AbstractDomain::REAL_ENUMERATION;
    return s_type;
  }

  EnumeratedDomain::EnumeratedDomain()
    :AbstractDomain(false, true, DomainListenerId::noId()){}

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
  }

  bool EnumeratedDomain::isFinite() const {
    check_error(!isDynamic());
    return true;
  }

  bool EnumeratedDomain::isSingleton() const {return (m_values.size() == 1);}

  bool EnumeratedDomain::isEmpty() const {
    return m_values.empty();
  }

  void EnumeratedDomain::empty() {
    m_values.clear();
    notifyChange(DomainListener::EMPTIED);
  }
  void EnumeratedDomain::close() {
    AbstractDomain::close();
  }

  int EnumeratedDomain::getSize() const {
    check_error(!isDynamic());
    return m_values.size();
  }

  void EnumeratedDomain::insert(double value){
    check_error(value <= PLUS_INFINITY && value >= MINUS_INFINITY);
    std::pair<std::set<double>::iterator, bool> result = m_values.insert(value);
    check_error(result.second || isDynamic()); // Ensure it has been added - i.e. was not present.

    // We only consider insertion a relaxation if  the domain is closed
    if(!isDynamic())
      notifyChange(DomainListener::RELAXED);
  }

  void EnumeratedDomain::remove(double value){
    m_values.erase(value);

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
    if (!isMember(value)){
      empty();
      return;
    }

    m_values.clear();
    m_values.insert(value);
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

    if(dom.isInterval()){
      bool changed = intersect(dom);

      if(isEmpty())
	return true;

      return (changed || dom.intersect(*this));
    }
    else {
      bool changed_a = false;
      bool changed_b = false;
      EnumeratedDomain& l_dom = static_cast<EnumeratedDomain&>(dom);
      std::set<double>::iterator it_a = m_values.begin();
      std::set<double>::iterator it_b = l_dom.m_values.begin();

      while(it_a != m_values.end() && it_b != l_dom.m_values.end()){
	double val_a = *it_a;
	double val_b = *it_b;

	if(val_a == val_b){
	  ++it_a;
	  ++it_b;
	}
	else if(val_a < val_b){
	  std::set<double>::iterator target = m_values.lower_bound(val_b);
	  m_values.erase(it_a, target);
	  it_a = target;
	  changed_a = true;
	  check_error(!isMember(val_a));
	}
	else {
	  std::set<double>::iterator target = l_dom.m_values.lower_bound(val_a);
	  l_dom.m_values.erase(it_b, target);
	  it_b = target;
	  changed_b = true;
	  check_error(!l_dom.isMember(val_b));
	}
      }

      if(it_a != m_values.end() && !l_dom.isEmpty()){
	m_values.erase(it_a, m_values.end());
	changed_a = true;
	check_error(it_b == l_dom.m_values.end());
      } 
      else if(it_b != l_dom.m_values.end() && !isEmpty()){
	l_dom.m_values.erase(it_b, l_dom.m_values.end());
	changed_b = true;
	check_error(it_a == m_values.end());
      }

      if(changed_a){
	if(isEmpty())
	  notifyChange(DomainListener::EMPTIED);
	else if(isSingleton())
	  notifyChange(DomainListener::RESTRICT_TO_SINGLETON);
	else
	  notifyChange(DomainListener::VALUE_REMOVED);
      }

      if(changed_b){
	if(l_dom.isEmpty())
	  l_dom.notifyChange(DomainListener::EMPTIED);
	else if(isSingleton())
	  l_dom.notifyChange(DomainListener::RESTRICT_TO_SINGLETON);
	else
	  l_dom.notifyChange(DomainListener::VALUE_REMOVED);
      }

      check_error(!isEmpty() || ! dom.isEmpty());
      check_error(isEmpty() || dom.isEmpty() || (l_dom.m_values == m_values));
      return (changed_a || changed_b);
    }
  }
 
  bool EnumeratedDomain::isMember(double value) const {
    check_error(!isDynamic());
    return m_values.find(value) != m_values.end();
  }

  bool EnumeratedDomain::operator==(const AbstractDomain& dom) const{
    bool equal;

    if(dom.isEnumerated()){
      const EnumeratedDomain& l_dom = static_cast<const EnumeratedDomain&>(dom);
      equal = (AbstractDomain::operator==(dom) && m_values == l_dom.m_values);
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
      notifyChange(DomainListener::RELAXED);
    }
  }

  double EnumeratedDomain::getSingletonValue() const {
    check_error(isSingleton());
    return *m_values.begin();
  }

  void EnumeratedDomain::getValues(std::list<double>& results) const{
    check_error(results.empty());
    check_error(isFinite());

    int i=0;
    for(std::set<double>::iterator it = m_values.begin(); it != m_values.end(); ++it)
      results.push_back(*it);
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
    lb = *m_values.begin();
    ub = *(--m_values.end());
    check_error(lb <= ub);
    return false; // ALWAYS FINITE
  }

  bool EnumeratedDomain::intersect(const AbstractDomain& dom){
    check_error(isDynamic() || dom.isDynamic() || (!isEmpty() && !dom.isEmpty()));

    bool changed = false;
    if(dom.isInterval()){
      std::set<double>::iterator it = m_values.begin();
      while(it != m_values.end()){
	double value = *it;
	if(!dom.isMember(value)){
	  changed = true;
	  if(value > dom.getUpperBound()){
	    m_values.erase(it, m_values.end());
	    break;
	  }
	  else
	    m_values.erase(it);
	}
	++it;
      }
    }
    else {
      const EnumeratedDomain& l_dom = static_cast<const EnumeratedDomain&>(dom);
      std::set<double>::iterator it_a = m_values.begin();
      std::set<double>::const_iterator it_b = l_dom.m_values.begin();

      while(it_a != m_values.end() && it_b != l_dom.m_values.end()){
	double val_a = *it_a;
	double val_b = *it_b;

	if(val_a == val_b){
	  ++it_a;
	  ++it_b;
	}
	else if(val_a < val_b){
	  std::set<double>::iterator target = m_values.find(val_b);
	  m_values.erase(it_a, target);
	  it_a = target;
	  changed = true;
	  check_error(!isMember(val_a));
	}
	else
	  it_b = l_dom.m_values.find(val_a);
      }

      if(it_a != m_values.end()){
	m_values.erase(it_a, m_values.end());
	changed = true;
      }
    }
    if(!changed)
      return false;

    if(isEmpty())
      notifyChange(DomainListener::EMPTIED);
    else if(isSingleton())
      notifyChange(DomainListener::RESTRICT_TO_SINGLETON);
    else
      notifyChange(DomainListener::VALUE_REMOVED);

    return true;
  }

  bool EnumeratedDomain::isSubsetOf(const AbstractDomain& dom) const{
    check_error(dom.isDynamic() || !dom.isEmpty());

    for (std::set<double>::const_iterator it = m_values.begin(); it != m_values.end(); ++it){
      if(!dom.isMember(*it))
	return false;
    }
    return true;
  }
}
