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
      : FlawManager(configData){

      checkError(strcmp(configData.Value(), "ThreatManager") == 0,
		 "Error in configuration file. Expected element <ThreatManager> but found " << configData.Value());

      // Load all filtering rules
      for (TiXmlElement * child = configData.FirstChildElement(); 
	   child != NULL; 
	   child = child->NextSiblingElement()) {
	debugMsg("ThreatManager:ThreatManager",
		 "Evaluating configuration element " << child->Value());

	// If we come across a token heuristic, register the factory.
	if(strcmp(child->Value(), "FlawHandler") == 0){
	  ThreatDecisionPointFactoryId factory = static_cast<ThreatDecisionPointFactoryId>(Component::AbstractFactory::allocate(*child));
	  m_factories.push_back(factory);
	}
	else { // Must be a token filter
	  checkError(strcmp(child->Value(), "FlawFilter") == 0,
		     "Error in configuratiuon file. Expected element <FlawFilter> but found " << child->Value());

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

    ThreatManager::~ThreatManager(){
      EUROPA::cleanup(m_staticMatchingRules);
      EUROPA::cleanup(m_dynamicMatchingRules);
      EUROPA::cleanup(m_factories);
    }

    void ThreatManager::handleInitialize() {}

    bool ThreatManager::inScope(const TokenId& token) const {
      checkError(m_db->getConstraintEngine()->constraintConsistent(), 
		 "Assumes the database is constraint consistent but it is not.");

      bool result =  (token->isActive() && 
		      !matches(token, m_staticMatchingRules) && 
		      !matches(token, m_dynamicMatchingRules));

      return result;
    }

    bool ThreatManager::matches(const TokenId& token,const std::list<TokenMatchingRuleId>& rules)  const{
      LabelStr objectType, predicate;
      TokenMatchingRule::extractParts(token, objectType, predicate);

      for(std::list<TokenMatchingRuleId>::const_iterator it = rules.begin(); it != rules.end(); ++it){
	TokenMatchingRuleId rule = *it;
	check_error(rule.isValid());
	if(rule->matches(objectType, predicate) && rule->matches(token)){
	  debugMsg("ThreatManager:matches", 
		   "Match found for " << TokenMatchingRule::makeExpression(token) << " with " << rule->getExpression());
	  return true;
	}
	else {
	  debugMsg("ThreatManager:matches", 
		   "No match for " << TokenMatchingRule::makeExpression(token) << " with " << rule->getExpression());
	}
      }

      return false;
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

    DecisionPointId ThreatManager::allocateDecisionPoint(const TokenId& tokenToOrder){
      static unsigned int sl_counter(0); // Helpful for debugging
      sl_counter++;

      std::list<ThreatDecisionPointFactoryId>::const_iterator it = m_factories.begin();
      LabelStr predicate(tokenToOrder->getPredicateName());
      LabelStr objectType(tokenToOrder->getBaseObjectType());

      DecisionPointId result;

      while(it != m_factories.end()){
	ThreatDecisionPointFactoryId factory = *it;
	if(factory->matches(predicate, objectType) && factory->matches(tokenToOrder)){
	  result = factory->create(m_db->getClient(), tokenToOrder);
	  break;
	}
	++it;
      }

      checkError(result.isValid(),
		 "At count " << sl_counter << ": No Decision Point could be allocated for " 
		 << tokenToOrder->toString());

      return result;
    }
  }
}
