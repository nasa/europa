#ifndef _H_ResourceFlawDecisionPoint
#define _H_ResourceFlawDecisionPoint

#include "ResourceDefs.hh"
#include "DecisionPoint.hh"
#include "Resource.hh"
#include <vector>

namespace EUROPA {

  class ResourceFlawDecisionPoint : public DecisionPoint {
  public:
    virtual ~ResourceFlawDecisionPoint();

    const bool assign();
    const bool retract();
    const bool hasRemainingChoices();
    void initializeChoices(); 

    const ResourceId& getResource() const { return m_resource; }

    void print(std::ostream& os) const;

  protected:
    ResourceFlawDecisionPoint(const ResourceId&);
  private:
    friend class ResourceOpenDecisionManager;
    
    std::vector<std::pair<TransactionId, TransactionId> > m_choices;
    unsigned int m_choiceIndex;

    ResourceId m_resource;
  };

  std::ostream& operator <<(std::ostream& os, const Id<ResourceFlawDecisionPoint>&);

}
#endif
