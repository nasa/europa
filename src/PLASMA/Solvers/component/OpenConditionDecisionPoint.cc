#include "OpenConditionDecisionPoint.hh"
#include "PlanDatabase.hh"
#include "Token.hh"
#include "TokenVariable.hh"
#include "ConstrainedVariable.hh"

// TODO: move this to the appropriate place
#ifdef _MSC_VER
#define EUROPA_UINT_MAX UINT_MAX
#else
#define EUROPA_UINT_MAX std::numeric_limits<unsigned int>::max()
#endif //_MSC_VER

namespace EUROPA {
  namespace SOLVERS {

	bool OpenConditionDecisionPoint::test(const EntityId& entity){
		return(TokenId::convertable(entity) || TokenId(entity)->isInactive());
	}

	OpenConditionDecisionPoint::OpenConditionDecisionPoint(const DbClientId& client, const TokenId& flawedToken, const TiXmlElement& configData,
			const LabelStr& explanation)
	: DecisionPoint(client, flawedToken->getKey(), explanation),
	  m_flawedToken(flawedToken),
	  m_mergeCount(0),
	  m_choiceCount(0),
	  m_mergeIndex(0),
	  m_choiceIndex(0) {

		// Retrieve policy information from configuration node

		// Ordering preference - merge vs. activation. Default is to merge, activate, reject.

		// Activation preference - direct vs. indirect. Default is to use direct activation
	}

	OpenConditionDecisionPoint::~OpenConditionDecisionPoint() {}

	const TokenId& OpenConditionDecisionPoint::getToken() const{ return m_flawedToken; }

	void OpenConditionDecisionPoint::handleInitialize(){
		const StateDomain stateDomain(m_flawedToken->getState()->lastDomain());

		// Next merge choices if there are any.
		if(stateDomain.isMember(Token::MERGED)){
			// Use exact test in this case
			m_flawedToken->getPlanDatabase()->getCompatibleTokens(
					m_flawedToken,
					m_compatibleTokens,
					EUROPA_UINT_MAX,
					true);
			m_mergeCount = m_compatibleTokens.size();
			if(m_mergeCount > 0) {
				m_choices.push_back(Token::MERGED);
				debugMsg("OpenConditionDecisionPoint:handleInitialize",
						"Adding choice '" << Token::MERGED.toString() << "' for token " <<
						m_flawedToken->getKey());
			}
			else {
				debugMsg("OpenConditionDecisionPoint:handleInitialize",
						"Skipping choice '" << Token::MERGED.toString() << "' for token " <<
						m_flawedToken->getKey() << " because there are no compatible tokens.");
			}
		}
		else {
			debugMsg("OpenConditionDecisionPoint:handleInitialize",
					"Skipping choice '" << Token::MERGED.toString() << "' for token " <<
					m_flawedToken->getKey() << " because it isn't in the state domain.");
		}

		if(stateDomain.isMember(Token::ACTIVE) ) {//&& m_flawedToken->getPlanDatabase()->hasOrderingChoice(m_flawedToken))
			debugMsg("OpenConditionDecisionPoint:handleInitialize",
					"Adding choice '" << Token::ACTIVE.toString() << "' for token " <<
					m_flawedToken->getKey());
			m_choices.push_back(Token::ACTIVE);
		}
		else {
			debugMsg("OpenConditionDecisionPoint:handleInitialize",
					"Skipping choice '" << Token::ACTIVE.toString() << "' for token " <<
					m_flawedToken->getKey() << " because it isn't in the state domain.");
		}

		if(stateDomain.isMember(Token::REJECTED)) {
			debugMsg("OpenConditionDecisionPoint:handleInitialize",
					"Adding choice '" << Token::REJECTED.toString() << "' for token " <<
					m_flawedToken->getKey());
			m_choices.push_back(Token::REJECTED);
		}
		else {
			debugMsg("OpenConditionDecisionPoint:handleInitialize",
					"Skipping choice '" << Token::REJECTED.toString() << "' for token " <<
					m_flawedToken->getKey() << " because it isn't in the state domain.");
		}

		m_choiceCount = m_choices.size();
	}

