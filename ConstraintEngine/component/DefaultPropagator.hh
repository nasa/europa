#ifndef _H_DefaultPropagator
#define _H_DefaultPropagator

#include "Propagator.hh"
#include <list>

namespace Prototype {

  class DefaultPropagator: public Propagator
  {
  public:
    DefaultPropagator(const ConstraintEngineId& constraintEngine);
    void execute();
    bool updateRequired() const;
  protected:
    void handleConstraintAdded(const ConstraintId& constrain);
    void handleConstraintRemoved(const ConstraintId& constraint);
    void handleNotification(const ConstrainedVariableId& variable, 
			    int argIndex, 
			    const ConstraintId& constraint, 
			    const DomainListener::ChangeType& changeType);
    bool isAcceptable(const ConstraintId& constraint) const;

  private:
    std::list<ConstraintId> m_agenda;
  };
}
#endif
