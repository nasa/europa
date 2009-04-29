#ifndef H_DecisionPoint
#define H_DecisionPoint

#include "SolverDefs.hh"
#include "MatchingRule.hh"

/**
 * @author Conor McGann
 * @date March, 2005
 */

namespace EUROPA {
  namespace SOLVERS {

    class DecisionPoint;

    /**
     * @brief Primary data element used by the solver for making and retracting decisions.
     *
     * Structure to hold details of decision made. This is a stateful command pattern,
     * where we log state of executed commands, ensure the same command is not repeated, and
     * provide an iteration capability through the set of possible choices. Here we have a pattern of
     * implementations of key functions in the base class to ensure that the basic state expectations
     * are enforced e.g. initialized, executed. This also allows us to provide instrumentation in the base class
     * where derived classes need only provide 'toString'.
     * @see FlawManager, Solver
     * @note Extends Entity to take advantage of key based identities that will be usable across solver runs.
     */
    class DecisionPoint: public Entity {
    public:
      virtual ~DecisionPoint();

      const DecisionPointId& getId() const;

      /**
       * @brief Does initialization of choices. This is an explicit call since we may wish to defer actual population
       * of choices till we really need them.
       */
      void initialize();

      /**
       * @brief Specialize output of decision state information
       */
      virtual std::string toString() const = 0;


      /**
       * @brief short version of toString() to be used in aggregate summaries
       */
      virtual std::string toShortString() const = 0;

      /**
       * @brief Hook for default behavior. Subclasses can make new static methods to specialize the static matching.
       * @see FlawHandler
       * @return true if the entity can be dynamically matched. Otherwise false.
       */
      static bool customStaticMatch(const EntityId& entity) {return true;}

      /**
       * @brief Hook for default behavior. Will allow subclasses to add a weighting 
       * @see FlawHandler
       */
      static unsigned int customStaticFilterCount() {return 0;}

      ContextId getContext() const {return m_context;}

      void setContext(const ContextId& ctx) {m_context = ctx;}

      void setCutoff(unsigned int maxChoices) {m_maxChoices = maxChoices;}

      const unsigned int getFlawedEntityKey() {return m_entityKey;}
    protected:
      DecisionPoint(const DbClientId& client, unsigned int entityKey, const LabelStr& explanation);

      friend class Solver; /*!< grants special access to execute and undo methods */

      /**
       * @brief Tests if execute has been called more times than undo.
       */
      bool isExecuted() const;

      /**
       * @brief Tests if the choices for this decision have been initialized yet.
       */
      bool isInitialized() const;

      /**
       * @brief Tests if the decision can be retracted. Designed for cases where the database may have been independently
       * relaxed.
       * @see m_entityKey, isExecuted
       * @note To support this properly, the key of the flawed entity must be retained.
       */
      virtual bool canUndo() const;

      /**
       * @brief Main accessor for the Solver to execute current choice.
       * @see handleExecute
       */
      void execute();

      /**
       * @brief Main accessor for the Solver to retract current choice.
       * @see handleUndo
       */
      void undo();

      /**
       * @brief Test to see if choices should be cut. This will supercede 'hasNext' which can
       * be specialized in a sub-class. Employs a test of number of choices made vs. maxChoices allowed
       */
      bool cut() const;

      /**
       * @brief Implement this method to construct the set of choices in the
       * required order on demand.
       * @see initialize
       */
      virtual void handleInitialize() = 0;

      /**
       * @brief Determines if there are any remaining choices to make.
       * @return true if there is another, otherwise false.
       */
      virtual bool hasNext() const = 0;

      /**
       * @brief Implement this method with behavior for making an update to the Plan
       * Database reflecting the current choice.
       * @param client The databasse client on which to invoke decision execution operations.
       */
      virtual void handleExecute() = 0;

      /**
       * @brief Implement this method to handle retraction of last choice executed.
       * @param client The databasse client on which to invoke decision retraction operations.
       */
      virtual void handleUndo() = 0;

      const DbClientId m_client;
      const unsigned int m_entityKey; /*!< The Key of underlying flawed entity. Store instead of ID so we can test it. */

      /**
       * @brief Get the justification behind the selection of this decision point
       */
      const LabelStr& getExplanation() {return m_explanation;}
    private:
      DecisionPointId m_id;
      LabelStr m_explanation;
      bool m_isExecuted; /*!< True if executed has been called, and undo has not */
      bool m_initialized; /*!< True if choices have been set up. Otherwise false.*/
      ContextId m_context;
      unsigned int m_maxChoices; /*!< Set to bound number of choices */
      unsigned int m_counter; /*!< Increment on execution */
    };
  }
}

#endif
