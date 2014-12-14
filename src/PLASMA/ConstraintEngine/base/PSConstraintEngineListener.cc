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
void PSConstraintEngineListener::notifyViolationAdded(const ConstraintId constraint){
  notifyViolationAdded(id_cast<PSConstraint>(constraint));
}

void PSConstraintEngineListener::notifyViolationRemoved(const ConstraintId constraint){
  notifyViolationRemoved(id_cast<PSConstraint>(constraint));
}

void PSConstraintEngineListener::notifyChanged(const ConstrainedVariableId variable,
                                               const DomainListener::ChangeType& changeType) {
  notifyChanged(id_cast<PSVariable>(variable), static_cast<PSChangeType>(changeType));
}

void PSConstraintEngineListener::notifyViolationAdded(PSConstraint*) {}
void PSConstraintEngineListener::notifyViolationRemoved(PSConstraint*) {}
void PSConstraintEngineListener::notifyChanged(PSVariable*, PSChangeType){}

void PSConstraintEngineListener::notifyAdded(const ConstraintId) {}
void PSConstraintEngineListener::notifyActivated(const ConstraintId) {}
void PSConstraintEngineListener::notifyDeactivated(const ConstraintId) {}
void PSConstraintEngineListener::notifyRemoved(const ConstraintId) {}
void PSConstraintEngineListener::notifyExecuted(const ConstraintId) {}

void PSConstraintEngineListener::notifyDeactivated(const ConstrainedVariableId) {}
void PSConstraintEngineListener::notifyActivated(const ConstrainedVariableId) {}
void PSConstraintEngineListener::notifyAdded(const ConstrainedVariableId) {}
void PSConstraintEngineListener::notifyRemoved(const ConstrainedVariableId) {}

}


