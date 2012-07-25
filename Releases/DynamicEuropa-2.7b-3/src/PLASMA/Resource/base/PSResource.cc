
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

	PSGenericProfile::PSGenericProfile(const ExplicitProfileId& profile)
		: m_profile(profile)
	{
	}

	PSList<TimePoint> PSGenericProfile::getTimes()
	{
		PSList<TimePoint> times;
		const std::map< eint,std::pair<edouble,edouble> >& entries = m_profile->getValues();

		//std::cout << "PSGenericProfile {" << std::endl;

		std::map< eint,std::pair<edouble,edouble> >::const_iterator it = entries.begin();
		for(; it != entries.end(); ++it) {
			TimePoint inst = cast_basis(it->first);
			times.push_back(inst);
			//std::cout << "    " << inst << " -> (" << it->second.first << "," << it->second.second << ")" << std::endl;
		}

		//std::cout << "}" << std::endl;

		return times;
	}

	double PSGenericProfile::getLowerBound(TimePoint time)
	{
		return cast_double(m_profile->getValue(time).first);
	}

	double PSGenericProfile::getUpperBound(TimePoint time)
	{
		return cast_double(m_profile->getValue(time).second);
	}

	PSUsageProfile::PSUsageProfile(const ProfileId& profile)
	: m_profile(profile)
	{
	}

	PSList<TimePoint> PSUsageProfile::getTimes()
	{
		PSList<TimePoint> times;

		ProfileIterator it(m_profile);
		while(!it.done()) {
			TimePoint inst = cast_basis(it.getTime());
			times.push_back(inst);
			it.next();
		}

		return times;
	}

	double PSUsageProfile::getLowerBound(TimePoint time)
	{
		IntervalDomain dom;
		m_profile->getLevel((eint) time, dom);
		return cast_double(dom.getLowerBound());
	}

	double PSUsageProfile::getUpperBound(TimePoint time)
	{
		IntervalDomain dom;
		m_profile->getLevel((eint) time, dom);
		return cast_double(dom.getUpperBound());
	}
}