	void OpenConditionDecisionPoint::handleExecute() {
		checkError(m_choiceIndex < m_choiceCount,
				"Tried to execute past available choices:" << m_choiceIndex << ">=" << m_choiceCount);
		if(m_choices[m_choiceIndex] == Token::ACTIVE) {
			debugMsg("SolverDecisionPoint:handleExecute", "For " << m_flawedToken->getPredicateName().toString() << "(" <<
					m_flawedToken->getKey() << "), assigning ACTIVE.");
			m_client->activate(m_flawedToken);
		}
		else if(m_choices[m_choiceIndex] == Token::MERGED) {
			checkError(m_mergeIndex < m_mergeCount, "Tried to merge past available compatible tokens.");
			TokenId activeToken = m_compatibleTokens[m_mergeIndex];
			debugMsg("SolverDecisionPoint:handleExecute", "For " << m_flawedToken->getPredicateName().toString() << "(" <<
					m_flawedToken->getKey() << "), assigning MERGED onto " << activeToken->getPredicateName().toString() <<
					"(" << activeToken->getKey() << ").");
			m_client->merge(m_flawedToken, activeToken);
		}
		else {
			checkError(m_choices[m_choiceIndex] == Token::REJECTED,
					"Expect this choice to be REJECTED instead of " + m_choices[m_choiceIndex].toString());
			debugMsg("SolverDecisionPoint:handleExecute", "For " << m_flawedToken->getPredicateName().toString() << "(" <<
					m_flawedToken->getKey() << "), assigning REJECTED.");
			m_client->reject(m_flawedToken);
		}
	}

	void OpenConditionDecisionPoint::handleUndo() {
		debugMsg("SolverDecisionPoint:handleUndo", "Retracting open condition decision on " << m_flawedToken->getPredicateName().toString() <<
				"(" << m_flawedToken->getKey() << ").");
		m_client->cancel(m_flawedToken);

		if(m_choices[m_choiceIndex] == Token::MERGED) {
			m_mergeIndex++;
			if(m_mergeIndex == m_mergeCount)
				m_choiceIndex++;
		}
		else
			m_choiceIndex++;
	}

	bool OpenConditionDecisionPoint::hasNext() const {
		return m_choiceIndex < m_choiceCount;
	}

	std::string OpenConditionDecisionPoint::toShortString() const{
		// This returns the last executed choice
		int idx = m_choiceIndex;
		std::stringstream os;

		if(m_choices.empty()) {
			os << "EMPTY";
		}
		else if(m_choices[idx] == Token::MERGED) {
			os << "MRG(" << m_flawedToken->getKey() << "," << m_compatibleTokens[m_mergeIndex]->getKey() << ")";
		}
		else if(m_choices[idx] == Token::ACTIVE) {
			os << "ACT(" << m_flawedToken->getKey() << ")";
		}
		else if(m_choices[idx] == Token::REJECTED) {
			os << "REJ(" << m_flawedToken->getKey() << ")";
		}
		else {
			check_error(ALWAYS_FAIL,"Unknown choice:"+m_choices[idx].toString());
		}

		return os.str();
	}

	std::string OpenConditionDecisionPoint::toString() const{
		std::stringstream strStream;
		strStream
		<< "TOKEN STATE:"
		<< "    TOKEN=" << m_flawedToken->getPredicateName().toString()  << "(" << m_flawedToken->getKey() << "):"
		<< "    OBJECT=" << Object::toString(m_flawedToken->getObject()) << ":"
		<< "    CHOICES(current=" << m_choiceIndex << ")=";

		if(!m_compatibleTokens.empty()){
			strStream << "MERGED {";
			for(std::vector<TokenId>::const_iterator it = m_compatibleTokens.begin(); it != m_compatibleTokens.end(); ++it){
				TokenId token = *it;
				strStream << " " << token->getKey() << " ";
			}
			strStream << "}";
		}

		if (m_flawedToken->getState()->lastDomain().isMember(Token::ACTIVE)  &&
				(!m_flawedToken->getPlanDatabase()->getConstraintEngine()->constraintConsistent() ||
						m_flawedToken->getPlanDatabase()->hasOrderingChoice(m_flawedToken)))
			strStream << " ACTIVE ";

		if (m_flawedToken->getState()->lastDomain().isMember(Token::REJECTED))
			strStream << " REJECTED ";

		return strStream.str();
	}

	bool OpenConditionDecisionPoint::canUndo() const {
		return DecisionPoint::canUndo() && m_flawedToken->getState()->isSpecified();
	}


    SupportedOCDecisionPoint::SupportedOCDecisionPoint(
    	const DbClientId& client,
    	const TokenId& flawedToken,
    	const TiXmlElement& configData,
    	const LabelStr& explanation)
    	: DecisionPoint(client, flawedToken->getKey(), explanation)
    	, m_flawedToken(flawedToken)
    	, m_currentChoice(0)
    {
    }

    SupportedOCDecisionPoint::~SupportedOCDecisionPoint()
    {
    	for(unsigned int i=0;i<m_choices.size();i++)
    		delete m_choices[i];

    	m_choices.clear();
    }

    std::string SupportedOCDecisionPoint::toString() const
    {
    	// TODO: implement this
    	return "SupportedOCDecisionPoint:" + m_flawedToken->toString();
    }

