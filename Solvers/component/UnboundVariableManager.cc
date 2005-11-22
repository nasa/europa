#include "UnboundVariableManager.hh"
#include "ConstrainedVariable.hh"
//#include "MatchingRule.hh"
#include "PlanDatabase.hh"
#include "ConstraintEngine.hh"
#include "Debug.hh"
#include "Constraint.hh"
#include "RuleVariableListener.hh"
#include "RuleInstance.hh"
#include "Token.hh"
#include "Utils.hh"
#include "SolverUtils.hh"
#include "ComponentFactory.hh"
#include "UnboundVariableDecisionPoint.hh"

/**
 * @file Provides implementation for UnboundVariableManager
 * @author Conor McGann
 * @date May, 2005
 */

namespace EUROPA {
  namespace SOLVERS {

    /**
     * @brief Constructor will evaluate the configuration information and construct assembly from there.
     * @see ComponentFactory
     */
    UnboundVariableManager::UnboundVariableManager(const TiXmlElement& configData)
      : FlawManager(configData) {}

    void UnboundVariableManager::handleInitialize(){

      // FILL UP VARIABLES
      const ConstrainedVariableSet& allVars = m_db->getConstraintEngine()->getVariables();
      for(ConstrainedVariableSet::const_iterator it = allVars.begin(); it != allVars.end(); ++it){
	ConstrainedVariableId var = *it;
	addFlaw(var);
      }

      // PROCESS CONSTRAINTS TO INITIALIZE GUARDS. We are looking for RuleVariableListener constraints since they 
      // determine if a variable is guarded or not.
      const ConstraintSet& allConstraints = m_db->getConstraintEngine()->getConstraints();
      for(ConstraintSet::const_iterator it = allConstraints.begin(); it != allConstraints.end(); ++it){ 
	ConstraintId constraint = *it;
	handleConstraintAddition(constraint);
      }
    }

    bool UnboundVariableManager::inScope(const EntityId& entity) const {
      bool result = false;
      if(ConstrainedVariableId::convertable(entity)){
	ConstrainedVariableId var = entity;
	result =  (!var->isSpecified() && FlawManager::inScope(entity));
      }

      return result;
    }

    DecisionPointId UnboundVariableManager::next(unsigned int priorityLowerBound,
						 unsigned int& bestPriority){
      // For the special case of a singleton flaw we will use the cached singlton flaw candidates
      if(bestPriority == 2)
	return next(priorityLowerBound, bestPriority, m_singletonFlawCandidates);
      else
	return next(priorityLowerBound, bestPriority, m_flawCandidates);
    }

    DecisionPointId UnboundVariableManager::next(unsigned int priorityLowerBound,
						 unsigned int& bestPriority,
						 const ConstrainedVariableSet& flawCandidates){
      checkError(bestPriority > priorityLowerBound, 
		 "Should not be calling this otherwise: " << bestPriority << ">=" << priorityLowerBound);

      ConstrainedVariableId flawedVariable;
      bool flawIsGuarded = false;

      debugMsg("UnboundVariableManager:next",
	       "Evaluating next decision to work on. Must beat priority of " << bestPriority);

      for(ConstrainedVariableSet::const_iterator it = flawCandidates.begin(); it != flawCandidates.end(); ++it){
	if(bestPriority <= priorityLowerBound) // Can't do better
	  break;

	ConstrainedVariableId candidate = *it;

	checkError(!variableOfNonActiveToken(candidate),
		   "Expect that " << candidate->toString() << " cannot belong to an inactive or merged token.");

	check_error(!candidate->isSpecified(),
		    "Should not be seeing this as a candidate flaw since it is already specified.");

	if(dynamicMatch(candidate)){
	  debugMsg("UnboundVariableManager:next",
		   candidate->toString() << " is out of dynamic scope.");
	  continue;
	}

	const AbstractDomain& derivedDomain = candidate->derivedDomain();

	if(!derivedDomain.isOpen() && derivedDomain.areBoundsFinite()){
	  unsigned int valueCount = candidate->lastDomain().getSize();

	  // If it is not a best case priority, and not a guard, but we have a guard, then skip it
	  if(valueCount > priorityLowerBound && flawIsGuarded){
	    debugMsg("UnboundVariableManager:next",
		     candidate->toString() << " does not beat a guarded variable decision.");
	    continue;
	  }

	  debugMsg("UnboundVariableManager:next",
		   candidate->toString() << " has a priority of " << valueCount);

	  if(valueCount < bestPriority){
	    bestPriority = valueCount;
	    flawedVariable = candidate;
	    flawIsGuarded = (m_guardCache.find(candidate) != m_guardCache.end());
	  }
	}

	debugMsg("UnboundVariableManager:next",
		 candidate->toString() <<
		 (candidate == flawedVariable ? " is the best candidate so far." : " is not a better candidate."));
      }

      if(flawedVariable.isNoId())
	return DecisionPointId::noId();

      // If not a best-case, but is a guard, then set bestPriority to 2 so it dominates all
      // non singleton decision types.
      if(bestPriority > priorityLowerBound && flawIsGuarded)
	bestPriority = priorityLowerBound+1;

      // If it is neither a singleton nor a guard, then bump up the priority so that it is dominated
      // by all other decision types
      if(bestPriority > priorityLowerBound)
	bestPriority = WORST_SCORE();

      DecisionPointId decisionPoint = allocateDecisionPoint(flawedVariable);

      checkError(decisionPoint.isValid(),
		 "Failed to allocate a decision point for " << flawedVariable->toString() <<
		 " . Indicates that now FlawHandler is configured for this flaw.");

      return decisionPoint;
    }

