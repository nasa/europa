#include "DefaultOpenDecisionManager.hh"
#include "HSTSOpenDecisionManager.hh"
#include "Object.hh"
#include "Choice.hh"
#include "ValueChoice.hh"
#include "TokenChoice.hh"
#include "HSTSHeuristics.hh"

namespace PLASMA {

  HSTSOpenDecisionManager::HSTSOpenDecisionManager(const DecisionManagerId& dm, const HSTSHeuristicsId& heur) : DefaultOpenDecisionManager(dm), m_heur(heur) {
  }

  HSTSOpenDecisionManager::~HSTSOpenDecisionManager() { }

  void HSTSOpenDecisionManager::addActive(const TokenId& token) {
    DefaultOpenDecisionManager::addActive(token);
    std::map<int,ObjectDecisionPointId>::iterator pos = m_objDecs.find(token->getKey());
    check_error(pos != m_objDecs.end());
    check_error(pos->second.isValid());
    m_sortedObjectDecs.insert(pos->second);
  }

  void HSTSOpenDecisionManager::condAddActive(const TokenId& token) {
    DefaultOpenDecisionManager::condAddActive(token);
    std::map<int,ObjectDecisionPointId>::iterator pos = m_objDecs.find(token->getKey());
    check_error(pos != m_objDecs.end());
    check_error(pos->second.isValid());
    m_sortedObjectDecs.insert(pos->second);
  }

