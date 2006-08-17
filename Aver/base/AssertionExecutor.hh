#ifndef _H_AssertionExecutor
#define _H_AssertionExecutor

/**
 * @file   AssertionExecutor.hh
 * @author Michael Iatauro <miatauro@email.arc.nasa.gov>
 * @brief A class for managing the execution of Assertions.
 * @date   Wed Jan 12 14:53:54 2005
 * @ingroup Aver
 */

#include "AbstractDomain.hh"
#include "AggregateListener.hh"
#include "AverDefs.hh"
#include "EventAggregator.hh"
#include "AverHelper.hh"
#include "tinyxml.h"
#include <list>

namespace EUROPA {

  /**
   * @class AssertionExecutor
   * @brief Manages the execution of Assertions.
   *
   * In Aver, every assertion must have a specification of the set of planning "steps" at which it must be true.  This specification is
   * itself a boolean assertion.  The AssertionExecutor represents that Assertion.  At each step, the AssertionExecutors evaluate the step
   * assertion to see if their constituent Assertions need to be executed.  There is one AssertionExecutor per step assertion--two
   * Assertions that need to be executed on step 2 are managed by the same AssertionExecutor.
   */
  class AssertionExecutor : public AggregateListener {
  public:

    /**
     * @brief Constructor.  The passed-in step assertion is reduced to a boolean and a domain.
     * @param step a pointer to the XML representing the step assertion.
     */
    AssertionExecutor(const TiXmlElement* step);

    /**
     * @brief Destructor.
     */
    ~AssertionExecutor();

    /**
     * @brief Returns the Id for this AssertionExecutor.
     * @return the Id for this AssertionExecutor.
     */
    AssertionExecutorId getId() const {return (AssertionExecutorId) AggregateListener::getId();}

    /**
     * @brief Adds an Assertion to be executed by this Executor.
     * @param assn the Id of the Assertion to be executed.
     */
    void add(AssertionId assn) {m_assertions.push_back(assn);}

    /**
     * @brief Called by the EventAggregator to signal that a step has completed.  This causes the step assertion to execute and if it
     * passes, the Assertions associated with this Executor are executed.
     * @param dec ignored.
     */
    void notifyStep(const SOLVERS::DecisionPointId& dec);
  protected:
  private:

    /**
     * @brief Executes the step assertion and returns its truth or falsehood
     * @return the result of the step assertion.
     */
    bool checkExecute();

    bool m_executed; /*!< In the case of a step assertion being qualified with "any", "first", or "last", this is used to short-circuit
                      execution of the step assertion if the Assertions have been executed once.*/
    //AssertionExecutorId m_id;
    std::list<AssertionId> m_assertions; /*!< The list of Assertions associated with this Executor. */
    std::string m_qualifier; /*!< One of "any", "all", "each", "first", or "last", this value can be used for short-circuiting the
                              step assertion. */
    std::string m_operator; /*!< The boolean operator in the step assertion. */
    DomainContainer* m_stepDom; /*!< The domain defined in the step assertion which is compared to the current step using the given 
                                 operator.*/
  };
}

#endif
