#include "DecisionManager.hh"
#include "OpenDecisionManager.hh"
#include "PlanDatabase.hh"
#include "Token.hh"
#include "TokenDecisionPoint.hh"
#include "ConstrainedVariable.hh"
#include "ConstrainedVariableDecisionPoint.hh"
#include "Object.hh"
#include "ObjectDecisionPoint.hh"
#include "Choice.hh"
#include "ValueChoice.hh"
#include "DefaultOpenDecisionManager.hh"

namespace PLASMA {

  DefaultOpenDecisionManager::DefaultOpenDecisionManager(const DecisionManagerId& dm) : OpenDecisionManager(dm) {
  }

  DefaultOpenDecisionManager::~DefaultOpenDecisionManager() {
    std::list<DecisionPointId> openDecs;
    getOpenDecisions(openDecs);
    std::map<int,DecisionPointId> alldecs;
    for (std::list<DecisionPointId>::iterator it = openDecs.begin(); it != openDecs.end(); ++it)
      alldecs.insert(std::make_pair((*it)->getKey(), *it));
    cleanup(alldecs);
    m_tokDecs.clear();
    m_unitVarDecs.clear();
    m_nonUnitVarDecs.clear();
    m_objDecs.clear();
  }


  void DefaultOpenDecisionManager::addActive(const TokenId& token) {
    check_error(token.isValid());
    check_error(m_objDecs.find(token->getKey()) == m_objDecs.end());
    DecisionPointId dp = createObjectDecisionPoint(token);
    check_error(dp->getEntityKey() == token->getKey());
    m_objDecs.insert(std::pair<int,ObjectDecisionPointId>(dp->getEntityKey(),dp));
    publishNewDecision(dp);
  }

  void DefaultOpenDecisionManager::condAddActive(const TokenId& token) {
    check_error(token.isValid());
    if (m_objDecs.find(token->getKey()) == m_objDecs.end()) {
      DecisionPointId dp = createObjectDecisionPoint(token);
      check_error(dp->getEntityKey() == token->getKey());
      m_objDecs.insert(std::pair<int,ObjectDecisionPointId>(dp->getEntityKey(),dp));
      publishNewDecision(dp);
    }
  }

  void DefaultOpenDecisionManager::condAdd(const ConstrainedVariableId& var, const bool units) {
    check_error(var.isValid());
    check_error(m_curDec.isValid() || m_curDec.isNoId());
    if (ConstrainedVariableDecisionPointId::convertable(m_curDec)) {
      ConstrainedVariableDecisionPointId vdp = m_curDec;
      if (vdp->getVariable()->getKey() == var->getKey())
	return;
    }
    if (units && m_unitVarDecs.find(var->getKey()) == m_unitVarDecs.end()) {
      ConstrainedVariableDecisionPointId dp = createConstrainedVariableDecisionPoint(var);
      check_error(dp->getEntityKey() == var->getKey());
      m_unitVarDecs.insert(std::pair<int,ConstrainedVariableDecisionPointId>(dp->getEntityKey(),dp));
      m_sortedUnitVarDecs.insert(dp);
      publishNewDecision(dp);
    }
    else if (m_nonUnitVarDecs.find(var->getKey()) == m_nonUnitVarDecs.end()) {
      ConstrainedVariableDecisionPointId dp = createConstrainedVariableDecisionPoint(var);
      check_error(dp->getEntityKey() == var->getKey());
      m_nonUnitVarDecs.insert(std::pair<int,ConstrainedVariableDecisionPointId>(dp->getEntityKey(),dp));
      m_sortedNonUnitVarDecs.insert(dp);
      publishNewDecision(dp);
    }
  }

  void DefaultOpenDecisionManager::add(const ConstrainedVariableId& var) {
    ConstrainedVariableDecisionPointId dp;
    dp = createConstrainedVariableDecisionPoint(var);
    check_error(dp->getEntityKey() == var->getKey());
    if (var->lastDomain().isSingleton()) {
      m_unitVarDecs.insert(std::pair<int,ConstrainedVariableDecisionPointId>(dp->getEntityKey(),dp));
      m_sortedUnitVarDecs.insert(dp);
      publishNewUnitDecision(dp);
    }
    else {
      m_nonUnitVarDecs.insert(std::pair<int,ConstrainedVariableDecisionPointId>(dp->getEntityKey(),dp));
      m_sortedNonUnitVarDecs.insert(dp);
      publishNewDecision(dp);
    }
  }

  void DefaultOpenDecisionManager::add(const TokenId& token) {
    // closing is something we only do once, so no need to see if it
    // already exists before adding to open decisions
    TokenDecisionPointId dp = createTokenDecisionPoint(token);
    check_error(dp->getEntityKey() == token->getKey());
    m_tokDecs.insert(std::pair<int,TokenDecisionPointId>(dp->getEntityKey(),dp));
    m_sortedTokDecs.insert(dp);
    publishNewDecision(dp);
  }