  void HSTSOpenDecisionManager::removeActive(const TokenId& token, const bool deleting) {
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

  void HSTSOpenDecisionManager::getBestObjectDecision(DecisionPointId& bestDec, HSTSHeuristics::Priority& bestp) {
    check_error(bestDec.isNoId());
    if (m_sortedObjectDecs.empty()) return;
    for (ObjectDecisionSet::iterator it = m_sortedObjectDecs.begin(); it != m_sortedObjectDecs.end(); ++it) {
      const HSTSHeuristics::Priority priority = m_heur->getPriorityForObjectDP(*it);
      if ((m_heur->getDefaultPriorityPreference() == HSTSHeuristics::HIGH && priority > bestp) ||
	  (m_heur->getDefaultPriorityPreference() == HSTSHeuristics::LOW && priority < bestp)) {
	bestDec = *it;
	bestp = priority;
      }
    }
    if (bestDec.isNoId() && !m_sortedObjectDecs.empty())
      bestDec = *m_sortedObjectDecs.begin();
  }

  void HSTSOpenDecisionManager::getBestTokenDecision(DecisionPointId& bestDec, HSTSHeuristics::Priority& bestp) {
    check_error(bestDec.isNoId());
    if (m_sortedTokDecs.empty()) return;
    for (TokenDecisionSet::iterator it = m_sortedTokDecs.begin(); it != m_sortedTokDecs.end(); ++it) {
      const HSTSHeuristics::Priority priority = m_heur->getPriorityForTokenDP(*it);
      //      std::cout << "Comparing priority = " << priority << " to bestp = " << bestp << std::endl;
      if ((m_heur->getDefaultPriorityPreference() == HSTSHeuristics::HIGH && priority > bestp) ||
	  (m_heur->getDefaultPriorityPreference() == HSTSHeuristics::LOW && priority < bestp)) {
	bestDec = *it;
	bestp = priority;
      }
    }
    if (bestDec.isNoId() && !m_sortedTokDecs.empty())
      bestDec = *m_sortedTokDecs.begin();
  }

  void HSTSOpenDecisionManager::getBestVariableDecision(DecisionPointId& bestDec, HSTSHeuristics::Priority& bestp) {
    check_error(bestDec.isNoId());
    if (m_sortedUnitVarDecs.empty() && m_sortedNonUnitVarDecs.empty()) return;
    for (VariableDecisionSet::iterator it = m_sortedUnitVarDecs.begin(); it != m_sortedUnitVarDecs.end(); ++it) {
      const HSTSHeuristics::Priority priority = m_heur->getPriorityForConstrainedVariableDP(*it);
      if ((m_heur->getDefaultPriorityPreference() == HSTSHeuristics::HIGH && priority > bestp) ||
	  (m_heur->getDefaultPriorityPreference() == HSTSHeuristics::LOW && priority < bestp)) {
	bestDec = *it;
	bestp = priority;
      }
    }
    if (bestDec.isNoId()) {
      if (!m_sortedUnitVarDecs.empty())
	bestDec = *m_sortedUnitVarDecs.begin();
      else {
	for (VariableDecisionSet::iterator it = m_sortedNonUnitVarDecs.begin(); it != m_sortedNonUnitVarDecs.end(); ++it) {
	  const HSTSHeuristics::Priority priority = m_heur->getPriorityForConstrainedVariableDP(*it);
	  if ((m_heur->getDefaultPriorityPreference() == HSTSHeuristics::HIGH && priority > bestp) ||
	      (m_heur->getDefaultPriorityPreference() == HSTSHeuristics::LOW && priority < bestp)) {
	    bestDec = *it;
	    bestp = priority;
	  }
	}
      }
    }
  }

  DecisionPointId HSTSOpenDecisionManager::getNextDecision() {
    DecisionPointId bestODec;
    DecisionPointId bestTDec;
    DecisionPointId bestVDec;
    HSTSHeuristics::Priority bestOP=MIN_PRIORITY;
    HSTSHeuristics::Priority bestTP=MIN_PRIORITY;
    HSTSHeuristics::Priority bestVP=MIN_PRIORITY;
    getBestObjectDecision(bestODec,bestOP);
    getBestTokenDecision(bestTDec,bestTP);
    getBestVariableDecision(bestVDec,bestVP);

    if (!bestODec.isNoId()) {
      ObjectDecisionPointId odec(bestODec);
      //      std::cout << " Best Object Decision = " << odec->getToken()->getName().c_str() << std::endl; 
    }
    //    else
    //      std::cout << " No Best Object Decision " << std::endl;

    if (!bestTDec.isNoId()) {
      TokenDecisionPointId tdec(bestTDec);
      //      std::cout << " Best Token Decision = " << tdec->getToken()->getName().c_str() << std::endl; 
    }
    //    else
    //      std::cout << " No Best Token Decision " << std::endl;

    if (!bestVDec.isNoId()) {
      ConstrainedVariableDecisionPointId vdec(bestVDec);
      //      std::cout << " Best Variable Decision = " << vdec->getVariable()->getName().c_str() << std::endl; 
    }
    //    else
    //      std::cout << " No Best Variable Decision " << std::endl;

    bool assignedBest(false);
    if (m_heur->getDefaultPriorityPreference() == HSTSHeuristics::HIGH) {
      if (bestOP > bestTP) {
	if (bestOP > bestVP) {
	  m_curDec = bestODec;
	  assignedBest = true;
	}
	else if (bestVP > bestOP) {
	  m_curDec = bestVDec;
	  assignedBest = true;
	}
      }
      else {
	if (bestTP > bestVP) {
	  m_curDec = bestTDec;
	  assignedBest = true;
	}
	else if (bestVP > bestTP) {
	  m_curDec = bestVDec;
	  assignedBest = true;
	}
      }
    } else {
      if (bestOP < bestTP) {
	if (bestOP < bestVP) {
	  m_curDec = bestODec;
	  assignedBest = true;
	}
	else if (bestVP < bestOP) {
	  m_curDec = bestVDec;
	  assignedBest = true;
	}
      }
      else {
	if (bestTP < bestVP) {
	  m_curDec = bestTDec;
	  assignedBest = true;
	}
	else if (bestVP < bestTP) {
	  m_curDec = bestVDec;
	  assignedBest = true;
	}
      }
    }

    if (!assignedBest) { // default to the key orderings
      if(!m_sortedObjectDecs.empty())
	m_curDec = *m_sortedObjectDecs.begin();
      else if (!m_sortedUnitVarDecs.empty())
	m_curDec = *m_sortedUnitVarDecs.begin();
      else if (!m_sortedTokDecs.empty()) 
	m_curDec = *m_sortedTokDecs.begin();
      else if (!m_sortedNonUnitVarDecs.empty()) 
	m_curDec = *m_sortedNonUnitVarDecs.begin();
      else m_curDec = DecisionPointId::noId();
    }
    /* Shold be able to require that current choices are empty */
    check_error(m_curDec.isNoId() || m_curDec->getCurrentChoices().empty());

    return m_curDec;
  }

  const ChoiceId HSTSOpenDecisionManager::getNextChoice() {
    check_error(m_curDec.isValid());
    m_curChoice = ChoiceId::noId();
    const std::list<ChoiceId>& choices = m_curDec->getUpdatedChoices();
    //    std::cout << "getNextChoice:: num choices " << choices.size() << std::endl;
    if (TokenDecisionPointId::convertable(m_curDec)) {
      TokenDecisionPointId tokDec(m_curDec);
      std::list<LabelStr> states;
      HSTSHeuristics::CandidateOrder order;
      m_heur->getOrderedStatesForTokenDP(tokDec, states, order);
      std::list<ChoiceId>::const_iterator it = choices.begin(); 
      bool found(false);
      for (std::list<LabelStr>::const_iterator sit = states.begin(); sit != states.end(); ++ sit) {
	for (; it != choices.end(); ++it) {
	  ChoiceId choice = *it;
	  check_error(choice.isValid());
	  check_error(choice->getType() == Choice::VALUE);
	  check_error(!m_curDec->hasDiscarded(choice));
	  LabelStr val = Id<ValueChoice>(choice)->getValue();
	  if (val == *sit) {
	    found = true;
	    break; // we'll do this first, no point in continuing to iterate
	  }
	}
	if (found) break;
      }      
      if (found)
	m_curChoice = *it;
    }
    else if (ConstrainedVariableDecisionPointId::convertable(m_curDec)) {
      ConstrainedVariableDecisionPointId varDec(m_curDec);
      std::list<LabelStr> domain;
      m_heur->getOrderedDomainForConstrainedVariableDP(varDec, domain);
      std::list<ChoiceId>::const_iterator it = choices.begin(); 
      bool found(false);
      for (std::list<LabelStr>::const_iterator sit = domain.begin(); sit != domain.end(); ++ sit) {
	for (; it != choices.end(); ++it) {
	  ChoiceId choice = *it;
	  check_error(choice.isValid());
	  check_error(choice->getType() == Choice::VALUE);
	  check_error(!m_curDec->hasDiscarded(choice));
	  LabelStr val = Id<ValueChoice>(choice)->getValue();
	  if (val == *sit) {
	    found = true;
	    break; // we'll do this first, no point in continuing to iterate
	  }
	}
	if (found) break;
      }      
      if (found)
	m_curChoice = *it;
    }
    else if (ObjectDecisionPointId::convertable(m_curDec)) {
      ObjectDecisionPointId objDec(m_curDec);
      HSTSHeuristics::CandidateOrder order;
      m_heur->getOrderForObjectDP(objDec, order);
    }

    // If we have no choice selected and there is a choice, pick the first
    if (!choices.empty() && m_curChoice == ChoiceId::noId())
      m_curChoice = choices.front();

    //    if(m_curChoice.isNoId())
      //      std::cout << "DEBUG: No more choices" << std::endl;
    return m_curChoice;
  }

}
