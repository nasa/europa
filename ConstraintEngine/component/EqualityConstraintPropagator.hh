#ifndef _H_EqualityConstraintPropagator
#define _H_EqualityConstraintPropagator

#include "Propagator.hh"
#include "EquivalenceClassCollection.hh"
#include <map>
#include <set>

namespace Prototype{

  class EqualityConstraintPropagator: public Propagator {
  public:
    EqualityConstraintPropagator(const ConstraintEngineId& constraintEngine);
    ~EqualityConstraintPropagator();
    void execute();
    bool updateRequired() const;

  private:
    void handleConstraintAdded(const ConstraintId& constraint);
    void handleConstraintRemoved(const ConstraintId& constraint);
    void handleNotification(const ConstrainedVariableId& variable, 
			    int argIndex, 
			    const ConstraintId& constraint, 
			    const DomainListener::ChangeType& changeType);
    bool isAcceptable(const ConstraintId& constraint) const;
    void equate(const std::set<ConstrainedVariableId>& scope);
    bool m_fullReprop;
    bool m_active;
    EquivalenceClassCollection m_eqClassCollection;
    std::set<int> m_eqClassAgenda;
  };

}
#endif
