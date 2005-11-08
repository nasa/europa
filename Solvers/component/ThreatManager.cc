#include "ThreatManager.hh"
#include "PlanDatabase.hh"
#include "Token.hh"
#include "TokenVariable.hh"
#include "Object.hh"
#include "Debug.hh"
#include "Utils.hh"

/**
 * @author Conor McGann
 * @date May, 2005
 */
namespace EUROPA {
  namespace SOLVERS {

    ThreatManager::ThreatManager(const TiXmlElement& configData)
      : FlawManager(configData){}

    ThreatManager::~ThreatManager(){}

    void ThreatManager::handleInitialize() {}

    bool ThreatManager::inScope(const EntityId& entity) const {
      checkError(TokenId::convertable(entity), entity->toString());
      TokenId token = entity;
      return token->isActive() && FlawManager::inScope(entity);
    }

    DecisionPointId ThreatManager::next(unsigned int priorityLowerBound,
					unsigned int& bestPriority){

      // First we filter and sort candidate tokens to order according to our flaw filtering rules and the previously
      // counted number of choices.
      const std::map<int, std::pair<TokenId, ObjectSet> >& tokensToOrder = m_db->getTokensToOrder();
      std::map<int, TokenId> candidates;
      for(std::map<int, std::pair<TokenId, ObjectSet> >::const_iterator it = tokensToOrder.begin(); 
	  it != tokensToOrder.end(); ++it){
	TokenId candidate = it->second.first;
	checkError(candidate.isValid(), candidate);
	checkError(candidate->isActive(), "It must be inactive to be a candidate. " 
		   << candidate->toString() << ";" << candidate->getState()->toString());

	if(!inScope(candidate)){
	  debugMsg("ThreatManager:next",
		   candidate->toString() << " is out of scope.");
	  continue;
	}

	// Now insert in order of last count computed to increase chance of finding better choices early.
	unsigned int lastCount = m_db->lastOrderingChoiceCount(candidate);
	candidates.insert(std::pair<int, TokenId>(lastCount, candidate));
      }


      // Now we may have some candidates in scope to evaluate in order to get the best token to order
      TokenId tokenToOrder;
      for(std::map<int, TokenId>::const_iterator it = candidates.begin(); it != candidates.end(); ++it){
	if(bestPriority == priorityLowerBound) // Can't do better
	  break;

	TokenId candidate = it->second;

	unsigned int priority = 0;

	priority = m_db->countOrderingChoices(candidate);

	if(priority < bestPriority){
	  bestPriority = priority;
	  tokenToOrder = candidate;
	}

	debugMsg("ThreatManager:next",
		 candidate->toString() << 
		 (candidate == tokenToOrder ? " is the best candidate so far." : " is not a better candidate."));
      }

      if(tokenToOrder.isNoId())
	return DecisionPointId::noId();

      DecisionPointId decisionPoint = allocateDecisionPoint(tokenToOrder);

      checkError(decisionPoint.isValid(),
		 "Failed to allocate a decision point for " << tokenToOrder->toString() <<
		 " Indicates that no FlawHandler has been configured for this flaw.");

      return decisionPoint;
    }
  }
}
