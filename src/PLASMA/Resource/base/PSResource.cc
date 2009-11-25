
#include "PSResource.hh"
#include "Resource.hh"
#include "Profile.hh"
#include "Transaction.hh"

namespace EUROPA
{
	// TODO:  Do we still need this?
  PSResource* PSResource::asPSResource(PSObject* obj)
  {
	  return dynamic_cast<PSResource*>(obj);
  }

  PSResourceProfile::PSResourceProfile(const double lb, const double ub)
    : m_isConst(true), m_lb(lb), m_ub(ub) {
    TimePoint inst = cast_basis((eint)MINUS_INFINITY); //being specific
    m_times.push_back(inst);
  }

  PSResourceProfile::PSResourceProfile(const ProfileId& profile)
    : m_isConst(false), m_profile(profile) {
    ProfileIterator it(m_profile);
    while(!it.done()) {
      TimePoint inst = cast_basis(it.getTime());
      m_times.push_back(inst);
      it.next();
    }
  }

  double PSResourceProfile::getLowerBound(TimePoint time) {
    if(m_isConst)
      return m_lb;

    IntervalDomain dom;
    m_profile->getLevel((eint) time, dom);
    return cast_double(dom.getLowerBound());
  }

  double PSResourceProfile::getUpperBound(TimePoint time) {
    if(m_isConst)
      return m_ub;
    IntervalDomain dom;
    m_profile->getLevel((eint) time, dom);
    return cast_double(dom.getUpperBound());
  }

  const PSList<TimePoint>& PSResourceProfile::getTimes() {return m_times;}


}
