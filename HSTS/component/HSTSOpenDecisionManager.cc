#include "TokenVariable.hh"
#include "HSTSOpenDecisionManager.hh"
#include "Object.hh"
#include "HeuristicsEngine.hh"
#include "Heuristic.hh"
#include "ConstrainedVariableDecisionPoint.hh"
#include "TokenDecisionPoint.hh"
#include "ObjectDecisionPoint.hh"
#include "TemporalVariableFilter.hh"
#include "Debug.hh"

namespace EUROPA {

  HSTSOpenDecisionManager::HSTSOpenDecisionManager(const PlanDatabaseId& db, 
						   const HeuristicsEngineId& heur, 
						   bool strictHeuristics)
    : OpenDecisionManager(db), m_heur(heur), m_strictHeuristics(strictHeuristics) {
    checkError(strictHeuristics, "Loose heuristics are not supported.");
  }

  HSTSOpenDecisionManager::~HSTSOpenDecisionManager() { }

  /**
   * @brief Obtain the heuristic value for the given UnboundVariable.
   * @param var The given unbound variable
   */
  Priority HSTSOpenDecisionManager::getPriorityForUnboundVariable(const ConstrainedVariableId& var) const {
    return m_heur->getPriority(var);
  }

  /**
   * @brief Obtain the heuristic value for the given inactive token.
   */
  Priority HSTSOpenDecisionManager::getPriorityForOpenCondition(const TokenId& token) const {
    return m_heur->getPriority(token);
  }

  /**
   * @brief Obtain the heuristic value for the given threatened token (one that requires ordering)
   */
  Priority HSTSOpenDecisionManager::getPriorityForThreat(const TokenId& token) const {
    return m_heur->getPriority(token);
  }

  void HSTSOpenDecisionManager::initializeTokenChoices(const TokenDecisionPointId& tdp) {
    check_error(tdp.isValid());
    check_error(tdp->m_choices.empty());
    OpenDecisionManager::initializeTokenChoices(tdp);

    if (tdp->m_choices.empty()) return;

    std::list<LabelStr> states;
    TokenHeuristic::CandidateOrder order = TokenHeuristic::UNKNOWN;
    m_heur->getChoices(tdp->getToken(), states, order);

    checkError (order == TokenHeuristic::NEAR || order == TokenHeuristic::FAR || 
		order == TokenHeuristic::EARLY || order == TokenHeuristic::LATE || order == TokenHeuristic::NONE,
		 "Unable to handle cases other than late, early, near, far, none. " << 
		 TokenHeuristic::candidateOrderStrings()[order]);
    
    debugMsg("HSTS:OpenDecisionManager:getNextChoice", "Token Decision Point (" << tdp->getKey() << ") for Token (" << tdp->getToken()->getKey() << ") has " << tdp->m_choices.size() + tdp->m_compatibleTokens.size() << " choices.");
    
    if (order == TokenHeuristic::NONE) {
      debugMsg("HSTS:OpenDecisionManager:getNextChoice", "Token Decision Point (" << tdp->getKey() << ") for Token (" << tdp->getToken()->getKey() << ") has best choice =  " << LabelStr(tdp->m_choices[0]).toString());
      return;
    }

    debugMsg("HSTS:OpenDecisionManager:getNextChoice", "Clearing unavailable states from the potential state list.");

    // TODO: Remove this streaming and place inside the guard of a debug message!
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

    if (order == TokenHeuristic::NEAR || order == TokenHeuristic::EARLY) {
      debugMsg("HSTS:OpenDecisionManager:getNextChoice", "NEAR || EARLY");
      numberToBeat = MAX_FINITE_TIME;
      for (unsigned int i = current; i != tdp->m_compatibleTokens.size(); ++i) {
        TokenId token = tdp->m_compatibleTokens[i];
        if (token.isNoId()) 
          number = MAX_FINITE_TIME;
        else {
          if (order == TokenHeuristic::NEAR) {
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
          if (order == TokenHeuristic::FAR) {
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
      m_heur->getChoices(vdp->m_var, domain);
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
    check_error(odp->m_choices.empty());
    OpenDecisionManager::initializeObjectChoices(odp);

    if (odp->m_choices.empty()) return;

    TokenHeuristic::CandidateOrder order = TokenHeuristic::UNKNOWN;
    m_heur->getChoices(odp->getToken(), order);

    checkError (order == TokenHeuristic::NEAR || order == TokenHeuristic::FAR || 
		order == TokenHeuristic::EARLY || order == TokenHeuristic::LATE || order == TokenHeuristic::NONE, 
		"Unable to handle cases other than late, early, near, far. " <<
		TokenHeuristic::candidateOrderStrings()[order]);
    
    debugMsg("HSTS:OpenDecisionManager:getNextChoice", "Object Decision Point (" << odp->getKey() << ") sorting choices according to order = " << order);

    if (order == TokenHeuristic::NONE) {
      debugMsg("HSTS:OpenDecisionManager:getNextChoice", "Object Decision Point (" << odp->getKey()  << ") with Token (" << odp->getToken()->getKey() << ") has best choice =  Obj (" << odp->m_choices[0].first->getKey() << ") Pred (" << odp->m_choices[0].second.first->getKey() << ") Succ (" << odp->m_choices[0].second.second->getKey() << ")");
      return;
    }
    
    // Order the successors
    unsigned int current = 0;
    std::pair< ObjectId, std::pair<TokenId, TokenId> > tmpChoice;
    int numberToBeat;
    int number;

    if (order == TokenHeuristic::NEAR || order == TokenHeuristic::EARLY) {
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
          if (order == TokenHeuristic::NEAR) {
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
          if (order == TokenHeuristic::FAR) {
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


  const Priority& HSTSOpenDecisionManager::bestCasePriority() const {
    return m_heur->bestCasePriority();
  }

  const Priority& HSTSOpenDecisionManager::worstCasePriority() const {
    return m_heur->worstCasePriority();
  }

  bool HSTSOpenDecisionManager::betterThan(const Priority p1, const Priority p2) const {
    return m_heur->betterThan(p1, p2);
  }
}
