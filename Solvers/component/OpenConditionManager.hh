#ifndef H_OpenConditionManager
#define H_OpenConditionManager

#include "SolverDefs.hh"
#include "FlawManager.hh"
#include "OpenConditionDecisionPoint.hh"
#include "PlanDatabaseListener.hh"
//#include "MatchingRule.hh"

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

      void notifyRemoved(const ConstrainedVariableId& variable);
      void notifyChanged(const ConstrainedVariableId& variable, const DomainListener::ChangeType& changeType);
      void handleInitialize();
      void addFlaw(const TokenId& token);
      void removeFlaw(const TokenId& token);

      TokenSet m_flawCandidates; /*!< The set of candidate token flaws */
    };
  }
}

#define REGISTER_TOKEN_DECISION_FACTORY(CLASS, NAME)\
REGISTER_DECISION_FACTORY(CLASS, EUROPA::Token, EUROPA::SOLVERS::TokenMatchingRule, NAME);

#endif
