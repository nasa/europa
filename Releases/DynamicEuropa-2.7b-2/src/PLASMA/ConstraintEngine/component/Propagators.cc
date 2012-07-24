/*
 * Propagators.cc
 *
 *  Created on: Apr 23, 2009
 *      Author: javier
 */

#include "Propagators.hh"

#include "Constraint.hh"
#include "Constraints.hh"
#include "ConstraintEngine.hh"
#include "ConstrainedVariable.hh"
#include "Domains.hh"
#include "Debug.hh"

namespace EUROPA {

  DefaultPropagator::DefaultPropagator(const LabelStr& name, const ConstraintEngineId& constraintEngine)
    : Propagator(name, constraintEngine), m_activeConstraint(0){}

  void DefaultPropagator::handleConstraintAdded(const ConstraintId& constraint){
    debugMsg("DefaultPropagator:handleConstraintAdded", "Adding to the agenda: " << constraint->getName().toString() << "(" << constraint->getKey() << ")");
    m_agenda.insert(constraint);
  }

  void DefaultPropagator::handleConstraintRemoved(const ConstraintId& constraint){
    // Remove from agenda
    debugMsg("DefaultPropagator:handleConstraintRemoved", "Removing from the agenda: " << constraint->getName().toString() << "(" << constraint->getKey() << ")");
    m_agenda.erase(constraint);
    check_error(isValid());
  }

  void DefaultPropagator::handleConstraintActivated(const ConstraintId& constraint){
    debugMsg("DefaultPropagator:handleConstraintActivated", "Adding to the agenda: " << constraint->getName().toString() << "(" << constraint->getKey() << ")");
    m_agenda.insert(constraint);
    check_error(isValid());
  }

  void DefaultPropagator::handleConstraintDeactivated(const ConstraintId& constraint){
    // Remove from agenda
    debugMsg("DefaultPropagator:handleConstraintDeactivated", "Removing from the agenda: " << constraint->getName().toString() << "(" << constraint->getKey() << ")");
    m_agenda.erase(constraint);
    check_error(isValid());
  }

  void DefaultPropagator::handleNotification(const ConstrainedVariableId& variable,
					     int argIndex,
					     const ConstraintId& constraint,
					     const DomainListener::ChangeType& changeType){
    checkError(!constraint->isDiscarded(), constraint);
    if(constraint->getKey() != m_activeConstraint) {
      debugMsg("DefaultPropagator:handleNotification",
          "Adding to the agenda: " << constraint->getName().toString() << "(" << constraint->getKey() << ")"
          << " because of " << DomainListener::toString(changeType) << " change to " << variable->toString()
      );
      m_agenda.insert(constraint);
    }
  }

  void DefaultPropagator::execute(){
    checkError(!m_agenda.empty(), "Should never be calling this with an empty agenda.");
    check_error(!getConstraintEngine()->provenInconsistent());
    check_error(m_activeConstraint == 0);

    if(!getConstraintEngine()->provenInconsistent()){
      ConstraintSet::iterator it = m_agenda.begin();
      ConstraintId constraint = *it;
      m_agenda.erase(constraint);

      if(constraint->isActive()){
	m_activeConstraint = constraint->getKey();
	Propagator::execute(constraint);
      }
    }

    // If we can continue propagation despite the discovered inconsistency,
    // keep agenda for when the ConstraintEngine recovers and decides to resume propagation
    if(getConstraintEngine()->provenInconsistent()) {
        if (getConstraintEngine()->canContinuePropagation()) {
	        debugMsg("DefaultPropagator:agenda","CE was proven inconsistent, keeping agenda because propagation can continue later");
	        // TODO: should remove from the agenda any constraints associated with the empty variable, since it'll be relaxed and they'll ba added again
        }
        else {
            m_agenda.clear();
	        debugMsg("DefaultPropagator:agenda","Cleared agenda because CE was proven inconsistent");
        }
    }
    m_activeConstraint = 0;
  }

  bool DefaultPropagator::updateRequired() const{
    return (m_agenda.size() > 0);
  }

  bool DefaultPropagator::isValid() const{
    for(ConstraintSet::const_iterator it = m_agenda.begin(); it != m_agenda.end(); ++it){
      ConstraintId constraint = *it;
      checkError(constraint.isValid(), constraint);
      checkError(!constraint->isDiscarded(),
		 constraint->getName().toString() << "(" << constraint->getKey() << ") Id=" << constraint);
    }
    return true;
  }


  EqualityConstraintPropagator::EqualityConstraintPropagator(const LabelStr& name, const ConstraintEngineId& constraintEngine)
    : Propagator(name, constraintEngine), m_fullReprop(false), m_active(false){}

  EqualityConstraintPropagator::~EqualityConstraintPropagator(){}

