#ifndef H_UnboundVariableDecisionPoint
#define H_UnboundVariableDecisionPoint

#include "SolverDecisionPoint.hh"
#include "ConstrainedVariable.hh"

/**
 * @brief Provides class declarations for handling variable flaws. Includes classes for min, max and random orderings.
 * @author Conor McGann
 */
namespace EUROPA {

  namespace SOLVERS {

    class ValueSource;

    /**
     * @brief Abstract base class for Variable decisions.
     *
     * This class adds the particulars of variable binding and unbinding, but
     * leaves all details of how choices are stored and how they are iterated through to
     * derived classes, thus allowing for specialized storage and selection.
     */
    class UnboundVariableDecisionPoint: public DecisionPoint {
    public:

      /**
       * @brief Utility to obtain the next decision point from the set of candidates if it can exceed the
       * given best priority.
       * @param flawCandidates A set of unbound variables to choose from
       * @param guardCache The set of guard variables, each with a reference to the number of guards posted on it.
       * @param bestPriority A mutable current best priority. If a new decision point is created, the new
       * bestPriority will be updated to the priority of the new decision point.
       * @return A noId if no better decision can be found, otherwise a new decision with a better priority.
       */
      static DecisionPointId next(const ConstrainedVariableSet& flawCandidates, 
				  const std::map< ConstrainedVariableId, unsigned int>& guardCache,
				  unsigned int& bestPriority);

      /**
       * @brief Used to prune entities out which are not variables
       */
      static bool test(const EntityId& entity);

      virtual ~UnboundVariableDecisionPoint();

      /**
       * @brief Dump contents to a String.
       */
      std::string toString() const;

      const ConstrainedVariableId& getFlawedVariable() const;

    protected:

      UnboundVariableDecisionPoint(const DbClientId& client, const ConstrainedVariableId& flawedVariable, const TiXmlElement& configData,
                                   const LabelStr& explanation = "unknown");

      const ConstrainedVariableId m_flawedVariable; /*!< The Flaw to resolve */

      ValueSource* m_choices;

      virtual void handleInitialize();

      virtual void handleExecute();

      virtual void handleUndo();

      virtual bool canUndo() const;

      /**
       * @brief Retrieves the next choice to be executed. Implementation will depend
       * on the representation of choices in the derived class.
       */
      virtual double getNext() = 0;
    };

    /**
     * @brief Provides for access in ascending order
     */
    class MinValue: public UnboundVariableDecisionPoint {
    public:
      MinValue(const DbClientId& client, const ConstrainedVariableId& flawedVariable, const TiXmlElement& configData, const LabelStr& explanation = "unknown");
      bool hasNext() const;
      double getNext();

    private:

      unsigned int m_choiceIndex; /*!< The current position in the list of choices. */
    };

    /**
     * @brief Provides for access in descending order
     */
    class MaxValue: public UnboundVariableDecisionPoint {
    public:
      MaxValue(const DbClientId& client, const ConstrainedVariableId& flawedVariable, const TiXmlElement& configData, const LabelStr& explanation = "unknown");
      bool hasNext() const;
      double getNext();

    private:

      unsigned int m_choiceIndex; /*!< The current position in the list of choices. */
    };


    /**
     * @brief Random selection over the set of values.
     *
     * @todo Extend to allow alternate distributions.
     */
    class RandomValue: public UnboundVariableDecisionPoint {
    public:
      RandomValue(const DbClientId& client, const ConstrainedVariableId& flawedVariable, const TiXmlElement& configData, const LabelStr& explanation = "unknown");
      bool hasNext() const;
      double getNext();

    protected:
      enum Distribution {UNIFORM, NORMAL};

    private:

      std::set<int> m_usedIndeces; /*!< The set of used choices so far. Each is the index of the choice
				     in m_choices. */
      Distribution m_distribution; /*!< Indicates the distribution used in random number selection. */
    };

  }
}

#define REGISTER_VARIABLE_DECISION_FACTORY(CLASS, NAME) REGISTER_FLAW_HANDLER(CLASS, NAME)

#endif
