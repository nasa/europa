#ifndef _H_ResourceOpenDecisionManager
#define _H_ResourceOpenDecisionManager

#include "CBPlannerDefs.hh"
#include "ObjectDecisionPoint.hh"
#include "TokenDecisionPoint.hh"
#include "ConstrainedVariableDecisionPoint.hh"
#include "ResourceFlawDecisionPoint.hh"
#include "ResourceDefs.hh"
#include "DefaultOpenDecisionManager.hh"

/**
 *  Reuse as much of DefaultOpenDecisionManager as possible
 */

namespace EUROPA {

  class ResourceOpenDecisionManager : public DefaultOpenDecisionManager {
  public:

    ResourceOpenDecisionManager(const DecisionManagerId& dm);
    // Destructor is not virtual
    ~ResourceOpenDecisionManager();

    // Overriding
    virtual DecisionPointId getNextDecision();
    virtual const int getNumberOfDecisions();

    virtual void getOpenDecisions(std::list<DecisionPointId>& decisions);
    virtual void printOpenDecisions(std::ostream& os = std::cout);

  protected:
    friend class DecisionManager;
    friend class DMResourceListener;

    // Overriding from DefaultOpenDecisionManager
    virtual void add(const ObjectId& object);

    void notifyResourceState( const DMResourceListenerId& rl, bool isFlawed );

    // Decision cache for ResourceFlaws, other caches are inherited from DODM
    // At any given moment, there is at most one open RFDP for each Resource.
    // There may be many closed ones, but we do not care.
    std::map<int,ResourceFlawDecisionPointId> m_resDecs;
  };

}

#endif
