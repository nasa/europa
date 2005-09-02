#include "TokenVariable.hh"
#include "HSTSOpenDecisionManager.hh"
#include "TokenDecisionPoint.hh"
#include "ObjectDecisionPoint.hh"
#include "ConstrainedVariableDecisionPoint.hh"
#include "Object.hh"
#include "HSTSHeuristics.hh"
#include "MasterMustBeInserted.hh"
#include "Debug.hh"

namespace EUROPA {

  HSTSOpenDecisionManager::HSTSOpenDecisionManager(const PlanDatabaseId& db, 
						   const HSTSHeuristicsId& heur, 
						   bool strictHeuristics)
    : OpenDecisionManager(db), m_heur(heur), m_strictHeuristics(strictHeuristics) {
  }

  HSTSOpenDecisionManager::~HSTSOpenDecisionManager() { }

  /**
   * Iterate over the set of candidates. If it gets a better priority than the current best priority.
   */
  DecisionPointId HSTSOpenDecisionManager::getBestObjectDecision(HSTSHeuristics::Priority& bestp) {
    debugMsg("HSTSOpenDecisionManager:getBestObjectDecision", 
	     "Evaluating at priority " << bestp << "." << m_db->getTokensToOrder().size() << " candidates to evaluate.");

    DecisionPointId betterDecision;
    TokenId flawToResolve;
    const std::map<TokenId, ObjectSet>& candidates = m_db->getTokensToOrder(); // Pushed via propagation
    for(std::map<TokenId, ObjectSet>::const_iterator it = candidates.begin(); it != candidates.end(); ++it){
      TokenId candidate = it->first;

      // First, if not a decision, skip it
      if(!isObjectDecision(candidate))
	continue;

      // Since it is a decision, obtain its priority
      HSTSHeuristics::Priority priority = m_heur->getPriorityForObjectDP(candidate);

      if(m_heur->preferredPriority(priority, bestp) == priority){
	flawToResolve = candidate;
	bestp = priority;
      }
    }
 
    // If we have a token to order, then it means we have a better priority and so
    // we allocate a new decision
    if(flawToResolve.isId())
      betterDecision = createObjectDecisionPoint(flawToResolve);

    debugMsg("HSTSOpenDecisionManager:getBestObjectDecision", "Finishing with priority " << bestp << " and decision: " << betterDecision);
    return betterDecision;
  }

  DecisionPointId HSTSOpenDecisionManager::getBestTokenDecision(HSTSHeuristics::Priority& bestp) {
    debugMsg("HSTSOpenDecisionManager:getBestTokenDecision", 
	     "Evaluating at priority " << bestp << "." << getTokenFlawCandidates().size() << " candidates to evaluate.");
    DecisionPointId betterDecision;
    TokenId flawToResolve;
    const TokenSet& candidates = getTokenFlawCandidates();
    for(TokenSet::const_iterator it = candidates.begin(); it != candidates.end(); ++it){
      TokenId candidate = *it;

      // First, if not a decision, skip it
      if(!isTokenDecision(candidate))
	continue;

      // Since it is a decision, obtain its priority
      HSTSHeuristics::Priority priority = m_heur->getPriorityForTokenDP(candidate);

      if(m_heur->preferredPriority(priority, bestp) == priority){
	flawToResolve = candidate;
	bestp = priority;
      }
    }
 
    // If we have a token to order, then it means we have a better priority and so
    // we allocate a new decision
    if(flawToResolve.isId())
      betterDecision = createTokenDecisionPoint(flawToResolve);

    debugMsg("HSTSOpenDecisionManager:getBestTokenDecision", "Finishing with priority " << bestp << " and decision: " << betterDecision);
    return betterDecision;
  }

