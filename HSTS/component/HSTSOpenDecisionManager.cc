#include "TokenVariable.hh"
#include "DefaultOpenDecisionManager.hh"
#include "HSTSOpenDecisionManager.hh"
#include "Object.hh"
#include "HeuristicsEngine.hh"
#include "MasterMustBeInserted.hh"
#include "Debug.hh"

namespace EUROPA {

  HSTSOpenDecisionManager::HSTSOpenDecisionManager(const DecisionManagerId& dm, const HeuristicsEngineId& heur, const bool strictHeuristics)
    : DefaultOpenDecisionManager(dm), m_heur(heur), m_strictHeuristics(strictHeuristics) {
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

  void HSTSOpenDecisionManager::getBestObjectDecision(DecisionPointId& bestDec, Priority& bestp) {
    check_error(bestDec.isNoId());
    if (m_sortedObjectDecs.empty()) return;
    unsigned int bestNrChoices=0;
    for (ObjectDecisionSet::iterator it = m_sortedObjectDecs.begin(); it != m_sortedObjectDecs.end(); ++it) {
      TokenId tok = (*it)->getToken();
      const Priority priority = m_heur->getPriority(tok);
      if (m_heur->betterThan(priority, bestp)) {
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
      bestp = m_heur->getPriority(ObjectDecisionPointId(bestDec)->getToken());
    }
  }

  void HSTSOpenDecisionManager::getBestTokenDecision(DecisionPointId& bestDec, Priority& bestp) {
    check_error(bestDec.isNoId());
    if (m_sortedTokDecs.empty()) return;
    unsigned int bestNrChoices = 999999;
    for (TokenDecisionSet::iterator it = m_sortedTokDecs.begin(); it != m_sortedTokDecs.end(); ++it) {
      TokenId tok = (*it)->getToken();
      const Priority priority = m_heur->getPriority(tok);
      debugMsg("HSTS:OpenDecisionManager:getBestTokenDecision", "Comparing priority = " << priority << " to bestp = " << bestp);
      if (m_heur->betterThan(priority, bestp)) {
        bestDec = *it;
        bestp = priority;
        //TokenDecisionPointId tokDec = bestDec; // CMG: THIS IS NOT USED. WHY NOT?
        if(tok->getState()->lastDomain().isMember(Token::MERGED))
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
      bestp = m_heur->getPriority(ObjectDecisionPointId(bestDec)->getToken());
      debugMsg("HSTS:OpenDecisionManager:getBestTokenDecision", "Selecting first decision as bestDec = " << bestDec);
    }
  }

  /**
   * Priority does not acutaully count for this since all units have the same priority
   */
  void HSTSOpenDecisionManager::getBestUnitVariableDecision(DecisionPointId& bestDec, Priority& bestp) {
    check_error(bestDec.isNoId());
    if (m_sortedUnitVarDecs.empty()) 
      return;

    for (VariableDecisionSet::iterator it = m_sortedUnitVarDecs.begin(); it != m_sortedUnitVarDecs.end(); ++it) {
      // ignore variables of uninserted tokens
      ConstrainedVariableDecisionPointId vdec(*it);

      checkError(vdec.isValid() && vdec->getVariable()->lastDomain().isSingleton(),
		 "Not a sinleton!" << vdec->getVariable()->toString());

      if(MasterMustBeInserted::executeTest( vdec->getVariable() )){
	bestDec = vdec;
	return;
      }
    }
  }

  void HSTSOpenDecisionManager::getBestNonUnitVariableDecision(DecisionPointId& bestDec, Priority& bestp) {
    check_error(bestDec.isNoId());
    if (m_sortedNonUnitVarDecs.empty()) return;
    ConstrainedVariableDecisionPointId bestFloatDec;
    Priority bestFloatp = bestp;
    VariableDecisionSet::iterator it = m_sortedNonUnitVarDecs.begin();
    for ( ; it != m_sortedNonUnitVarDecs.end(); ++it) {
      // Ignore variables of inactive tokens.
      ConstrainedVariableDecisionPointId vdec(*it);
      check_error(vdec.isValid());
      if( !MasterMustBeInserted::executeTest( vdec->getVariable() ) )
	continue;
      if (TokenId::convertable(vdec->getVariable()->getParent())) {
        TokenId parent = vdec->getVariable()->getParent();
        if (!parent->isActive())
          continue;
      }
      const Priority& priority = m_heur->getPriority(vdec->getVariable());
      if (vdec->getVariable()->lastDomain().isFinite()) {
        if (bestDec.isNoId() ||
            m_heur->betterThan(priority, bestp)) {
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
            m_heur->betterThan(priority, bestFloatp)) {
          bestFloatDec = vdec;
          bestFloatp = priority;
        } // else ... messy to compare sizes of infinite domains, but it could be done if needed
      }
    }
    if (bestDec.isNoId() && !bestFloatDec.isNoId()) {
      bestDec = bestFloatDec;
      bestp = bestFloatp;
    }
  }

  DecisionPointId HSTSOpenDecisionManager::getNextDecision() {
    return (m_strictHeuristics ? getNextDecisionStrict() : getNextDecisionLoose());
  }

  DecisionPointId HSTSOpenDecisionManager::getNextDecisionStrict() {
    DecisionPointId bestDec = DecisionPointId::noId();
    Priority bestP = m_heur->worstCasePriority() * 2;

    //prefer unit variable decisions over everything
    getBestUnitVariableDecision(bestDec, bestP);
    
    //if we don't have a decision yet, get the best of the object, token, 
    //and non-unit variable decisions
    if(bestDec.isNoId()) {
      DecisionPointId oDec = DecisionPointId::noId();
      DecisionPointId tDec = DecisionPointId::noId();
      DecisionPointId vDec = DecisionPointId::noId();
      Priority bestOP = bestP;
      Priority bestTP = bestP;
      Priority bestVP = bestP;

      getBestObjectDecision(oDec, bestOP);
      getBestNonUnitVariableDecision(vDec, bestVP);
      getBestTokenDecision(tDec, bestTP);

      bestP = bestOP;
      bestDec = oDec;

      if(m_heur->betterThan(bestVP, bestP)){
          bestDec = vDec;
          bestP = bestVP;
      }

      if(m_heur->betterThan(bestTP, bestP)){
          bestDec = tDec;
          bestP = bestTP;
      }
    }
    
    debugMsg("HSTS:OpenDecisionManager:getNextDecision", "Best Dec = [" << bestP << "] " << bestDec);
    m_curDec = bestDec;
    return bestDec;
  }

  DecisionPointId HSTSOpenDecisionManager::getNextDecisionLoose() {
    DecisionPointId bestODec;
    DecisionPointId bestTDec;
    DecisionPointId bestVDec;
    Priority bestOP = m_heur->worstCasePriority() * 2;
    Priority bestTP = bestOP;
    Priority bestVP = bestOP;

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

    if(bestODec.isNoId() && bestTDec.isNoId() && bestVDec.isNoId()) {
      debugMsg("HSTS:OpenDecisionManager:getNextDecision", "No decisions to make.  Returning noId.");
      return DecisionPointId::noId();
    }

    /* If I'm here, I need to compare bestTDec to bestVDec */
    if (m_heur->betterThan(bestTP, bestVP)) {
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

    //check_error(assignedBest || m_curDec.isNoId() || m_curDec->hasRemainingChoices());

    return m_curDec;
  } /* we have found a best */

  void HSTSOpenDecisionManager::initializeTokenChoicesInternal(const TokenDecisionPointId& tdp) {
    const StateDomain stateDomain(tdp->getToken()->getState()->lastDomain());
    TokenId tok(tdp->getToken());
    if(stateDomain.isMember(Token::MERGED)) {
      tok->getPlanDatabase()->getCompatibleTokens(tok, tdp->m_compatibleTokens, PLUS_INFINITY, true);
      debugMsg("HSTSOpenDecisionManager:initializeTokenChoices", "Found " << tdp->m_compatibleTokens.size() << " compatible tokens");
      if(tdp->m_compatibleTokens.size() > 0) {
        debugMsg("HSTSOpenDecisionManager:initializeTokenChoices", "Pushing token:merged m_choices");
	tdp->m_choices.push_back(Token::MERGED);
      }
    }
    if(stateDomain.isMember(Token::ACTIVE) && tok->getPlanDatabase()->hasOrderingChoice(tok))
      tdp->m_choices.push_back(Token::ACTIVE);
    if(stateDomain.isMember(Token::REJECTED))
      tdp->m_choices.push_back(Token::REJECTED);
  }

  void HSTSOpenDecisionManager::initializeObjectChoicesInternal(const ObjectDecisionPointId& odp) {
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

  void HSTSOpenDecisionManager::initializeTokenChoices(const TokenDecisionPointId& tdp) {
    check_error(tdp.isValid());
    check_error(tdp->m_choices.empty());
    initializeTokenChoicesInternal(tdp);

    if (tdp->m_choices.empty()) return;

    m_heur->orderChoices(tdp->getToken(), tdp->m_choices, tdp->m_compatibleTokens);
  }

  void HSTSOpenDecisionManager::initializeVariableChoices(const ConstrainedVariableDecisionPointId& vdp) {
    check_error(vdp.isValid());
    check_error(vdp->getVariable()->lastDomain().isFinite());
    check_error(vdp->m_choices.empty());

    if (vdp->m_var->lastDomain().isNumeric() && vdp->m_var->lastDomain().getSize() > 50) {
      vdp->m_choices.push_back(vdp->m_var->lastDomain().getLowerBound());
      vdp->m_choices.push_back(vdp->m_var->lastDomain().getUpperBound()); // we'll keep the initial lb and ub for ref.
    }
    else {
      std::list<double> values;
      vdp->m_var->lastDomain().getValues(values);
      m_heur->orderChoices(vdp->m_var, values);
      check_error(!values.empty(), "That would be an inconsistency which we should have detected already.");
      for(std::list<double>::const_iterator it = values.begin(); it != values.end(); ++it)
	vdp->m_choices.push_back(*it);
    }
  }

  void HSTSOpenDecisionManager::initializeObjectChoices(const ObjectDecisionPointId& odp) {
    check_error(odp.isValid());
    check_error(odp->m_choices.empty());
    initializeObjectChoicesInternal(odp);

    if (odp->m_choices.empty()) return;

    // Invoke heuristics engine to order choices according to whatever heuristics it has
    m_heur->orderChoices(odp->getToken(), odp->m_choices);

  }
}
