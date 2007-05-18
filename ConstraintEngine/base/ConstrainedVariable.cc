#include "ConstrainedVariable.hh"
#include "AbstractDomain.hh"
#include "ConstraintEngine.hh"
#include "Constraint.hh"
#include "Utils.hh"
#include "Debug.hh"
#include "Error.hh"
#include <sstream>

namespace EUROPA {

  ConstrainedVariableListener::ConstrainedVariableListener(const ConstrainedVariableId& var)
    : m_id(this), m_var(var) {
    var->notifyAdded(m_id);
  }

  const ConstrainedVariableListenerId& ConstrainedVariableListener::getId() const {
    return(m_id);
  }

  ConstrainedVariableListener::~ConstrainedVariableListener() {
    m_id.remove();
  }

  ConstrainedVariable::ConstrainedVariable(const ConstraintEngineId& constraintEngine,
                                           bool canBeSpecified,
                                           const LabelStr& name,
                                           const EntityId& parent,
                                           int index)
    : Entity(), m_id(this), m_lastRelaxed(0), m_constraintEngine(constraintEngine), m_name(name),
      m_canBeSpecified(canBeSpecified), m_specifiedFlag(false), m_specifiedValue(0),  
      m_index(index), m_parent(parent), m_deactivationRefCount(0), m_deleted(false) {
    check_error(m_constraintEngine.isValid());
    check_error(m_index >= NO_INDEX);
    check_error(m_index == NO_INDEX || parent.isValid());
    m_constraintEngine->add(m_id);
    m_listener = m_constraintEngine->allocateVariableListener(m_id, m_constraints);
  }

  ConstrainedVariable::~ConstrainedVariable() {
    debugMsg("ConstrainedVariable:~ConstrainedVariable", 
	     "NAME=" << getName().toString() << " KEY=" << getKey() << " ID=" << m_id);

    discard(false);

    m_id.remove();
  }

  void ConstrainedVariable::restrictBaseDomain(const AbstractDomain& dom){
    checkError(isActive(), toString());
    checkError(dom.isSubsetOf(baseDomain()), dom.toString() << " not in " << baseDomain().toString());

    debugMsg("ConstrainedVariable:restrictBaseDomain", 
	     toString() << " restricted from " << baseDomain().toString() << " to " << dom.toString());

    handleRestrictBaseDomain(dom);

    if(dom.isSingleton() && !m_specifiedFlag && canBeSpecified())
      specify(dom.getSingletonValue());

    // Iterate over all constraints and notify of this restriction
    for(ConstraintList::const_iterator it = m_constraints.begin(); it != m_constraints.end(); ++it){
      ConstraintId constraint = it->first;
      constraint->notifyBaseDomainRestricted(m_id);
    }
  }

  void ConstrainedVariable::handleDiscard(){
    check_error(m_constraintEngine.isValid());
    m_deleted = true;
    // Remove constraints if they apply and we are not purging
    if(!Entity::isPurging()){
      for (ConstraintList::const_iterator it = m_constraints.begin(); it != m_constraints.end(); ++it){
	ConstraintId constraint = it->first;
	constraint->discard();
      }
    }

    m_constraintEngine->remove(m_id);

    cleanup(m_listeners);

    delete (DomainListener*) m_listener;

    Entity::handleDiscard();
  }

  const ConstrainedVariableId& ConstrainedVariable::getId() const {
    return(m_id);
  }

  int ConstrainedVariable::getIndex() const {
    return(m_index);
  }

  void ConstrainedVariable::deactivate() {
    check_error(!Entity::isPurging());
    m_deactivationRefCount++;

    // Iterate over all constraints and deactivate all. Note, we don't care if they are already active. To support
    // non-chronological updates in the database (retractions) we need to use reference counting and accumulate
    // references so we can locally evaluate all the restrictions on the variable.
    for(ConstraintList::const_iterator it = m_constraints.begin(); it != m_constraints.end(); ++it){
      ConstraintId constraint = it->first;
      constraint->deactivate();
    }

    debugMsg("ConstrainedVariable:deactivation", 
	     "RefCount: [" << m_deactivationRefCount << "]" << toString());

    // If this is a transition, handle it
    if(m_deactivationRefCount == 1)
      m_constraintEngine->notifyDeactivated(m_id);
  }

  void ConstrainedVariable::undoDeactivation() {
    check_error(!Entity::isPurging());

    m_deactivationRefCount--;

    debugMsg("ConstrainedVariable:undoDeactivation", 
	     "RefCount: [" << m_deactivationRefCount << "]" << toString());

    // Iterate over all constraints and undo deactivation request
    for(ConstraintList::const_iterator it = m_constraints.begin(); it != m_constraints.end(); ++it){
      ConstraintId constraint = it->first;
      constraint->undoDeactivation();
    }

    // If this is a transition, handle it
    if(isActive())
      m_constraintEngine->notifyActivated(m_id);
  }