  // needs to be here because this may trigger custom object decisions to
  // be created. However, we don't trigger these decisions in the default
  // decision model. 
  void DefaultOpenDecisionManager::add(const ObjectId& object) {
  }

  void DefaultOpenDecisionManager::condAdd(const TokenId& token) {
    if (m_tokDecs.find(token->getKey()) != m_tokDecs.end()) return;
    TokenDecisionPointId dp = createTokenDecisionPoint(token);
    m_tokDecs.insert(std::pair<int,TokenDecisionPointId>(dp->getEntityKey(),dp));
    m_sortedTokDecs.insert(dp);
    publishNewDecision(dp);
  }

  const bool DefaultOpenDecisionManager::removeVarDP(const ConstrainedVariableId& var, const bool deleting, std::map<int,ConstrainedVariableDecisionPointId>& varMap, VariableDecisionSet& sortedVars) {

    std::map<int,ConstrainedVariableDecisionPointId>::iterator it = varMap.find(var->getKey());
    if (it != varMap.end()) {
      if (deleting) {
	ConstrainedVariableDecisionPointId dec = it->second;
	check_error(dec.isValid());
	sortedVars.erase(dec);
	varMap.erase(it);
	m_dm->deleteDecision(dec);
      } else {
	sortedVars.erase(it->second);
	varMap.erase(it);
      }
      publishRemovedDecision(var);
    }
    else return false;

    return true;
  }

  void DefaultOpenDecisionManager::removeVar(const ConstrainedVariableId& var, const bool deleting) {
    check_error(var.isValid());

    m_dm->getVariableChangeBuffer().erase(var);
    m_dm->getRelaxedBuffer().erase(var);

    if (!removeVarDP(var, deleting, m_unitVarDecs, m_sortedUnitVarDecs))
      removeVarDP(var, deleting, m_nonUnitVarDecs, m_sortedNonUnitVarDecs);

  }

  void DefaultOpenDecisionManager::condRemoveVar(const ConstrainedVariableId& var) { 
    check_error(m_curDec.isValid() || m_curDec.isNoId());
    if (ConstrainedVariableDecisionPointId::convertable(m_curDec)) {
      ConstrainedVariableDecisionPointId cvdec = m_curDec;
      if (cvdec->getVariable()->getKey() != var->getKey())
	removeVar(var, true);
      else 
	removeVar(var, false);
    }
    else 
      removeVar(var, true);
  }


  const bool DefaultOpenDecisionManager::removeTokenDP(const TokenId& token, const bool deleting, std::map<int,TokenDecisionPointId>& tokMap, TokenDecisionSet& sortedToks) {
    std::map<int,TokenDecisionPointId>::iterator it = tokMap.find(token->getKey());
    if (it != tokMap.end()) {
      //      if (deleting) {
      if (it->second->isOpen() || deleting) {
	TokenDecisionPointId dec = it->second;
	sortedToks.erase(dec);
	check_error(dec.isValid());
	tokMap.erase(it);
	m_dm->deleteDecision(dec);
      }
      else {
	sortedToks.erase(it->second);
	tokMap.erase(it);
      }
      publishRemovedDecision(token);
      return true;
    }
    return false;
  }

  void DefaultOpenDecisionManager::removeToken(const TokenId& token, const bool deleting) {
    check_error(token.isValid());

    m_dm->getTokenChangeBuffer().erase(token);
    m_dm->getCancelledBuffer().erase(token);

    removeTokenDP(token, deleting, m_tokDecs, m_sortedTokDecs);

  }

  void DefaultOpenDecisionManager::removeActive(const TokenId& token, const bool deleting) {
    /*
    if (ObjectDecisionPointId::convertable(m_curDec)) {
      ObjectDecisionPointId objdec = m_curDec;
      if (objdec->getToken()->getKey() == token->getKey())
	return;
    }
    */
    std::map<int,ObjectDecisionPointId>::iterator it = m_objDecs.find(token->getKey());
    if (it != m_objDecs.end()) {
      if (it->second->isOpen() || deleting) {
	ObjectDecisionPointId dec = it->second;
	check_error(dec.isValid());
	m_objDecs.erase(it);
	m_dm->deleteDecision(dec);
      }
      else {
	m_objDecs.erase(it);
      }
      publishRemovedDecision(token);
    }
  }

  const int DefaultOpenDecisionManager::getNumberOfDecisions() {
    return m_objDecs.size() + m_unitVarDecs.size() + m_nonUnitVarDecs.size() + m_tokDecs.size();
  }