  DecisionPointId HSTSOpenDecisionManager::getBestNonUnitVariableDecision(HSTSHeuristics::Priority& bestp) {
    debugMsg("HSTSOpenDecisionManager:getBestNonUnitVariableDecision",
	     "Evaluating at priority " << bestp  << "." << getVariableFlawCandidates().size() << " candidates to evaluate.");

    DecisionPointId betterDecision;
    ConstrainedVariableId flawToResolve;
    const ConstrainedVariableSet& candidates = getVariableFlawCandidates();
    for (ConstrainedVariableSet::iterator it = candidates.begin(); it != candidates.end(); ++it) {
      ConstrainedVariableId candidate(*it);
      checkError(candidate.isValid() &&  !candidate->lastDomain().isSingleton(), 
		 "Is a singleton!" << candidate->toString());

      // If it fails to pass the dynamic filter, we exclude and move on
      if(!passesDynamicConditions(candidate))
	continue;

      // Since it is a decision, obtain its priority
      HSTSHeuristics::Priority priority = m_heur->getPriorityForConstrainedVariableDP(candidate);

      if(m_heur->preferredPriority(priority, bestp) == priority){
	flawToResolve = candidate;
	bestp = priority;
      }
    }

    if(flawToResolve.isId())
      betterDecision = createConstrainedVariableDecisionPoint(flawToResolve);

    debugMsg("HSTSOpenDecisionManager:getBestNonUnitVariableDecision", "Finishing with priority " << bestp << " and decision: " << betterDecision);
    return betterDecision;
  }


  void updateDecisionPoints(DecisionPointId& candidate, DecisionPointId& best){
    if(candidate.isId()){
      if(best.isId())
	delete (DecisionPoint*) best;
      best = candidate;
      candidate = DecisionPointId::noId();
    }
  }

  DecisionPointId HSTSOpenDecisionManager::getNextDecision() {
    debugMsg("HSTSOpenDecisionManager:getNextDecision", "Entering. Heuristics are " << (m_strictHeuristics ? "strict" : "loose"));
    //prefer zero commitment decisions over everything. So just try to get one.
    DecisionPointId bestDecision = getZeroCommitmentDecision();

    HSTSHeuristics::Priority bestp = MIN_PRIORITY - 1; // Low is worst

    // See if we switch interpretation around.
    if(m_heur->getDefaultPriorityPreference() == HSTSHeuristics::LOW)
      bestp = MAX_PRIORITY + 1;

    // If we failed, look for the priority based best decision
    if(bestDecision.isNoId()){
      DecisionPointId candidateDecision = getBestObjectDecision(bestp);

      updateDecisionPoints(candidateDecision, bestDecision);

      if(m_strictHeuristics)
	candidateDecision = getBestNonUnitVariableDecision(bestp);

      updateDecisionPoints(candidateDecision, bestDecision);

      if(m_strictHeuristics)
	candidateDecision = getBestTokenDecision(bestp);

      updateDecisionPoints(candidateDecision, bestDecision);
    }

    if(bestDecision.isId())
      initializeChoices(bestDecision);

    debugMsg("HSTS:OpenDecisionManager:getNextDecision", "Best Dec = [" << bestp << "] " << bestDecision);
    return bestDecision;
  }

  void HSTSOpenDecisionManager::initializeTokenChoices(const TokenDecisionPointId& tdp) {
    check_error(tdp.isValid());
    check_error(tdp->m_choices.empty());
    OpenDecisionManager::initializeTokenChoices(tdp);

    if (tdp->m_choices.empty()) return;

    std::list<LabelStr> states;
    HSTSHeuristics::CandidateOrder order = HSTSHeuristics::UNKNOWN;
    m_heur->getOrderedStatesForTokenDP(tdp->getToken(), states, order);

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

  void HSTSOpenDecisionManager::initializeVariableChoices(const ConstrainedVariableDecisionPointId& vdp) {
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
      m_heur->getOrderedDomainForConstrainedVariableDP(vdp->m_var, domain);
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

  void HSTSOpenDecisionManager::initializeObjectChoices(const ObjectDecisionPointId& odp) {
    check_error(odp.isValid());
    OpenDecisionManager::initializeObjectChoices(odp);

    if (odp->hasRemainingChoices()) return;

    HSTSHeuristics::CandidateOrder order = HSTSHeuristics::UNKNOWN;
    m_heur->getOrderForObjectDP(odp->getToken(), order);

    check_error (order == HSTSHeuristics::NEAR || order == HSTSHeuristics::FAR || order == HSTSHeuristics::EARLY || order == HSTSHeuristics::LATE || order == HSTSHeuristics::NONE, "Unable to handle cases other than late, early, near, far.");
    
    debugMsg("HSTS:OpenDecisionManager:getNextChoice", "Object Decision Point (" << odp->getKey() << ") sorting choices according to order = " << order);

    if (order == HSTSHeuristics::NONE) {
      debugMsg("HSTS:OpenDecisionManager:getNextChoice", "Object Decision Point: " << odp->toString());
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