    /**
     * We may filter based on static information only.
     */
    void UnboundVariableManager::addFlaw(const ConstrainedVariableId& var){
      if(!variableOfNonActiveToken(var) && var->canBeSpecified() && !var->isSpecified() && !staticMatch(var)){
	debugMsg("UnboundVariableManager:addFlaw",
		 "Adding " << var->toString() << " as a candidate flaw.");
	m_flawCandidates.insert(var);

	if(var->lastDomain().isSingleton())
	  m_singletonFlawCandidates.insert(var);
      }
    }

    void UnboundVariableManager::removeFlaw(const ConstrainedVariableId& var){
      condDebugMsg(m_flawCandidates.find(var) != m_flawCandidates.end(), "UnboundVariableManager:removeFlaw", "Removing " << var->toString() << " as a flaw.");
      m_flawCandidates.erase(var);
      m_singletonFlawCandidates.erase(var);
    }

    void UnboundVariableManager::toggleSingletonFlaw(const ConstrainedVariableId& var){
      debugMsg("UnboundVariableManager:toggleSingletonFlaw", var->toString());

      // Only if it has already passed the test to be a flaw candidate do we do anything with it
      if(m_flawCandidates.find(var) != m_flawCandidates.end()){
	if(var->lastDomain().isSingleton())
	  m_singletonFlawCandidates.insert(var);
	else
	  m_singletonFlawCandidates.erase(var);
      } 
    }

    void UnboundVariableManager::addGuard(const ConstrainedVariableId& var){
      std::map<ConstrainedVariableId, unsigned int>::iterator it = m_guardCache.find(var);
      unsigned int refCount = 1;
      // If already guarded just increment the ref count
      if(it != m_guardCache.end()){
	refCount = it->second;
	refCount++;
	it->second = refCount;
      }
      else // Insert a new pair
	m_guardCache.insert(std::pair<ConstrainedVariableId, unsigned int>(var, 1));

      debugMsg("UnboundVariableManager:addGuard", 
	       "GUARDS=" << refCount << " for " << var->getName().toString() << "(" << var->getKey() << ")");
    }

    void UnboundVariableManager::removeGuard(const ConstrainedVariableId& var){
      std::map<ConstrainedVariableId, unsigned int>::iterator it = m_guardCache.find(var);
      check_error(it != m_guardCache.end(), "Cannot see how guard would not be here so force it to be.");

      unsigned int refCount = 0;
      if(it->second == 1)
	m_guardCache.erase(it);
      else {
	refCount = it->second - 1;
	it->second = refCount;
      }

      debugMsg("UnboundVariableManager:removeGuard", 
	       "GUARDS=" << refCount << " for " << var->getName().toString() << "(" << var->getKey() << ")");
    }

