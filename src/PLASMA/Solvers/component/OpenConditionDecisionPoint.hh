#ifndef H_OpenConditionDecisionPoint
#define H_OpenConditionDecisionPoint

#include "SolverDefs.hh"
#include "SolverDecisionPoint.hh"
#include <vector>

/**
 * @author Conor McGann
 * @date March, 2005
 */
namespace EUROPA {
  namespace SOLVERS {

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
    class OpenConditionDecisionPoint: public DecisionPoint {
    public:
      /**
       * @brief Constructor. Test signature for DecisionPointFactory
       */
      OpenConditionDecisionPoint(const DbClientId& client, const TokenId& flawedToken, const TiXmlElement& configData,
                                 const LabelStr& explanation = "unknown");

      virtual ~OpenConditionDecisionPoint();

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

      /**
       * @brief Used to prune entities out which are not inactive tokens
       */
      static bool test(const EntityId& entity);

      virtual std::string toString() const;
      virtual std::string toShortString() const;

      /**
       * @brief Accessor to flawed token
       */
      const TokenId& getToken() const;

    protected:
      virtual void handleInitialize();
      virtual void handleExecute();
      virtual void handleUndo();
      virtual bool hasNext() const;
      virtual bool canUndo() const;

      const TokenId m_flawedToken; /*!< The token to be resolved. */
      std::vector<LabelStr> m_choices; /*!< The sequences list of states to choose. */
      std::vector<TokenId> m_compatibleTokens; /*!< A possibly empty collection of tokens to merge with. */
      unsigned int m_mergeCount; /*!< The size of m_compatibleTokens */
      unsigned int m_choiceCount; /*!< The size of m_choices. */
      unsigned int m_mergeIndex; /*!< The position of the next choice in m_compatibleTokens. */
      unsigned int m_choiceIndex; /*!< The position of the next choice in m_choices. */

    };


    class OCDecision
    {
    public:
    	virtual ~OCDecision() {}

    	virtual void execute() = 0;
    	virtual void undo() = 0;
    	virtual bool hasNext() = 0;
    };

    /*
     * @brief In addition to the standard Activate/Merge/Reject decisions, this decision point will
     * generate choices for supporting actions if that applies to the flawed token
     */
    // TODO: reshape OpenConditionDecisionPoint so that this can inherit from it instead
    class SupportedOCDecisionPoint : public DecisionPoint
    {
    public:
        SupportedOCDecisionPoint(
        	const DbClientId& client,
        	const TokenId& flawedToken,
        	const TiXmlElement& configData,
        	const LabelStr& explanation = "unknown");

        virtual ~SupportedOCDecisionPoint();

        virtual std::string toString() const;
        virtual std::string toShortString() const;

    protected:
        virtual void handleInitialize();
        virtual void handleExecute();
        virtual void handleUndo();
        virtual bool hasNext() const;
        virtual bool canUndo() const;

        const TokenId m_flawedToken; /*!< The token to be resolved. */
        std::vector<OCDecision*> m_choices;
        unsigned int m_currentChoice;
    };

    class ChangeTokenState : public OCDecision
    {
    public:
    	ChangeTokenState(const DbClientId& dbClient, const TokenId& token);
    	virtual ~ChangeTokenState();

    	virtual void undo();
    	virtual bool hasNext();

    protected:
    	DbClientId m_dbClient;
    	TokenId m_token;
    	bool m_isExecuted;
    };

    class ActivateToken : public ChangeTokenState
    {
    public:
    	ActivateToken(const DbClientId& dbClient, const TokenId& token);
    	virtual ~ActivateToken();

    	virtual void execute();
    };

    class MergeToken : public ChangeTokenState
    {
    public:
    	MergeToken(const DbClientId& dbClient, const TokenId& token, const std::vector<TokenId>& compatibleTokens);
    	virtual ~MergeToken();

    	virtual void execute();

    protected:
    	std::vector<TokenId> m_compatibleTokens;
    	unsigned int m_currentChoice;
    };

    class RejectToken : public ChangeTokenState
    {
    public:
    	RejectToken(const DbClientId& dbClient, const TokenId& token);
    	virtual ~RejectToken();

    	virtual void execute();
    };

    class SupportToken : public ChangeTokenState
    {
    public:
    	SupportToken(const DbClientId& dbClient, const TokenId& token, const std::vector<TokenTypeId>& supportActionTypes);
    	virtual ~SupportToken();

    	virtual void execute();
    	virtual void undo();

    protected:
    	std::vector<std::pair<TokenTypeId,int> > m_choices;
    	int m_actionIndex;
    	int m_effectIndex;
    	TokenId m_action;
    };
  }
}


#define REGISTER_OPENCONDITION_DECISION_FACTORY(MGR,CLASS, NAME)\
REGISTER_DECISION_FACTORY(MGR,CLASS, NAME);
#endif
