#ifndef _H_GroundedProfile
#define _H_GroundedProfile

#include "ResourceDefs.hh"
#include "Profile.hh"
#include "DomainListener.hh"
#include "TimetableProfile.hh"

/**
 * @file GroundedProfile.hh
 * @author Tristan Smith
 * @brief Fast resource profiles for reusable resources
 * quantity and time are used.
 * @date January, 2008
 * @ingroup Resource
 *
 * FlowProfile calculations are accurate but slow;  TimetableProfile calculations are fast but inaccurate (for example, they can report
 * flaws that are impossible to resolve).  This class is a hybrid that can be used in cases where the user only needs flaws for the
 * early-start plan (ie every transaction at its earliest time).
 *
 * This class must be used in conjunction with GroundedFVDetector (you won't be allowed to do anything else).  Together, they:
 * a) Use the TimetableProfile (non-grounded) approach to produce LowerLevelMin and UpperLevelMax, which are used
 *    to report violations only.
 * b) Use LowerLevelMax and UpperLevelMin to represent GroundedMin and GroundedMax profiles, which are used to report flaws.
 *    Again, these are  computed with every transaction assume to occur at its earliest time.
 *
 * WARNINGS:
 * - Even though EUROPA users are used to flexible times, it will NOT be true that you could start/stop resource production
 *   or consumption at any point within the domain.  Because flaw reporting assumes every transaction occurs as early as possible,
 *   ONLY the 'early-start plan' is necessarily feasible.
 * - Note that LowerLevelMax and UpperLevelMin variables are hijacked and used in an unintended way.  This code doesn't make sense
 *   unless you think:
 *   LowerLevelMax => GroundedMin
 *   UpperLevelMin => GroundedMax
 *   (this only works because you are forced to used this in conjunction with GroundedFVDetector, which understands the
 *   approach).
 * - For now, the behavior of instantaneous/cumulative production/consumption is identical to TimetableProfile.  We may want
 *   those to be treated in a grounded fashion as well.
 *
 * EXAMPLES:
 *
 * Consider a unary reusable resource with a long horizon, and two tokens, A and B, each using 1 unit of the resource and with unit
 * duration:
 * 	- Both TimetableProfile and GroundedProfile will report a flaw.
 *  - However, if a precedence constraint is added between A and B, TimetableProfile will still report a flaw whereas
 *    GroundedReuslabeProfile will not.
 *  - If instead A and B are constrained to start at time 0, both TimetableProfile and GroundedProfile will report a violation
 *
 */

namespace EUROPA {

class GroundedProfile : public TimetableProfile {
public:
	GroundedProfile(const PlanDatabaseId db, const FVDetectorId flawDetector,
			const edouble initCapacityLb = 0, const edouble initCapacityUb = 0);

protected:

	// Slight variants on what is done in TimetableProfile:
	void handleTransactionStart(bool isConsumer, const edouble & lb, const edouble & ub);
	void handleTransactionEnd(bool isConsumer, const edouble & lb, const edouble & ub);
};
}

#endif
