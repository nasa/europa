#include "Utils.hh"
#include "Debug.hh"
#include "Error.hh"
#include "Token.hh"
#include "TokenVariable.hh"
#include "OpenConditionManager.hh"
#include "PlanDatabase.hh"


/**
 * @author Conor McGann
 * @date March, 2005
 */
namespace EUROPA {
  namespace SOLVERS {

    OpenConditionManager::OpenConditionManager(const TiXmlElement& configData)
      : FlawManager(configData) {}

    void OpenConditionManager::handleInitialize(){
      // FILL UP TOKENS
      const TokenSet& allTokens = m_db->getTokens();
      for(TokenSet::const_iterator it = allTokens.begin(); it != allTokens.end(); ++it){
	TokenId token = *it;
	addFlaw(token);
      }
    }

    bool OpenConditionManager::inScope(const EntityId& entity){
      bool result = false;

      if(TokenId::convertable(entity)){
	TokenId token = entity;

	result = token->isInactive() && FlawManager::inScope(entity);
      }

      return result;
    }

    void OpenConditionManager::addFlaw(const TokenId& token){
      if(token->isInactive() && !staticMatch(token)){
	debugMsg("OpenConditionManager:addFlaw",
		 "Adding " << token->toString() << " as a candidate flaw.");
	m_flawCandidates.insert(token);
      }
    }

    void OpenConditionManager::removeFlaw(const TokenId& token){
      condDebugMsg(m_flawCandidates.find(token) != m_flawCandidates.end(), "OpenConditionManager:removeFlaw", "Removing " << token->toString() << " as a flaw.");
      m_flawCandidates.erase(token);
    }

    void OpenConditionManager::notifyRemoved(const ConstrainedVariableId& variable){
      if(Token::isStateVariable(variable))
	removeFlaw(variable->getParent());
    }

    void OpenConditionManager::notifyChanged(const ConstrainedVariableId& variable, 
					     const DomainListener::ChangeType& changeType){
      if(!Token::isStateVariable(variable))
	return;

      if(changeType == DomainListener::RESET)
	addFlaw(variable->getParent());
      else if(changeType == DomainListener::SET_TO_SINGLETON)
	removeFlaw(variable->getParent());
      else if(changeType == DomainListener::CLOSED)
	addFlaw(variable->getParent());
    }

    IteratorId OpenConditionManager::createIterator(){
      IteratorId retval = (new FlawIterator(*this))->getId();
      return retval;
    }

    OpenConditionManager::FlawIterator::FlawIterator(OpenConditionManager& manager) 
      : m_visited(0), m_timestamp(manager.m_db->getConstraintEngine()->cycleCount()),
	m_manager(manager), m_it(manager.m_flawCandidates.begin()), m_end(manager.m_flawCandidates.end()) {

      // Must advance to the first available flaw in scope.
      while(!done()){
	TokenId tok = *m_it;
	if(!m_manager.dynamicMatch(tok))
	  break;
	else
	  ++m_it;
      }
    }

    bool OpenConditionManager::FlawIterator::done() const {
      return m_it == m_end;
    }

    const EntityId OpenConditionManager::FlawIterator::next() {
      check_error(m_manager.m_db->getConstraintEngine()->cycleCount() == m_timestamp,
		  "Error: potentially stale flaw iterator.");
      checkError(!done(), "Cannot be done when you call next.");

      // Pick up the flaw for the current position
      TokenId flaw = *m_it;
      checkError(!m_manager.dynamicMatch(flaw), "Not advancing correctly.");
      ++m_visited;

      // Advance till we get another hit
      ++m_it;
      while(!done()){
	TokenId tok = *m_it;
	if(!m_manager.dynamicMatch(tok))
	  break;
	else
	  ++m_it;
      }

      return flaw;
    }

    std::string OpenConditionManager::toString(const EntityId& entity) const {
      checkError(TokenId::convertable(entity), entity->toString());
      TokenId token = entity;
      std::stringstream os;
      os << "TOKEN: " << token->toString();
      return os.str();
    }
  }
}
