#include "SAVH_GroundedReusableProfile.hh"
#include "SAVH_Instant.hh"
#include "SAVH_Transaction.hh"
#include "ConstrainedVariable.hh"
#include "Debug.hh"



namespace EUROPA {
namespace SAVH {
GroundedReusableProfile::GroundedReusableProfile(const PlanDatabaseId db, const FVDetectorId flawDetector, const double initCapacityLb, const double initCapacityUb)
: TimetableProfile(db, flawDetector, initCapacityLb, initCapacityUb) {}



void GroundedReusableProfile::handleTransactionStart(bool isConsumer, const double & lb, const double & ub)
{
	if(isConsumer) {
		m_lowerLevelMin -= ub;
		m_lowerLevelMax -= lb;
	}
	else {
		m_upperLevelMin += lb;
		m_upperLevelMax += ub;
		m_lowerLevelMin += lb;
		m_lowerLevelMax += ub;
	}
}

void GroundedReusableProfile::handleTransactionEnd(bool isConsumer, const double & lb, const double & ub)
{
	if(isConsumer) {
		m_upperLevelMax -= lb;
		m_upperLevelMin -= ub;
	}
}

}
}
