#ifndef _H_DefaultPropagator
#define _H_DefaultPropagator

#include "Propagator.hh"
#include <set>

namespace EUROPA {

  class DefaultPropagator: public Propagator
  {
  public:
    DefaultPropagator(const LabelStr& name, const ConstraintEngineId& constraintEngine);
    virtual void execute();
    virtual bool updateRequired() const;
  protected:
    virtual void handleConstraintAdded(const ConstraintId& constrain);
    virtual void handleConstraintRemoved(const ConstraintId& constraint);
    virtual void handleConstraintActivated(const ConstraintId& constrain);
    virtual void handleConstraintDeactivated(const ConstraintId& constraint);
    virtual void handleNotification(const ConstrainedVariableId& variable, 
				    int argIndex, 
				    const ConstraintId& constraint, 
				    const DomainListener::ChangeType& changeType);

    ConstraintSet m_agenda;

    int m_activeConstraint;
  private:
    bool isValid() const;
  };
}
#endif