  void EqualityConstraintPropagator::execute() {
    check_error(!m_active);
    m_active = true;
    if(m_fullReprop){
      m_eqClassCollection.getGraphKeys(m_eqClassAgenda);
      m_fullReprop = false;
    }

    // Now process the agenda
    for(std::set<int>::iterator it = m_eqClassAgenda.begin(); it != m_eqClassAgenda.end(); ++it){
      const std::set<ConstrainedVariableId>& eqClassScope = m_eqClassCollection.getGraphVariables(*it);
      equate(eqClassScope);
    }

    m_eqClassAgenda.clear();
    m_active = false;
  }

  bool EqualityConstraintPropagator::updateRequired() const {
    return (!m_eqClassAgenda.empty() || m_fullReprop);
  }

  void EqualityConstraintPropagator::handleConstraintAdded(const ConstraintId& constraint){
    check_error(!m_active);
    const ConstrainedVariableId& x = constraint->getScope()[0];
    const ConstrainedVariableId& y = constraint->getScope()[1];

    // Remove old equivalence classes for these variables from the agenda since they are abut to be merged.
    m_eqClassAgenda.erase(m_eqClassCollection.getGraphKey(x));
    m_eqClassAgenda.erase(m_eqClassCollection.getGraphKey(y));

    // Now add the merged equivalence class back to the agenda
    m_eqClassCollection.addConnection(x,y);
    m_eqClassAgenda.insert(m_eqClassCollection.getGraphKey(x));
  }

  void EqualityConstraintPropagator::handleConstraintRemoved(const ConstraintId& constraint){
    check_error(!m_active);
    const ConstrainedVariableId& x = constraint->getScope()[0];
    const ConstrainedVariableId& y = constraint->getScope()[1];
    m_eqClassCollection.removeConnection(x, y);
    m_fullReprop = true;
  }

  void EqualityConstraintPropagator::handleConstraintActivated(const ConstraintId& constraint){
    handleConstraintAdded(constraint);
  }

  void EqualityConstraintPropagator::handleConstraintDeactivated(const ConstraintId& constraint){
    handleConstraintRemoved(constraint);
  }

  void EqualityConstraintPropagator::handleNotification(const ConstrainedVariableId& variable,
                                                        int argIndex,
                                                        const ConstraintId& constraint,
                                                        const DomainListener::ChangeType& changeType){
    check_error(Id<EqualConstraint>::convertable(constraint));

    if(!m_fullReprop && !m_active){
      int eqClassKey = m_eqClassCollection.getGraphKey(variable);
      check_error(m_eqClassCollection.getGraphVariables(eqClassKey).size() > 0);
      check_error(m_eqClassCollection.getGraphVariables(eqClassKey).find(variable) != m_eqClassCollection.getGraphVariables(eqClassKey).end());
      m_eqClassAgenda.insert(eqClassKey);
    }
  }

  void processScope(const std::set<ConstrainedVariableId>& scope) {
    Domain& domain(EqualConstraint::getCurrentDomain(* (scope.begin())));

    if (domain.isOpen())
      return;

    // Set up the initial values to match others against.
    bool isFinite = domain.isFinite();

    // int domainType = domain.getType(); // Unused; see below.

    // Iterate over, restricting domain as we go.
    for (std::set<ConstrainedVariableId>::const_iterator it = scope.begin(); it != scope.end(); ++it) {
      Domain& currentDomain = EqualConstraint::getCurrentDomain(*it);

      // This next check is incorrect: could be false when they can be
      // compared (real enumerations and integer intervals, e.g.) and
      // could be true when they cannot be compared (e.g., two distinct
      // label sets, which both have a DomainType of USER_DEFINED).
      // The correct check follows.
      // --wedgingt@ptolemy.arc.nasa.gov 2004 Apr 21
      // check_error(currentDomain.getType() == domainType);

      check_error(Domain::canBeCompared(domain, currentDomain));

      // This will preclude possible propagation to the point of allowing
      // an arbitrary amount of useless search during planning, including
      // not even intersecting with closed domains later in the scope.
      // --wedgingt@ptolemy.arc.nasa.gov 2004 Apr 21
      if (currentDomain.isOpen())
        return;

      // What if the domains are (numeric) [-Inf Inf] and (integer) [0 1] ?
      // This would appear to incorrectly provoke an inconsistency in that case.
      // --wedgingt@ptolemy.arc.nasa.gov 2004 Apr 22
      if (currentDomain.isFinite() != isFinite ||
          domain.intersect(currentDomain) && domain.isEmpty()) {
        currentDomain.empty();
        return;
      }
    }

    // If we get to here, we have computed the new domain for all
    // variables in the scope and we know that no domain has been
    // emptied (this could be optimized by recording the last change
    // to domain).
    for (std::set<ConstrainedVariableId>::const_iterator it = scope.begin(); it != scope.end(); ++it) {
      Domain& currentDomain = EqualConstraint::getCurrentDomain(*it);
      currentDomain.intersect(domain);
    }
  }

  void EqualityConstraintPropagator::equate(const std::set<ConstrainedVariableId>& scope) {
    check_error(!scope.empty());
    processScope(scope);
  }

}
