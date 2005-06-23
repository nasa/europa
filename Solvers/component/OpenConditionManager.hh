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

      bool inScope(const TokenId& token) const;

    private:
      // TODO: Special code for units. virtual DecisionPointId nextZeroCommitmentDecision();
      virtual DecisionPointId next(unsigned int priorityLowerBound, unsigned int& bestPriority);
      DecisionPointId allocateDecisionPoint(const TokenId& flawedToken);
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

      /**
       * @brief Helper method to iterate over the rules to match
       */
      bool matches(const TokenId& token, const std::list<TokenMatchingRuleId>& rules) const;

      /**
       * @brief Helper method to obtain the most restrictive decision point factory
       */
      OpenConditionDecisionPointFactoryId matchFactory(const TokenId& token) const;

      std::list<TokenMatchingRuleId> m_staticMatchingRules;
      std::list<TokenMatchingRuleId> m_dynamicMatchingRules;
      std::list<OpenConditionDecisionPointFactoryId> m_factories;
    };
  }
}

#define REGISTER_TOKEN_DECISION_FACTORY(CLASS, NAME)\
REGISTER_DECISION_FACTORY(CLASS, EUROPA::Token, EUROPA::SOLVERS::TokenMatchingRule, NAME);

#endif
