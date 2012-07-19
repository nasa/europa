#ifndef _H_DefaultTemporalAdvisor
#define _H_DefaultTemporalAdvisor

#include "PlanDatabaseDefs.hh"
#include "TemporalAdvisor.hh"

namespace EUROPA{
 /**
   * @class DefaultTemporalAdvisor
   * @brief Implementation of TemporalAdvisor
   * @ingroup PlanDatabase
   */
  class DefaultTemporalAdvisor: public TemporalAdvisor {

  public:

    DefaultTemporalAdvisor(const ConstraintEngineId& ce);
    virtual ~DefaultTemporalAdvisor();

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
                                          std::vector<int>& lbs,
                                          std::vector<int>& ubs);
    virtual unsigned int mostRecentRepropagation() const;

    const TemporalAdvisorId& getId() const;
  protected:
    TemporalAdvisorId m_id;
    const ConstraintEngineId m_ce;
  };

}

#endif
