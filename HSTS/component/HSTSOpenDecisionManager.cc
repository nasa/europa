#include "DefaultOpenDecisionManager.hh"
#include "HSTSOpenDecisionManager.hh"
#include "Object.hh"
#include "Choice.hh"
#include "ValueChoice.hh"
#include "TokenChoice.hh"
#include "HSTSHeuristics.hh"

namespace EUROPA {

  HSTSOpenDecisionManager::HSTSOpenDecisionManager(const DecisionManagerId& dm, const HSTSHeuristicsId& heur)
    : DefaultOpenDecisionManager(dm), m_heur(heur) {
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

  void HSTSOpenDecisionManager::initializeNumberToBeat(const HSTSHeuristics::CandidateOrder& order, int& numberToBeat) {
    switch (order) {
    case HSTSHeuristics::TGENERATOR:
      check_error(ALWAYS_FAIL, "Successor Token Generators not yet supported");
      break;
    case HSTSHeuristics::NEAR: 
      numberToBeat=MAX_FINITE_TIME;
      break;
    case HSTSHeuristics::FAR:
      numberToBeat=MIN_FINITE_TIME;
      break;
    case HSTSHeuristics::EARLY:
      numberToBeat = MAX_FINITE_TIME;
      break;
    case HSTSHeuristics::LATE:
      numberToBeat = MIN_FINITE_TIME;
      break;
    case HSTSHeuristics::MAX_FLEXIBLE:
      check_error(ALWAYS_FAIL, "MAX_FLEXIBLE is not yet implemented.");
      break;
    case HSTSHeuristics::MIN_FLEXIBLE:
      check_error(ALWAYS_FAIL, "MIN_FLEXIBLE is not yet implemented.");
      break;
    case HSTSHeuristics::LEAST_SPECIFIED:
      check_error(ALWAYS_FAIL, "LEAST_SPECIFIED is not yet implemented.");
      break;
    case HSTSHeuristics::MOST_SPECIFIED:
      check_error(ALWAYS_FAIL, "MOST_SPECIFIED is not yet implemented.");
      break;
    case HSTSHeuristics::NONE:
      break;
    case HSTSHeuristics::UNKNOWN:
      check_error(ALWAYS_FAIL, "Unknown/uninitialized heuristics order given.");
      break;
    }
  }

  void HSTSOpenDecisionManager::compareTokensAccordingToOrder(const HSTSHeuristics::CandidateOrder& order, const ChoiceId& choice,
                                                              ChoiceId& bestChoice, const int est, const int lst, int& numberToBeat) {
    TokenId succ;
    if (Id<TokenChoice>::convertable(choice))
      succ = Id<TokenChoice>(choice)->getSuccessor();
    else
      succ = Id<ValueChoice>(choice)->getToken();
    switch (order) {
     case HSTSHeuristics::TGENERATOR:
       check_error(ALWAYS_FAIL, "Successor Token Generators not yet supported");
       break;
     case HSTSHeuristics::NEAR: 
       {
         //	std::cout << "NEAR" << std::endl;
         //	int diff=999999999;
         if (!succ.isNoId()) { // for successors other than the end 
           int estdiff = abs((int)succ->getStart()->lastDomain().getLowerBound() - est);
           int lstdiff = abs((int)succ->getStart()->lastDomain().getUpperBound() - lst);
           if (estdiff+lstdiff < numberToBeat) {
             bestChoice = choice;
             numberToBeat = estdiff+lstdiff;
             //	    std::cout << " best succ so far (" << succ->getKey() << ") [" << succ->getStart()->lastDomain().getLowerBound() << "," <<  succ->getStart()->lastDomain().getUpperBound() << "]" << std::endl;
           }
         }
       }
       break;
     case HSTSHeuristics::FAR:
       {
         //	  std::cout << "FAR" << std::endl;
         //	  int diff=0;
         if (succ.isNoId()) { 
           bestChoice = choice;
           return;
         }
         else {
           int estdiff = abs((int)succ->getStart()->lastDomain().getLowerBound() - (int)est);
           int lstdiff = abs((int)succ->getStart()->lastDomain().getUpperBound() - (int)lst);
           if (estdiff+lstdiff > numberToBeat) {
             bestChoice = choice;
             numberToBeat = estdiff+lstdiff;
             //	      std::cout << " best succ so far (" << succ->getKey() << ") [" << succ->getStart()->lastDomain().getLowerBound() << "," <<  succ->getStart()->lastDomain().getUpperBound() << "]" << std::endl;
           }
         }
       }
       break;
     case HSTSHeuristics::EARLY:
       {
         //	  std::cout << "EARLY" << std::endl;
         //	  double start = 99999999999.9;
         if (!succ.isNoId()) { // for successors other than the end 
           int sest = (int)succ->getStart()->lastDomain().getLowerBound();
           if (sest < numberToBeat) {
             bestChoice = choice;
             numberToBeat = sest;
             //	      std::cout << " best succ so far (" << succ->getKey() << ") [" << succ->getStart()->lastDomain().getLowerBound() << "," <<  succ->getStart()->lastDomain().getUpperBound() << "]" << std::endl;
           }
         }
       }
       break;
     case HSTSHeuristics::LATE:
       {
         //	  std::cout << "LATE" << std::endl;
         //	  double start = 0.0;
         if (succ.isNoId()) { 
           bestChoice = choice;
           return;
         }
         else {
           int sest = (int)succ->getStart()->lastDomain().getLowerBound();
           if (sest > numberToBeat) {
             bestChoice = choice;
             numberToBeat = sest;
             //	      std::cout << " best succ so far (" << succ->getKey() << ") [" << succ->getStart()->lastDomain().getLowerBound() << "," <<  succ->getStart()->lastDomain().getUpperBound() << "]" << std::endl;
           }
         }
       }
       break;
     case HSTSHeuristics::MAX_FLEXIBLE:
       check_error(ALWAYS_FAIL, "MAX_FLEXIBLE is not yet implemented.");
       break;
     case HSTSHeuristics::MIN_FLEXIBLE:
       check_error(ALWAYS_FAIL, "MIN_FLEXIBLE is not yet implemented.");
       break;
     case HSTSHeuristics::LEAST_SPECIFIED:
       check_error(ALWAYS_FAIL, "LEAST_SPECIFIED is not yet implemented.");
       break;
     case HSTSHeuristics::MOST_SPECIFIED:
       check_error(ALWAYS_FAIL, "MOST_SPECIFIED is not yet implemented.");
       break;
     case HSTSHeuristics::NONE:
       break;
     case HSTSHeuristics::UNKNOWN:
       check_error(ALWAYS_FAIL, "Unknown/uninitialized heuristics order given.");
       break;
    }
  }

  const ChoiceId HSTSOpenDecisionManager::getNextChoice() {
    check_error(m_curDec.isValid());
    m_curChoice = ChoiceId::noId();
    const std::list<ChoiceId>& choices = m_curDec->getUpdatedChoices();
    //    std::cout << "getNextChoice:: num choices " << choices.size() << std::endl;
    if (TokenDecisionPointId::convertable(m_curDec)) {
      TokenDecisionPointId tokDec(m_curDec);
      std::list<LabelStr> states;
      HSTSHeuristics::CandidateOrder order = HSTSHeuristics::UNKNOWN;
      m_heur->getOrderedStatesForTokenDP(tokDec, states, order);
      int bestChoiceMeasure;
      initializeNumberToBeat(order, bestChoiceMeasure);
      std::list<ChoiceId>::const_iterator it = choices.begin(); 
      bool found(false);
      ChoiceId bestChoice;
      for (std::list<LabelStr>::const_iterator sit = states.begin(); sit != states.end(); ++ sit) {
	bool evalMerged(false);
	for (; it != choices.end(); ++it) {
	  ChoiceId choice = *it;
	  check_error(choice.isValid());
	  check_error(choice->getType() == Choice::VALUE);
	  check_error(!m_curDec->hasDiscarded(choice));
	  check_error(LabelStr::isString(Id<ValueChoice>(choice)->getValue()));
	  LabelStr val(Id<ValueChoice>(choice)->getValue());
	  if (val == *sit) {
	    if (val == Token::MERGED) {
	      LabelStr val = Id<ValueChoice>(choice)->getValue();
	      const AbstractDomain& startDom = tokDec->getToken()->getStart()->lastDomain();
	      compareTokensAccordingToOrder(order,choice,bestChoice,(int)startDom.getLowerBound(),(int)startDom.getUpperBound(), bestChoiceMeasure);
	      evalMerged = true;
	    }
	    else {
	      found = true;
	      bestChoice = *it;
	      break; // we'll do this first, no point in continuing to iterate
	    }
	  }
	}
	if (found || evalMerged) break;
      }      
      m_curChoice = bestChoice;
    }
    else if (ConstrainedVariableDecisionPointId::convertable(m_curDec)) {
      ConstrainedVariableDecisionPointId varDec(m_curDec);
      std::list<double> domain;
      m_heur->getOrderedDomainForConstrainedVariableDP(varDec, domain);
      std::list<ChoiceId>::const_iterator it = choices.begin(); 
      bool found(false);
      for (std::list<double>::const_iterator sit = domain.begin(); sit != domain.end(); ++ sit) {
	for (; it != choices.end(); ++it) {
	  ChoiceId choice = *it;
	  check_error(choice.isValid());
	  check_error(choice->getType() == Choice::VALUE);
	  check_error(!m_curDec->hasDiscarded(choice));
	  double val = Id<ValueChoice>(choice)->getValue();
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
      HSTSHeuristics::CandidateOrder order = HSTSHeuristics::UNKNOWN;
      m_heur->getOrderForObjectDP(objDec, order);
      int bestChoiceMeasure;
      initializeNumberToBeat(order, bestChoiceMeasure);
      TokenId thisToken(objDec->getToken());
      double est(thisToken->getStart()->lastDomain().getLowerBound());
      double lst(thisToken->getStart()->lastDomain().getUpperBound());
      //      std::cout << " Token (" << thisToken->getKey() << ") [" << est << "," << lst << "]" << std::endl;
      ChoiceId bestChoice;
      for (std::list<ChoiceId>::const_iterator it = choices.begin(); it != choices.end(); ++it) {
	ChoiceId choice(*it);
	check_error(choice.isValid());
	check_error(choice->getType() == Choice::TOKEN);
	check_error(!m_curDec->hasDiscarded(choice));
	compareTokensAccordingToOrder(order,choice,bestChoice,(int)est, (int)lst, bestChoiceMeasure);
      }
      m_curChoice = bestChoice;
    }

    // If we have no choice selected and there is a choice, pick the first
    if (!choices.empty() && m_curChoice == ChoiceId::noId())
      m_curChoice = choices.front();

    //    if(m_curChoice.isNoId())
      //      std::cout << "DEBUG: No more choices" << std::endl;
    return m_curChoice;
  }

}
