#include "SAVH_ProfilePropagator.hh"
#include "SAVH_Profile.hh"
#include "Constraint.hh"
#include "ConstraintEngine.hh"
#include "Debug.hh"

namespace EUROPA {
  namespace SAVH {
    ProfilePropagator::ProfilePropagator(const LabelStr& name,
					 const ConstraintEngineId& constraintEngine)
      : DefaultPropagator(name, constraintEngine), m_updateRequired(false) {}

    void ProfilePropagator::handleConstraintAdded(const ConstraintId& constraint) {
      check_error(constraint.isValid());
      if(constraint->getName() == Profile::VariableListener::CONSTRAINT_NAME()) {
	m_newConstraints.insert(constraint);
	m_updateRequired = true;
      }
      DefaultPropagator::handleConstraintAdded(constraint);
    }

    void ProfilePropagator::handleConstraintRemoved(const ConstraintId& constraint) {
      check_error(constraint.isValid());
      if(m_newConstraints.erase(constraint) > 0)
	m_updateRequired = true;
      DefaultPropagator::handleConstraintRemoved(constraint);
    }

    void ProfilePropagator::execute() {
      debugMsg("ProfilePropagator:execute", "Executing propagator.");
      check_error(!getConstraintEngine()->provenInconsistent());
      check_error(m_activeConstraint == 0);

      while(!m_agenda.empty()) {
	ConstraintSet::iterator it = m_agenda.begin();
	ConstraintId constraint = *it;
	m_agenda.erase(constraint);
	if(constraint->isActive()) {
	  m_activeConstraint = constraint->getKey();
	  execute(constraint);
	}
	if(getConstraintEngine()->provenInconsistent())
	  m_agenda.clear();
      }

      m_activeConstraint = 0;

      for(std::set<ConstraintId>::const_iterator it = m_newConstraints.begin(); it != m_newConstraints.end(); ++it) {
	ConstraintId constraint = *it;
	check_error(constraint.isValid());
	check_error(Id<Profile::VariableListener>::convertable(constraint));
	debugMsg("ProfilePropagator:execute", "Handling addition of constraint " << constraint->toString());
	Profile::VariableListener* listener = (Profile::VariableListener*) constraint;
	check_error(listener != NULL);
	check_error(listener->getProfile().isValid());
	debugMsg("ProfilePropagator:execute", "Adding profile " << listener->getProfile());
	m_profiles.insert(listener->getProfile());
      }


      for(std::set<ProfileId>::iterator it = m_profiles.begin(); it != m_profiles.end(); ++it) {
	ProfileId profile = *it;
	check_error(profile.isValid());
	if( !getConstraintEngine()->provenInconsistent() 
	    && 
	    profile->needsRecompute()) {
	  debugMsg("ProfilePropagator:execute", "Recomputing profile " << profile);
	  profile->recompute();
	}
      }
      m_updateRequired = false;
    }

    void ProfilePropagator::execute(const ConstraintId& constraint) {
      Propagator::execute(constraint);
      if(constraint->getName() == Profile::VariableListener::CONSTRAINT_NAME()) {
	Profile::VariableListener* listener = (Profile::VariableListener*) constraint;
	if(listener->getProfile()->needsRecompute())
	  m_updateRequired = true;
      }
    }

    bool ProfilePropagator::updateRequired() const {
      return DefaultPropagator::updateRequired() || m_updateRequired;
    }
  }
}
