#ifndef H_SearchListener
#define H_SearchListener

/**
 * @file SearchListener.hh
 * @author Michael Iatauro
 * @date June, 2005
 * @brief Defines a class for listening to the search process.
 * @ingroup Solvers
 */

#include "Id.hh"
#include "SolverDecisionPoint.hh"

namespace EUROPA {
  namespace SOLVERS {
    class SearchListener;
    typedef Id<SearchListener> SearchListenerId;

    /**
     * @brief Defines a base class for listening for events during the search process.
     *
     * Such events include, but are not limited to, creating/removing/undoing decision points,
     * successfully/unsuccessfully applying a decision to a plan, finding a plan,
     * exhausting the search, or timing out.
     *
     * @see Solver, DecisionPoint
     */

    class SearchListener {
    public:
      /**
       * @brief Constructor
       */
      SearchListener() : m_id(this) {};

      /**
       * @brief Required for safe deletion of subclasses.
       */
      virtual ~SearchListener() {m_id.remove();};

      /**
       * @brief Get the Id for this SearchListener instance.
       */
      SearchListenerId& getId() {return m_id;};

      /**
       * @brief Notify that a new decision point was created.
       * @param dp The new decision point.
       */
      virtual void notifyCreated(DecisionPointId& dp) {};

      /**
       * @brief Notify that a decision point was removed for some reason (i.e. backtracked over).
       * @param dp The removed decision point.
       */
      virtual void notifyDeleted(DecisionPointId& dp) {};

      /**
       * @brief Notify that a decision point was undone (the decision was retracted).
       * @param dp The retracted decision.
       */
      virtual void notifyUndone(DecisionPointId& dp) {};

      /**
       * @brief Notify of a successful step (a decision followed by constraint propagation to quiescence).
       */
      virtual void notifyStepSucceeded(DecisionPointId& dp) {};

      /**
       * @brief Notify of a failed step (a decision led to an inconsistency).
       */
      virtual void notifyStepFailed(DecisionPointId& dp) {};

      /**
       * @brief Notify of a successful retraction.
       */
      virtual void notifyRetractSucceeded(DecisionPointId& dp) {};

      /**
       * @brief Notify of a retraction that requires further retraction
       */
      virtual void notifyRetractNotDone(DecisionPointId& dp) {};

      /**
       * @brief Notify of a completed search (a consistent plan was found).
       */
      virtual void notifyCompleted() {};

      /**
       * @brief Notify of a failed search (the search space was exhausted without finding a plan).
       */
      virtual void notifyExhausted() {};

      /**
       * @brief Notify of a failed search (the search took more steps than was allowed).
       */
      virtual void notifyTimedOut() {};
    protected:
    private:
      SearchListenerId m_id;
    };
  }
}

#endif
