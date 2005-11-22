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

    bool OpenConditionManager::inScope(const EntityId& entity) const {
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

    /**
     * Implementation employs a crude heuristic to obtain the most constrained
     * decision point. Idea is to look ahead to how many merges it has, and if activation
     * and rejection are options. Can do much better with this by incorportaing look ahead for
     * active tokens in prioritizing and also be developing much better caching mechanisms
     * that can use prior results to guide the retrieval.
     */
    DecisionPointId OpenConditionManager::next(unsigned int priorityLowerBound,
					   unsigned int& bestPriority){

      // First we filter and sort candidate tokens to order according to our flaw filtering rules and the previously
      // counted number of choices. A likely useful method for leveraging cached information to find unit decisions
      // sooner.
      std::map<int, TokenId> candidates;
      for(TokenSet::const_iterator it = m_flawCandidates.begin(); it != m_flawCandidates.end(); ++it){
	TokenId candidate = *it;
	checkError(candidate->isInactive(), 
		   "It must be inactive to be a candidate." << candidate->getState()->lastDomain().toString());

	if(dynamicMatch(candidate)){
	  debugMsg("OpenConditionManager:next",
		   candidate->toString() << " is out of dynamic scope.");
	  continue;
	}

	// Now insert in order of last count computed to increase chance of finding better choices early.
	unsigned int lastCount = m_db->lastOrderingChoiceCount(candidate) + m_db->lastCompatibleTokenCount(candidate);
	candidates.insert(std::pair<int, TokenId>(lastCount, candidate));
      }

      TokenId flawedToken;

      for(std::map<int, TokenId>::const_iterator it = candidates.begin(); it != candidates.end(); ++it){
	if(bestPriority == priorityLowerBound) // Can't do better so go with what we have
	  break;

	TokenId candidate = it->second;

	unsigned int priority = getPriority(candidate, bestPriority);

	// If we have a better candidate, update the best pick data
	if(priority < bestPriority){
	  bestPriority = priority;
	  flawedToken = candidate;
	}

	debugMsg("OpenConditionManager:next",
		 candidate->toString() << 
		 (candidate == flawedToken ? " is the best candidate so far." : " is not a better candidate."));
      }

      if(flawedToken.isNoId())
	return DecisionPointId::noId();

      DecisionPointId decisionPoint = allocateDecisionPoint(flawedToken);

      checkError(decisionPoint.isValid(),
		 "Failed to allocate a decision point for " << flawedToken->toString() <<
		 ". This indicates a failure to configure a FlawHandler for this flaw.");

      return decisionPoint;
    }


    unsigned int OpenConditionManager::getPriority(const TokenId& candidate, unsigned int bestPriority){
      checkError(candidate->isInactive(), 
		 "Only expet to get priorities on inactive tokens: " << candidate->toString());

      unsigned int priority = 1;

      if(candidate->getState()->lastDomain().isMember(Token::REJECTED))
	priority++;

      // If a merge is allowed we want to evaluate the number of likely choices and add them
      // to the current priority. Note that the count has to beat the bestPriority by the priority
      // accumulated so far. We do this last since it is the most expensive one.
      if(priority < bestPriority && candidate->getState()->lastDomain().isMember(Token::MERGED))
	priority = priority + candidate->getPlanDatabase()->countCompatibleTokens(candidate, 2);

      // If an activation is allowed, we want to evaluate the number of ordering choices on this token
      // to see if it is actually worthwhile. This lookAhead may be expensive but could reduce it alot
      // with some useful caching
      if(priority < bestPriority &&
	 candidate->getState()->lastDomain().isMember(Token::ACTIVE) &&
	 candidate->getPlanDatabase()->hasOrderingChoice(candidate))
	priority++;

      return priority;
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

    IteratorId OpenConditionManager::createIterator() const {
      IteratorId retval = (new FlawIterator(*this))->getId();
      return retval;
    }

    OpenConditionManager::FlawIterator::FlawIterator(const OpenConditionManager& manager) 
      : m_visited(0), m_timestamp(manager.m_db->getConstraintEngine()->cycleCount()),
	m_manager(manager), m_it(manager.m_flawCandidates.begin()), m_end(manager.m_flawCandidates.end()) {}

    bool OpenConditionManager::FlawIterator::done() const {
      return m_it == m_end;
    }

    const EntityId OpenConditionManager::FlawIterator::next() {
      check_error(m_manager.m_db->getConstraintEngine()->cycleCount() == m_timestamp,
		  "Error: potentially stale flaw iterator.");
      TokenId retval = TokenId::noId();

      for(; !done(); ++m_it) {
	TokenId tok = *m_it;
	check_error(tok.isValid());
	if(m_manager.inScope(tok)) {	
	  retval = tok;
	  ++m_visited;
	  ++m_it;
	  break;
	}
      }
      return retval;
    }
  }
}
