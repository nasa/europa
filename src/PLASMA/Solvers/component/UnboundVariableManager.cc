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
	updateFlaw(var);
      }

      // PROCESS CONSTRAINTS TO INITIALIZE GUARDS. We are looking for RuleVariableListener constraints since they 
      // determine if a variable is guarded or not.
      const ConstraintSet& allConstraints = m_db->getConstraintEngine()->getConstraints();
      for(ConstraintSet::const_iterator it = allConstraints.begin(); it != allConstraints.end(); ++it){ 
	ConstraintId constraint = *it;
	handleConstraintAddition(constraint);
      }
    }

    DecisionPointId UnboundVariableManager::nextZeroCommitmentDecision(){
      for(ConstrainedVariableSet::const_iterator it = m_singletonFlawCandidates.begin(); 
          it != m_singletonFlawCandidates.end(); ++it){
        ConstrainedVariableId var = *it;
        checkError(var.isValid(), var);
        checkError(var->lastDomain().isSingleton(), "Buffer management error:" << var->toLongString());
        
        if(!dynamicMatch(var)){
          debugMsg("UnboundVariableManager:nextZeroCommitmentDecision", "Allocating for " << var->toLongString());
          return allocateDecisionPoint(var, "unit");
        }
      }
      
      return DecisionPointId::noId();
    }

    /**
     * Filter out if not a variable
     */
    bool UnboundVariableManager::staticMatch(const EntityId& entity){
      return !ConstrainedVariableId::convertable(entity) || FlawManager::staticMatch(entity);
    }

    bool UnboundVariableManager::dynamicMatch(const EntityId& entity){
      if (FlawManager::dynamicMatch(entity))
	return true;

      ConstrainedVariableId var = entity;

      // We also exclude singletons unless they are guards
      if(!isCompatGuard(var) && var->lastDomain().isSingleton())
	return true;

      // Finally, we exlude if the bounds are not finite
      return !var->lastDomain().areBoundsFinite();
    }

    /**
     * We may filter based on static information only.
     */
    void UnboundVariableManager::updateFlaw(const ConstrainedVariableId& var){
      debugMsg("UnboundVariableManager:updateFlaw", var->toLongString());
      m_flawCandidates.erase(var);
      m_singletonFlawCandidates.erase(var);

      if(variableOfNonActiveToken(var) || !var->canBeSpecified() || var->isSpecified() || staticMatch(var)){
        debugMsg("UnboundVariableManager:updateFlaw", "Excluding: " << var->toLongString());
        condDebugMsg(variableOfNonActiveToken(var), "UnboundVariableManager:updateFlaw", "Parent is not active.");
        condDebugMsg(!var->canBeSpecified(), "UnboundVariableManager:updateFlaw", "Variable can't be specified.");
        condDebugMsg(var->isSpecified(), "UnboundVariableManager:updateFlaw", "Variable is already specified.");
        return;
      }

      debugMsg("UnboundVariableManager:addFlaw",
	       "Adding " << var->toLongString() << " as a candidate flaw.");

      m_flawCandidates.insert(var);

      if(var->lastDomain().isSingleton() && isCompatGuard(var))
	m_singletonFlawCandidates.insert(var);
    }

    void UnboundVariableManager::removeFlaw(const ConstrainedVariableId& var){
      condDebugMsg(m_flawCandidates.find(var) != m_flawCandidates.end(), 
		   "UnboundVariableManager:removeFlaw", "Removing " << var->toLongString() << " as a flaw.");

      m_flawCandidates.erase(var);
      m_singletonFlawCandidates.erase(var);
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
	       "GUARDS=" << refCount << " for " << var->toLongString() << "(" << var->getKey() << ")");
      updateFlaw(var);
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
	       "GUARDS=" << refCount << " for " << var->toLongString() << "(" << var->getKey() << ")");

      updateFlaw(var);
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
      if(TokenId::convertable(var->parent())){
	TokenId token(var->parent());
	return !token->isActive();
      }

      // Otherwise false
      return false;
    }


    void UnboundVariableManager::notifyRemoved(const ConstrainedVariableId& variable){
      removeFlaw(variable);
      FlawManager::notifyRemoved(variable);
    }

    void UnboundVariableManager::notifyChanged(const ConstrainedVariableId& variable, 
					       const DomainListener::ChangeType& changeType){

      // In the event it is bound to a singleton, we remove it altogether as a flaw.
      if(changeType == DomainListener::SET_TO_SINGLETON){
	// If it is a token state variable, we test if a case for activation
	if(Token::isStateVariable(variable) && variable->getSpecifiedValue() == Token::ACTIVE){
	  TokenId token = variable->parent();
	  const std::vector<ConstrainedVariableId>& variables = token->getVariables();
	  for(std::vector<ConstrainedVariableId>::const_iterator it = variables.begin(); it != variables.end(); ++it){
	    ConstrainedVariableId var = *it;
	    checkError(var.isValid(), var);
	    updateFlaw(var);
	  }
	}
	else
	  removeFlaw(variable);

	return;
      }

      if(changeType == DomainListener::RESET && Token::isStateVariable(variable)){
	TokenId token = variable->parent();
	const std::vector<ConstrainedVariableId>& variables = token->getVariables();
	for(std::vector<ConstrainedVariableId>::const_iterator it = variables.begin(); it != variables.end(); ++it){
	  ConstrainedVariableId var = *it;
	  removeFlaw(var);
	}
	return;
      }

      // Now listen for all the other events of interest. We can ignore other cases of restriction since
      // the event set below is sufficient to capture all the meaningful changes without incurring
      // all the evaluation costs on every propagation.
      if(changeType == DomainListener::RESET || 
	 changeType == DomainListener::CLOSED ||
	 changeType == DomainListener::RELAXED ||
	 changeType == DomainListener::RESTRICT_TO_SINGLETON)
	updateFlaw(variable);
    }

    void UnboundVariableManager::notifyAdded(const ConstraintId& constraint){
      handleConstraintAddition(constraint);
    }

    void UnboundVariableManager::notifyRemoved(const ConstraintId& constraint){
      handleConstraintRemoval(constraint);
    }

    bool UnboundVariableManager::isCompatGuard(const ConstrainedVariableId& var) const{
      return (m_guardCache.find(var) != m_guardCache.end());
    }

    //reordered FROM guard, key pref TO choice count, guard
    //re-reordered from choice count, guard TO guard, choice count
    bool UnboundVariableManager::betterThan(const EntityId& a, const EntityId& b, LabelStr& explanation){
      //we only ever get here because the priority is equal
      //Added to duplicate behavior from HTX.  This may not be the best idea ever. ~MJI
      const ConstrainedVariableId va = a;
      const ConstrainedVariableId vb = b;
      if(a.isId() && b.isId()) {
        bool aCompat = isCompatGuard(a);
        bool bCompat = isCompatGuard(b);
        if(aCompat && !bCompat) {
          debugMsg("UnboundVariableManager:betterThan", a->getKey() << " is better than " << b->getKey() << " because it is a guard.");
          explanation = "aGuard";
          return true;
        }
        else if(!aCompat && bCompat) {
          debugMsg("UnboundVariableManager:betterThan", b->getKey() << " is better than " << a->getKey() << " because it is a guard.");
          return false;
        }

        if(va->lastDomain().getSize() < vb->lastDomain().getSize()) {
          debugMsg("UnboundVariableManager:betterThan", a->getKey() << " is better than " << b->getKey() << 
                   " because it has " << va->lastDomain().getSize() << " choices, as opposed to " << vb->lastDomain().getSize());
          explanation = "aSmaller";
          return true; //here goes nothin'...
        }
        else if(va->lastDomain().getSize() > vb->lastDomain().getSize()) {
          debugMsg("UnboundVariableManager:betterThan", b->getKey() << " is better than " << a->getKey() << 
                   " because it has " << vb->lastDomain().getSize() << " choices, as opposed to " << va->lastDomain().getSize());
          return false;
        }
      }
      //if a isn't provably better, we return false
      return false;
    }

    std::string UnboundVariableManager::toString(const EntityId& entity) const {
      checkError(ConstrainedVariableId::convertable(entity), entity->toString());
      ConstrainedVariableId var = entity;
      std::string compatStr = (isCompatGuard(var) ? " GUARD" : "");
      std::string unitStr = (var->lastDomain().isSingleton() ? " UNIT" : "");
      std::stringstream os;
      os << "VAR:   " << var->toLongString() << unitStr << compatStr;
      return os.str();
    }

    class UnboundVariableIterator : public FlawIterator {
    public:
      UnboundVariableIterator(UnboundVariableManager& manager)
	: FlawIterator(manager), m_it(manager.m_flawCandidates.begin()), m_end(manager.m_flawCandidates.end()) {
	advance();
      }

    private:
      const EntityId nextCandidate() {
	EntityId candidate;
	if(m_it != m_end){
	  candidate = *m_it;
	  ++m_it;
	}
	return candidate;
      }

      ConstrainedVariableSet::const_iterator m_it;
      ConstrainedVariableSet::const_iterator m_end;
    };


    IteratorId UnboundVariableManager::createIterator() {
      return (new UnboundVariableIterator(*this))->getId();
    }
  }
}
