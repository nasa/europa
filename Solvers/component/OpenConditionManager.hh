#ifndef H_OpenConditionManager
#define H_OpenConditionManager

#include "SolverDefs.hh"
#include "FlawManager.hh"
#include "OpenConditionDecisionPoint.hh"
#include "PlanDatabaseListener.hh"
#include "MatchingRule.hh"

#include <vector>

/**
 * @author Conor McGann
 * @date March, 2005
 */
namespace EUROPA {
  namespace SOLVERS {

    class OpenConditionManager: public FlawManager {
    public:
      OpenConditionManager(const TiXmlElement& configData);

      virtual ~OpenConditionManager();

      bool inScope(const EntityId& entity) const;

    private:
      // TODO: Special code for units. virtual DecisionPointId nextZeroCommitmentDecision();

      virtual DecisionPointId next(unsigned int priorityLowerBound, unsigned int& bestPriority);

      /**
       * @brief Obtains a priority for the given candidate token
       * @param candidate An inactive token for evaluation
       * @param bestPriority A current best priority to beat. Allows early termination when we know we can't beat it.
       */
      virtual unsigned int getPriority(const TokenId& candidate, unsigned int bestPriority);

      void handleInitialize();
      void addFlaw(const TokenId& token);
      void removeFlaw(const TokenId& token);

      TokenSet m_flawCandidates; /*!< The set of candidate token flaws */

      /**
       * @brief Plugs into Plan Database events on the Plan
       * Database to synchronize flaw candidates.
       */
      class DbListener: public PlanDatabaseListener {
      public:
	DbListener(const PlanDatabaseId& db,
		   OpenConditionManager& dm);

      private:
	void notifyAdded(const TokenId& token);
	void notifyRemoved(const TokenId& token);
	void notifyActivated(const TokenId& token);
	void notifyDeactivated(const TokenId& token);
	void notifyMerged(const TokenId& token);
	void notifySplit(const TokenId& token);
	void notifyRejected(const TokenId& token);
	void notifyReinstated(const TokenId& token);

	OpenConditionManager& m_dm;
      };

      friend class OpenConditionManager::DbListener;
      OpenConditionManager::DbListener* m_dbListener; /*!< For processing Plan Database events */
    };
  }
}

#define REGISTER_TOKEN_DECISION_FACTORY(CLASS, NAME)\
REGISTER_DECISION_FACTORY(CLASS, EUROPA::Token, EUROPA::SOLVERS::TokenMatchingRule, NAME);

#endif
