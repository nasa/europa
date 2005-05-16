#include "VariableFlawManager.hh"
#include "ConstrainedVariable.hh"
#include "MatchingRule.hh"
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
#include "VariableDecisionPoint.hh"

/**
 * @file Provides implementation for VariableFlawManager
 * @author Conor McGann
 * @date May, 2005
 */

namespace EUROPA {
  namespace SOLVERS {

    /* REGSITER DEFAULT FLAW MANAGERS */
    REGISTER_COMPONENT_FACTORY(VariableFlawManager, VariableFlawManager);

    /**
     * @brief Constructor will evaluate the configuration information and construct assembly from there.
     * @see ComponentFactory
     */
    VariableFlawManager::VariableFlawManager(const TiXmlElement& configData)
      : FlawManager(configData), m_ceListener(NULL), m_dbListener(NULL){

      checkError(strcmp(configData.Value(), "VariableFlawManager") == 0,
		 "Expected element <VariableFlawManager> but found " << configData.Value());

      // Load all filtering rules
      for (TiXmlElement * child = configData.FirstChildElement(); 
	   child != NULL; 
	   child = child->NextSiblingElement()) {
	debugMsg("VariableFlawManager:VariableFlawManager",
		 "Evaluating configuration element " << child->Value());

	// If we come across a variable heuristic, register the factory.
	if(strcmp(child->Value(), "VariableHandler") == 0){
	  VariableDecisionPointFactoryId factory = static_cast<VariableDecisionPointFactoryId>(Component::AbstractFactory::allocate(*child));
	  m_factories.push_back(factory);
	}
	else { // Must be a variable filter
	  checkError(strcmp(child->Value(), "VariableFilter") == 0,
		     "Expected element <VariableFilter> but found " << child->Value());

	  const char* component = child->Attribute("component");

	  if(component == NULL){ // Allocate default. It will be static.
	    VariableMatchingRuleId rule = (new VariableMatchingRule(*child))->getId();
	    m_staticMatchingRules.push_back(rule);
	  }
	  else{ // Allocate via registered factory
	    VariableMatchingRuleId rule = static_cast<VariableMatchingRuleId>(Component::AbstractFactory::allocate(*child));
	    m_dynamicMatchingRules.push_back(rule);
	  }
	}
      }
    }

