#ifndef _H_STNTemporalAdvisor
#define _H_STNTemporalAdvisor

#include "PlanDatabaseDefs.hh"
#include "DefaultTemporalAdvisor.hh"
#include "TemporalNetworkDefs.hh"

namespace Prototype{

  class STNTemporalAdvisor: public DefaultTemporalAdvisor {

  public:

    STNTemporalAdvisor(const TemporalPropagatorId& propagator);
    virtual ~STNTemporalAdvisor();

    virtual bool canPrecede(const TokenId& first, const TokenId& second);
    virtual bool canFitBetween(const TokenId& token, const TokenId& predecessor,
			       const TokenId& successor);
    virtual bool canBeConcurrent(const TokenId& first, const TokenId& second);

  private:
    TemporalPropagatorId m_propagator;

  };

}

#endif