    void UnboundVariableManager::handleConstraintAddition(const ConstraintId& constraint){
      if(constraint->getName() == RuleVariableListener::CONSTRAINT_NAME()){
	const std::vector<ConstrainedVariableId>& scope = constraint->getScope();
	for(std::vector<ConstrainedVariableId>::const_iterator it = scope.begin(); it != scope.end(); ++it){
	  ConstrainedVariableId guard = *it;
	  addGuard(guard);
	}
      }
    }

    void UnboundVariableManager::handleConstraintRemoval(const ConstraintId& constraint){
      if(constraint->getName() == RuleVariableListener::CONSTRAINT_NAME()){
	const std::vector<ConstrainedVariableId>& scope = constraint->getScope();
	for(std::vector<ConstrainedVariableId>::const_iterator it = scope.begin(); it != scope.end(); ++it){
	  ConstrainedVariableId guard = *it;
	  removeGuard(guard);
	}
      }
    }

    bool UnboundVariableManager::variableOfNonActiveToken(const ConstrainedVariableId& var){
      // If var parent is a token and the state is active, then true.
      if(TokenId::convertable(var->getParent())){
	TokenId token(var->getParent());
	return !token->isActive();
      }

      // Otherwise false
      return false;
    }


    void UnboundVariableManager::notifyRemoved(const ConstrainedVariableId& variable){
      removeFlaw(variable);
    }

    void UnboundVariableManager::notifyChanged(const ConstrainedVariableId& variable, 
					       const DomainListener::ChangeType& changeType){
      if(changeType == DomainListener::SET_TO_SINGLETON){
	// If it is a token state variable, we test if a case for activation
	if(Token::isStateVariable(variable) &&
	   variable->getSpecifiedValue() == Token::ACTIVE){
	  TokenId token = variable->getParent();
	  const std::vector<ConstrainedVariableId>& variables = token->getVariables();
	  for(std::vector<ConstrainedVariableId>::const_iterator it = variables.begin(); it != variables.end(); ++it){
	    ConstrainedVariableId var = *it;
	    addFlaw(var);
	  }
	}
	else
	  removeFlaw(variable);
      }
      else if(changeType == DomainListener::RESET || changeType == DomainListener::CLOSED){
	// If it is a token state variable, we remove all its variables
	if(Token::isStateVariable(variable)){
	  TokenId token = variable->getParent();
	  const std::vector<ConstrainedVariableId>& variables = token->getVariables();
	  for(std::vector<ConstrainedVariableId>::const_iterator it = variables.begin(); it != variables.end(); ++it){
	    ConstrainedVariableId var = *it;
	    removeFlaw(var);
	  }
	}
	else
	  addFlaw(variable);
      }
      else if(changeType == DomainListener::OPENED)
	addFlaw(variable);
      else if(changeType == DomainListener::RELAXED || changeType == DomainListener::RESTRICT_TO_SINGLETON)
	toggleSingletonFlaw(variable);
    }

    void UnboundVariableManager::notifyAdded(const ConstraintId& constraint){
      handleConstraintAddition(constraint);
    }

    void UnboundVariableManager::notifyRemoved(const ConstraintId& constraint){
      handleConstraintRemoval(constraint);
    }

    IteratorId UnboundVariableManager::createIterator() const {
      return (new UnboundVariableManager::FlawIterator(*this))->getId();
    }

    UnboundVariableManager::FlawIterator::FlawIterator(const UnboundVariableManager& manager)
      : m_visited(0), m_timestamp(manager.m_db->getConstraintEngine()->cycleCount()),
	m_manager(manager), m_it(manager.m_flawCandidates.begin()), m_end(manager.m_flawCandidates.end())  {}
    
    bool UnboundVariableManager::FlawIterator::done() const { return m_it == m_end;}

    const EntityId UnboundVariableManager::FlawIterator::next() {
      check_error(m_manager.m_db->getConstraintEngine()->cycleCount() == m_timestamp,
		  "Error: potentially stale flaw iterator.");
      ConstrainedVariableId retval = ConstrainedVariableId::noId();
      
      for(; !done(); ++m_it) {
	ConstrainedVariableId var = *m_it;
	check_error(var.isValid());
	if(m_manager.inScope(*m_it)) {
	  retval = var;
	  ++m_visited;
	  ++m_it;
	  break;
	}
      }
      return retval;
    }
  }
}
