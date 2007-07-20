#include "ConstraintEngine.hh"
#include "VariableChangeListener.hh"
#include "ConstraintEngineListener.hh"
#include "Propagator.hh"
#include "ConstrainedVariable.hh"
#include "Constraint.hh"
#include "AbstractDomain.hh"
#include "Utils.hh"
#include "Debug.hh"
#include "DomainListener.hh"

#include <string>
#include <iterator>

namespace EUROPA
{

	class ViolationMgrImpl : public ViolationMgr
	{
	public:
		ViolationMgrImpl(unsigned int maxViolationsAllowed);
		virtual ~ViolationMgrImpl();
	
		virtual unsigned int getMaxViolationsAllowed();
		virtual void setMaxViolationsAllowed(unsigned int i);
	
		virtual double getViolation() const;
		virtual std::string getViolationExpl() const;
	
		virtual bool handleEmpty(ConstrainedVariableId v);
		virtual bool handleRelax(ConstrainedVariableId v);
		virtual bool canContinuePropagation();
	
		virtual bool isViolated(ConstraintId c) const;
		
		virtual void addViolatedConstraint(ConstraintId c);
		virtual void removeViolatedConstraint(ConstraintId c);
	
	protected:
		unsigned int m_maxViolationsAllowed;
		ConstraintSet m_violatedConstraints;
	};  
	
	ViolationMgrImpl::ViolationMgrImpl(unsigned int maxViolationsAllowed)
	: m_maxViolationsAllowed(maxViolationsAllowed) 
	{
	}
	
	ViolationMgrImpl::~ViolationMgrImpl() 
	{
	}  
	
	unsigned int ViolationMgrImpl::getMaxViolationsAllowed() 
	{ 
		return m_maxViolationsAllowed; 
	}
	
	void ViolationMgrImpl::setMaxViolationsAllowed(unsigned int i)
	{
		check_runtime_error(m_violatedConstraints.size() == 0, "Can only call ViolationMgrImpl::setMaxViolationsAllowed if there are no violations");
		m_maxViolationsAllowed = i;
	}
	
	double ViolationMgrImpl::getViolation() const 
	{ 
		double total = 0.0;
		for (ConstraintSet::const_iterator it = m_violatedConstraints.begin(); it != m_violatedConstraints.end(); ++it) {
			ConstraintId c = *it;
			total += c->getViolation();
		}  
	
		return total;
	}
	
	std::string ViolationMgrImpl::getViolationExpl() const 
	{ 
		std::ostringstream os;
	
		for (ConstraintSet::const_iterator it = m_violatedConstraints.begin(); it != m_violatedConstraints.end(); ++it) {
			ConstraintId c = *it;
			os << c->getViolationExpl() << std::endl;
		}  
	
		return os.str();
	}
	
	bool ViolationMgrImpl::isViolated(ConstraintId c) const
	{
		return m_violatedConstraints.find(c) != m_violatedConstraints.end();
	}
	
	bool ViolationMgrImpl::canContinuePropagation()
	{
		return m_violatedConstraints.size() < m_maxViolationsAllowed;
	} 
	
	void ViolationMgrImpl::addViolatedConstraint(ConstraintId c)
	{
		debugMsg("ConstraintEngine:ViolationMgr", "Marking constraint as violated : " << c->toString());
		c->deactivate(); // Deactivate will cause propagators to ignore constraint, including removing from current agendas  	
		m_violatedConstraints.insert(c);		
	}
	
	bool canPropagate(ConstraintId constraint)
	{
		/*
		const std::vector<ConstrainedVariableId>& vars = constraint->getScope();
		for (unsigned int i=0; i<vars.size(); i++) {
			if (vars[i]->derivedDomain().isEmpty())
			    return false;
		}
		 */
	
		return true;  	
	}
	
