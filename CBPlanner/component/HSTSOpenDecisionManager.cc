#include "DefaultOpenDecisionManager.hh"
#include "HSTSOpenDecisionManager.hh"

namespace Prototype {

  HSTSOpenDecisionManager::HSTSOpenDecisionManager(const DecisionManagerId& dm) : DefaultOpenDecisionManager(dm) {
  }

  HSTSOpenDecisionManager::~HSTSOpenDecisionManager() { }

  void HSTSOpenDecisionManager::deleteAllMatchingObjects(const ObjectId& object, const TokenId& token) {
    std::multimap<int,ObjectDecisionPointId>::iterator it = m_objDecs.lower_bound(object->getKey());
    while(it != m_objDecs.upper_bound(object->getKey())) {
      ObjectDecisionPointId dec = it->second;
      check_error(dec.isValid());
      m_objDecs.erase(it++);
      delete (ObjectDecisionPoint*) dec;
      publishRemovedDecision(object);
    }
  }

  void HSTSOpenDecisionManager::add(const ObjectId& object, const TokenId& token) {
    DecisionPointId dp = createObjectDecisionPoint(object, token);
    check_error(dp->getEntityKey() == object->getKey());
    m_objDecs.insert(std::pair<int,ObjectDecisionPointId>(dp->getEntityKey(),dp));
    publishNewDecision(dp);
  }

  void HSTSOpenDecisionManager::add(const ObjectId& object) {
    check_error(object.isValid());
    std::vector<TokenId> tokens;
    object->getTokensToOrder(tokens);
    std::vector<TokenId>::iterator it = tokens.begin();
    for (; it != tokens.end(); ++it) {
      ObjectDecisionPointId dp = createObjectDecisionPoint(object, *it);
      check_error(dp->getEntityKey() == object->getKey());
      m_objDecs.insert(std::pair<int,ObjectDecisionPointId>(dp->getEntityKey(),dp));
      publishNewDecision(dp);
    }
    // no need to remove any token decision points because only call path
    // is from recomputeDecisions.
  }

