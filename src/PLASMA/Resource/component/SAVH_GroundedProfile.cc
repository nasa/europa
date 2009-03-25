#include "SAVH_GroundedProfile.hh"
#include "SAVH_Instant.hh"
#include "SAVH_Transaction.hh"
#include "ConstrainedVariable.hh"
#include "Debug.hh"



namespace EUROPA {
namespace SAVH {
GroundedProfile::GroundedProfile(const PlanDatabaseId db, const FVDetectorId flawDetector, const double initCapacityLb, const double initCapacityUb)
: TimetableProfile(db, flawDetector, initCapacityLb, initCapacityUb) {}


void GroundedProfile::handleTransactionStart(bool isConsumer, const double & lb, const double & ub)
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

void GroundedProfile::handleTransactionEnd(bool isConsumer, const double & lb, const double & ub)
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
