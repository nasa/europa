#include "DefaultOpenDecisionManager.hh"
#include "HSTSOpenDecisionManager.hh"
#include "Object.hh"
#include "Choice.hh"
#include "ValueChoice.hh"
#include "TokenChoice.hh"
#include "HSTSHeuristics.hh"
#include "Debug.hh"

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
    /*
    unsigned int bestNrChoices=0;
    for (ObjectDecisionSet::iterator it = m_sortedObjectDecs.begin(); it != m_sortedObjectDecs.end(); ++it) {
      TokenId tok = (*it)->getToken();
      const HSTSHeuristics::Priority priority = m_heur->getPriorityForObjectDP(*it);
      if ((m_heur->getDefaultPriorityPreference() == HSTSHeuristics::HIGH && priority > bestp) ||
	  (m_heur->getDefaultPriorityPreference() == HSTSHeuristics::LOW && priority < bestp)) {
	bestDec = *it;
	bestp = priority;
	bestNrChoices = tok->getPlanDatabase()->countOrderingChoices(tok);
      }
      else if (priority == bestp && !bestDec.isNoId()) {
	unsigned int nrChoices = tok->getPlanDatabase()->countOrderingChoices(tok,bestNrChoices+1);
	if (nrChoices < bestNrChoices) {
	bestDec = *it;
	bestNrChoices = nrChoices;
	}
      }
    }
    */
    if (bestDec.isNoId() && !m_sortedObjectDecs.empty()) {
      bestDec = *m_sortedObjectDecs.begin();
      bestp = m_heur->getPriorityForObjectDP(bestDec);
    }
  }

  void HSTSOpenDecisionManager::getBestTokenDecision(DecisionPointId& bestDec, HSTSHeuristics::Priority& bestp) {
    check_error(bestDec.isNoId());
    if (m_sortedTokDecs.empty()) return;
    unsigned int bestNrChoices = 999999;
    for (TokenDecisionSet::iterator it = m_sortedTokDecs.begin(); it != m_sortedTokDecs.end(); ++it) {
      const HSTSHeuristics::Priority priority = m_heur->getPriorityForTokenDP(*it);
      TokenId tok = (*it)->getToken();
      debugMsg("HSTS:OpenDecisionManager:getBestTokenDecision", "Comparing priority = " << priority << " to bestp = " << bestp);
      if ((m_heur->getDefaultPriorityPreference() == HSTSHeuristics::HIGH && priority > bestp) ||
	  (m_heur->getDefaultPriorityPreference() == HSTSHeuristics::LOW && priority < bestp)) {
	bestDec = *it;
	bestp = priority;
	TokenDecisionPointId tokDec = bestDec;
	bestNrChoices = tok->getPlanDatabase()->countCompatibleTokens(tok);
	if (tok->getState()->lastDomain().isMember(Token::ACTIVE) && tok->getPlanDatabase()->hasOrderingChoice(tok)) bestNrChoices++;
	debugMsg("HSTS:OpenDecisionManager:getBestTokenDecision", "Selecting new bestDec = " << bestDec << " with bestNrChoices = " << bestNrChoices);
      }
      else if (priority == bestp && !bestDec.isNoId() && bestNrChoices > 0) {
	unsigned int nrChoices = tok->getPlanDatabase()->countCompatibleTokens(tok,bestNrChoices);
	if (tok->getState()->lastDomain().isMember(Token::ACTIVE) && tok->getPlanDatabase()->hasOrderingChoice(tok)) nrChoices++;
	debugMsg("HSTS:OpenDecisionManager:getBestTokenDecision", "Evaluated nrChoices for decision " << *it << " = " << nrChoices << " vs. bestNrChoices = " << bestNrChoices);
	if (nrChoices < bestNrChoices) {
	bestDec = *it;
	bestNrChoices = nrChoices;
	debugMsg("HSTS:OpenDecisionManager:getBestTokenDecision", "Selecting new bestDec = " << bestDec << " with bestNrChoices = " << bestNrChoices);
	if (bestNrChoices == 0) break;
	}
      }
    }
    if (bestDec.isNoId() && !m_sortedTokDecs.empty()) {
      bestDec = *m_sortedTokDecs.begin();
      bestp = m_heur->getPriorityForTokenDP(bestDec);
      debugMsg("HSTS:OpenDecisionManager:getBestTokenDecision", "Selecting first decision as bestDec = " << bestDec);
    }
  }

  void HSTSOpenDecisionManager::getBestUnitVariableDecision(DecisionPointId& bestDec, HSTSHeuristics::Priority& bestp) {
    check_error(bestDec.isNoId());
    if (m_sortedUnitVarDecs.empty()) return;
    // these must be compat guards
    VariableDecisionSet::iterator it = m_sortedUnitVarDecs.begin();
    bestDec = *it;
    check_error(bestDec.isValid());
    ++it;
    bestp = m_heur->getPriorityForConstrainedVariableDP(bestDec);
    for (; it != m_sortedUnitVarDecs.end(); ++it) {
      // ignore variables of uninserted tokens
      ConstrainedVariableDecisionPointId vdec(*it);
      if (TokenId::convertable(vdec->getVariable()->getParent())) {
	TokenId parent = vdec->getVariable()->getParent();
	if (!parent->isActive()) continue;
      }
      const HSTSHeuristics::Priority priority = m_heur->getPriorityForConstrainedVariableDP(vdec);
      if ((m_heur->getDefaultPriorityPreference() == HSTSHeuristics::HIGH && priority > bestp) ||
	  (m_heur->getDefaultPriorityPreference() == HSTSHeuristics::LOW && priority < bestp)) {
	bestDec = vdec;
	bestp = priority;
      }
    }
  }

  void HSTSOpenDecisionManager::getBestNonUnitVariableDecision(DecisionPointId& bestDec, HSTSHeuristics::Priority& bestp) {
    check_error(bestDec.isNoId());
    if (m_sortedNonUnitVarDecs.empty()) return;
    ConstrainedVariableDecisionPointId bestFloatDec;
    HSTSHeuristics::Priority bestFloatp = bestp;
    VariableDecisionSet::iterator it = m_sortedNonUnitVarDecs.begin();
    for ( ; it != m_sortedNonUnitVarDecs.end(); ++it) {
      // Ignore variables of inactive tokens.
      ConstrainedVariableDecisionPointId vdec(*it);
      check_error(vdec.isValid());
      if (TokenId::convertable(vdec->getVariable()->getParent())) {
        TokenId parent = vdec->getVariable()->getParent();
        if (!parent->isActive())
          continue;
      }
      const HSTSHeuristics::Priority& priority = m_heur->getPriorityForConstrainedVariableDP(vdec);
      if (vdec->getVariable()->lastDomain().isFinite()) {
        if (bestDec.isNoId() ||
            m_heur->getDefaultPriorityPreference() == HSTSHeuristics::HIGH && priority > bestp ||
            m_heur->getDefaultPriorityPreference() == HSTSHeuristics::LOW && priority < bestp) {
          bestDec = vdec;
          bestp = priority;
        } else
          if (priority == bestp) { // secondary heuristic - domain size
	  ConstrainedVariableDecisionPointId bdec(bestDec); // casting necessary
	  ConstrainedVariableId  bdecVar(bdec->getVariable());
	  ConstrainedVariableId vdecVar(vdec->getVariable());
	  if (!m_dm->isCompatGuard(bdecVar->getKey()) && m_dm->isCompatGuard(vdecVar->getKey()))
	    bestDec = vdec;
	  else // terciary heuristic - domain size
	    if (bdec->getVariable()->lastDomain().getSize() > vdec->getVariable()->lastDomain().getSize())
	      bestDec = vdec;
          }
      } else {
        if (bestFloatDec.isNoId() ||
            m_heur->getDefaultPriorityPreference() == HSTSHeuristics::HIGH && priority > bestFloatp ||
            m_heur->getDefaultPriorityPreference() == HSTSHeuristics::LOW && priority < bestFloatp) {
          bestFloatDec = vdec;
          bestFloatp = priority;
        } // else ... messy to compare sizes of infinite domains, but it could be done if needed
      }
    }
    if (bestDec.isNoId() && !bestFloatDec.isNoId()) {
      bestDec = bestFloatDec;
      bestp = bestFloatp;
    }


    /*
    if (!m_sortedNonUnitVarDecs.empty()) { // there are no unit vars and some non-unit vars
      ConstrainedVariableDecisionPointId bestFloatDec;
      HSTSHeuristics::Priority bestFloatp;
      VariableDecisionSet::iterator it = m_sortedNonUnitVarDecs.begin();
      if ((*it)->getVariable()->lastDomain().isFinite())
      
      bestDec = *it;
      check_error(bestDec.isValid());
      ++it;
      bestp = m_heur->getPriorityForConstrainedVariableDP(bestDec);
      for (; it != m_sortedNonUnitVarDecs.end(); ++it) {
	// ignore variables of uninserted tokens
	ConstrainedVariableDecisionPointId vdec(*it);
	check_error(vdec.isValid());
	if (TokenId::convertable(vdec->getVariable()->getParent())) {
	  TokenId parent = vdec->getVariable()->getParent();
	  if (!parent->isActive()) continue;
	}
	const HSTSHeuristics::Priority priority = m_heur->getPriorityForConstrainedVariableDP(vdec);
	if ((m_heur->getDefaultPriorityPreference() == HSTSHeuristics::HIGH && priority > bestp) ||
	    (m_heur->getDefaultPriorityPreference() == HSTSHeuristics::LOW && priority < bestp)) {
	  if (vdec->getVariable()->lastDomain().isFinite()) {
	    bestDec = vdec;
	    bestp = priority;
	  }
	  else {
	    bestFloatDec = vdec;
	    bestFloatp = priority;
	  }
	}
	else if (priority == bestp) { // secondary heuristic - compat guards first
	  ConstrainedVariableDecisionPointId bdec(bestDec); // casting necessary
	  ConstrainedVariableId  bdecVar(bdec->getVariable());
	  ConstrainedVariableId vdecVar(vdec->getVariable());
	  if (!m_dm->isCompatGuard(bdecVar->getKey()) && m_dm->isCompatGuard(vdecVar->getKey()))
	    bestDec = vdec;
	  else // terciary heuristic - domain size
	    if (vdec->getVariable()->lastDomain().isFinite() && bdec->getVariable()->lastDomain().getSize() > vdec->getVariable()->lastDomain().getSize())
	      bestDec = vdec;
	}
      }
      if (bestDec.isNoId()) { // must return the best float dec.
	bestDec = bestFloatDec;
	bestp = bestFloatp;
      }
    }
    */
  }

  DecisionPointId HSTSOpenDecisionManager::getNextDecision() {
    DecisionPointId bestODec;
    DecisionPointId bestTDec;
    DecisionPointId bestVDec;
    HSTSHeuristics::Priority bestOP = MIN_PRIORITY - 1;
    HSTSHeuristics::Priority bestTP = MIN_PRIORITY - 1;
    HSTSHeuristics::Priority bestVP = MIN_PRIORITY - 1;
    if (m_heur->getDefaultPriorityPreference() == HSTSHeuristics::LOW) {
      bestOP = MAX_PRIORITY + 1;
      bestTP = MAX_PRIORITY + 1;
      bestVP = MAX_PRIORITY + 1;
    }
    bool assignedBest(false);

    if(!m_sortedObjectDecs.empty())  getBestObjectDecision(bestODec, bestOP);

    if (!bestODec.isNoId()) { /* Europa doesn't distinguish between object and token decisions so we make any object decisions */
      ObjectDecisionPointId odec(bestODec);
      debugMsg("HSTS:OpenDecisionManager:getNextDecision", "Best Obj Dec = [" << bestOP << "] (" << odec->getKey() << ") with Token " << odec->getToken()->getName().c_str());
      m_curDec = bestODec;
      assignedBest = true;

      return m_curDec;
    }

    if (!m_sortedUnitVarDecs.empty()) getBestUnitVariableDecision(bestVDec, bestVP);

    if (!bestVDec.isNoId()) {
      m_curDec = bestVDec;
      assignedBest = true;

      ConstrainedVariableDecisionPointId vdec(bestVDec);
      debugMsg("HSTS:OpenDecisionManager:getNextDecision", "Best Var Dec = [" << bestVP << "] (" << vdec->getKey() << ") with Variable " << vdec->getVariable()->getName().c_str() << " with domain " << vdec->getVariable()->lastDomain()) ;

      return m_curDec;
    }

    if (!m_sortedTokDecs.empty()) getBestTokenDecision(bestTDec, bestTP);
    if (!m_sortedNonUnitVarDecs.empty()) getBestNonUnitVariableDecision(bestVDec, bestVP);

    if (bestTDec.isNoId() && !bestVDec.isNoId()) {
      m_curDec = bestVDec;
      assignedBest = true;

      ConstrainedVariableDecisionPointId vdec(bestVDec);
      debugMsg("HSTS:OpenDecisionManager:getNextDecision", "Best Var Dec = [" << bestVP << "] (" << vdec->getKey() << ") with Variable " << vdec->getVariable()->getName().c_str() << " with domain " << vdec->getVariable()->lastDomain()) ;

      return m_curDec;
    }

    if (bestVDec.isNoId() && !bestTDec.isNoId()) {
      m_curDec = bestTDec;
      assignedBest = true;
	
      TokenDecisionPointId tdec(bestTDec);
      debugMsg("HSTS:OpenDecisionManager:getNextDecision", "Best Tok Dec = [" << bestTP << "] (" << tdec->getKey() << ") with Token " << tdec->getToken()->getName().c_str() << " with domain " << tdec->getToken()->getState()->lastDomain()) ;
	
      return m_curDec;
    }

    /* If I'm here, I need to compare bestTDec to bestVDec */
    if (m_heur->getDefaultPriorityPreference() == HSTSHeuristics::HIGH) {
      /* pick max */
      if (bestTP >= bestVP) {
	/* prefer units though */
	ConstrainedVariableDecisionPointId vdec(bestVDec);
	debugMsg("HSTS:OpenDecisionManager:getNextDecision", "Best Var Dec = [" << bestVP << "] (" << vdec->getKey() << ") with Variable " << vdec->getVariable()->getName().c_str() << " with domain " << vdec->getVariable()->lastDomain()) ;
	TokenDecisionPointId tdec(bestTDec);
	debugMsg("HSTS:OpenDecisionManager:getNextDecision", "Best Tok Dec = [" << bestTP << "] (" << tdec->getKey() << ") with Token " << tdec->getToken()->getName().c_str() << " with domain " << tdec->getToken()->getState()->lastDomain()) ;
	if (bestTP == bestVP && vdec->getVariable()->lastDomain().isSingleton())  m_curDec = bestVDec;
	else m_curDec = bestTDec;
	assignedBest = true;
      }
      else { /* bestVP > bestTP */
	ConstrainedVariableDecisionPointId vdec(bestVDec);
	debugMsg("HSTS:OpenDecisionManager:getNextDecision", "Best Var Dec = [" << bestVP << "] (" << vdec->getKey() << ") with Variable " << vdec->getVariable()->getName().c_str() << " with domain " << vdec->getVariable()->lastDomain()) ;
	m_curDec = bestVDec;
	assignedBest = true;
      }
    }
    else { /* default priority preference is low */
      if (bestTP <= bestVP) {
	/* prefer units though */
	ConstrainedVariableDecisionPointId vdec(bestVDec);
	debugMsg("HSTS:OpenDecisionManager:getNextDecision", "Best Var Dec = [" << bestVP << "] (" << vdec->getKey() << ") with Variable " << vdec->getVariable()->getName().c_str() << " with domain " << vdec->getVariable()->lastDomain()) ;
	TokenDecisionPointId tdec(bestTDec);
	debugMsg("HSTS:OpenDecisionManager:getNextDecision", "Best Tok Dec = [" << bestTP << "] (" << tdec->getKey() << ") with Token " << tdec->getToken()->getName().c_str() << " with domain " << tdec->getToken()->getState()->lastDomain()) ;
	if (bestTP == bestVP && vdec->getVariable()->lastDomain().isSingleton())  m_curDec = bestVDec;
	else m_curDec = bestTDec;
	assignedBest = true;
      }
      else { /* bestVP < bestTP */
	ConstrainedVariableDecisionPointId vdec(bestVDec);
	debugMsg("HSTS:OpenDecisionManager:getNextDecision", "Best Var Dec = [" << bestVP << "] (" << vdec->getKey() << ") with Variable " << vdec->getVariable()->getName().c_str() << " with domain " << vdec->getVariable()->lastDomain()) ;
	m_curDec = bestVDec;
	assignedBest = true;
      }
    }

    //    check_error(assignedBest || m_curDec.isNoId() || m_curDec->getCurrentChoices.empty());

    return m_curDec;
  } /* we have found a best */


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

    debugMsg("HSTS:OpenDecisionManager:getNextChoice","UpdatedChoices for decision point (" << m_curDec->getKey() << ") = " << choices.size());

    if (choices.empty()) return m_curChoice;

    if (TokenDecisionPointId::convertable(m_curDec)) {
      TokenDecisionPointId tokDec(m_curDec);
      std::list<LabelStr> states;
      HSTSHeuristics::CandidateOrder order = HSTSHeuristics::UNKNOWN;
      m_heur->getOrderedStatesForTokenDP(tokDec, states, order);
      int bestChoiceMeasure;
      initializeNumberToBeat(order, bestChoiceMeasure);
      std::list<ChoiceId>::const_iterator it;
      bool found(false);
      ChoiceId bestChoice;

      debugMsg("HSTS:OpenDecisionManager:getNextChoice", "Token Decision Point (" << tokDec->getKey() << ") for Token (" << tokDec->getToken()->getKey() << ") has " << choices.size() << " choices.");

      for (std::list<LabelStr>::const_iterator sit = states.begin(); sit != states.end(); ++ sit) {
	bool evalMerged(false);
	for (it = choices.begin(); it != choices.end(); ++it) {
	  ChoiceId choice = *it;
	  check_error(choice.isValid());
	  check_error(choice->getType() == Choice::VALUE);
	  check_error(!m_curDec->hasDiscarded(choice));
	  check_error(LabelStr::isString(Id<ValueChoice>(choice)->getValue()));
	  LabelStr val(Id<ValueChoice>(choice)->getValue());

	  debugMsg("HSTS:OpenDecisionManager:getNextChoice","comparing " << val.c_str() << " with " << (*sit).c_str());

	  if (strcmp(val.c_str(),(*sit).c_str()) == 0) {
	    if (strcmp(val.c_str(), "REJECTED") == 0) {
	      found = true;
	      bestChoice = *it;
	      break; // we'll do this first, no point in continuing to iterate
	    }
	    if (strcmp(val.c_str(), "MERGED") == 0) {
	      LabelStr val = Id<ValueChoice>(choice)->getValue();
	      const AbstractDomain& startDom = tokDec->getToken()->getStart()->lastDomain();
	      compareTokensAccordingToOrder(order,choice,bestChoice,(int)startDom.getLowerBound(),(int)startDom.getUpperBound(), bestChoiceMeasure);
	      evalMerged = true;
	    }
	    else {
	      check_error(strcmp(val.c_str(), "ACTIVE") == 0, "Unexpected token state, expecting ACTIVE, MERGED, REJECTED");
	      found = true;
	      if (!evalMerged)
		bestChoice = *it;
	      break; // we'll do this first, no point in continuing to iterate
	    }
	  }
	}
	if (found || evalMerged) break;
      }      
      m_curChoice = bestChoice;
      debugMsg("HSTS:OpenDecisionManager:getNextChoice", "Token Decision Point (" << tokDec->getKey() << ") for Token (" << tokDec->getToken()->getKey() << ") best choice =  " << bestChoice);
    }
    else if (ConstrainedVariableDecisionPointId::convertable(m_curDec)) {
      ConstrainedVariableDecisionPointId varDec(m_curDec);
      bool found(false);
      check_error(varDec->getVariable()->lastDomain().isFinite());
      std::list<double> domain;
      m_heur->getOrderedDomainForConstrainedVariableDP(varDec, domain);
      debugMsg("HSTS:OpenDecisionManager:getNextChoice", "Variable Decision Point (" << varDec->getKey() << ") for Variable (" << varDec->getVariable()->getKey() << ") has " << choices.size() << " choices.");
      std::list<ChoiceId>::const_iterator it = choices.begin(); 
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
      debugMsg("HSTS:OpenDecisionManager:getNextChoice", "Variable Decision Point (" << varDec->getKey() << ") for Variable (" << varDec->getVariable()->getKey() << ") bestChoice = " << m_curChoice);
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
      ChoiceId bestChoice;
      
      debugMsg("HSTS:OpenDecisionManager:getNextChoice", "Object Decision Point (" << objDec->getKey() << ") for Token (" << thisToken->getKey() << ") has " << choices.size() << " choices.");

      for (std::list<ChoiceId>::const_iterator it = choices.begin(); it != choices.end(); ++it) {
	ChoiceId choice(*it);
	check_error(choice.isValid());
	check_error(choice->getType() == Choice::TOKEN);
	check_error(!m_curDec->hasDiscarded(choice));
	compareTokensAccordingToOrder(order,choice,bestChoice,(int)est, (int)lst, bestChoiceMeasure);
      }
      m_curChoice = bestChoice;
      debugMsg("HSTS:OpenDecisionManager:getNextChoice", "bestChoice = " << bestChoice);
    }

    // If we have no choice selected and there is a choice, pick the first
    if (!choices.empty() && m_curChoice == ChoiceId::noId())
      m_curChoice = choices.front();

    //    if(m_curChoice.isNoId())
      //      std::cout << "DEBUG: No more choices" << std::endl;
    return m_curChoice;
  }

}
