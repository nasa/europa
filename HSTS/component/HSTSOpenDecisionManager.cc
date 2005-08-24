#include "TokenVariable.hh"
#include "DefaultOpenDecisionManager.hh"
#include "HSTSOpenDecisionManager.hh"
#include "Object.hh"
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
    std::map<int, ObjectDecisionPointId>::iterator it = m_objDecs.find(token->getKey());
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
        TokenDecisionPointId tokDec = bestDec; // CMG: THIS IS NOT USED. WHY NOT?
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
	checkError(bestVDec.isValid(), "We assume we have a valid unit var decision, but we don't");
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

    //check_error(assignedBest || m_curDec.isNoId() || m_curDec->hasRemainingChoices());

    return m_curDec;
  } /* we have found a best */

  void HSTSOpenDecisionManager::initializeTokenChoices(TokenDecisionPointId& tdp) {
    check_error(tdp.isValid());
    check_error(tdp->m_choices.empty());
    DefaultOpenDecisionManager::initializeTokenChoices(tdp);

    if (tdp->m_choices.empty()) return;

    std::list<LabelStr> states;
    HSTSHeuristics::CandidateOrder order = HSTSHeuristics::UNKNOWN;
    m_heur->getOrderedStatesForTokenDP(tdp, states, order);

    check_error (order == HSTSHeuristics::NEAR || order == HSTSHeuristics::FAR || order == HSTSHeuristics::EARLY || order == HSTSHeuristics::LATE || order == HSTSHeuristics::NONE, "Unable to handle cases other than late, early, near, far, none.");
    
    debugMsg("HSTS:OpenDecisionManager:getNextChoice", "Token Decision Point (" << tdp->getKey() << ") for Token (" << tdp->getToken()->getKey() << ") has " << tdp->m_choices.size() + tdp->m_compatibleTokens.size() << " choices.");
    
    if (order == HSTSHeuristics::NONE) {
      debugMsg("HSTS:OpenDecisionManager:getNextChoice", "Token Decision Point (" << tdp->getKey() << ") for Token (" << tdp->getToken()->getKey() << ") has best choice =  " << LabelStr(tdp->m_choices[0]).toString());
      return;
    }

    debugMsg("HSTS:OpenDecisionManager:getNextChoice", "Clearing unavailable states from the potential state list.");

    std::stringstream data1;
    for(std::list<LabelStr>::const_iterator it = states.begin(); it != states.end(); ++it) {
      data1 << " " << (*it).toString();
    }
    std::stringstream data2;
    for(std::vector<LabelStr>::const_iterator it = tdp->m_choices.begin(); it != tdp->m_choices.end(); ++it) {
      data2 << " " << (*it).toString();
    }
    debugMsg("HSTS:OpenDecisionManager:getNextChoice", "Allowable states:" << data1.str());
    debugMsg("HSTS:OpenDecisionManager:getNextChoice", "Available states: " << data2.str());
    


    for(std::list<LabelStr>::iterator it = states.begin(); it != states.end(); ++it) {
      bool found = false;
      for(std::vector<LabelStr>::iterator cit = tdp->m_choices.begin(); cit != tdp->m_choices.end(); ++cit) {
        if(fabs((double)(*cit) - (double)(*it)) < EPSILON)
          found = true;
      }
      if(!found) {
        debugMsg("HSTS:OpenDecisionManager:getNextChoice", "Removing " << (*it).toString());
        std::list<LabelStr>::iterator tmp = it;
        --it;
        states.erase(tmp);
      }
        // if(!std::binary_search(tdp->m_choices.begin(), tdp->m_choices.end(), *it)) {
        //  debugMsg("HSTS:OpenDecisionManager:getNextChoice", "Removing " << (*it).toString());
        //  std::list<LabelStr>::iterator tmp = it;
        //  --it;
        //  states.erase(tmp);
        //}
    }
 
    debugMsg("HSTS:OpenDecisionManager:getNextChoice", "Assigning ordered states to the token.");
    tdp->m_choices.clear();
    tdp->m_choices.insert(tdp->m_choices.begin(), states.begin(), states.end());

    debugMsg("HSTS:OpenDecisionManager:getNextChoice", "Done.  TDP now has " << tdp->m_choices.size() << " choices.");

     unsigned int current = 0;
//     LabelStr tmpChoice;
//     for (std::list<LabelStr>::const_iterator sit = states.begin(); sit != states.end(); ++ sit) {
//       for (unsigned int i = current; i != tdp->m_choices.size(); ++i) {
//         LabelStr val(tdp->m_choices[i]);
	
//         debugMsg("HSTS:OpenDecisionManager:getNextChoice","comparing " << val.c_str() << " with " << (*sit).c_str());

//         if (strcmp(val.c_str(),(*sit).c_str()) == 0) {
//           if (i != current) {
//             tmpChoice = tdp->m_choices[current];
//             tdp->m_choices[current] = tdp->m_choices[i];
//             tdp->m_choices[i] = tmpChoice;
//           }
//           current++;
//         }
//       }
//     }

    // Next, order the compatibleTokens

    TokenId tmpCompatibleToken;
    int numberToBeat;
    int number;
    current = 0;

    if (order == HSTSHeuristics::NEAR || order == HSTSHeuristics::EARLY) {
      debugMsg("HSTS:OpenDecisionManager:getNextChoice", "NEAR || EARLY");
      numberToBeat = MAX_FINITE_TIME;
      for (unsigned int i = current; i != tdp->m_compatibleTokens.size(); ++i) {
        TokenId token = tdp->m_compatibleTokens[i];
        if (token.isNoId()) 
          number = MAX_FINITE_TIME;
        else {
          if (order == HSTSHeuristics::NEAR) {
            debugMsg("HSTS:OpenDecisionManager:getNextChoice", "NEAR");
            number = abs((int)token->getStart()->lastDomain().getLowerBound() - (int)tdp->getToken()->getStart()->lastDomain().getLowerBound()) +
              abs((int)token->getStart()->lastDomain().getUpperBound() - (int)tdp->getToken()->getStart()->lastDomain().getUpperBound());
          }
          else {
            debugMsg("HSTS:OpenDecisionManager:getNextChoice","EARLY");
            number = (int)token->getStart()->lastDomain().getLowerBound();
          }
        }
        if (number < numberToBeat) {
          if (current != i) {
            tmpCompatibleToken = tdp->m_compatibleTokens[current];
            tdp->m_compatibleTokens[current] = token;
            tdp->m_compatibleTokens[i] = tmpCompatibleToken;
          }
          numberToBeat = number;
          current++;
          debugMsg("HSTS:OpenDecisionManager:getNextChoice", 
                   " best token so far (" << token->getKey() << ") [" << token->getStart()->lastDomain().getLowerBound() 
                   << "," <<  token->getStart()->lastDomain().getUpperBound() << "]");
        }
      }
    }
    else { // LATE or FAR
      debugMsg("HSTS:OpenDecisionManager:getNextChoice", "LATE || FAR");
      numberToBeat = MIN_FINITE_TIME;
      for (unsigned int i = current; i != tdp->m_compatibleTokens.size(); ++i) {
        TokenId token = tdp->m_compatibleTokens[i];
        if (token.isNoId()) 
          number = MAX_FINITE_TIME;
        else {
          if (order == HSTSHeuristics::FAR) {
            debugMsg("HSTS:OpenDecisionManager:getNextChoice","FAR");
            number = abs((int)token->getStart()->lastDomain().getLowerBound() - (int)tdp->getToken()->getStart()->lastDomain().getLowerBound()) +
              abs((int)token->getStart()->lastDomain().getUpperBound() - (int)tdp->getToken()->getStart()->lastDomain().getUpperBound());
          }
          else {
            debugMsg("HSTS:OpenDecisionManager:getNextChoice","LATE");
            number = (int)token->getStart()->lastDomain().getLowerBound();
          }
        }
        if (number > numberToBeat) {
          if (current != i) {
            tmpCompatibleToken = tdp->m_compatibleTokens[current];
            tdp->m_compatibleTokens[current] = token;
            tdp->m_compatibleTokens[i] = tmpCompatibleToken;
          }
          numberToBeat = number;
          current++;
          debugMsg("HSTS:OpenDecisionManager:getNextChoice", " best token so far (" << token->getKey() << ") [" 
                   << token->getStart()->lastDomain().getLowerBound() << "," <<  
                   token->getStart()->lastDomain().getUpperBound() << "]");
        }
      }
    }

    debugMsg("HSTS:OpenDecisionManager:getNextChoice", "Token Decision Point (" << tdp->getKey() << ") for Token (" << tdp->getToken()->getKey() << ") has best choice =  " << tdp->m_choices[0].toString() << " of " << tdp->m_choices.size());
  }

  void HSTSOpenDecisionManager::initializeVariableChoices(ConstrainedVariableDecisionPointId& vdp) {
    check_error(vdp.isValid());
    check_error(vdp->getVariable()->lastDomain().isFinite());
    check_error(vdp->m_choices.empty());
    if (vdp->m_var->lastDomain().isNumeric() && vdp->m_var->lastDomain().getSize() > 50) {
      vdp->m_choices.push_back(vdp->m_var->lastDomain().getLowerBound());
      vdp->m_choices.push_back(vdp->m_var->lastDomain().getUpperBound()); // we'll keep the initial lb and ub for reference
    }
    else {
      std::list<double> values;
      vdp->m_var->lastDomain().getValues(values);

      if (values.empty()) return;
      
      std::list<double> domain;
      m_heur->getOrderedDomainForConstrainedVariableDP(vdp, domain);
      if (domain.empty() || values.size() == 1) {
        for(std::list<double>::const_iterator it = values.begin(); it != values.end(); ++it) {
          double value = (*it);
          vdp->m_choices.push_back(value);
        }
      }
      else {
        //std::cout << " domain.size = " << domain.size() << " values.size = " << values.size() << std::endl;
        for (std::list<double>::const_iterator sit = domain.begin(); sit != domain.end(); ++ sit)
          if (vdp->m_var->lastDomain().isMember((*sit)))
            vdp->m_choices.push_back((*sit));
      }
      debugMsg("HSTS:OpenDecisionManager:initializeChoices", "Variable Decision Point (" << vdp->getKey() << ") for Variable (" << vdp->getVariable()->getKey() << ") has " << vdp->m_choices.size() << " choices.");
      debugMsg("HSTS:OpenDecisionManager:initializeChoices", "Variable Decision Point (" << vdp->getKey() << ") for Variable (" << vdp->getVariable()->getKey() << ") has best choice = " << vdp->m_choices[0]);
    } 
  }

  void HSTSOpenDecisionManager::initializeObjectChoices(ObjectDecisionPointId& odp) {
    check_error(odp.isValid());
    check_error(odp->m_choices.empty());
    DefaultOpenDecisionManager::initializeObjectChoices(odp);

    if (odp->m_choices.empty()) return;

    HSTSHeuristics::CandidateOrder order = HSTSHeuristics::UNKNOWN;
    m_heur->getOrderForObjectDP(odp, order);

    check_error (order == HSTSHeuristics::NEAR || order == HSTSHeuristics::FAR || order == HSTSHeuristics::EARLY || order == HSTSHeuristics::LATE || order == HSTSHeuristics::NONE, "Unable to handle cases other than late, early, near, far.");
    
    debugMsg("HSTS:OpenDecisionManager:getNextChoice", "Object Decision Point (" << odp->getKey() << ") sorting choices according to order = " << order);

    if (order == HSTSHeuristics::NONE) {
      debugMsg("HSTS:OpenDecisionManager:getNextChoice", "Object Decision Point (" << odp->getKey()  << ") with Token (" << odp->getToken()->getKey() << ") has best choice =  Obj (" << odp->m_choices[0].first->getKey() << ") Pred (" << odp->m_choices[0].second.first->getKey() << ") Succ (" << odp->m_choices[0].second.second->getKey() << ")");
      return;
    }
    
    // Order the successors
    unsigned int current = 0;
    std::pair< ObjectId, std::pair<TokenId, TokenId> > tmpChoice;
    int numberToBeat;
    int number;

    if (order == HSTSHeuristics::NEAR || order == HSTSHeuristics::EARLY) {
      numberToBeat = MAX_FINITE_TIME;
      for (unsigned int i = current; i != odp->m_choices.size(); ++i) {
        TokenId token;
        if (odp->m_choices[i].second.first == odp->m_token)
          token = odp->m_choices[i].second.second;
        else
          token = odp->m_choices[i].second.first;
        if (token.isNoId()) 
          number = MAX_FINITE_TIME;
        else {
          if (order == HSTSHeuristics::NEAR) {
            //	  std::cout << "NEAR" << std::endl;
            number = abs((int)token->getStart()->lastDomain().getLowerBound() - (int)odp->m_token->getStart()->lastDomain().getLowerBound()) +
              abs((int)token->getStart()->lastDomain().getUpperBound() - (int)odp->m_token->getStart()->lastDomain().getUpperBound());
          }
          else {
            //	  std::cout << "EARLY" << std::endl;
            number = (int)token->getStart()->lastDomain().getLowerBound();
          }
        }
        if (number < numberToBeat) {
          if (current != i) {
            tmpChoice = odp->m_choices[current];
            odp->m_choices[current] = odp->m_choices[i];
            odp->m_choices[i] = tmpChoice;
          }
          numberToBeat = number;
          current++;
          //	      std::cout << " best token so far (" << token->getKey() << ") [" << token->getStart()->lastDomain().getLowerBound() << "," <<  token->getStart()->lastDomain().getUpperBound() << "]" << std::endl;
        }
      }
    }
    else { // LATE or FAR
      numberToBeat = MIN_FINITE_TIME;
      for (unsigned int i = current; i != odp->m_choices.size(); ++i) {
        TokenId token;
        if (odp->m_choices[i].second.first == odp->m_token)
          token = odp->m_choices[i].second.second;
        else
          token = odp->m_choices[i].second.first;
        if (token.isNoId()) 
          number = MAX_FINITE_TIME;
        else {
          if (order == HSTSHeuristics::FAR) {
            //	  std::cout << "FAR" << std::endl;
            number = abs((int)token->getStart()->lastDomain().getLowerBound() - (int)odp->m_token->getStart()->lastDomain().getLowerBound()) +
              abs((int)token->getStart()->lastDomain().getUpperBound() - (int)odp->m_token->getStart()->lastDomain().getUpperBound());
          }
          else {
            //	  std::cout << "LATE" << std::endl;
            number = (int)token->getStart()->lastDomain().getLowerBound();
          }
        }
        if (number > numberToBeat) {
          if (current != i) {
            tmpChoice = odp->m_choices[current];
            odp->m_choices[current] = odp->m_choices[i];
            odp->m_choices[i] = tmpChoice;
          }
          numberToBeat = number;
          current++;
          //	      std::cout << " best token so far (" << token->getKey() << ") [" << token->getStart()->lastDomain().getLowerBound() << "," <<  token->getStart()->lastDomain().getUpperBound() << "]" << std::endl;
        }
      }
    }

    debugMsg("HSTS:OpenDecisionManager:getNextChoice", "Object Decision Point (" << odp->getKey()  << ") with Token (" << odp->getToken()->getKey() << ") has best choice =  Obj (" << odp->m_choices[0].first->getKey() << ") Pred (" << odp->m_choices[0].second.first->getKey() << ") Succ (" << odp->m_choices[0].second.second->getKey() << ")");
  }

}