    std::string SupportedOCDecisionPoint::toShortString() const
    {
    	// TODO: implement this
    	return "SupportedOCDecisionPoint:" + m_flawedToken->toString();
    }

    void SupportedOCDecisionPoint::handleInitialize()
    {
        const StateDomain stateDomain(m_flawedToken->getState()->lastDomain());

        if(stateDomain.isMember(Token::MERGED)){
        	std::vector<TokenId> compatibleTokens;

            m_flawedToken->getPlanDatabase()->getCompatibleTokens(
            		m_flawedToken,
            		compatibleTokens,
            		EUROPA_UINT_MAX,
            		true);

            // TODO: if flawed token is a fact, make sure we only look at other facts
            if (compatibleTokens.size() > 0)
            	m_choices.push_back(new MergeToken(m_client,m_flawedToken,compatibleTokens));
        }

		if(stateDomain.isMember(Token::ACTIVE)) {
			SchemaId schema = m_client->getSchema();
			// TODO: PSToken::getTokenType() should return token type, not token type name
			TokenTypeId tokenType = schema->getTokenType(m_flawedToken->getFullTokenType());
			std::vector<TokenTypeId> supportActionTypes = schema->getTypeSupporters(tokenType);
			// TODO: allow heuristic function to be passed as a parameter to SupportToken
			if (!m_flawedToken->isFact() && (supportActionTypes.size() > 0))
				m_choices.push_back(new SupportToken(m_client,m_flawedToken,supportActionTypes));
			else
				m_choices.push_back(new ActivateToken(m_client,m_flawedToken));
		}

        if(stateDomain.isMember(Token::REJECTED))
        	m_choices.push_back(new RejectToken(m_client,m_flawedToken));
    }

    void SupportedOCDecisionPoint::handleExecute()
    {
    	OCDecision* currentChoice = m_choices[m_currentChoice];
    	currentChoice->execute();
    }

    void SupportedOCDecisionPoint::handleUndo()
    {
    	OCDecision* currentChoice = m_choices[m_currentChoice];
    	currentChoice->undo();
    	if (!currentChoice->hasNext())
    		m_currentChoice++;
    }

    bool SupportedOCDecisionPoint::hasNext() const
    {
    	return (m_currentChoice < m_choices.size());
    }

    bool SupportedOCDecisionPoint::canUndo() const
    {
        return DecisionPoint::canUndo() && m_flawedToken->getState()->isSpecified();
    }


    ChangeTokenState::ChangeTokenState(const DbClientId& dbClient, const TokenId& token)
    	: m_dbClient(dbClient)
    	, m_token(token)
    	, m_isExecuted(false)
    {
    }

	ChangeTokenState::~ChangeTokenState()
	{
	}

	void ChangeTokenState::undo()
	{
		m_dbClient->cancel(m_token);
	}

	bool ChangeTokenState::hasNext()
	{
		return !m_isExecuted;
	}

	ActivateToken::ActivateToken(const DbClientId& dbClient, const TokenId& token)
		: ChangeTokenState(dbClient,token)
	{
	}

	ActivateToken::~ActivateToken()
	{
	}

	void ActivateToken::execute()
	{
		m_dbClient->activate(m_token);
		m_isExecuted=true;
	}

	MergeToken::MergeToken(const DbClientId& dbClient, const TokenId& token, const std::vector<TokenId>& compatibleTokens)
		: ChangeTokenState(dbClient,token)
		, m_compatibleTokens(compatibleTokens)
		, m_currentChoice(0)
	{
	}

	MergeToken::~MergeToken()
	{
	}

	void MergeToken::execute()
	{
		TokenId activeToken = m_compatibleTokens[m_currentChoice++];
		m_dbClient->merge(m_token,activeToken);
		m_isExecuted=(m_currentChoice >= m_compatibleTokens.size());
	}

	RejectToken::RejectToken(const DbClientId& dbClient, const TokenId& token)
		: ChangeTokenState(dbClient,token)
	{
	}

	RejectToken::~RejectToken()
	{
	}

	void RejectToken::execute()
	{
		m_dbClient->reject(m_token);
		m_isExecuted=true;
	}

