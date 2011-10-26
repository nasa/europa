#ifndef _H_PSResource
#define _H_PSResource

#include "PSPlanDatabase.hh"
#include "ResourceDefs.hh"

namespace EUROPA {

	typedef eint::basis_type TimePoint;

	class PSResourceProfile;

	class PSResource : public virtual PSObject
	{
	public:
		PSResource() {}
		virtual ~PSResource() {}

		virtual PSResourceProfile* getLimits() = 0;
		virtual PSResourceProfile* getFDLevels() = 0;
		virtual PSResourceProfile* getVDLevels() = 0;

		virtual PSList<PSEntityKey> getOrderingChoices(TimePoint t) = 0;

		static PSResource* asPSResource(PSObject* obj);
	};

	class PSResourceProfile
	{
	public:
		PSResourceProfile() {}
		virtual ~PSResourceProfile() {}

		virtual PSList<TimePoint> getTimes() = 0;
		virtual double getLowerBound(TimePoint time) = 0;
		virtual double getUpperBound(TimePoint time) = 0;
	};

	class PSGenericProfile : public PSResourceProfile
	{
	public:
		PSGenericProfile(const ExplicitProfileId& profile);
		virtual ~PSGenericProfile() {}

		virtual PSList<TimePoint> getTimes();
		virtual double getLowerBound(TimePoint time);
		virtual double getUpperBound(TimePoint time);

	protected:
		ExplicitProfileId m_profile;
	};

	class PSUsageProfile : public PSResourceProfile
	{
	public:
		PSUsageProfile(const ProfileId& profile);
		virtual ~PSUsageProfile() {}

		virtual PSList<TimePoint> getTimes();
		virtual double getLowerBound(TimePoint time);
		virtual double getUpperBound(TimePoint time);

	protected:
		ProfileId m_profile;
	};
}

#endif