	void ViolationMgrImpl::removeViolatedConstraint(ConstraintId c)
	{
		if (canPropagate(c)) {
			check_error(isViolated(c),"Tried to remove constraint that is not violated "+c->toString());
			debugMsg("ConstraintEngine:ViolationMgr", "Removing constraint from violated set : " << c->toString());
			c->undoDeactivation(); // This will put the constraint back on the Propagators' agendas
			m_violatedConstraints.erase(c);
		}		
	}

	bool ViolationMgrImpl::handleEmpty(ConstrainedVariableId v) 
	{
		if (m_violatedConstraints.size() >= m_maxViolationsAllowed)
			return false;
	
		ConstraintId c = v->getCurrentPropagatingConstraint();
	
		// if c is noId, this was caused directly by an specify, just relax the var and the next time around we'll catch the constraint
		if (c != ConstraintId::noId()) 
			addViolatedConstraint(c);		
	
		return true;
	}
	
	bool ViolationMgrImpl::handleRelax(ConstrainedVariableId v) 
	{
		if (m_violatedConstraints.size() == 0)
			return true;
	
		// restore all constraints attached to v to propagation
		// as long as all the vars in the constraint are now non-empty
		std::set<ConstraintId> constraints;
		v->constraints(constraints);
		for (std::set<ConstraintId>::iterator it = constraints.begin(); it != constraints.end(); ++it) {
			ConstraintId c = *it;
			if (isViolated(c)) 
				removeViolatedConstraint(c);
		}
	
		return true;
	}  	    
	
  static bool allActiveVariables(const std::vector<ConstrainedVariableId>& vars){
    for (std::vector<ConstrainedVariableId>::const_iterator it = vars.begin(); it != vars.end(); ++it){
      ConstrainedVariableId var = *it;
      check_error(var.isValid());
      condDebugMsg(!var->isActive(), "allActiveVariables",
		   var->toString() << " is not active but it participates in an active constraint.");

      if(!var->isActive())
	return false;
    }

    return true;
  }

