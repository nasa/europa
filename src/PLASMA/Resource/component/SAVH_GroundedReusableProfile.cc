#include "SAVH_GroundedReusableProfile.hh"
#include "SAVH_Instant.hh"
#include "SAVH_Transaction.hh"
#include "ConstrainedVariable.hh"
#include "Debug.hh"



namespace EUROPA {
namespace SAVH {
GroundedReusableProfile::GroundedReusableProfile(const PlanDatabaseId db, const FVDetectorId flawDetector, const double initCapacityLb, const double initCapacityUb)
: TimetableProfile(db, flawDetector, initCapacityLb, initCapacityUb) {}


// Notice that the lb value passed in is ignored?

// a) LowerLevelMin and UpperLevelMax treated the same as timetable
// b) LowerLevelMax and UpperLevelMin both happen at transaction start, and use lb and ub to set
// ie:  LowerLevelMax should really be called GroundedMin and UpperLevelMin should really be called GroundedMax (!)

// WHOA:  Timetable is really bad with flexible amount - will never go back to zero!!!

void GroundedReusableProfile::handleTransactionStart(bool isConsumer, const double & lb, const double & ub)
{
	if(isConsumer) {
		m_lowerLevelMin -= ub;

		// These two are the grounded min/max profiles:
		m_lowerLevelMax -= ub;
		m_upperLevelMin -= lb;
	}
	else {
		m_upperLevelMax += ub;

		// These two are the grounded min/max profiles:
		m_upperLevelMin += ub;
		m_lowerLevelMax += lb;
	}
}

void GroundedReusableProfile::handleTransactionEnd(bool isConsumer, const double & lb, const double & ub)
{
	if(isConsumer) {
		m_upperLevelMax -= lb;
	}
	else {
		m_lowerLevelMin += lb;
	}
}
}
}
