#include "PSConstraintEngineListener.hh"
#include "Constraint.hh"
#include "ConstrainedVariable.hh"

/*
 * PSConstraintEngineListener.cpp
 *
 *  Created on: Aug 5, 2008
 *      Author: tsmith
 */

namespace EUROPA {

// Methods to convert notifications involving internal Europa types to notifications involving 'PS' types:
void PSConstraintEngineListener::notifyViolationAdded(const ConstraintId& constraint){
	notifyViolationAdded((PSConstraint *) constraint);
}

void PSConstraintEngineListener::notifyViolationRemoved(const ConstraintId& constraint){
	notifyViolationRemoved((PSConstraint *) constraint);
}
}
