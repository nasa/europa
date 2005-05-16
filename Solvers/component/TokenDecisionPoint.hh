#ifndef H_TokenDecisionPoint
#define H_TokenDecisionPoint

#include "SolverDefs.hh"
#include "DecisionPoint.hh"
#include "MatchingRule.hh"
#include <vector>

/**
 * @author Conor McGann
 * @date March, 2005
 */
namespace EUROPA {
  namespace SOLVERS {

    typedef DecisionPoint::AbstractFactory<Token, TokenMatchingRule> TokenDecisionPointFactory;
    typedef Id<TokenDecisionPointFactory> TokenDecisionPointFactoryId;

    /**
     * @brief Defines a class for formulation, execution and retraction of a decision
     * to resolve a flaw concerning the state of a token.
     *
     * The resolutions are:
     * @li MERGE. If MERGED is a valid choice, then we will address this flaw by merging
     * with a compatible token.
     * @li ACTIVE. If ACTIVE is a valid choice, then we will address this flaw by activating
     * this token. We will also permit resolution by creation and allocation of a token and then merging on that new one.
     * @li REJECT. If REJECT is a valid choice, then we will address this flaw by rejecting
     * this token.
     */
    class TokenDecisionPoint: public DecisionPoint {
    public:
      /**
       * @brief Constructor. Matches signature for DecisionPointFactory
       */
      TokenDecisionPoint(const DbClientId& client, const TokenId& flawedToken, const TiXmlElement& configData);

      virtual ~TokenDecisionPoint();

      /**
       * @brief Utility to obtain the next decision point from the set of candidates if it can exceed the
       * given best priority.
       * @param flawCandidates A set of inactive tokens to choose from
       * @param bestPriority A mutable current best priority. If a new decision point is created, the new
       * bestPriority will be updated to the priority of the new decision point.
       * @return A noId if no better decision can be found, otherwise a new decision with a better priority.
       */
      static DecisionPointId next(const TokenSet& flawCandidates, 
				  unsigned int& bestPriority);

      std::string toString() const;

    private:
      void handleInitialize();
      bool hasNext() const;
      void handleExecute();
      void handleUndo();
      bool canUndo() const;

      const TokenId m_flawedToken; /*!< The token to be resolved. */
      std::vector<TokenId> m_compatibleTokens; /*!< A possibly empty collection of tokens to merge with. */
      unsigned int m_mergeIndex; /*!< The position of the next choice in m_compatibleTokens. */
      unsigned int m_mergeCount; /*!< The size of m_compatibleTokens */
      std::vector<LabelStr> m_choices; /*!< The sequences list of states to choose. */
      unsigned int m_choiceIndex; /*!< The position of the next choice in m_choices. */
      unsigned int m_choiceCount; /*!< The size of m_choices. */
    };
  }
}

#define REGISTER_TOKEN_DECISION_FACTORY(CLASS, NAME)\
REGISTER_DECISION_FACTORY(CLASS, Token, TokenMatchingRule, NAME);

#endif