  unsigned int ConstrainedVariable::refCount() const {return m_deactivationRefCount;}

  /* Methods that delegate directly to the the ConstraintEngine */

  bool ConstrainedVariable::provenInconsistent() const {
    return(m_constraintEngine->provenInconsistent());
  }

  bool ConstrainedVariable::constraintConsistent() const {
    return(m_constraintEngine->constraintConsistent());
  }

  bool ConstrainedVariable::pending() const {
    return(m_constraintEngine->pending());
  }

  void ConstrainedVariable::update() {
    m_constraintEngine->propagate();
  }

  const EntityId& ConstrainedVariable::getParent() const {
    return(m_parent);
  }

  const ConstraintEngineId& ConstrainedVariable::getConstraintEngine() const {
    return m_constraintEngine;
  }

  std::string ConstrainedVariable::toString() const{
    std::stringstream sstr;
    sstr << Entity::toString() << (specifiedFlag() ? " (S) " : "") << " DERIVED=" << lastDomain().toString();
    return sstr.str();
  }

  void ConstrainedVariable::addConstraint(const ConstraintId& constraint, int argIndex) {
    check_error(!Entity::isPurging());
    check_error(constraint.isValid());
    debugMsg("ConstrainedVariable:addConstraint", "Adding " << constraint->toString() << " to " << toString());
    m_constraints.push_back(ConstraintEntry(constraint, argIndex));

    // If this variable is inactive, then the constraint should be immediately deactivated
    if(!isActive())
      constraint->deactivate();

    handleConstraintAdded(constraint);
    for(std::set<ConstrainedVariableListenerId>::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
      (*it)->notifyConstraintAdded(constraint, argIndex);

    check_error(isConstrainedBy(constraint));
  }

  void ConstrainedVariable::removeConstraint(const ConstraintId& constraint, int argIndex) {
    if(m_deleted) // Nothing to do
      return;

    check_error(!Entity::isPurging());
    check_error(constraint.isValid());
    check_error(isConstrainedBy(constraint));
    check_error(!Entity::isPurging()); // Should not be getting this message
    m_constraints.remove(ConstraintEntry(constraint, argIndex));

    handleConstraintRemoved(constraint);
    for(std::set<ConstrainedVariableListenerId>::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
      (*it)->notifyConstraintRemoved(constraint, argIndex);
  }

  bool ConstrainedVariable::isSpecified() const { 
    return m_specifiedFlag || (baseDomain().isSingleton() && baseDomain().isClosed());
  }

  double ConstrainedVariable::getSpecifiedValue() const {
    checkError(isSpecified(), toString());
    if(m_specifiedFlag)
      return m_specifiedValue;
    else
      return baseDomain().getSingletonValue();
  }

  bool ConstrainedVariable::isValid() const {
    // Is the Listener correctly set up?
    if (lastDomain().getListener() != m_listener)
      return(false);

    // If the derived domain is not empty, it should be a subset of the specified domain
    if (!lastDomain().isEmpty() && !lastDomain().isSubsetOf(baseDomain()))
      return(false);

    // Relationship with constraints is synchronized
    ConstraintList::const_iterator it = m_constraints.begin();
    for ( ; it != m_constraints.end(); ++it)
      if (!it->first->isVariableOf(m_id))
        return(false);

    /* Relationhip between domains is correct - and delegate to derived class for any extra tests! */
    return validate();
  }

  bool ConstrainedVariable::isConstrainedBy(const ConstraintId& constraint) {
    ConstraintList::const_iterator it = m_constraints.begin();
    for ( ; it !=  m_constraints.end(); ++it)
      if (it->first == constraint)
        return(true);
    return(false);
  }

  void ConstrainedVariable::updateLastRelaxed(int cycleCount) {
    check_error(!Entity::isPurging());
    check_error(m_lastRelaxed < cycleCount);
    m_lastRelaxed = cycleCount;
  }

  int ConstrainedVariable::lastRelaxed() const {
    return(m_lastRelaxed);
  }

  void ConstrainedVariable::constraints(std::set<ConstraintId>& results) const {
    check_error(!Entity::isPurging());
    ConstraintList::const_iterator it = m_constraints.begin();
    for ( ; it != m_constraints.end(); ++it)
      results.insert(it->first);
  }

  unsigned int ConstrainedVariable::constraintCount() const{
    return m_constraints.size();
  }

  const ConstraintId& ConstrainedVariable::getFirstConstraint() const {
    check_error(!Entity::isPurging());
    if (m_constraints.empty())
      return(ConstraintId::noId());
    return(m_constraints.front().first);
  }

  bool ConstrainedVariable::hasActiveConstraint() const{
    for (ConstraintList::const_iterator it = m_constraints.begin() ; it != m_constraints.end(); ++it){
      ConstraintId constraint = it->first;
      if(constraint->isActive()){
	debugMsg("ConstrainedVariable:hasActiveConstraint", constraint->toString());
	return true;
      }
    }

    return false;
  }

  bool ConstrainedVariable::canBeSpecified() const {
    return(m_canBeSpecified);
  }

  bool ConstrainedVariable::isClosed() const {
    return(baseDomain().isClosed());
  }

  const LabelStr& ConstrainedVariable::getName() const {
    return(m_name);
  }

  void ConstrainedVariable::specify(double singletonValue) {
    check_error(canBeSpecified());
    internalSpecify(singletonValue);
  }

  void ConstrainedVariable::internalSpecify(double singletonValue) {
    checkError(baseDomain().isMember(singletonValue), singletonValue << " not in " << baseDomain().toString());
    checkError(isActive(), toString());
    checkError(!m_specifiedFlag || singletonValue == m_specifiedValue,
	       "Cannot specify " << toString() << " to " << singletonValue);

    // If not already specified, then execute it. Must set flag first so the variable has a record of being specified
    // as events are handled when we set the current domain
    if (!m_specifiedFlag){
      m_specifiedFlag = true;
      m_specifiedValue = singletonValue;
      if(getCurrentDomain().isMember(singletonValue))
	getCurrentDomain().set(singletonValue);
      else
	getCurrentDomain().empty();
    }

    if(getCurrentDomain().isOpen())
      getCurrentDomain().close();

    check_error(isValid());
  }

  void ConstrainedVariable::reset() {
    reset(internal_baseDomain());
  }

  void ConstrainedVariable::reset(const AbstractDomain& domain){
    checkError(domain.isSubsetOf(internal_baseDomain()),
				 domain.toString() << " not in " << internal_baseDomain().toString());

    // If the base domain is a singleton, and it is closed, then we can consider the variable continues to be
    // specified so we exit with no change
    if(baseDomain().isSingleton() && baseDomain().isClosed())
      return;
    
    m_specifiedFlag = false;
    getCurrentDomain().reset(domain);
  }

  void ConstrainedVariable::close() {
    checkError(internal_baseDomain().isOpen(), 
	       "Attempted to close a variable but the base domain is already closed.");

    internal_baseDomain().close();

    if(getCurrentDomain().isOpen())
      getCurrentDomain().close();

    if(getCurrentDomain().isEmpty())
      return;
  }

  void ConstrainedVariable::open() {
    check_error(internal_baseDomain().isClosed());
    internal_baseDomain().open();
    if(getCurrentDomain().isClosed())
      getCurrentDomain().open();
    
  }

  void ConstrainedVariable::relax() {
    // If it has been specified, relax to the specified domain
    if(m_specifiedFlag)
      getCurrentDomain().relax(m_specifiedValue);
    else // Rleax to the base domain
      getCurrentDomain().relax(internal_baseDomain());
  }

  void ConstrainedVariable::insert(double value) {
    // the base domain has to be open in order for insertion to occur
    check_error(internal_baseDomain().isOpen(), "Can't insert a member into a variable with a closed base domain.");
    internal_baseDomain().insert(value);

    // Pass on insertion to derived domain if the variable has not yet been specified
    if (!m_specifiedFlag && getCurrentDomain().isOpen())
        getCurrentDomain().insert(value);
  }

  void ConstrainedVariable::remove(double value) {
    // Always remove from base domain
    internal_baseDomain().remove(value);

    // Remove from derived domain
    if (getCurrentDomain().isMember(value))
      getCurrentDomain().remove(value);
  }

  bool ConstrainedVariable::validate() const {
    return(true);
  }

  void ConstrainedVariable::notifyAdded(const ConstrainedVariableListenerId& listener){
    check_error(m_listeners.find(listener) == m_listeners.end());
    m_listeners.insert(listener);
  }

  void ConstrainedVariable::notifyRemoved(const ConstrainedVariableListenerId& listener) {
    check_error(m_listeners.find(listener) != m_listeners.end());
    m_listeners.erase(listener);
  }

  /**
   * @brief Default is just a pass through.
   */
  std::string ConstrainedVariable::toString(double value) const {
    return baseDomain().toString(value);
  }

  bool ConstrainedVariable::specifiedFlag() const {return m_specifiedFlag;}
  
  double ConstrainedVariable::getViolation() const
  {
  	  double total = 0.0;
  	  
  	  for (ConstraintList::const_iterator it = m_constraints.begin() ; it != m_constraints.end(); ++it){
          ConstraintId constraint = it->first;
          total += constraint->getViolation();
  	  }  	
  	  
  	  return total;
  }  
}
