#ifndef _H_ResourceFlawChoice
#define _H_ResourceFlawChoice

#include "ResourceDefs.hh"
#include "ResourceFlawDecisionPoint.hh"
#include "CBPlannerDefs.hh"
#include "Choice.hh"

namespace PLASMA {

  class ResourceFlawChoice : public Choice {
  public:
    virtual ~ResourceFlawChoice();
    
    const TransactionId& getBefore() const { return m_before; }
    const TransactionId& getAfter() const { return m_after; }

    bool operator==(const Choice& choice) const;
    void print(std::ostream& os) const;

    double getValue() const;

    // Order producer before consumer
    ResourceFlawChoice( const ResourceFlawDecisionPointId&, 
			const TransactionId& before, const TransactionId& after);
    // Push consumer beyond horizon
    ResourceFlawChoice( const ResourceFlawDecisionPointId&, 
			const TransactionId& consumer );
  private:
    TransactionId m_before;	/**< Producer or noId for horizon */
    TransactionId m_after;	/**< Consumer */
  };

}
#endif 
