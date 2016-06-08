#ifndef H_ProfilePropagator
#define H_ProfilePropagator

#include "Propagators.hh"
#include "ConstraintEngineDefs.hh"
#include "PlanDatabaseDefs.hh"
#include "ResourceDefs.hh"

namespace EUROPA {
class ProfilePropagator : public DefaultPropagator {
private:
  ProfilePropagator(const ProfilePropagator&);
  ProfilePropagator& operator=(const ProfilePropagator&);
 public:
  ProfilePropagator(const std::string& name,
                    const ConstraintEngineId constraintEngine);

  virtual ~ProfilePropagator();

  // Batch mode delays all propagation until Batch mode is exited
  virtual void enterBatchMode();
  virtual void exitBatchMode();
  virtual bool inBatchMode() const { return m_inBatchMode; }

  static const std::string& PROPAGATOR_NAME() {
    static const std::string sl_const("Resource");
    return sl_const;
  }
  void addProfile(const ProfileId profile);
  void removeProfile(const ProfileId profile);
 protected:
  friend class Profile;
  void setUpdateRequired(const bool update) {m_updateRequired = update;}
 private:
  void execute();
  void execute(const ConstraintId constraint);
  bool updateRequired() const;
  void handleConstraintAdded(const ConstraintId constraint);
  void handleConstraintRemoved(const ConstraintId constraint);

  std::set<ProfileId> m_profiles;
  std::set<ConstraintId> m_newConstraints;
  bool m_updateRequired;
  bool m_inBatchMode;
  ConstraintEngineListener* m_batchListener;
};
}

#endif
