#ifndef _H_TemporalAdvisor
#define _H_TemporalAdvisor

#include "PlanDatabaseDefs.hh"

namespace EUROPA{

  /**
   * @brief An abstract interface for inquiring about possible
   * temporal relations among tokens. It is a vital source of look-ahead information when formulating
   * @author Sailesh Ramakrishan
   * merging and ordering choices.
   * @ingroup PlanDatabase
   */

  class TemporalAdvisor{

  public:
    /**
     * @brief Test if the first token can precede the second token.
     * @param first Candidate to be predecessor
     * @param second Candidate to be successor
     * @return true if temporal distance first.end to second.start can be >= 0
     */
    virtual bool canPrecede(const TokenId& first, const TokenId& second) = 0;

    virtual bool canPrecede(const TimeVarId& first, const TimeVarId& second) = 0;

    /**
     * @brief test if the given token can fit between the predecessor and successor.
     * @param token The token to be tested if it can fit in the middle
     * @param predecessor The token to be placed before 'token'
     * @param successor The token to be placed after 'token'
     * @return True if sumultaneously the temporal distance between predecessor.end and successor.start >= minimum token.duration and
     * canPrecede(predecessor, token) and canPrecede(token, successor)
     */
    virtual bool canFitBetween(const TokenId& token, const TokenId& predecessor,
			       const TokenId& successor) = 0;

    /**
     * @brief test of the given tokens can have a zero temporal distance between their respective timepoints. Particularly
     * useful as a look-ahead when evaluating merge candidates.
     * @param first A token to consider
     * @param second A token to consider
     * @return true if distance beteen start times includes 0 and distance between end times includes 0.
     */
    virtual bool canBeConcurrent(const TokenId& first, const TokenId& second) = 0;


    /**
     * @brief General utility for obtaining the min and max temporal distance between two timepoints.
     * @param first The first time point
     * @param second The second time point
     * @param exact if true, it will enforce most rigourous test and give tightest bounds. If false, it can use
     * previously calcuated results but may be quite wrong.
     */
    virtual const IntervalIntDomain getTemporalDistanceDomain(const TimeVarId& first, 
							      const TimeVarId& second,
							      const bool exact) = 0;

     /**
     * @brief Obtains exact min/max temporal distance between one and several timepoints.
     * @param first The first time point
     * @param seconds The other time points
     * @param domains The returned calculated domains.
     */
    virtual void getTemporalDistanceDomains(const ConstrainedVariableId& first,
                                            const std::vector<ConstrainedVariableId>&
                                            seconds,
                                            std::vector<IntervalIntDomain>& domains) = 0;

    /**
     * @brief Obtains min/max temporal distance signs between one and several timepoints.  Only the signs (-,+,0)
     * are guaranteed accurate; the values may be arbitrary.  Utility for determining precedence relations.
     * @param first The first time point
     * @param seconds The other time points
     * @param lbs The returned lower-bound signs as numbers with correct signs but arbitrary values.
     * @param ubs The returned upper-bound signs as numbers with correct signs but arbitrary values.
     */
    virtual void getTemporalDistanceSigns(const ConstrainedVariableId& first,
                                          const std::vector<ConstrainedVariableId>&
                                          seconds,
                                          std::vector<int>& lbs,
                                          std::vector<int>& ubs) = 0;

    /**
     * @brief Obtains the most recent repropagation of relevant information w.r.t. time
     */
    virtual unsigned int mostRecentRepropagation() const = 0;

    virtual ~TemporalAdvisor() {};
  };

}

#endif
 
