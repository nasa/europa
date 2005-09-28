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
#include "Error.hh"

namespace EUROPA {

  DefaultOpenDecisionManager::DefaultOpenDecisionManager(const DecisionManagerId& dm) : OpenDecisionManager(dm) {
  }

  DefaultOpenDecisionManager::~DefaultOpenDecisionManager() {
    std::list<DecisionPointId> openDecs;
    getOpenDecisions(openDecs);

    for (std::list<DecisionPointId>::iterator it = openDecs.begin(); it != openDecs.end(); ++it){
      DecisionPointId decision = *it;
      m_dm->deleteDecision(decision);
    }
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

  bool DefaultOpenDecisionManager::unitDecisionExistsForVariable(const ConstrainedVariableId& var) {
    return !(m_unitVarDecs.find(var->getKey()) == m_unitVarDecs.end());
  }

  bool DefaultOpenDecisionManager::nonUnitDecisionExistsForVariable(const ConstrainedVariableId& var) {
    return !(m_nonUnitVarDecs.find(var->getKey()) == m_nonUnitVarDecs.end());
  }

  void DefaultOpenDecisionManager::add(const ConstrainedVariableId& var) {
    if(m_curDec.isValid() && m_curDec->getEntityKey() == var->getKey()) {
      debugMsg("DefaultOpenDecisionManager:add", "Ignoring request - the variable " << var->toString() << " is in the current decision (" << m_curDec << ")");
      return;
    }
    ConstrainedVariableDecisionPointId dp;
    std::map<int, ConstrainedVariableDecisionPointId>::const_iterator mapIt;
    
    if(isUnitVarDec(var)) {
      if((mapIt = m_unitVarDecs.find(var->getKey())) != m_unitVarDecs.end()) {
	debugMsg("DefaultOpenDecisionManager:add", "Ignoring request - a unit decision (" <<  (*mapIt).second << ")already exists for variable " << var->toString());
	return;
      }
      mapIt = m_nonUnitVarDecs.find(var->getKey());
      if(mapIt != m_nonUnitVarDecs.end()) {
	debugMsg("DefaultOpenDecisionManager:add", "A non-unit decision point (" << (*mapIt).second << ") already exists for " << var->toString() 
		 << ".  Moving it to the unit decision list.");
	dp = (*mapIt).second;
	m_nonUnitVarDecs.erase(var->getKey());
	m_sortedNonUnitVarDecs.erase(dp);
      }
      else {
	dp = createConstrainedVariableDecisionPoint(var);
	debugMsg("DefaultOpenDecisionManager:add", "Created a unit decision point: " << dp);
      }
      m_unitVarDecs.insert(std::pair<int, ConstrainedVariableDecisionPointId>(var->getKey(), dp));
      m_sortedUnitVarDecs.insert(dp);
    }
    else {
      if((mapIt = m_nonUnitVarDecs.find(var->getKey())) != m_nonUnitVarDecs.end()) {
	debugMsg("DefaultOpenDecisionManager:add", "Ignoring request - a non-unit decision (" << (*mapIt).second << ") already exists for variable " << var->toString());
	return;
      }
      mapIt = m_unitVarDecs.find(var->getKey());
      if(mapIt != m_unitVarDecs.end()) {
	debugMsg("DefaultOpenDecisionManager:add", "A unit decision point (" << (*mapIt).second << ") already exists for " << var->toString()
		 << ".  Moving it to the non-unit decision list.");
	dp = (*mapIt).second;
	m_unitVarDecs.erase(var->getKey());
	m_sortedUnitVarDecs.erase(dp);
      }
      else {
	dp = createConstrainedVariableDecisionPoint(var);
	debugMsg("DefaultOpenDecisionManager:add", "Created a non-unit decision point: " << dp);
      }
      m_nonUnitVarDecs.insert(std::pair<int, ConstrainedVariableDecisionPointId>(var->getKey(), dp));
      m_sortedNonUnitVarDecs.insert(dp);
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
    else {
      debugMsg("DefaultOpenDecisionManager:removeVarDp", "variable not found in varMap so remove request was ignored (" << var-> getKey() << ")");
      return false;
    }

    return true;
  }

  void DefaultOpenDecisionManager::removeVar(const ConstrainedVariableId& var, const bool deleting) {
    check_error(var.isValid());
       
    m_dm->getVariableChangeBuffer().erase(var);
    m_dm->getRelaxedBuffer().erase(var);

    removeVarDP(var, deleting, m_unitVarDecs, m_sortedUnitVarDecs);
    removeVarDP(var, deleting, m_nonUnitVarDecs, m_sortedNonUnitVarDecs);
  }

  void DefaultOpenDecisionManager::condRemoveVar(const ConstrainedVariableId& var) { 
    check_error(m_curDec.isValid() || m_curDec.isNoId());   
    if (ConstrainedVariableDecisionPointId::convertable(m_curDec)) {
      ConstrainedVariableDecisionPointId cvdec = m_curDec;
      if (cvdec->getVariable()->getKey() != var->getKey()) {
        removeVar(var, true);
      } else {
        removeVar(var, false);
      }
    }
    else {
      removeVar(var, true);
    }
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
    check_error(cachesAreValid());
    return m_objDecs.size() + m_unitVarDecs.size() + m_nonUnitVarDecs.size() + m_tokDecs.size();
  }

  DecisionPointId DefaultOpenDecisionManager::getNextDecision() {
    check_error(cachesAreValid());
    if(!m_objDecs.empty())
      m_curDec = m_objDecs.begin()->second;
    else if (!m_sortedUnitVarDecs.empty())
      m_curDec = *m_sortedUnitVarDecs.begin();
    else if (!m_sortedTokDecs.empty()) 
      m_curDec = *m_sortedTokDecs.begin();
    else if (!m_sortedNonUnitVarDecs.empty()) 
      m_curDec = *m_sortedNonUnitVarDecs.begin();
    else m_curDec = DecisionPointId::noId();

    check_error(m_curDec.isNoId() || m_curDec->isOpen(), "Can only return a decision if it is open.");
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

  void DefaultOpenDecisionManager::getOpenDecisions(std::list<DecisionPointId>& decisions) {
    check_error(cachesAreValid());
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
    check_error(cachesAreValid());
    os << "Object Decs ";
    std::map<int,ObjectDecisionPointId>::iterator oit = m_objDecs.begin();
    for (; oit != m_objDecs.end(); ++oit)
      os << oit->second << std::endl;
    
    os << "Unit var Decs ";
    std::map<int,ConstrainedVariableDecisionPointId>::iterator vit = m_unitVarDecs.begin();
    for (; vit != m_unitVarDecs.end(); ++vit)
      os << vit->second << std::endl;

    os << "Token Decision Point Decs ";
    std::map<int,TokenDecisionPointId>::iterator it = m_tokDecs.begin();
    for (; it != m_tokDecs.end(); ++it)
      os << it->second << std::endl;

    os << "Non Unit Var Decs ";
    for (vit = m_nonUnitVarDecs.begin(); vit != m_nonUnitVarDecs.end(); ++vit)
      os << vit->second << std::endl;
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

  bool DefaultOpenDecisionManager::isUnit(const ConstrainedVariableDecisionPointId& dp) const {
    bool inUnits = (m_unitVarDecs.find(dp->getEntityKey()) != m_unitVarDecs.end());
    bool inSortedUnits = (m_sortedUnitVarDecs.find(dp) != m_sortedUnitVarDecs.end());
    bool inNonUnits = (m_nonUnitVarDecs.find(dp->getEntityKey()) != m_nonUnitVarDecs.end());
    bool inSortedNonUnits = (m_sortedNonUnitVarDecs.find(dp) != m_sortedNonUnitVarDecs.end());
    checkError((inUnits && inSortedUnits && !inNonUnits && !inSortedNonUnits) ||
               (!inUnits && !inSortedUnits && inNonUnits && inSortedNonUnits),
               "Synchronization error with decision " << dp << " inUnits: " << inUnits << " inSortedUnits: " << inSortedUnits << " inNonUnits: " << inNonUnits << " inSortedNonUnits: " << inSortedNonUnits);
    
    return inUnits && inSortedUnits;
  }
  bool DefaultOpenDecisionManager::isUnit(const TokenDecisionPointId& dp) const {
    return isUnitTokDec(dp);
  }
  bool DefaultOpenDecisionManager::isUnit(const ObjectDecisionPointId& dp) const {
    return false;
  }


  bool DefaultOpenDecisionManager::cachesAreValid() {
    // length of list and sorted counter parts are equal
    check_error( m_sortedUnitVarDecs.size() == m_unitVarDecs.size() );
    check_error( m_sortedNonUnitVarDecs.size() == m_nonUnitVarDecs.size() );

    for(VariableDecisionSet::const_iterator it = m_sortedUnitVarDecs.begin(); it != m_sortedUnitVarDecs.end(); ++it) {
      check_error((*it).isValid());
      checkError(isUnit(*it), "Supposed unit var dec " << (*it) << " isn't.");
    }
    for(VariableDecisionSet::const_iterator it = m_sortedNonUnitVarDecs.begin(); it != m_sortedNonUnitVarDecs.end(); ++it) {
      check_error((*it).isValid());
      checkError(!isUnit(*it), "Supposed non-unit var dec " << (*it) << " isn't.");
    }
    for(std::map<int,ConstrainedVariableDecisionPointId>::const_iterator it = m_unitVarDecs.begin(); it != m_unitVarDecs.end(); ++it) {
      check_error((*it).second.isValid());
      checkError(isUnit((*it).second), "Supposed unit var dec " << (*it).second << " isn't.");
    }
    for(std::map<int,ConstrainedVariableDecisionPointId>::const_iterator it = m_nonUnitVarDecs.begin(); it != m_nonUnitVarDecs.end(); ++it) {
      check_error((*it).second.isValid());
      checkError(!isUnit((*it).second), "Supposed non-unit var dec " << (*it).second << " isn't.");
    }
    
    for(VariableDecisionSet::const_iterator it = m_sortedUnitVarDecs.begin(); it != m_sortedUnitVarDecs.end(); ++it) {
      const int key = (*it)->getEntityKey();
      checkError(m_unitVarDecs.find(key) != m_unitVarDecs.end(), "Unit decision " << (*it) << " appears in the sorted list but not the unsorted.");
      checkError(m_nonUnitVarDecs.find(key) == m_nonUnitVarDecs.end(), "Unit decision " << (*it) << " appears in the sorted list and the unsorted non-unit list." );
      checkError(m_sortedNonUnitVarDecs.find(*it) == m_sortedNonUnitVarDecs.end(), "Unit decision " << (*it) << " appears in the sorted list and the sorted non-unit list.");
      VariableDecisionSet::const_iterator subIt = it;
      ++subIt;
      for(; subIt != m_sortedUnitVarDecs.end(); ++subIt) {
	checkError(key != (*subIt)->getEntityKey(), "There exists two unit decisions for " << key << " (" << (*it) << ", " << (*subIt) << ")");
      }
      for(subIt = m_sortedNonUnitVarDecs.begin(); subIt != m_sortedNonUnitVarDecs.end(); ++subIt) {
	checkError(key != (*subIt)->getEntityKey(), "There exists a unit and a non-unit decision for " << key << " (" << (*it) << ", " << (*subIt) << ")");
      }
    }

    for(VariableDecisionSet::const_iterator it = m_sortedNonUnitVarDecs.begin(); it != m_sortedNonUnitVarDecs.end(); ++it) {
      const int key = (*it)->getEntityKey();
      checkError(m_nonUnitVarDecs.find(key) != m_nonUnitVarDecs.end(), "Non-unit decision " << (*it) << " appears in the sorted list but n ot the unsorted.");
      checkError(m_unitVarDecs.find(key) == m_unitVarDecs.end(), "Non-unit decision " << (*it) << " appears in the sorted list and the unsorted unit list.");
      checkError(m_sortedUnitVarDecs.find(*it) == m_sortedUnitVarDecs.end(), "Non-unit decision " << (*it) << "appars in the sorted list and the sorted unit list");
      VariableDecisionSet::const_iterator subIt = it;
      ++subIt;
      for(; subIt != m_sortedNonUnitVarDecs.end(); ++subIt) {
	checkError(key != (*subIt)->getEntityKey(), "There exists two non-unit decisions for " << key << " (" << (*it) << ", " << (*subIt) << ")");
      }
      for(subIt = m_sortedUnitVarDecs.begin(); subIt != m_sortedUnitVarDecs.end(); ++subIt) {
	checkError(key != (*subIt)->getEntityKey(), "There exists a non-unit and a unit decision for " << key << " (" << (*it) << ", " << (*subIt) << ")");
      }
    }


    return true;
  }

}
 
