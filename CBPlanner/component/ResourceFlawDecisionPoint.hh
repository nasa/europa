#ifndef _H_ResourceFlawDecisionPoint
#define _H_ResourceFlawDecisionPoint

#include "CBPlannerDefs.hh"
#include "DecisionPoint.hh"
#include "ResourceDefs.hh"

namespace Prototype {

  class ResourceFlawDecisionPoint : public DecisionPoint {
  public:
    virtual ~ResourceFlawDecisionPoint();

    const bool assign(const ChoiceId&);
    const bool retract();
    virtual std::list<ChoiceId>& getChoices();
    virtual std::list<ChoiceId>& getUpdatedChoices();
    const ResourceId& getResource() const { return m_resource; }

    void print(std::ostream& os) const;
  private:
    friend class ResourceOpenDecisionManager;

    ResourceFlawDecisionPoint(const ResourceId&);

    ResourceId m_resource;
  };

  std::ostream& operator <<(std::ostream& os, const Id<ResourceFlawDecisionPoint>&);

}
#endif
