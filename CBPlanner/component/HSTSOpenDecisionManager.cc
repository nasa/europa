#include "DefaultOpenDecisionManager.hh"
#include "HSTSOpenDecisionManager.hh"
#include "Object.hh"
#include "Choice.hh"
#include "ValueChoice.hh"
#include "TokenChoice.hh"

namespace PLASMA {

  HSTSOpenDecisionManager::HSTSOpenDecisionManager(const DecisionManagerId& dm) : DefaultOpenDecisionManager(dm) {
  }

  HSTSOpenDecisionManager::~HSTSOpenDecisionManager() { }

  void HSTSOpenDecisionManager::addActive(const TokenId& token) {
    check_error(token.isValid());
    check_error(m_objDecs.find(token->getKey()) == m_objDecs.end());
    DecisionPointId dp = createObjectDecisionPoint(token);
    check_error(dp->getEntityKey() == token->getKey());
    m_objDecs.insert(std::pair<int,ObjectDecisionPointId>(dp->getEntityKey(),dp));
    m_sortedObjectDecs.insert(dp);
    publishNewDecision(dp);
  }

  void HSTSOpenDecisionManager::condAddActive(const TokenId& token) {
    check_error(token.isValid());
    if (m_objDecs.find(token->getKey()) == m_objDecs.end()) {
      DecisionPointId dp = createObjectDecisionPoint(token);
      check_error(dp->getEntityKey() == token->getKey());
      m_objDecs.insert(std::pair<int,ObjectDecisionPointId>(dp->getEntityKey(),dp));
      m_sortedObjectDecs.insert(dp);
      publishNewDecision(dp);
    }
  }

  const bool HSTSOpenDecisionManager::removeVarDP(const ConstrainedVariableId& var, const bool deleting, std::map<int,ConstrainedVariableDecisionPointId>& varMap, HSTSVariableDecisionSet& sortedVars) {

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

  const bool HSTSOpenDecisionManager::removeTokenDP(const TokenId& token, const bool deleting, std::map<int,TokenDecisionPointId>& tokMap, HSTSTokenDecisionSet& sortedToks) {
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

  void HSTSOpenDecisionManager::removeActive(const TokenId& token, const bool deleting) {
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
	m_sortedObjectDecs.erase(dec);
	m_dm->deleteDecision(dec);
      }
      else {
	m_objDecs.erase(it);
	m_sortedObjectDecs.erase(it->second);
      }
      publishRemovedDecision(token);
    }
  }

  void HSTSOpenDecisionManager::cleanupAllDecisionCaches() {
    DefaultOpenDecisionManager::cleanupAllDecisionCaches();
    m_sortedObjectDecs.clear();
  }

  DecisionPointId HSTSOpenDecisionManager::getNextDecision() {
    if(!m_sortedObjectDecs.empty())
      m_curDec = *m_sortedObjectDecs.begin();
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

  // if there's a merge choice, keep the first one you get and return that one.
  // otherwise prefer to activate
  // finally prefer to reject
  const ChoiceId HSTSOpenDecisionManager::getNextChoice() {
    check_error(m_curDec.isValid());
    bool assigned(false); 
    std::list<ChoiceId> choices = m_curDec->getUpdatedChoices();
    //    std::cout << "getNextChoice:: num choices " << choices.size() << std::endl;
    if (TokenDecisionPointId::convertable(m_curDec)) {
      ChoiceId merge;
      ChoiceId reject;
      ChoiceId activate;
      std::list<ChoiceId>::iterator it = choices.begin();
      for ( ; it != choices.end(); ++it) {
	ChoiceId choice = *it;
	check_error(choice.isValid());
	check_error(choice->getType() == Choice::VALUE);
	check_error(!m_curDec->hasDiscarded(choice));
	LabelStr val = Id<ValueChoice>(*it)->getValue();
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
    std::map<int,TokenDecisionPointId>::iterator it = m_tokDecs.begin();
    for (; it != m_tokDecs.end(); ++it)
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
    std::map<int,TokenDecisionPointId>::iterator it = m_tokDecs.begin();
    for (; it != m_tokDecs.end(); ++it)
      os << it->second << std::endl;
    for (vit = m_nonUnitVarDecs.begin(); vit != m_nonUnitVarDecs.end(); ++vit)
      os << vit->second << std::endl;
  }

}
