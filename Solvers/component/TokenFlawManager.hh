#ifndef H_TokenFlawManager
#define H_TokenFlawManager

#include "SolverDefs.hh"
#include "FlawManager.hh"
#include "TokenDecisionPoint.hh"
#include "PlanDatabaseListener.hh"

#include <vector>

/**
 * @author Conor McGann
 * @date March, 2005
 */
namespace EUROPA {
  namespace SOLVERS {

    class TokenFlawManager: public FlawManager {
    public:
      TokenFlawManager(const TiXmlElement& configData);

      virtual ~TokenFlawManager();

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
		   TokenFlawManager& dm);

      private:
	void notifyAdded(const TokenId& token);
	void notifyRemoved(const TokenId& token);
	void notifyActivated(const TokenId& token);
	void notifyDeactivated(const TokenId& token);
	void notifyMerged(const TokenId& token);
	void notifySplit(const TokenId& token);
	void notifyRejected(const TokenId& token);
	void notifyReinstated(const TokenId& token);

	TokenFlawManager& m_dm;
      };

      friend class TokenFlawManager::DbListener;
      TokenFlawManager::DbListener* m_dbListener; /*!< For processing Plan Database events */

      /**
       * @brief Helper method to iterate over the rules to match
       */
      bool matches(const TokenId& token, const std::list<TokenMatchingRuleId>& rules) const;

      /**
       * @brief Helper method to obtain the most restrictive decision point factory
       */
      TokenDecisionPointFactoryId matchFactory(const TokenId& token) const;

      std::list<TokenMatchingRuleId> m_staticMatchingRules;
      std::list<TokenMatchingRuleId> m_dynamicMatchingRules;
      std::list<TokenDecisionPointFactoryId> m_factories;
    };
  }
}
#endif
