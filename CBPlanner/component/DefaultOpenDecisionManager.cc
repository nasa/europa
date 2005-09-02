#include "Debug.hh"
#include "ConstrainedVariable.hh"
#include "Object.hh"
#include "DecisionManager.hh"
#include "OpenDecisionManager.hh"
#include "PlanDatabase.hh"
#include "UnifyMemento.hh"
#include "Token.hh"
#include "TokenVariable.hh"
#include "TokenDecisionPoint.hh"
#include "ConstrainedVariableDecisionPoint.hh"
#include "ObjectDecisionPoint.hh"
#include "DefaultOpenDecisionManager.hh"

namespace EUROPA {

  DefaultOpenDecisionManager::DefaultOpenDecisionManager(const PlanDatabaseId& db) : OpenDecisionManager(db) {}

  DefaultOpenDecisionManager::~DefaultOpenDecisionManager() {
  }

  DecisionPointId DefaultOpenDecisionManager::getNextDecision(){
    return DecisionPointId::noId();
  }

  void DefaultOpenDecisionManager::initializeTokenChoices(TokenDecisionPointId& tdp) {
    const StateDomain stateDomain(tdp->getToken()->getState()->lastDomain());
    TokenId tok(tdp->getToken());
    if(stateDomain.isMember(Token::MERGED)) {
      tok->getPlanDatabase()->getCompatibleTokens(tok, tdp->m_compatibleTokens, PLUS_INFINITY, true);
      debugMsg("ObjectDecisionPoint:initializeTokenChoices", "Found " << tdp->m_compatibleTokens.size() << " compatible tokens");
      if(tdp->m_compatibleTokens.size() > 0) {
        debugMsg("ObjectDecisionPoint:initializeTokenChoices", "Pushing token:merged m_choices");
	tdp->m_choices.push_back(Token::MERGED);
      }
    }
    if(stateDomain.isMember(Token::ACTIVE) && tok->getPlanDatabase()->hasOrderingChoice(tok))
      tdp->m_choices.push_back(Token::ACTIVE);
    if(stateDomain.isMember(Token::REJECTED))
      tdp->m_choices.push_back(Token::REJECTED);
  }

  void DefaultOpenDecisionManager::initializeVariableChoices(ConstrainedVariableDecisionPointId& vdp) {
    if (vdp->m_var->lastDomain().isNumeric() && vdp->m_var->lastDomain().getSize() > 50) {
      vdp->m_choices.push_back(vdp->m_var->lastDomain().getLowerBound());
      vdp->m_choices.push_back(vdp->m_var->lastDomain().getUpperBound()); // we'll keep the initial lb and ub for reference
    }
    else {
      std::list<double> values;
      vdp->m_var->lastDomain().getValues(values);
      values.sort();
      for(std::list<double>::const_iterator it = values.begin(); it != values.end(); ++it)
	vdp->m_choices.push_back(*it);
    }
  }

  void DefaultOpenDecisionManager::initializeObjectChoices(ObjectDecisionPointId& odp) {
    std::list<double> values;
    TokenId tok(odp->getToken());
    tok->getObject()->getLastDomain().getValues(values);
    std::list<double>::iterator it = values.begin();
    for ( ; it != values.end(); ++it) {
      ObjectId obj = *it;
      check_error(obj.isValid());
      std::vector<std::pair<TokenId, TokenId> > tuples;
      obj->getOrderingChoices(tok, tuples);
      std::vector<std::pair<TokenId, TokenId> >::iterator it = tuples.begin();
      debugMsg("ObjectDecisionPoint:getChoices", "Choices constraining (" << tok->getKey() << ")");
      for (; it != tuples.end(); it++) {
	TokenId predecessor = it->first;
	TokenId successor = it->second;
	check_error(predecessor.isValid());
	check_error(successor.isValid());
	odp->m_choices.push_back(std::make_pair<ObjectId,std::pair<TokenId,TokenId> > (obj, *it));
	debugMsg("ObjectDecisionPoint:getChoices", "  constrain(" << predecessor->getKey() << ", " << successor->getKey() << ") on Object ( " << obj->getKey() << ")");
      }
    }
  }

}
