#ifndef _H_ResourcePropagator
#define _H_ResourcePropagator

#include "../ConstraintEngine/Propagator.hh"
#include "PlanDatabaseDefs.hh"
#include "PlanDatabaseListener.hh"
#include "ResourceDefs.hh"
#include <set>

namespace Prototype {

  namespace ResourceProp {

    class DbResourceConnector : public PlanDatabaseListener {

    private:
      friend class ResourcePropagator;

      DbResourceConnector(const PlanDatabaseId& planDatabase, const ResourcePropagatorId& resourcePropagator);
      void notifyConstrained(const ObjectId& object, const TokenId& token, const TokenId& successor);
      const ResourcePropagatorId m_resourcePropagator;
    };
  }

  using namespace ResourceProp;


  class ResourcePropagator: public Propagator
  {
  public:
    ResourcePropagator(const LabelStr& name, const ConstraintEngineId& constraintEngine, const PlanDatabaseId& planDatabase);
    virtual ~ResourcePropagator();
    void execute();
    bool updateRequired() const;

  protected:
    void handleConstraintAdded(const ConstraintId& constraint) {}
    void handleConstraintRemoved(const ConstraintId& constraint) {}
    void handleConstraintActivated(const ConstraintId& constrain) {}
    void handleConstraintDeactivated(const ConstraintId& constraint) {}
    void handleNotification(const ConstrainedVariableId& variable, 
    			    int argIndex, 
    			    const ConstraintId& constraint, 
    			    const DomainListener::ChangeType& changeType);

    friend class DbResourceConnector;
    void handleResourcePropagation(const ResourceId& r, const ConstrainedVariableId& variable);


  private:
    
    void handleObjectChange(const ConstrainedVariableId& variable);
    void handleQuantityChange(const ConstrainedVariableId& variable, 
    			    int argIndex, 
    			    const ConstraintId& constraint, 
    			    const DomainListener::ChangeType& changeType);
    void handleTimeChange(const ConstrainedVariableId& variable, 
    			    int argIndex, 
    			    const ConstraintId& constraint, 
    			    const DomainListener::ChangeType& changeType);

    std::set<ResourceId> m_resources;
    ConstrainedVariableId m_forempty;
    const PlanDatabaseId m_planDb;
    PlanDatabaseListenerId m_planDbListener;
  };
}
#endif
