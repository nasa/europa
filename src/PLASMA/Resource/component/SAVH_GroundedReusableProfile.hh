#ifndef _H_SAVH_GroundedProfile
#define _H_SAVH_GroundedProfile

#include "SAVH_ResourceDefs.hh"
#include "SAVH_Profile.hh"
#include "DomainListener.hh"
#include "SAVH_TimetableProfile.hh"

/**
 * @file SAVH_GroundedProfile.hh
 * @author Tristan Smith
 * @brief Fast resource profiles for reusable resources
 * quantity and time are used.
 * @date January, 2008
 * @ingroup Resource
 *
 * OVERVIEW:

 * A hybrid profile that inherits from, and only slightly modifies, TimetableProfile:
 * a) For upper bounds, it is equivalent to TimetableProfile.
 * b) For lower bounds, it assumes all production/consumption events occur at their earliest possible time (ie. a grounding)
 *
 * For reusable resources, violations occur when upper bounds get too low.  Therefore, using this class (instead of TimetableProfile)
 * has no effect on reported  violations.  However, it results in decreased flaw detection:  TimetableProfile results in a
 * flaw when any possible schedule could go below the lower limit, whereas this class results in a flaw
 * only when a specific schedule (namely the one with every transaction as early as possible) could go below the lower limit.
 *
 * Therefore, this class can be used for Reusable resource situations where the TimetableProfile results in unresolveable states,
 * but FlowProfile is too slow.
 *
 * EXAMPLES:
 *
 * Consider a unary resource with a long horizon, and two tokens, A and B, each using 1 unit of the resource and with unit duration:
 * 	- Both TimetableProfile and GroundedReusableProfile will report a flaw.
 *  - However, if a precedence constraint is added between A and B, TimetableProfile will still report a flaw whereas
 *    GroundedReuslabeProfile will not.
 *  - If instead A and B are constrained to start at time 0, both TimetableProfile and GroundedReusableProfile will report a violation
 *
 * NOTES:
 *
 * - This profile is only intended for reusable resources because:
 *  a) If there is only production or only consumption, I believe it provides no additional benefit.
 *  b) If (for some strange reason), you had the opposite of reusable, where every production is followed later by an equivalent
 *     consumption, you should use the opposite of this - use TimetableProfile for lower bounds, but the grounded plan for
 *     upper bounds.
 *  c) If there can be arbitrary combinations of production and consumption, what you probably (?) need is a combination of this
 *     class and b).  Ie. three profiles, where the upper and lower profiles are the same as TimetableProfile, but a middle profile
 *     corresponds to the grounded plan (ie every transaction as early as possible).  However, this would require a significant
 *     rework to Instant class, as well as the flaw and violation detectors that use it.
 *
 * - The only difference code-wise with TimetableProfile is that m_lowerLevelMin/m_lowerLevelMax are increased when
 *   the transaction begins, instead of when it ends.
 *
 * - This approach only makes sense for reusable resources:
 *   a)  For resources that involve only production or only consumption, any flaw is a violation, and this
 *
 * - For this to work required Resources to have upper limits set to infinity, instead of capacity, because TimetableProfile
 *   upper-bounds could be higher than capacity, but those cases aren't flaws for reusable resources and we don't want them
 *   flagged as such.
 * - The resulting profiles will look odd to a user.  Possible solutions:
 *   a)  Go ahead and compute the more expensive FlowProfile profiles for a user, since that needn't be done within search.
 *   b)  At a minimum, chop the upper bounds off at capacity (they will exceed it in places (see previous note), but that is weird)
 * - The behavior of instantaneous and cumulative production/consumption is the same as TimetableProfile (is that reasonable?)
 */

namespace EUROPA {
namespace SAVH {

class GroundedReusableProfile : public TimetableProfile {
public:
	GroundedReusableProfile(const PlanDatabaseId db, const FVDetectorId flawDetector,
			const double initCapacityLb = 0, const double initCapacityUb = 0);

protected:

	// Slight variants on what is done in TimetableProfile:
	void handleTransactionStart(bool isConsumer, const double & lb, const double & ub);
	void handleTransactionEnd(bool isConsumer, const double & lb, const double & ub);
};
}
}

#endif
