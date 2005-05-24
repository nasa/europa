#include "OpenConditionManager.hh"
#include "PlanDatabase.hh"
#include "Token.hh"
#include "Utils.hh"
#include "Debug.hh"
#include "Error.hh"

/**
 * @author Conor McGann
 * @date March, 2005
 */
namespace EUROPA {
  namespace SOLVERS {

    /* REGSITER DEFAULT TOKEN FLAW MANAGER */
    REGISTER_COMPONENT_FACTORY(OpenConditionManager, OpenConditionManager);

    OpenConditionManager::OpenConditionManager(const TiXmlElement& configData)
      : FlawManager(configData), m_dbListener(NULL) {

      checkError(strcmp(configData.Value(), "OpenConditionManager") == 0,
		 "Expected element <OpenConditionManager> but found " << configData.Value());

      // Load all filtering rules
      for (TiXmlElement * child = configData.FirstChildElement(); 
	   child != NULL; 
	   child = child->NextSiblingElement()) {
	debugMsg("OpenConditionManager:OpenConditionManager",
		 "Evaluating configuration element " << child->Value());

	// If we come across a token heuristic, register the factory.
	if(strcmp(child->Value(), "FlawHandler") == 0){
	  OpenConditionDecisionPointFactoryId factory = static_cast<OpenConditionDecisionPointFactoryId>(Component::AbstractFactory::allocate(*child));
	  m_factories.push_back(factory);
	}
	else { // Must be a token filter
	  checkError(strcmp(child->Value(), "FlawFilter") == 0,
		     "Expected element <FlawFilter> but found " << child->Value());

	  const char* component = child->Attribute("component");

	  if(component == NULL){ // Allocate default. It will be static.
	    TokenMatchingRuleId rule = (new TokenMatchingRule(*child))->getId();
	    m_staticMatchingRules.push_back(rule);
	  }
	  else{ // Allocate via registered factory
	    TokenMatchingRuleId rule = static_cast<TokenMatchingRuleId>(Component::AbstractFactory::allocate(*child));
	    m_dynamicMatchingRules.push_back(rule);
	  }
	}
      }
    }

    void OpenConditionManager::handleInitialize(){
      m_dbListener = new DbListener(m_db, *this);

      // FILL UP TOKENS
      const TokenSet& allTokens = m_db->getTokens();
      for(TokenSet::const_iterator it = allTokens.begin(); it != allTokens.end(); ++it){
	TokenId token = *it;
	addFlaw(token);
      }
    }

    OpenConditionManager::~OpenConditionManager(){
      if(m_dbListener != NULL)
	delete m_dbListener;

      EUROPA::cleanup(m_staticMatchingRules);
      EUROPA::cleanup(m_dynamicMatchingRules);
      EUROPA::cleanup(m_factories);
    }


    bool OpenConditionManager::inScope(const TokenId& token) const {
      checkError(m_db->getConstraintEngine()->constraintConsistent(), "Assumes the database is constraint consistent but it is not.");
      bool result =  (!token->isActive() && !matches(token, m_staticMatchingRules) && !matches(token, m_dynamicMatchingRules));
      return result;
    }

    bool OpenConditionManager::matches(const TokenId& token,const std::list<TokenMatchingRuleId>& rules)  const{
      LabelStr objectType, predicate;
      TokenMatchingRule::extractParts(token, objectType, predicate);

      for(std::list<TokenMatchingRuleId>::const_iterator it = rules.begin(); it != rules.end(); ++it){
	TokenMatchingRuleId rule = *it;
	check_error(rule.isValid());
	if(rule->matches(objectType, predicate) && rule->matches(token)){
	  debugMsg("OpenConditionManager:matches", "Match found for " << TokenMatchingRule::makeExpression(token) << " with " << rule->getExpression());
	  return true;
	}
	else {
	  debugMsg("OpenConditionManager:matches", "No match for " << TokenMatchingRule::makeExpression(token) << " with " << rule->getExpression());
	}
      }

      return false;
    }


    /**
     * @brief Now we conduct a simple match where we select based on first avalaibale.
     */
    DecisionPointId OpenConditionManager::allocateDecisionPoint(const TokenId& flawedToken){
      static unsigned int sl_counter(0); // Helpful for debugging
      sl_counter++;

      std::list<OpenConditionDecisionPointFactoryId>::const_iterator it = m_factories.begin();
      LabelStr predicate(flawedToken->getPredicateName());
      LabelStr objectType(flawedToken->getBaseObjectType());

      DecisionPointId result;

      while(it != m_factories.end()){
	OpenConditionDecisionPointFactoryId factory = *it;
	if(factory->matches(predicate, objectType) && factory->matches(flawedToken)){
	  result = factory->create(m_db->getClient(), flawedToken);
	  break;
	}
	++it;
      }

      checkError(result.isValid(),
		 "At count " << sl_counter << ": No Decision Point could be allocated for " 
		 << flawedToken->toString());

      return result;
    }

    void OpenConditionManager::addFlaw(const TokenId& token){
      if(token->isInactive() && !matches(token, m_staticMatchingRules)){
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
	checkError(candidate->isInactive(), "It must be inactive to be a candidate.");


	if(matches(candidate, m_dynamicMatchingRules)){
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

	unsigned int priority = 0;

	if(candidate->getState()->lastDomain().isMember(Token::REJECTED))
	  priority++;

	// Provide a quick exit if this candiate has met or exceded the best priority already
	if(priority >= bestPriority)
	  continue;

	// If a merge is allowed we want to evaluate the number of likely choices and add them
	// to the current priority. Note that the count has to beat the bestPriority by the priority
	// accumulated so far. We do this last since it is the most expensive one
	if(candidate->getState()->lastDomain().isMember(Token::MERGED))
	  priority = priority + candidate->getPlanDatabase()->countCompatibleTokens(candidate, 
										    bestPriority-priority);

	// Skip if we can!
	if(priority >= bestPriority)
	  continue;

	// If an activation is allowed, we want to evaluate the number of ordering choices on this token
	// to see if it is actually worthwhile. This lookAhead may be expensive but could reduce it alot
	// with some useful caching
	if(candidate->getState()->lastDomain().isMember(Token::ACTIVE) &&
	   candidate->getPlanDatabase()->hasOrderingChoice(candidate))
	  priority++;

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
		 "Failed to allocate a decision point for " << flawedToken->toString());

      return decisionPoint;
    }

    OpenConditionManager::DbListener::DbListener(const PlanDatabaseId& planDb,
					     OpenConditionManager& dm)
      : PlanDatabaseListener(planDb), m_dm(dm) {}

    void OpenConditionManager::DbListener::notifyAdded(const TokenId& token){
      m_dm.addFlaw(token);
    }

    void OpenConditionManager::DbListener::notifyRemoved(const TokenId& token){
      m_dm.removeFlaw(token);
    }

    void OpenConditionManager::DbListener::notifyActivated(const TokenId& token){
      m_dm.removeFlaw(token);
    }

    void OpenConditionManager::DbListener::notifyDeactivated(const TokenId& token){
      m_dm.addFlaw(token);
    }

    void OpenConditionManager::DbListener::notifyMerged(const TokenId& token){
      m_dm.removeFlaw(token);
    }

    void OpenConditionManager::DbListener::notifySplit(const TokenId& token){
      m_dm.addFlaw(token);
    }

    void OpenConditionManager::DbListener::notifyRejected(const TokenId& token){
      m_dm.removeFlaw(token);
    }

    void OpenConditionManager::DbListener::notifyReinstated(const TokenId& token){
      m_dm.addFlaw(token);
    }

  }
}
