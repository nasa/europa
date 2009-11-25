#ifndef _H_ProfilePropagator
#define _H_ProfilePropagator

#include "Propagators.hh"
#include "ConstraintEngineDefs.hh"
#include "PlanDatabaseDefs.hh"
#include "ResourceDefs.hh"

namespace EUROPA {
    class ProfilePropagator : public DefaultPropagator {
    public:
      ProfilePropagator(const LabelStr& name,
			const ConstraintEngineId& constraintEngine);
    protected:
      friend class Profile;
      void setUpdateRequired(const bool update) {m_updateRequired = update;}
    private:
      void execute();
      void execute(const ConstraintId& constraint);
      bool updateRequired() const;
      void handleConstraintAdded(const ConstraintId& constraint);
      void handleConstraintRemoved(const ConstraintId& constraint);

      std::set<ProfileId> m_profiles;
      std::set<ConstraintId> m_newConstraints;
      bool m_updateRequired;
    };
}

#endif