	SupportToken::SupportToken(const DbClientId& dbClient, const TokenId& token, const std::vector<TokenTypeId>& supportActionTypes)
		: ChangeTokenState(dbClient,token)
	{
		for (unsigned int i = 0;i<supportActionTypes.size(); i++) {
			TokenTypeId tt = supportActionTypes[i];
			int mergeCnt = 0;
			PSList<PSTokenType*> effectTypes = tt->getSubgoalsByAttr(PSTokenType::EFFECT);
			debugMsg("SupportToken", "Support Type:" << tt->getName());
			for (int j=0;j<effectTypes.size();j++) {
				// TODO: use ids for faster comparison instead
				debugMsg("SupportToken", "Comparing: " << token->getTokenType() << " and " << effectTypes.get(j)->getName());
				if (token->getTokenType() == effectTypes.get(j)->getName()) {
					mergeCnt++;
					debugMsg("SupportToken", "Found match!");
				}
			}

			checkError(mergeCnt > 0, "Expected to find at list one merge point for " << token->getTokenType() << " in " << tt->getName());
			// TODO: use heuristic function to sort choices
			m_choices.push_back(std::pair<TokenTypeId,int>(tt,mergeCnt));
		}

		m_actionIndex = 0;
		m_effectIndex = 0;
		m_action = TokenId::noId();
		m_targetEffect = TokenId::noId();
	}

	SupportToken::~SupportToken()
	{
	}

	/*
	 * TODO: some performance optimizations can be added for chronological backtracking:
	 * 1. Activate token that needs support only once
	 * 2. Activate each action only once and only change which effect is merged into token
	 */

	std::string autoName(const std::string& prefix)
	{
	    static int i=0;
	    std::stringstream os;

	    os << prefix << "-" << (i++);

	    return os.str();
	}

	/* For each candidate action
	 * 	For each possible effect that this token can be merged with
	 *		1. Activate token that needs support
	 *		2. Activate candidate supporting action
	 *		3. Merge target effect from action into token that needs support
	 * 		4. Add all of the action's effects into the plan
	 */
	void SupportToken::execute()
	{
		TokenTypeId actionType = m_choices[m_actionIndex].first;

		debugMsg("SupportToken", "Activating supporting action:" << actionType->getName() << " for \n" << m_token->toLongString());

		// 1. Activate token that needs support
		m_dbClient->activate(m_token);

		// 2. Activate candidate supporting action
		m_action = m_dbClient->createToken(
				actionType->getSignature().c_str(), // TODO: getSignature() should be getQualifiedName(), or something like that
				autoName(actionType->getName()).c_str(),
				false, //isRejectable
				false //isFact
		);
		m_dbClient->activate(m_action);

		debugMsg("SupportToken", "Activated supporting action:\n" << m_action->toLongString());

		// 3. Merge target effect from action into token that needs support
		int effectCnt = 0;
		PSList<PSToken*> slaves = m_action->getSlaves();
		for (int i=0;i<slaves.size();i++) {
			PSToken* slave = slaves.get(i);
			if (slave->getAttributes() & PSTokenType::EFFECT) { // TODO: attribute should be evaluated by token type
				if (slave->getTokenType() == m_token->getTokenType()) { // TODO: use ids
					if (effectCnt == m_effectIndex) {
						// TODO: eliminate the need for the dynamic cast below
						TokenId slaveId = dynamic_cast<Token*>(slave)->getId();
						m_targetEffect = slaveId;
						m_dbClient->merge(m_targetEffect,m_token);
						break;
					}
					else
						effectCnt++;
				}
			}
		}
		checkError(m_targetEffect.isId(),"Expected effect to support fluent");

		debugMsg("SupportToken", "Merged with target effect:\n" << m_targetEffect->toLongString());
		debugMsg("SupportToken", "Supported token:\n" << m_token->toLongString());
		debugMsg("SupportToken", "Supporting action:\n" << m_action->toLongString());

		// 4. Add all of the action's effects into the plan
		// TODO: make sure that all of the activate/merge decisions for the action's effects are processed next
		// before any other choicepoints are considered

		m_effectIndex++;
		if (m_effectIndex >= m_choices[m_actionIndex].second) {
			m_actionIndex++;
			m_effectIndex=0;
		}
		m_isExecuted = ((unsigned int)m_actionIndex >= m_choices.size());
	}

	/*
	 * Rollback execute() in reverse order:
	 * 	1. Remove action's effects from the plan
	 * 	2. Cancel the support provided by the candidate action
	 * 	3. Destroy candidate action that was created by this decision point
	 * 	4. Cancel the activation of the token that needed support
	 */
	void SupportToken::undo()
	{
		// 1. Remove action's effects from the plan
		// TODO: In the context of chronological backtracking all the effects should be removed by now
		// otherwise we may have to explicitly undo the entire slave tree for the supporting action

		// 2. Cancel the support provided by the candidate action
		m_dbClient->cancel(m_targetEffect);
		m_targetEffect = TokenId::noId();

		// 3. Destroy candidate action that was created by this decision point
		m_dbClient->cancel(m_action);
		delete (Token*)m_action;
		m_action = TokenId::noId();

		// 4. Cancel the activation of the token that needed support
		m_dbClient->cancel(m_token);
	}
  }
}

