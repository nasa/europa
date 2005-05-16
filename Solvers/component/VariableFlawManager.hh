#ifndef H_VariableFlawManager
#define H_VariableFlawManager

#include "FlawManager.hh"
#include "ConstraintEngineListener.hh"
#include "PlanDatabaseListener.hh"
#include "VariableDecisionPoint.hh"

/**
 * @brief Provides class declaration for handling variable flaws.
 * @author Conor McGann
 */
namespace EUROPA {
  namespace SOLVERS {

    class VariableFlawManager: public FlawManager {
    public:
      VariableFlawManager(const TiXmlElement& configData);

      virtual ~VariableFlawManager();

      /**
       * @brief True if the given variable is in scope.
       */
      bool inScope(const ConstrainedVariableId& var) const;

    private:
      virtual DecisionPointId next(unsigned int priorityLowerBound, unsigned int& bestPriority);
      DecisionPointId allocateDecisionPoint(const ConstrainedVariableId& flawedVariable);
      void handleInitialize();
      void addFlaw(const ConstrainedVariableId& var);
      void removeFlaw(const ConstrainedVariableId& var);
      void addGuard(const ConstrainedVariableId& var);
      void removeGuard(const ConstrainedVariableId& var);
      void handleConstraintAddition(const ConstraintId& constraint);
      void handleConstraintRemoval(const ConstraintId& constraint);

      /**
       * @brief Utility to test if the given variable is part of a token that is merged, rejected or inactive.
       */
      static bool variableOfNonActiveToken(const ConstrainedVariableId& var);

      ConstrainedVariableSet m_flawCandidates; /*!< All variables that have passed the static filter */

      std::map<ConstrainedVariableId, unsigned int> m_guardCache; /*!< Cache of variables that are
								    guarded. Includes reference counts. */

      /**
       * @brief Plugs manager into ConstraintEngine events to
       * synchronize flaw candidates
       */
      class CeListener: public ConstraintEngineListener {
      public:
	CeListener(const ConstraintEngineId& ce, 
		   VariableFlawManager& dm);

	void notifyRemoved(const ConstrainedVariableId& variable);
	void notifyChanged(const ConstrainedVariableId& variable, const DomainListener::ChangeType& changeType);
	void notifyAdded(const ConstraintId& constraint);
	void notifyRemoved(const ConstraintId& constraint);

      private:
	VariableFlawManager& m_fm;
      };

      friend class VariableFlawManager::CeListener;
      VariableFlawManager::CeListener* m_ceListener; /*!< For Processing constraint engine events */

      /**
       * @brief Plugs into Token Activation and Deactivation events on the Plan
       * Database to synchronize flaw candidates.
       */
      class DbListener: public PlanDatabaseListener {
      public:
	DbListener(const PlanDatabaseId& db,
		   VariableFlawManager& dm);
      private:
	void notifyActivated(const TokenId& token);
	void notifyDeactivated(const TokenId& token);
	VariableFlawManager& m_fm;
      };

      friend class VariableFlawManager::DbListener;
      VariableFlawManager::DbListener* m_dbListener; /*!< For processing Plan Database events */

      /**
       * @brief Helper method to iterate over the rules to match
       */
      bool matches(const ConstrainedVariableId& var, const std::list<VariableMatchingRuleId>& rules) const;

      /**
       * @brief Helper method to obtain the most restrictive decision point factory
       */
      VariableDecisionPointFactoryId matchFactory(const ConstrainedVariableId& var) const;

      std::list<VariableMatchingRuleId> m_staticMatchingRules;
      std::list<VariableMatchingRuleId> m_dynamicMatchingRules;
      std::list<VariableDecisionPointFactoryId> m_factories;
    };
  }
}
#endif