  std::string DomainListener::toString(const ChangeType& changeType){
    switch (changeType) {
    case DomainListener::UPPER_BOUND_DECREASED:
      return "UPPER_BOUND_DECREASED";
    case DomainListener::LOWER_BOUND_INCREASED:
      return "LOWER_BOUND_INCREASED";
    case DomainListener::BOUNDS_RESTRICTED:
      return "BOUNDS_RESTRICTED";
    case DomainListener::VALUE_REMOVED:
      return "VALUE_REMOVED";
    case DomainListener::RESTRICT_TO_SINGLETON:
      return "RESTRICT_TO_SINGLETON";
    case DomainListener::SET_TO_SINGLETON:
      return "SET_TO_SINGLETON";
    case DomainListener::RESET:
      return "RESET";
    case DomainListener::RELAXED:
      return "RELAXED";
    case DomainListener::CLOSED:
      return "CLOSED";
    case DomainListener::OPENED:
      return "OPENED";
    case DomainListener::EMPTIED:
      return "EMPTIED";
    case DomainListener::LAST_CHANGE_TYPE:
      break;
    }
    return "ERROR";
  }

#define notifyMsg(Event, Source) \
    condDebugMsg(changeType == DomainListener::Event,\
		 "ConstraintEngine:notify", \
		 Source->getName().toString() << " (" << Source->getKey() << ") " <<\
		 DomainListener::toString(changeType) << " => " << Source->lastDomain());

#define  publish(message){\
    check_error(!Entity::isPurging());\
    m_dirty = true; \
    for(std::set<ConstraintEngineListenerId>::const_iterator lit = m_listeners.begin(); lit != m_listeners.end(); ++lit)\
      (*lit)->message;\
  }

  /** DEFINE CONSTANTS DECLARED IN COMMON DEFS **/
  DEFINE_GLOBAL_CONST(char*, g_noVarName, "NO_NAME");

  DomainListenerId ConstraintEngine::allocateVariableListener(const ConstrainedVariableId& variable,
							      const ConstraintList& constraints) const{
    return ((new VariableChangeListener(variable, m_id))->getId());
  }

  ConstraintEngine::ConstraintEngine() 
    : m_id(this)
    , m_relaxed(false)
    , m_propInProgress(false)
    , m_cycleCount(1)
    , m_mostRecentRepropagation(1)
    , m_deleted(false)
    , m_purged(false)
    , m_dirty(false)    
    , m_relaxingViolation(false)
  {
  	m_violationMgr = new ViolationMgrImpl(0);
  }

  ConstraintEngine::~ConstraintEngine(){
    m_deleted = true;

    if(!m_purged)
      purge();

    cleanup(m_listeners);

    m_id.remove();
    
    delete m_violationMgr;
  }

  const ConstraintEngineId& ConstraintEngine::getId() const {
    return(m_id);
  }

  void ConstraintEngine::purge() {
    m_purged = true;
    // Iteratively delete constraints. Note that each deletion will update the set
    // through notification of removal.
    check_error(Entity::isPurging() || m_constraints.empty());
    while(!m_constraints.empty()){
      ConstraintId constraint = * m_constraints.begin();
      check_error(constraint.isValid());
      constraint->discard();
    }

    // Iteratively delete variables. Note that each deletion will update the set
    // through notification of removal.
    check_error(Entity::isPurging() || m_variables.empty());
    while(!m_variables.empty()){
      ConstrainedVariableId var = * m_variables.begin();
      check_error(var.isValid());
      var->discard();
    }

    // Always delete the propagators when we purge
    for(std::list<PropagatorId>::const_iterator it = m_propagators.begin(); it != m_propagators.end(); ++it){
      PropagatorId prop = *it;
      check_error(prop.isValid());
      prop->decRefCount();
    }
  }

  bool ConstraintEngine::provenInconsistent() const {
    return(hasEmptyVariable());
  }

  bool ConstraintEngine::constraintConsistent() const {
    if (provenInconsistent())
      return(false);
    return(getNextPropagator().isNoId());
  }

  bool ConstraintEngine::pending() const {
    // Quick check for cases where there is nothing to do for sure
    if(m_dirty == false) 
      return false;

    return (m_relaxed || (!provenInconsistent() && !constraintConsistent()));
  }

  bool ConstraintEngine::isPropagating() const {
    return m_propInProgress;
  }

  void ConstraintEngine::add(const ConstrainedVariableId& variable){
    check_error(m_variables.find(variable) == m_variables.end());
    m_variables.insert(variable);
    publish(notifyAdded(variable));

    debugMsg("ConstraintEngine:add:ConstrainedVariable",
	     variable->getName().toString() << "(" << variable->getKey() <<  ")");
  }

  void ConstraintEngine::remove(const ConstrainedVariableId& variable){
    check_error(!m_propInProgress); /*!< Prohibit relaxations during propagation */
    check_error(m_variables.find(variable) != m_variables.end());
    m_variables.erase(variable);

    if(Entity::isPurging())
      return;

    if(m_emptied == variable)
      clearEmptiedVariable();

    publish(notifyRemoved(variable));

    debugMsg("ConstraintEngine:remove:ConstrainedVariable",
	     variable->getName().toString() << "(" << variable->getKey() <<  ")");
  }

  void ConstraintEngine::add(const ConstraintId& constraint, const LabelStr& propagatorName){
    check_error(m_constraints.find(constraint) == m_constraints.end(),
		"Attempted to add constraint " + constraint->getName().toString() + " but it already exists.");
    check_error(m_propagatorsByName.find(propagatorName.getKey()) != m_propagatorsByName.end(),
		"Propagator " + propagatorName.toString() + " has not been registered.");

    m_constraints.insert(constraint);

    // If constraint initially redundant, then store it.
    if(constraint->isRedundant())
      m_redundantConstraints.insert(constraint);

    PropagatorId propagator = m_propagatorsByName.find(propagatorName.getKey())->second;
    propagator->addConstraint(constraint);
    constraint->setPropagator(propagator);
    publish(notifyAdded(constraint));

    debugMsg("ConstraintEngine:add:constraint:Propagator",
	     constraint->getName().toString() << "(" << constraint->getKey() <<  ") added to " << propagatorName.toString());
  }

  /**
   * Process removals even if purging, since compound constraints will cause multiple
   * deletions and so we have to synchronize the set.
   */
  void ConstraintEngine::remove(const ConstraintId& constraint){
    check_error(m_constraints.find(constraint) != m_constraints.end());
    check_error(!m_propInProgress); /*!< Prohibit relaxations during propagation */
    m_constraints.erase(constraint);
    m_redundantConstraints.erase(constraint);

    if(Entity::isPurging())
      return;

    constraint->getPropagator()->removeConstraint(constraint);

    // If the constraint is inactive, there is no need to relax. So just worry if it is actually active
    if(constraint->isActive()){

      const std::vector<ConstrainedVariableId>& scope = constraint->getScope();
      for(std::vector<ConstrainedVariableId>::const_iterator it = scope.begin(); it != scope.end(); ++it){
	ConstrainedVariableId id(*it);
	if(!id->isDiscarded() && id->lastRelaxed() < m_cycleCount)
	  id->relax();
      }
    }

    publish(notifyRemoved(constraint));

    debugMsg("ConstraintEngine:remove:Constraint",
	     constraint->getName().toString() << "(" << constraint->getKey() <<  ")");
  }

  void ConstraintEngine::add(const PropagatorId& propagator){
    check_error(propagator.isValid());
    check_error(m_propagatorsByName.find(propagator->getName().getKey()) == m_propagatorsByName.end());
    m_propagators.push_back(propagator);
    m_propagatorsByName.insert(std::pair<double, PropagatorId>(propagator->getName().getKey(), propagator));

    debugMsg("ConstraintEngine:add:Propagator",  propagator->getName().toString());
  }


  void ConstraintEngine::add(const ConstraintEngineListenerId& listener){
    check_error(!Entity::isPurging());
    check_error(listener.isValid());
    check_error(m_listeners.count(listener) == 0);
    m_listeners.insert(listener);
  }

  void ConstraintEngine::remove(const ConstraintEngineListenerId& listener){
    check_error(m_listeners.count(listener) == 1);
    if(!m_deleted)
      m_listeners.erase(listener);
  }

  void ConstraintEngine::notifyDeactivated(const ConstraintId& deactivatedConstraint){
    check_error(!Entity::isPurging());
    check_error(deactivatedConstraint.isValid() && !deactivatedConstraint->isActive());
    deactivatedConstraint->getPropagator()->handleConstraintDeactivated(deactivatedConstraint);
    publish(notifyDeactivated(deactivatedConstraint));

    debugMsg("ConstraintEngine:notifyDeactivated:Constraint",  
	     deactivatedConstraint->getName().toString() << "(" << deactivatedConstraint->getKey() << ")");
  }

  void ConstraintEngine::notifyActivated(const ConstraintId& constraint){
    check_error(!Entity::isPurging());
    check_error(constraint.isValid() && constraint->isActive());
    constraint->getPropagator()->handleConstraintActivated(constraint);
    publish(notifyActivated(constraint));

    condDebugMsg(constraint->isRedundant(), "ConstraintEngine:notifyRedundant:Constraint",  
	     constraint->getName().toString() << "(" << constraint->getKey() << ")");

    condDebugMsg(!constraint->isRedundant(), "ConstraintEngine:notifyActivated:Constraint",  
	     constraint->getName().toString() << "(" << constraint->getKey() << ")");
  }

  void ConstraintEngine::notifyRedundant(const ConstraintId& redundantConstraint){
    debugMsg("ConstraintEngine:notifyRedundant", redundantConstraint->toString());

    // If active, want to push it on the agenda for propagation
    if(redundantConstraint->isActive()){
      m_redundantConstraints.insert(redundantConstraint);
      notifyActivated(redundantConstraint); // Make sure it gets propagated
    }
  }

  void ConstraintEngine::notifyDeactivated(const ConstrainedVariableId& var){
    check_error(!Entity::isPurging());
    check_error(var.isValid() && !var->isActive());

    for(std::list<PropagatorId>::const_iterator it = m_propagators.begin(); it != m_propagators.end(); ++it){
      PropagatorId propagator = *it;
      propagator->handleVariableDeactivated(var);
    }

    publish(notifyDeactivated(var));

    debugMsg("ConstraintEngine:notifyDeactivated:ConstrainedVariable",  
	     var->getName().toString() << "(" << var->getKey() << ")");
  }

  void ConstraintEngine::notifyActivated(const ConstrainedVariableId& var){
    check_error(!Entity::isPurging());
    check_error(var.isValid() && var->isActive());

    for(std::list<PropagatorId>::const_iterator it = m_propagators.begin(); it != m_propagators.end(); ++it){
      PropagatorId propagator = *it;
      propagator->handleVariableActivated(var);
    }

    publish(notifyActivated(var));

    debugMsg("ConstraintEngine:notifyActivated:ConstrainedVariable",  
	     var->getName().toString() << "(" << var->getKey() << ")");
  }
  
  std::string ConstraintEngine::dumpPropagatorState(const std::list<PropagatorId>& propagators) const
  {
  	std::ostringstream os;
  	
  	os << std::endl;
    for(std::list<PropagatorId>::const_iterator it = propagators.begin(); it != propagators.end(); ++it){
      PropagatorId propagator = *it;
      os << propagator->getName().toString() << "(";
      
      const std::set<ConstraintId>& constraints = propagator->getConstraints();
      int i=0;
      for(std::set<ConstraintId>::const_iterator cit = constraints.begin(); cit != constraints.end(); ++cit){
          if (i>0)
            os << ",";
          i++;
          os << (*cit)->getName().toString();        	
      }
      os << ")";
      
      os << " enabled=" << propagator->isEnabled() 
         << " updateRequired=" << propagator->updateRequired()
         << std::endl; 
    }
  	os << std::endl;
  	
  	return os.str();
  }
  
  bool ConstraintEngine::doPropagate()
  {
    check_error(!Entity::isPurging(), "Cannot propagate the network when purging. Messages are not sent or processed.");
    check_error(!m_propInProgress, "Attempted to propagate while in propagation. Not allowed to call this cyclically.");

    // Allow for a quick escape if no work to be done
    if(!m_dirty) {
      debugMsg("ConstraintEngine:propagate", "Nothing to do, bailing out");
      debugMsg("ConstraintEngine:propagate", dumpPropagatorState(m_propagators));
      return true;
    }

    // If we have an empty domain, then we sould relax it.
    if(hasEmptyVariable())
      m_emptied->relax();

    // If we still have an empty variable, which we might in special cases dealing with empty
    // base or derived domains. then simply return false.
    if(hasEmptyVariable())
      return false;

    m_relaxed = false; // Reset by default
    m_propInProgress = true;
    bool started = false;
    incrementCycle();

    // Now we have the main propagation loop. 
    PropagatorId activePropagator = getNextPropagator();
    while(!activePropagator.isNoId() && !provenInconsistent()){
      // Publish this event first time around only
      if (!started){
        started = true;
	    publish(notifyPropagationCommenced());
	    debugMsg("ConstraintEngine:propagate", "Commenced");
      }

      debugMsg("ConstraintEngine:propagate", 
	       "Executing " << activePropagator->getName().toString() << " propagator.");

      activePropagator->execute();
      activePropagator = getNextPropagator();
    }
    
    incrementCycle();
    check_error(!pending());

    m_propInProgress = false;
    
    bool result = constraintConsistent();
    if (result && started){
      publish(notifyPropagationCompleted());
      m_dirty = false;

      // Clean up now that we are in a quiescent state
      processRedundantConstraints(); 
      Entity::garbageCollect();
      checkError(constraintConsistent(), 
		 "Must be a bug in cleaning up constraints since we have regressed through garbage collection." <<
		 " See discard and handleDiscard.");

      debugMsg("ConstraintEngine:propagate", "Completed");
    }
    else  if (!result && started){
      publish(notifyPropagationPreempted());
      debugMsg("ConstraintEngine:propagate", "Preempted");
    }
    else {
      debugMsg("ConstraintEngine:propagate", "Completed without doing any work");
    }

    return (result);
  }

  bool ConstraintEngine::canContinuePropagation() const
  {
  	return m_violationMgr->canContinuePropagation();
  }
  
  bool ConstraintEngine::propagate(){
  	
  	bool result = true;
  	
  	bool done = false;  	
  	while (!done) {
  	  result = doPropagate();	
      if (!result && canContinuePropagation()) {
      	  m_relaxingViolation = true;
          m_emptied->relax();
      	  m_relaxingViolation = false;
      }
      else {
      	done = true;
      }
  	}
  	
    return result;  
  }

  void ConstraintEngine::notify(const ConstrainedVariableId& source, const DomainListener::ChangeType& changeType){
    check_error(!Entity::isPurging());
    check_error(source.isValid());

    m_dirty = true;

    // If variable is inavtice, no impact.
    if(!source->isActive())
      return;

    if(changeType == DomainListener::EMPTIED)
      handleEmpty(source);
    else if (changeType == DomainListener::RELAXED ||
	     changeType == DomainListener::OPENED)
      handleRelax(source);

    // In all cases, notify the propagators as well, unless over-ruled by by an empty variable or a decision to ignore it
    for(ConstraintList::const_iterator it = source->m_constraints.begin(); it != source->m_constraints.end(); ++it){
      const ConstraintId& constraint = it->first;
      unsigned int argIndex = it->second;
      if(constraint->isActive() && !constraint->isDiscarded() &&
	 changeType != DomainListener::EMPTIED && !constraint->canIgnore(source, argIndex, changeType))
	constraint->getPropagator()->handleNotification(source, argIndex, constraint, changeType);
    }

    publish(notifyChanged(source, changeType));

    notifyMsg(UPPER_BOUND_DECREASED, source);
    notifyMsg(LOWER_BOUND_INCREASED, source);
    notifyMsg(BOUNDS_RESTRICTED, source);
    notifyMsg(VALUE_REMOVED, source);
    notifyMsg(RESTRICT_TO_SINGLETON, source);
    notifyMsg(SET_TO_SINGLETON, source);
    notifyMsg(RESET, source);
    notifyMsg(RELAXED, source);
    notifyMsg(CLOSED, source);
    notifyMsg(EMPTIED, source);
  }

  void ConstraintEngine::handleEmpty(const ConstrainedVariableId& variable){
    check_error(variable.isValid());
    check_error(variable->getCurrentDomain().isEmpty());

    m_emptied = variable;
    debugMsg("ConstraintEngine:emptied","Emptied var:" << m_emptied->toString()
        << " parent:" << (m_emptied->getParent().isNoId() ? "NULL" : m_emptied->getParent()->toString()));
    check_error(m_relaxed == false);    
    m_violationMgr->handleEmpty(variable);            
  }

  void ConstraintEngine::handleRelax(const ConstrainedVariableId& variable){
    check_error(variable.isValid());
    check_error(!m_propInProgress); /*!< Prohibit relaxations during propagation */

    debugMsg("ConstraintEngine:relaxed",m_cycleCount << "-relaxing var(lastRelaxed=" << variable->lastRelaxed() << "):" << variable->toString()
        << " parent:" << (variable->getParent().isNoId() ? "NULL" : variable->getParent()->toString()));

    if (!m_relaxingViolation)    
        m_violationMgr->handleRelax(variable);

    // If we are not currently in a relaxed state then this is the initiating
    // variable - must have come from a user! Note that it cannot be the emptied variable.
    if(!m_relaxed){
        m_relaxed = true;
        incrementCycle();
        variable->updateLastRelaxed(m_cycleCount);
    }
    else if(variable->lastRelaxed() < m_cycleCount)
        variable->updateLastRelaxed(m_cycleCount);

    // Now this should be true for all
    check_error(variable->lastRelaxed() == m_cycleCount);

    // If there is an empty variable, clear it.
    if(m_emptied.isId()) {
    	
      // If it is not this variable, relax it
      if(m_emptied != variable)
          m_emptied->relax();

      clearEmptiedVariable();
    }

    // Now relax all the variables of all the constraints and ping the propagator of the constraint
    for(ConstraintList::const_iterator c_it = variable->m_constraints.begin(); c_it != variable->m_constraints.end(); ++c_it){
      const ConstraintId& constraint= c_it->first;
      if(constraint->isActive()){
          debugMsg("ConstraintEngine:relaxed",m_cycleCount << "-relaxing var:" << variable->toString() << " relaxing variables for constraint:" << constraint->toString());
          const std::vector<ConstrainedVariableId>& scope =
	          constraint->getModifiedVariables(variable);
	      for(std::vector<ConstrainedVariableId>::const_iterator v_it = scope.begin(); v_it != scope.end(); ++v_it){
	          const ConstrainedVariableId& id = *v_it;
	          if(id->lastRelaxed() < m_cycleCount){
                  debugMsg("ConstraintEngine:relaxed",m_cycleCount << "-relaxing var:" << variable->toString() 
                                                                   << " relaxing connected variable(lastRelaxed=" << id->lastRelaxed() << "):" << id->toString());
	              id->updateLastRelaxed(m_cycleCount);
	              id->relax();
	          }
	      }
      }
    }

    debugMsg("ConstraintEngine:relaxed",m_cycleCount << "-relaxed var:" << variable->toString()
        << " parent:" << (variable->getParent().isNoId() ? "NULL" : variable->getParent()->toString()));        
  }

  void ConstraintEngine::execute(const ConstraintId& constraint){
    check_error(!provenInconsistent() && constraint.isValid());
    check_error(constraint->isActive());
    check_error(constraint->isValid());
    check_error(m_propInProgress);
    checkError(allActiveVariables(constraint->getScope()), 
	       "All variables of an active constraint must also be active. Clients must ensure this happens. " <<
	       constraint->toString());

    publish(notifyExecuted(constraint));

    debugMsg("ConstraintEngine:execute", "BEFORE " << constraint->toString());
    constraint->execute();
    debugMsg("ConstraintEngine:execute", "AFTER " << constraint->toString());
  }

  void ConstraintEngine::execute(const ConstraintId& constraint,
				 const ConstrainedVariableId& variable, 
				 int argIndex,
				 const DomainListener::ChangeType& changeType){
    check_error(!provenInconsistent() && constraint.isValid());
    check_error(constraint->isActive());
    check_error(variable->isActive());
    check_error(constraint->isValid());
    check_error(m_propInProgress);
    publish(notifyExecuted(constraint));
    debugMsg("ConstraintEngine:execute", constraint->getName().toString() << "(" << constraint->getKey() << ")");
    constraint->execute(variable, argIndex, changeType);
  }

  void ConstraintEngine::incrementCycle(){
    m_cycleCount++;

    if(m_relaxed){
      checkError(!m_propInProgress, "Cannot imagine how we could be propagating while relaxed!.");
      m_mostRecentRepropagation = m_cycleCount;
    }
  }

  PropagatorId ConstraintEngine::getNextPropagator() const{
    for(std::list<PropagatorId>::const_iterator it = m_propagators.begin(); it != m_propagators.end(); ++it){
      PropagatorId propagator = *it;
      if(propagator->isEnabled() && propagator->updateRequired())
	return propagator;
    }

    return PropagatorId::noId();
  }

  unsigned int ConstraintEngine::mostRecentRepropagation() const{
    return m_mostRecentRepropagation;
  }

  const ConstrainedVariableSet& ConstraintEngine::getVariables() const {return m_variables;}

  /**
   * The naive implementation will just scan through the set with an iterator. However,
   * we can always add a vector for faster replay, and populate it as long as we have not backtracked,
   * or some other trick.
   * @see getIndex
   */
  ConstrainedVariableId ConstraintEngine::getVariable(unsigned int index){
    check_error(index < m_variables.size());
    unsigned int i = 0;
    for(ConstrainedVariableSet::const_iterator it = m_variables.begin(); it != m_variables.end(); ++it){
      if(i == index){
	ConstrainedVariableId var = *it;
	check_error(var.isValid());
	return var;
      }
      i++;
    }
    return ConstrainedVariableId::noId();
  }

  /**
   * The naive implementation will just scan through the set with an iterator. Will have to find a faster way.
   * @see getVariable
   */
  unsigned int ConstraintEngine::getIndex(const ConstrainedVariableId& var){
    check_error(var.isValid());
    unsigned int index = 0;
    for(ConstrainedVariableSet::const_iterator it = m_variables.begin(); it != m_variables.end(); ++it){
      ConstrainedVariableId candidate = *it;
      check_error(candidate.isValid());
      if(candidate == var)
	return index;
      index++;
    }
    check_error(ALWAYS_FAILS);
    return 0;
  }

  const ConstraintSet& ConstraintEngine::getConstraints() const {return m_constraints;}

  ConstraintId ConstraintEngine::getConstraint(unsigned int index) {
    check_error(index < m_constraints.size());
    ConstraintSet::iterator it = m_constraints.begin();
    std::advance(it, index);
    check_error(it->isValid());
    return *it;
  }

  unsigned int ConstraintEngine::getIndex(const ConstraintId& constr) {
    check_error(constr.isValid());
    ConstraintSet::iterator it = m_constraints.find(constr);
    check_error(it != m_constraints.end());
    check_error(it->isValid());
    return (unsigned int) std::distance(m_constraints.begin(), it);
  }

  const PropagatorId& ConstraintEngine::getPropagatorByName(const LabelStr& name)  const {
    if (m_propagatorsByName.find(name.getKey()) == m_propagatorsByName.end())
      return PropagatorId::noId();
    else
      return(m_propagatorsByName.find(name.getKey())->second);
  }

  /**
   * @brief Process redundant constraints
   */
  void ConstraintEngine::processRedundantConstraints(){
    checkError(constraintConsistent(), "Can only process when fully propagated.");
    for(ConstraintSet::const_iterator it = m_redundantConstraints.begin();
	it != m_redundantConstraints.end();
	++it){
      ConstraintId constraint = *it;
      checkError(constraint.isValid(), constraint);
      checkError(constraint->isRedundant(), "Must be redundant. Buffering error.");
      constraint->deactivate();
      debugMsg("ConstraintEngine:processRedundantConstraints", constraint->toString());
    }

    m_redundantConstraints.clear();
  }
  
  bool ConstraintEngine::getAllowViolations() const
  {
  	return m_violationMgr->getMaxViolationsAllowed() > 0;
  }
  
  void ConstraintEngine::setAllowViolations(bool v)
  {
  	if (v) 
  		m_violationMgr->setMaxViolationsAllowed((unsigned int)INT_MAX);
    else
  		m_violationMgr->setMaxViolationsAllowed(0);
  }
    
  double ConstraintEngine::getViolation() const
  {
  	return m_violationMgr->getViolation();
  }
  
  std::string ConstraintEngine::getViolationExpl() const
  {
  	return m_violationMgr->getViolationExpl();
  }
  
  bool ConstraintEngine::isViolated(ConstraintId c) const
  {
  	return m_violationMgr->isViolated(c);
  }  
}
