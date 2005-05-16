#ifndef H_VariableDecisionPoint
#define H_VariableDecisionPoint

#include "DecisionPoint.hh"
#include "ConstrainedVariable.hh"

/**
 * @brief Provides class declarations for handling variable flaws. Includes classes for min, max and random orderings.
 * @author Conor McGann
 */
namespace EUROPA {

  namespace SOLVERS {

    class ValueSource;
    typedef DecisionPoint::AbstractFactory<ConstrainedVariable, VariableMatchingRule> VariableDecisionPointFactory;
    typedef Id<VariableDecisionPointFactory> VariableDecisionPointFactoryId;

    /**
     * @brief Abstract base class for Variable decisions.
     *
     * This class adds the particulars of variable binding and unbinding, but
     * leaves all details of how choices are stored and how they are iterated through to
     * derived classes, thus allowing for specialized storage and selection.
     */
    class VariableDecisionPoint: public DecisionPoint {
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

      virtual ~VariableDecisionPoint();

      /**
       * @brief Dump contents to a String.
       */
      std::string toString() const;

    protected:

      VariableDecisionPoint(const DbClientId& client, const ConstrainedVariableId& flawedVariable, const TiXmlElement& configData);

      const ConstrainedVariableId& getFlawedVariable() const;

      const ConstrainedVariableId m_flawedVariable; /*!< The Flaw to resolve */

      ValueSource* m_choices;

    private:

      void handleInitialize();

      void handleExecute();

      void handleUndo();

      bool canUndo() const;

      /**
       * @brief Retrieves the next choice to be executed. Implementation will depend
       * on the representation of choices in the derived class.
       */
      virtual double getNext() = 0;
    };

    /**
     * @brief Provides for access in ascending order
     */
    class MinValue: public VariableDecisionPoint {
    public:
      MinValue(const DbClientId& client, const ConstrainedVariableId& flawedVariable, const TiXmlElement& configData);
      bool hasNext() const;
      double getNext();

    private:

      unsigned int m_choiceIndex; /*!< The current position in the list of choices. */
    };

    /**
     * @brief Provides for access in descending order
     */
    class MaxValue: public VariableDecisionPoint {
    public:
      MaxValue(const DbClientId& client, const ConstrainedVariableId& flawedVariable, const TiXmlElement& configData);
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
    class RandomValue: public VariableDecisionPoint {
    public:
      RandomValue(const DbClientId& client, const ConstrainedVariableId& flawedVariable, const TiXmlElement& configData);
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

#define REGISTER_VARIABLE_DECISION_FACTORY(CLASS, NAME)\
REGISTER_DECISION_FACTORY(CLASS, ConstrainedVariable, VariableMatchingRule, NAME);

#endif
