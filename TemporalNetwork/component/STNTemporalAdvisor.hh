#ifndef _H_STNTemporalAdvisor
#define _H_STNTemporalAdvisor

#include "PlanDatabaseDefs.hh"
#include "TemporalAdvisor.hh"

namespace Prototype{

  class STNTemporalAdvisor: public TemporalAdvisor {

  public:

    STNTemporalAdvisor(const PropagatorId& propagator);
    virtual ~STNTemporalAdvisor();

    virtual bool canPrecede(const TokenId& first, const TokenId& second);
    virtual bool canFitBetween(const TokenId& token, const TokenId& predecessor,
			       const TokenId& successor);

    const TemporalAdvisorId& getId() const;
  private:
    TemporalAdvisorId m_id;
    PropagatorId m_propagator;

  };

}

#endif