  DecisionPointId DefaultOpenDecisionManager::getNextDecision() {
    if(!m_objDecs.empty())
      m_curDec = m_objDecs.begin()->second;
    else if (!m_sortedUnitVarDecs.empty())
      m_curDec = *m_sortedUnitVarDecs.begin();
    else if (!m_sortedTokDecs.empty()) 
      m_curDec = *m_sortedTokDecs.begin();
    else if (!m_sortedNonUnitVarDecs.empty()) 
      m_curDec = *m_sortedNonUnitVarDecs.begin();
    else m_curDec = DecisionPointId::noId();

    /* Shold be able to require that current choices are empty */
    check_error(m_curDec.isNoId() || m_curDec->getCurrentChoices().empty());

    return m_curDec;
  }

  void DefaultOpenDecisionManager::cleanupAllDecisionCaches() {
    cleanup(m_tokDecs);
    cleanup(m_unitVarDecs);
    cleanup(m_nonUnitVarDecs);
    cleanup(m_objDecs);
    
    m_tokDecs.clear();
    m_unitVarDecs.clear();
    m_nonUnitVarDecs.clear();
    m_objDecs.clear();

    m_sortedTokDecs.clear();
    m_sortedUnitVarDecs.clear();
    m_sortedNonUnitVarDecs.clear();
  }

  // if there's a merge choice, keep the first one you get and return that one.
  // otherwise prefer to activate
  // finally prefer to reject
  const ChoiceId DefaultOpenDecisionManager::getNextChoice() {
    check_error(m_curDec.isValid());
    m_curChoice = ChoiceId::noId();
    const std::list<ChoiceId>& choices = m_curDec->getUpdatedChoices();
    //    std::cout << "getNextChoice:: num choices " << choices.size() << std::endl;
    if (TokenDecisionPointId::convertable(m_curDec)) {
      ChoiceId merge;
      ChoiceId reject;
      ChoiceId activate;
 
      for (std::list<ChoiceId>::const_iterator it = choices.begin(); it != choices.end(); ++it) {
	ChoiceId choice = *it;
	check_error(choice.isValid());
	check_error(choice->getType() == Choice::VALUE);
	check_error(!m_curDec->hasDiscarded(choice));

	LabelStr val = Id<ValueChoice>(choice)->getValue();
	if (merge.isNoId() && val == Token::MERGED) {
	  merge = choice;
	  break; // we'll do this first, no point in assigning the rest.
	}
	if (val == Token::ACTIVE)
	  activate = choice;
	if (val == Token::REJECTED)
	  reject = choice;
	// we ignore choices for INACTIVE and INCOMPLETE.
      }

      if (!merge.isNoId())
	m_curChoice = merge;
      else if (!activate.isNoId())
	m_curChoice = activate;
      else if (!reject.isNoId()) 
	m_curChoice = reject;
    }      

    // If we have no choice selected and there is a choice, pick the first
    if (!choices.empty() && m_curChoice == ChoiceId::noId())
      m_curChoice = choices.front();

    //    if(m_curChoice.isNoId())
      //      std::cout << "DEBUG: No more choices" << std::endl;
    return m_curChoice;
  }


  void DefaultOpenDecisionManager::getOpenDecisions(std::list<DecisionPointId>& decisions) {
    std::map<int,ObjectDecisionPointId>::iterator oit = m_objDecs.begin();
    for (; oit != m_objDecs.end(); ++oit)
      decisions.push_back(oit->second);
    std::map<int,ConstrainedVariableDecisionPointId>::iterator vit = m_unitVarDecs.begin();
    for (; vit != m_unitVarDecs.end(); ++vit)
      decisions.push_back(vit->second);
    std::map<int,TokenDecisionPointId>::iterator it = m_tokDecs.begin();
    for (; it != m_tokDecs.end(); ++it)
      decisions.push_back(it->second);
    for (vit = m_nonUnitVarDecs.begin(); vit != m_nonUnitVarDecs.end(); ++vit)
      decisions.push_back(vit->second);
  }

  void DefaultOpenDecisionManager::printOpenDecisions(std::ostream& os) {
    std::map<int,ObjectDecisionPointId>::iterator oit = m_objDecs.begin();
    for (; oit != m_objDecs.end(); ++oit)
      os << oit->second << std::endl;
    std::map<int,ConstrainedVariableDecisionPointId>::iterator vit = m_unitVarDecs.begin();
    for (; vit != m_unitVarDecs.end(); ++vit)
      os << vit->second << std::endl;
    std::map<int,TokenDecisionPointId>::iterator it = m_tokDecs.begin();
    for (; it != m_tokDecs.end(); ++it)
      os << it->second << std::endl;
    for (vit = m_nonUnitVarDecs.begin(); vit != m_nonUnitVarDecs.end(); ++vit)
      os << vit->second << std::endl;
  }

}
