#ifndef PSCONSTRAINTENGINELISTENER_H_
#define PSCONSTRAINTENGINELISTENER_H_

#include "ConstraintEngineListener.hh"
#include "PSConstraintEngine.hh"

/*
 * PSConstraintEngineListener.h
 *
 * This method is modeled after PSPlanDatabaseListener.  See notes in that header file
 * for details about how this works.
 *
 *  Created on: Aug 5, 2008
 *      Author: tsmith
 */

namespace EUROPA {



class PSConstraintEngineListener: public ConstraintEngineListener {
public:
	enum PSChangeType { UPPER_BOUND_DECREASED, LOWER_BOUND_INCREASED,  BOUNDS_RESTRICTED,
		              VALUE_REMOVED, RESTRICT_TO_SINGLETON,  SET_TO_SINGLETON,  RESET,
		              RELAXED, CLOSED, OPENED,  EMPTIED,  LAST_CHANGE_TYPE};

	virtual ~PSConstraintEngineListener() {}

	/* The subset of notifications available through PSEngine interface */
	virtual void notifyViolationAdded(PSConstraint* constraint) {}
	virtual void notifyViolationRemoved(PSConstraint* constraint) {}
	virtual void notifyChanged(PSVariable* variable, PSChangeType changeType){}

private:

	/* We override these base class methods (called by ConstraintEngine)
	 * to call the above PS interface versions of the same methods.
	 */
	virtual void notifyViolationAdded(const ConstraintId& variable);
	virtual void notifyViolationRemoved(const ConstraintId& variable);
	virtual void notifyChanged(const ConstrainedVariableId& variable, const DomainListener::ChangeType& changeType);

	/* These methods are likely unnecessary to a user.  We override the base
	 * class version only to make private (they still don't do anything)
	 */
	virtual void notifyPropagationCommenced() {}
	virtual void notifyPropagationCompleted() {}
	virtual void notifyPropagationPreempted() {}
	virtual void notifyAdded(const ConstraintId& constraint) {}
	virtual void notifyActivated(const ConstraintId& constraint) {}
	virtual void notifyDeactivated(const ConstraintId& constraint) {}
	virtual void notifyRemoved(const ConstraintId& constraint) {}
	virtual void notifyExecuted(const ConstraintId& constraint) {}

	virtual void notifyDeactivated(const ConstrainedVariableId& variable) {}
	virtual void notifyActivated(const ConstrainedVariableId& variable) {}
	virtual void notifyAdded(const ConstrainedVariableId& variable) {}
	virtual void notifyRemoved(const ConstrainedVariableId& variable) {}
};
}

#endif /* PSCONSTRAINTENGINELISTENER_H_ */