    void VariableFlawManager::handleInitialize(){
      m_ceListener = new CeListener(m_db->getConstraintEngine(), *this);
      m_dbListener = new DbListener(m_db, *this);

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

    VariableFlawManager::~VariableFlawManager(){
      if(m_ceListener != NULL)
	delete m_ceListener;

      if(m_dbListener != NULL)
	delete m_dbListener;

      EUROPA::cleanup(m_staticMatchingRules);
      EUROPA::cleanup(m_dynamicMatchingRules);
      EUROPA::cleanup(m_factories);
    }

    bool VariableFlawManager::inScope(const ConstrainedVariableId& var) const {
      checkError(m_db->getConstraintEngine()->constraintConsistent(), "Assumes the database is constraint consistent but it is not.");
      bool result =  (!var->specifiedDomain().isSingleton() && !matches(var, m_staticMatchingRules) && !matches(var, m_dynamicMatchingRules));
      return result;
    }

    bool VariableFlawManager::matches(const ConstrainedVariableId& var,const std::list<VariableMatchingRuleId>& rules)  const{
      LabelStr objectType;
      LabelStr predicate;
      LabelStr varName = var->getName();
      VariableMatchingRule::extractParts(var, objectType, predicate);

      for(std::list<VariableMatchingRuleId>::const_iterator it = rules.begin(); it != rules.end(); ++it){
	VariableMatchingRuleId rule = *it;
	check_error(rule.isValid());
	// Test for a match against the statndard match and the extendable match
	if(rule->matches(varName, objectType, predicate) && rule->matches(var)){
	  debugMsg("VariableFlawManager:matches", "Match found for " << VariableMatchingRule::makeExpression(var) << " with " << rule->getExpression());
	  return true;
	}
	else {
	  debugMsg("VariableFlawManager:matches", "No match for " << VariableMatchingRule::makeExpression(var) << " with " << rule->getExpression());
	}
      }

      return false;
    }

    DecisionPointId VariableFlawManager::next(unsigned int priorityLowerBound,
					      unsigned int& bestPriority){
      checkError(bestPriority > priorityLowerBound, "Should not be calling this otherwise.");
      ConstrainedVariableId flawedVariable;
      bool flawIsGuarded = false;

      debugMsg("VariableFlawManager:next",
	       "Evaluating next decision to work on. Must beat priority of " << bestPriority);

      for(ConstrainedVariableSet::const_iterator it = m_flawCandidates.begin(); it != m_flawCandidates.end(); ++it){
	if(bestPriority <= priorityLowerBound) // Can't do better
	  break;

	ConstrainedVariableId candidate = *it;

	checkError(!variableOfNonActiveToken(candidate),
		   "Expect that " << candidate->toString() << " cannot belong to an inactive or merged token.");

	check_error(!candidate->specifiedDomain().isSingleton(),
		    "Should not be seeing this as a candidate flaw since it is already specified.");

	if(matches(candidate, m_dynamicMatchingRules)){
	  debugMsg("VariableFlawManager:next",
		   candidate->toString() << " is out of dynamic scope.");
	  continue;
	}

	const AbstractDomain& derivedDomain = candidate->derivedDomain();

	if(!derivedDomain.isOpen() && derivedDomain.areBoundsFinite()){
	  unsigned int valueCount = candidate->lastDomain().getSize();

	  // If it is not a best case priority, and not a guard, but we have a guard, then skip it
	  if(valueCount > priorityLowerBound && flawIsGuarded){
	    debugMsg("VariableFlawManager:next",
		     candidate->toString() << " does not beat a guarded variable decision.");
	    continue;
	  }

	  debugMsg("VariableFlawManager:next",
		   candidate->toString() << " has a priority of " << valueCount);

	  if(valueCount < bestPriority){
	    bestPriority = valueCount;
	    flawedVariable = candidate;
	    flawIsGuarded = (m_guardCache.find(candidate) != m_guardCache.end());
	  }
	}

	debugMsg("VariableFlawManager:next",
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
		 "Failed to allocate a decision point for " << flawedVariable->toString());

      return decisionPoint;
    }

    /**
     * @brief Now we conduct a simple match where we select based on first avalaibale.
     */
    DecisionPointId VariableFlawManager::allocateDecisionPoint(const ConstrainedVariableId& flawedVariable){
      static unsigned int sl_counter(0); // Helpful for debugging
      sl_counter++;

      std::list<VariableDecisionPointFactoryId>::const_iterator it = m_factories.begin();
      LabelStr varName(flawedVariable->getName());
      LabelStr objectType, predicate;
      VariableMatchingRule::extractParts(flawedVariable, objectType, predicate);

      DecisionPointId result;

      while(it != m_factories.end()){
	VariableDecisionPointFactoryId factory = *it;
	if(factory->matches(varName, objectType, predicate) && factory->matches(flawedVariable)){
	  result = factory->create(m_db->getClient(), flawedVariable);
	  break;
	}
	++it;
      }

      checkError(result.isValid(),
		 "At count " << sl_counter << ": No Decision Point could be allocated for " 
		 << flawedVariable->toString());

      return result;
    }

    /**
     * We may filter based on static information only.
     */
    void VariableFlawManager::addFlaw(const ConstrainedVariableId& var){
      if(!var->specifiedDomain().isSingleton() && !matches(var, m_staticMatchingRules)){
	debugMsg("VariableFlawManager:addFlaw",
		 "Adding " << var->toString() << " as a candidate flaw.");
	m_flawCandidates.insert(var);
      }
    }

    void VariableFlawManager::removeFlaw(const ConstrainedVariableId& var){
      condDebugMsg(m_flawCandidates.find(var) != m_flawCandidates.end(), "VariableFlawManager:removeFlaw", "Removing " << var->toString() << " as a flaw.");
      m_flawCandidates.erase(var);
    }

    void VariableFlawManager::addGuard(const ConstrainedVariableId& var){
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

      debugMsg("VariableFlawManager:addGuard", 
	       "GUARDS=" << refCount << " for " << var->getName().toString() << "(" << var->getKey() << ")");
    }

    void VariableFlawManager::removeGuard(const ConstrainedVariableId& var){
      std::map<ConstrainedVariableId, unsigned int>::iterator it = m_guardCache.find(var);
      check_error(it != m_guardCache.end(), "Cannot see how guard would not be here so force it to be.");

      unsigned int refCount = 0;
      if(it->second == 1)
	m_guardCache.erase(it);
      else {
	refCount = it->second - 1;
	it->second = refCount;
      }

      debugMsg("VariableFlawManager:removeGuard", 
	       "GUARDS=" << refCount << " for " << var->getName().toString() << "(" << var->getKey() << ")");
    }

    void VariableFlawManager::handleConstraintAddition(const ConstraintId& constraint){
      if(constraint->getName() == RuleVariableListener::CONSTRAINT_NAME()){
	const std::vector<ConstrainedVariableId>& scope = constraint->getScope();
	for(std::vector<ConstrainedVariableId>::const_iterator it = scope.begin(); it != scope.end(); ++it){
	  ConstrainedVariableId guard = *it;
	  addGuard(guard);
	}
      }
    }

    void VariableFlawManager::handleConstraintRemoval(const ConstraintId& constraint){
      if(constraint->getName() == RuleVariableListener::CONSTRAINT_NAME()){
	const std::vector<ConstrainedVariableId>& scope = constraint->getScope();
	for(std::vector<ConstrainedVariableId>::const_iterator it = scope.begin(); it != scope.end(); ++it){
	  ConstrainedVariableId guard = *it;
	  removeGuard(guard);
	}
      }
    }

    bool VariableFlawManager::variableOfNonActiveToken(const ConstrainedVariableId& var){
      // If var parent is a token and the state is not active, then true.
      if(TokenId::convertable(var->getParent())){
	TokenId token(var->getParent());
	return !token->isActive();
      }

      // Otherwise false
      return false;
    }

    VariableFlawManager::CeListener::CeListener(const ConstraintEngineId& ce,
						    VariableFlawManager& dm)
      : ConstraintEngineListener(ce), m_fm(dm){}

    void VariableFlawManager::CeListener::notifyRemoved(const ConstrainedVariableId& variable){
      m_fm.removeFlaw(variable);
    }

    void VariableFlawManager::CeListener::notifyChanged(const ConstrainedVariableId& variable, 
							    const DomainListener::ChangeType& changeType){
      if(changeType == DomainListener::SET_TO_SINGLETON)
	m_fm.removeFlaw(variable);
      else if(changeType == DomainListener::RESET || changeType == DomainListener::CLOSED)
	m_fm.addFlaw(variable);
    }

    void VariableFlawManager::CeListener::notifyAdded(const ConstraintId& constraint){
      m_fm.handleConstraintAddition(constraint);
    }

    void VariableFlawManager::CeListener::notifyRemoved(const ConstraintId& constraint){
      m_fm.handleConstraintRemoval(constraint);
    }

    VariableFlawManager::DbListener::DbListener(const PlanDatabaseId& db,
						    VariableFlawManager& dm)
      : PlanDatabaseListener(db), m_fm(dm){}

    void VariableFlawManager::DbListener::notifyActivated(const TokenId& token){
      const std::vector<ConstrainedVariableId>& variables = token->getVariables();
      for(std::vector<ConstrainedVariableId>::const_iterator it = variables.begin(); it != variables.end(); ++it){
	ConstrainedVariableId var = *it;
	m_fm.addFlaw(var);
      }
    }

    void VariableFlawManager::DbListener::notifyDeactivated(const TokenId& token){
      const std::vector<ConstrainedVariableId>& variables = token->getVariables();
      for(std::vector<ConstrainedVariableId>::const_iterator it = variables.begin(); it != variables.end(); ++it){
	ConstrainedVariableId var = *it;
	m_fm.removeFlaw(var);
      }
    }
  }
}
