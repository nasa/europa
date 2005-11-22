#ifndef H_UnboundVariableManager
#define H_UnboundVariableManager

#include "FlawManager.hh"
#include "UnboundVariableDecisionPoint.hh"
#include "EntityIterator.hh"

/**
 * @brief Provides class declaration for handling variable flaws.
 * @author Conor McGann
 */
namespace EUROPA {
  namespace SOLVERS {

    class UnboundVariableManager: public FlawManager {
    public:
      UnboundVariableManager(const TiXmlElement& configData);

      bool inScope(const EntityId& entity) const;

      IteratorId createIterator() const;
    private:
      virtual DecisionPointId next(unsigned int priorityLowerBound, unsigned int& bestPriority);

      virtual DecisionPointId next(unsigned int priorityLowerBound,
				   unsigned int& bestPriority,
				   const ConstrainedVariableSet& flawCandidates);


      void notifyAdded(const ConstraintId& constraint);
      void notifyRemoved(const ConstraintId& constraint);
      void notifyRemoved(const ConstrainedVariableId& var);
      void notifyChanged(const ConstrainedVariableId& variable, const DomainListener::ChangeType& changeType);
      void handleInitialize();
      void addFlaw(const ConstrainedVariableId& var);
      void removeFlaw(const ConstrainedVariableId& var);
      void toggleSingletonFlaw(const ConstrainedVariableId& var);
      void addGuard(const ConstrainedVariableId& var);
      void removeGuard(const ConstrainedVariableId& var);
      void handleConstraintAddition(const ConstraintId& constraint);
      void handleConstraintRemoval(const ConstraintId& constraint);

      /**
       * @brief Utility to test if the given variable is part of a token that is merged, rejected or inactive.
       */
      static bool variableOfNonActiveToken(const ConstrainedVariableId& var);

      ConstrainedVariableSet m_flawCandidates; /*!< All variables that have passed the static filter */
      ConstrainedVariableSet m_singletonFlawCandidates; /*!< All singleton variables that have passed the static filter */
      std::map<ConstrainedVariableId, unsigned int> m_guardCache; /*!< Cache of variables that are
								    guarded. Includes reference counts. */

      class FlawIterator : public Iterator {
      public:
	FlawIterator(const UnboundVariableManager& manager);
	bool done() const;
	const EntityId next();
	unsigned int visited() const {return m_visited;}
      protected:
      private:
	unsigned int m_visited;
	unsigned int m_timestamp;
	const UnboundVariableManager& m_manager;
	ConstrainedVariableSet::const_iterator m_it;
	ConstrainedVariableSet::const_iterator m_end;
      };
    };
  }
}
#endif
