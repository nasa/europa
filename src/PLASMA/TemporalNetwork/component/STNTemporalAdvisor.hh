#ifndef _H_STNTemporalAdvisor
#define _H_STNTemporalAdvisor

#include "PlanDatabaseDefs.hh"
#include "DefaultTemporalAdvisor.hh"
#include "TemporalNetworkDefs.hh"

namespace EUROPA{

 /**
   * @class STNTemporalAdvisor
   * @brief Implementation of DefaultTemporalAdvisor for @ref stp "Simple Temporal Networks". Class supports
   * the quering of temporal relations between tokens.
   *
   * Specializes methods for properties of STNs.
   * @ingroup TemporalNetwork
   */
  class STNTemporalAdvisor: public DefaultTemporalAdvisor {

  public:

    STNTemporalAdvisor(const TemporalPropagatorId& propagator);
    virtual ~STNTemporalAdvisor();

    virtual bool canPrecede(const TokenId& first, const TokenId& second);
    virtual bool canPrecede(const TimeVarId& first, const TimeVarId& second);
    virtual bool canFitBetween(const TokenId& token, const TokenId& predecessor,
			       const TokenId& successor);
    virtual bool canBeConcurrent(const TokenId& first, const TokenId& second);
    virtual const IntervalIntDomain getTemporalDistanceDomain(const TimeVarId& first, 
							      const TimeVarId& second,
							      const bool exact);

    virtual void getTemporalDistanceDomains(const ConstrainedVariableId& first,
                                            const std::vector<ConstrainedVariableId>&
                                            seconds,
                                            std::vector<IntervalIntDomain>& domains);
    virtual void getTemporalDistanceSigns(const ConstrainedVariableId& first,
                                          const std::vector<ConstrainedVariableId>&
                                          seconds,
                                          std::vector<Time>& lbs,
                                          std::vector<Time>& ubs);

    unsigned int mostRecentRepropagation() const;
  private:
    TemporalPropagatorId m_propagator;




  };

}

#endif
