#include "OpenConditionDecisionPoint.hh"
#include "PlanDatabase.hh"
#include "Token.hh"
#include "TokenVariable.hh"
#include "ConstrainedVariable.hh"

namespace EUROPA {
  namespace SOLVERS {

    OpenConditionDecisionPoint::OpenConditionDecisionPoint(const DbClientId& client, const TokenId& flawedToken, const TiXmlElement& configData)
      : DecisionPoint(client, flawedToken->getKey()),
	m_flawedToken(flawedToken), 
	m_mergeIndex(0),
	m_mergeCount(0),
	m_choiceIndex(0),
	m_choiceCount(0){

      // Retrieve policy information from configuration node

      // Ordering preference - merge vs. activation. Default is to merge, activate, reject.

      // Activation preference - direct vs. indirect. Default is to use direct activation
    }

    OpenConditionDecisionPoint::~OpenConditionDecisionPoint() {}

    void OpenConditionDecisionPoint::handleInitialize(){
      const StateDomain stateDomain(m_flawedToken->getState()->lastDomain());

      // Next merge choices if there are any.
      if(stateDomain.isMember(Token::MERGED)){
	// Use exact test in this case
	m_flawedToken->getPlanDatabase()->getCompatibleTokens(m_flawedToken, m_compatibleTokens, PLUS_INFINITY, true);
	m_mergeCount = m_compatibleTokens.size();
	if(m_mergeCount > 0)
	  m_choices.push_back(Token::MERGED);
      }

      if(stateDomain.isMember(Token::ACTIVE) && m_flawedToken->getPlanDatabase()->hasOrderingChoice(m_flawedToken))
	m_choices.push_back(Token::ACTIVE);

      if(stateDomain.isMember(Token::REJECTED))
	m_choices.push_back(Token::REJECTED);

      m_choiceCount = m_choices.size();
    }

    void OpenConditionDecisionPoint::handleExecute(){
      checkError(m_choiceIndex < m_choiceCount, 
		 "Tried to execute past available choices:" << m_choiceIndex << ">=" << m_choiceCount);

      if(m_mergeIndex < m_mergeCount){
	checkError(m_choiceIndex == 0, 
		   "Expect Merging to be the first choice but index is:" << m_choiceIndex);
	checkError(m_choices[m_choiceIndex] == Token::MERGED, 
		   "Expect this choice to be a merge instead it is:" << m_choices[m_choiceIndex].toString());
	TokenId activeToken = m_compatibleTokens[m_mergeIndex];
	m_client->merge(m_flawedToken, activeToken);
      }
      else if(m_choices[m_choiceIndex] == Token::ACTIVE)
	m_client->activate(m_flawedToken);
      else {
	checkError(m_choices[m_choiceIndex] == Token::REJECTED, 
		   "Expect this choice to be REJECTED instead of " + m_choices[m_choiceIndex].toString());
	m_client->reject(m_flawedToken);
      }
    }

    void OpenConditionDecisionPoint::handleUndo(){
      m_client->cancel(m_flawedToken);

      if(m_mergeIndex < m_mergeCount)
	m_mergeIndex++;

      if(m_mergeIndex == m_mergeCount)
	m_choiceIndex++;
    }

    bool OpenConditionDecisionPoint::hasNext() const {
      return m_choiceIndex < m_choiceCount;
    }

    std::string OpenConditionDecisionPoint::toString() const{
      std::stringstream strStream;
      strStream << "TOKEN STATE: TOKEN=" << 
	m_flawedToken->getName().toString()  << "(" << m_flawedToken->getKey() << "), " <<
	"CHOICES=";

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
  }
}