  void HSTSOpenDecisionManager::condAdd(const ConstrainedVariableId& var, const bool units) {
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

  void HSTSOpenDecisionManager::add(const ConstrainedVariableId& var) {
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

  void HSTSOpenDecisionManager::add(const TokenId& token) {
    // closing is something we only do once, so no need to see if it
    // already exists before adding to open decisions
    if (token->getState()->lastDomain().isSingleton()) {
      TokenDecisionPointId dp = createTokenDecisionPoint(token);
      check_error(dp->getEntityKey() == token->getKey());
      m_unitTokDecs.insert(std::pair<int,TokenDecisionPointId>(dp->getEntityKey(),dp));
      m_sortedUnitTokDecs.insert(dp);
      publishNewUnitDecision(dp);
    }
    else {
      TokenDecisionPointId dp = createTokenDecisionPoint(token);
      check_error(dp->getEntityKey() == token->getKey());
      m_nonUnitTokDecs.insert(std::pair<int,TokenDecisionPointId>(dp->getEntityKey(),dp));
      m_sortedNonUnitTokDecs.insert(dp);
      publishNewDecision(dp);
    }
  }

  void HSTSOpenDecisionManager::condAdd(const TokenId& token) {
    TokenDecisionPointId dp;
    if (token->getState()->lastDomain().isSingleton()) {
      if (m_unitTokDecs.find(token->getKey()) != m_unitTokDecs.end()) return;
      dp = createTokenDecisionPoint(token);
      m_unitTokDecs.insert(std::pair<int,TokenDecisionPointId>(dp->getEntityKey(),dp));
      m_sortedUnitTokDecs.insert(dp);
    }
    else {
      if (m_nonUnitTokDecs.find(token->getKey()) != m_nonUnitTokDecs.end()) return;
      dp = createTokenDecisionPoint(token);
      m_nonUnitTokDecs.insert(std::pair<int,TokenDecisionPointId>(dp->getEntityKey(),dp));
      m_sortedNonUnitTokDecs.insert(dp);
    }
    publishNewDecision(dp);
  }

  void HSTSOpenDecisionManager::removeObject(const ObjectId& object, const TokenId& token, const bool deleting) {
    std::multimap<int,ObjectDecisionPointId>::iterator it = m_objDecs.lower_bound(object->getKey());
    while(it != m_objDecs.upper_bound(object->getKey())) {
      if (it->second->getToken()->getKey() == token->getKey()) {
	if (deleting) {
	  DecisionPointId dec = it->second;
	  m_dm->getRetractedBuffer().erase(dec);
	  check_error(dec.isValid());
	  m_objDecs.erase(it++);
	  delete (DecisionPoint*) dec;
	}
	else m_objDecs.erase(it++);
	publishRemovedDecision(object);
	break;
      }
      else ++it;
    }
  }

  DecisionPointId& HSTSOpenDecisionManager::getNextDecision() {
    if(!m_objDecs.empty())
      m_curDec = m_objDecs.begin()->second;
    else if (!m_sortedUnitVarDecs.empty())
      m_curDec = *m_sortedUnitVarDecs.begin();
    else if (!m_sortedUnitTokDecs.empty())
      m_curDec = *m_sortedUnitTokDecs.begin();
    else if (!m_sortedNonUnitTokDecs.empty()) 
      m_curDec = *m_sortedNonUnitTokDecs.begin();
    else if (!m_sortedNonUnitVarDecs.empty()) 
      m_curDec = *m_sortedNonUnitVarDecs.begin();
    else m_curDec = DecisionPointId::noId();

    return m_curDec;
  }

  void HSTSOpenDecisionManager::cleanupAllDecisionCaches() {
    Default::cleanupAllDecisionCaches();
    m_sortedObjectDecs.clear();
  }

  // if there's a merge choice, keep the first one you get and return that one.
  // otherwise prefer to activate
  // finally prefer to reject
  const ChoiceId& HSTSOpenDecisionManager::getNextChoice() {
    check_error(m_curDec.isValid());
    bool assigned(false); 
    std::list<ChoiceId> choices = m_curDec->getChoices();
    //    std::cout << "getNextChoice:: num choices " << choices.size() << std::endl;
    if (TokenDecisionPointId::convertable(m_curDec)) {
      ChoiceId merge;
      ChoiceId reject;
      ChoiceId activate;
      std::list<ChoiceId>::iterator it = choices.begin();
      for ( ; it != choices.end(); ++it) {
	check_error((*it)->getType() == Choice::VALUE);
	TokenDecisionPoint::State val = (TokenDecisionPoint::State)Id<ValueChoice>(*it)->getValue();
	if (merge.isNoId() && val == TokenDecisionPoint::MERGED) {
	  merge = (*it);
	  break; // we'll do this first, no point in assigning the rest.
	}
	if (val == TokenDecisionPoint::ACTIVE)
	  activate = (*it);
	if (val == TokenDecisionPoint::REJECTED)
	  reject = (*it);
	// we ignore choices for INACTIVE and INCOMPLETE.
      }
      if (!merge.isNoId())
	m_curChoice = merge;
      else if (!activate.isNoId())
	m_curChoice = activate;
      else if (!reject.isNoId()) 
	m_curChoice = reject;
      assigned = true;
    }      
    if (choices.empty())
      m_curChoice = ChoiceId::noId();
    else if (!assigned) {
      //      std::cout << " choices is not empty " << std::endl;
      m_curChoice = choices.front();
    }

    //    if(m_curChoice.isNoId())
      //      std::cout << "DEBUG: No more choices" << std::endl;
    return m_curChoice;
  }

  void HSTSOpenDecisionManager::getOpenDecisions(std::list<DecisionPointId>& decisions) {
    std::multimap<int,ObjectDecisionPointId>::iterator oit = m_objDecs.begin();
    for (; oit != m_objDecs.end(); ++oit)
      decisions.push_back(oit->second);
    std::map<int,ConstrainedVariableDecisionPointId>::iterator vit = m_unitVarDecs.begin();
    for (; vit != m_unitVarDecs.end(); ++vit)
      decisions.push_back(vit->second);
    std::map<int,TokenDecisionPointId>::iterator it = m_unitTokDecs.begin();
    for (; it != m_unitTokDecs.end(); ++it)
      decisions.push_back(it->second);
    for (it = m_nonUnitTokDecs.begin(); it != m_nonUnitTokDecs.end(); ++it)
      decisions.push_back(it->second);
    for (vit = m_nonUnitVarDecs.begin(); vit != m_nonUnitVarDecs.end(); ++vit)
      decisions.push_back(vit->second);
  }

  void HSTSOpenDecisionManager::printOpenDecisions(std::ostream& os) {
    std::multimap<int,ObjectDecisionPointId>::iterator oit = m_objDecs.begin();
    for (; oit != m_objDecs.end(); ++oit)
      os << oit->second << std::endl;
    std::map<int,ConstrainedVariableDecisionPointId>::iterator vit = m_unitVarDecs.begin();
    for (; vit != m_unitVarDecs.end(); ++vit)
      os << vit->second << std::endl;
    std::map<int,TokenDecisionPointId>::iterator it = m_unitTokDecs.begin();
    for (; it != m_unitTokDecs.end(); ++it)
      os << it->second << std::endl;
    for (it = m_nonUnitTokDecs.begin(); it != m_nonUnitTokDecs.end(); ++it)
      os << it->second << std::endl;
    for (vit = m_nonUnitVarDecs.begin(); vit != m_nonUnitVarDecs.end(); ++vit)
      os << vit->second << std::endl;
  }

}
