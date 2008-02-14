
#include "PSResourceImpl.hh"
#include "SAVH_Resource.hh"
#include "SAVH_Profile.hh"
#include "SAVH_Transaction.hh"

namespace EUROPA 
{
  PSResource* PSResource::asPSResource(PSObject* obj)
  {
	  return dynamic_cast<PSResource*>(obj);
  }

  PSResourceImpl::PSResourceImpl(const SAVH::ResourceId& res) 
      : PSObjectImpl(res) 
      , m_res(res) 
  {	  
  }

  PSResourceProfile* PSResourceImpl::getLimits() {
    return new PSResourceProfileImpl(m_res->getLowerLimit(), m_res->getUpperLimit());
  }

  PSResourceProfile* PSResourceImpl::getLevels() {
    return new PSResourceProfileImpl(m_res->getProfile());
  }
  
  PSList<PSEntityKey> PSResourceImpl::getOrderingChoices(TimePoint t)
  {
	  PSList<PSEntityKey> retval;
	  
	  SAVH::InstantId instant;
	  
	  SAVH::ProfileIterator it(m_res->getProfile());
	  while(!it.done()) {
	      TimePoint inst = (TimePoint) it.getTime();
	      if (inst == t) {
	          instant = it.getInstant();
	          break;
	      }
	      it.next();
	  }
	  
	  if (instant.isNoId()) {
		  // TODO: log error
		  return retval;
	  }
	  
	  std::vector<std::pair<SAVH::TransactionId, SAVH::TransactionId> > results;
	  m_res->getOrderingChoices(instant,results);
	  for (unsigned int i = 0;i<results.size(); i++) {
	      SAVH::TransactionId predecessor = results[i].first;
	      SAVH::TransactionId successor = results[i].second;	
	      retval.push_back(predecessor->time()->getParent()->getKey());
	      retval.push_back(successor->time()->getParent()->getKey());
	  }
	  
	  return retval;
  }

  PSResourceProfileImpl::PSResourceProfileImpl(const double lb, const double ub)
    : m_isConst(true), m_lb(lb), m_ub(ub) {
    TimePoint inst = (TimePoint) MINUS_INFINITY;
    m_times.push_back(inst);
  }

  PSResourceProfileImpl::PSResourceProfileImpl(const SAVH::ProfileId& profile)
    : m_isConst(false), m_profile(profile) {
    SAVH::ProfileIterator it(m_profile);
    while(!it.done()) {
      TimePoint inst = (TimePoint) it.getTime();
      m_times.push_back(inst);
      it.next();
    }
  }

  double PSResourceProfileImpl::getLowerBound(TimePoint time) {
    if(m_isConst)
      return m_lb;

    IntervalDomain dom;
    m_profile->getLevel((int) time, dom);
    return dom.getLowerBound();
  }

  double PSResourceProfileImpl::getUpperBound(TimePoint time) {
    if(m_isConst)
      return m_ub;
    IntervalDomain dom;
    m_profile->getLevel((int) time, dom);
    return dom.getUpperBound();
  }

  const PSList<TimePoint>& PSResourceProfileImpl::getTimes() {return m_times;}
}
