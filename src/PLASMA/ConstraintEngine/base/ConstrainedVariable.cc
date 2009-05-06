#include "ConstrainedVariable.hh"
#include "AbstractDomain.hh"
#include "ConstraintEngine.hh"
#include "Constraint.hh"
#include "Utils.hh"
#include "Debug.hh"
#include "Error.hh"
#include "VariableChangeListener.hh"
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
	  m_var->notifyRemoved(m_id);
	  m_id.remove();
  }

  ConstrainedVariable::ConstrainedVariable(const ConstraintEngineId& constraintEngine,
                                           const bool internal,
                                           bool canBeSpecified,
                                           const LabelStr& name,
                                           const EntityId& parent,
                                           int index)
    : Entity(), m_id(this), m_lastRelaxed(0), m_constraintEngine(constraintEngine), m_name(name),
      m_internal(internal), m_canBeSpecified(canBeSpecified), m_specifiedFlag(false), m_specifiedValue(0),
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

  void ConstrainedVariable::setCurrentPropagatingConstraint(ConstraintId c) { m_propagatingConstraint = c; }
  ConstraintId ConstrainedVariable::getCurrentPropagatingConstraint() const { return m_propagatingConstraint; }

  void ConstrainedVariable::restrictBaseDomain(const AbstractDomain& dom){
    checkError(isActive(), toString());
    checkError(dom.intersects(baseDomain()), dom.toString() << " not intersecting " << baseDomain().toString());

    // If already restricted, nothing to be done
    if(baseDomain().isSubsetOf(dom) && (isClosed() || dom.isOpen()))
      return;

    debugMsg("ConstrainedVariable:restrictBaseDomain",
	     toString() << " restricted from " << baseDomain().toString() << " intersecting " << dom.toString());

    handleRestrictBaseDomain(dom);

    if(dom.isSingleton() && !m_specifiedFlag && canBeSpecified())
      specify(dom.getSingletonValue());

    // Trigger events for propagation of this variable restriction, even if no domain restriction has occured, since it does
    // refelct a status change of a variable and further inference may be possible if we know the base domain
    // has been restricted.
    m_constraintEngine->notify(m_id, DomainListener::BOUNDS_RESTRICTED);

    // Iterate over all constraints and notify of this restriction
    for(ConstraintList::const_iterator it = m_constraints.begin(); it != m_constraints.end(); ++it){
      ConstraintId constraint = it->first;
      constraint->notifyBaseDomainRestricted(m_id);
    }
  }

  void ConstrainedVariable::handleDiscard(){
	  // TODO:  using toString OR toLongString here can break during shutdown, if our variable
	  // points to an object (see #170)
    debugMsg("ConstrainedVariable:handleDiscard", "Discarding " << Entity::toString());

    check_error(m_constraintEngine.isValid());
    m_deleted = true;
    // Remove constraints if they apply and we are not purging
    if(!Entity::isPurging()){
      for (ConstraintList::const_iterator it = m_constraints.begin(); it != m_constraints.end(); ++it){
	ConstraintId constraint = it->first;
	debugMsg("ConstrainedVariable:handleDiscard", "Discarding constraint " << constraint->getKey());
	checkError(constraint.isValid(), constraint);
	constraint->discard();
	checkError(constraint.isValid(), constraint << " should remain valid");
      }
    }

    m_constraintEngine->remove(m_id);

    // Let listeners know the variable is about to be discarded.
    // NOTE:  We don't just iterate through here, because when a listener is notified, it should be deleted, which
    // in turn calls our notifyRemoved method, which removes it from m_listeners (ie. listeners are disappearing out from under us)
    while(!m_listeners.empty())
    {
    	(*m_listeners.begin())->notifyDiscard();
    }

    delete (DomainListener*) m_listener;

    Entity::handleDiscard();
  }

  const ConstrainedVariableId& ConstrainedVariable::getId() const {
    return(m_id);
  }

  const std::string& ConstrainedVariable::getEntityType() const {
 	  static const std::string CV_STR("ConstrainedVariable");
 	  return CV_STR;
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

  const EntityId& ConstrainedVariable::parent() const {
    return m_parent;
  }

  const ConstraintEngineId& ConstrainedVariable::getConstraintEngine() const {
    return m_constraintEngine;
  }

  void ConstrainedVariable::addConstraint(const ConstraintId& constraint, int argIndex) {
    check_error(!Entity::isPurging());
    check_error(constraint.isValid());
    debugMsg("ConstrainedVariable:addConstraint", "Adding " << constraint->toString() << " to " << toString());
    m_constraints.push_back(ConstraintEntry(constraint, argIndex));

    handleConstraintAdded(constraint);
    for(std::set<ConstrainedVariableListenerId>::iterator it = m_listeners.begin(); it != m_listeners.end(); ++it)
      (*it)->notifyConstraintAdded(constraint, argIndex);

    check_error(isConstrainedBy(constraint));
  }

  void ConstrainedVariable::removeConstraint(const ConstraintId& constraint, int argIndex) {
    if(m_deleted) // Nothing to do
      return;

    debugMsg("ConstrainedVariable:removeConstraint", "Unlinking " << constraint->toString());

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
    {
    	return m_specifiedValue;
    }
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
    debugMsg("ConstrainedVariable:updateLastRelaxed",getName().toString() << " lastRelaxed updated to " << m_lastRelaxed);
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

  void ConstrainedVariable::constraints(ConstraintSet& results) const {
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
    debugMsg("ConstrainedVariable:specify", "specifying value:" << toString());
    check_error(canBeSpecified());
    internalSpecify(singletonValue);
    debugMsg("ConstrainedVariable:specify", "specified value:" << toString());
  }

  void ConstrainedVariable::internalSpecify(double singletonValue) {
    debugMsg("ConstrainedVariable:internalSpecify", "specifying value:" << toString());
    checkError(baseDomain().isMember(singletonValue), singletonValue << " not in " << baseDomain().toString());
    checkError(isActive(), toString());

    bool violated = !getCurrentDomain().isMember(singletonValue);
    if(violated && getConstraintEngine()->getAllowViolations()) {
        reset();
        debugMsg("ConstrainedVariable:internalSpecify", "after reset():" << toString());
    }

    // Must set flag first so the variable has a record of being specified
    // as events are handled when we set the current domain
    m_specifiedFlag = true;
    m_specifiedValue = singletonValue;

    violated = !getCurrentDomain().isMember(singletonValue);
    if (violated)
        getCurrentDomain().empty();
    else
        getCurrentDomain().set(singletonValue);

    debugMsg("ConstrainedVariable:internalSpecify", "specified value:" << toString());
    check_error(isValid());
  }

  void ConstrainedVariable::reset() {
    reset(internal_baseDomain());
    // TODO: Turn this on  (it makes a test fail, in the expected way
//    if (getConstraintEngine()->getAutoPropagation())
//       getConstraintEngine()->propagate();
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

  void ConstrainedVariable::touch() {
    getCurrentDomain().touch();
  }

  void ConstrainedVariable::relax() {
    // If it has been specified, relax to the specified domain
    if(m_specifiedFlag)
      getCurrentDomain().relax(m_specifiedValue);
    else // Relax to the base domain
      getCurrentDomain().relax(baseDomain());
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

  std::string ConstrainedVariable::getViolationExpl() const
  {
  	  std::ostringstream os;

  	  for (ConstraintList::const_iterator it = m_constraints.begin() ; it != m_constraints.end(); ++it){
          ConstraintId constraint = it->first;
          if (constraint->getViolation() > 0.0)
              os << constraint->getViolationExpl() << std::endl;
  	  }

  	  return os.str();
  }

  const DataTypeId& ConstrainedVariable::getDataType() const
  {
	  return baseDomain().getDataType();
  }


  // PS-Specific stuff below here:
  PSVarType ConstrainedVariable::getType() const
  {
	  PSVarType answer = STRING;
	  if(baseDomain().isString())
		  answer =  STRING;
	  else if(baseDomain().isSymbolic()) {
		  if(baseDomain().isEmpty() || LabelStr::isString(baseDomain().getLowerBound()))
			  answer =  STRING;
		  else
			  answer =  OBJECT; //this may not be the best assumption ~MJI
	  }
	  else if(baseDomain().isBool())
		  answer =  BOOLEAN;
	  else if(baseDomain().isNumeric()) {
		  if(baseDomain().minDelta() < 1)
			  answer =  DOUBLE;
		  else
			  answer =  INTEGER;
	  }
	  else {
		  checkError(ALWAYS_FAIL, "Failed to correctly determine the type of " << toString());
	  }
	  return answer;
  }


  bool ConstrainedVariable::isEnumerated() const {
    check_runtime_error(isValid());
    return baseDomain().isEnumerated();
  }

  bool ConstrainedVariable::isInterval() const {
    check_runtime_error(isValid());
    return baseDomain().isInterval();
  }


  bool ConstrainedVariable::isNull() const {
    check_runtime_error(isValid());
    return lastDomain().isEmpty() && !isSpecified();
  }

  bool ConstrainedVariable::isSingleton() const {
    check_runtime_error(isValid());
    return isSpecified() || lastDomain().isSingleton();
  }

  PSVarValue ConstrainedVariable::getSingletonValue() const {
    check_runtime_error(isValid());
    check_runtime_error(isSingleton(), toLongString());
    if (isSpecified())
    	return PSVarValue(getSpecifiedValue(), getType());
    else
    	return PSVarValue(lastDomain().getSingletonValue(), getType());
  }


  PSList<PSVarValue> ConstrainedVariable::getValues() const {
    check_runtime_error(isValid());
    check_runtime_error(!isSingleton() && isEnumerated());
    PSList<PSVarValue> retval;
    std::list<double> values;
    lastDomain().getValues(values);
    PSVarType type = getType();

    for(std::list<double>::const_iterator it = values.begin(); it != values.end(); ++it) {
      PSVarValue value(*it, type);
      retval.push_back(value);
    }
    return retval;
  }

  PSList<PSConstraint*> ConstrainedVariable::getConstraints() const
  {
	  check_runtime_error(isValid());
	  check_error(!Entity::isPurging());
	  PSList<PSConstraint*> retval;
	  ConstraintList::const_iterator it = m_constraints.begin();
	  for ( ; it != m_constraints.end(); ++it) {
		  ConstraintId cId = it->first;
		  retval.push_back( (PSConstraint *) cId);
	  }
	  return retval;
    }



  double ConstrainedVariable::getLowerBound() const {
    check_runtime_error(isValid());
    check_runtime_error(isInterval());
    return lastDomain().getLowerBound();
  }

  double ConstrainedVariable::getUpperBound() const {
    check_runtime_error(isValid());
    check_runtime_error(isInterval());
    return lastDomain().getUpperBound();
  }

  void ConstrainedVariable::specifyValue(PSVarValue& v) {
    check_runtime_error(isValid());
    check_runtime_error(getType() == v.getType());

    debugMsg("ConstrainedVariable:specify","Specifying var:" << toString() << " to value:" << v.toString());

    // If specifying to the same value it already has, do nothing
    if (isSpecified() && (getSpecifiedValue() == v.asDouble())) {
        debugMsg("ConstrainedVariable:specify","Tried to specify to same value, so bailing out without doing any work");
        return;
    }

    specify(v.asDouble());
    debugMsg("ConstrainedVariable:specify","After specify for var:" << toString() << " to value:" << v.toString());
    // TODO: move this to ConstrainedVariable::specify()
    if (getConstraintEngine()->getAutoPropagation())
       getConstraintEngine()->propagate();
    debugMsg("ConstrainedVariable:specify","After propagate for var:" << toString());
  }

  	PSEntity* ConstrainedVariable::getParent() const {
    //EntityId parent(m_parent);//getParentEntity());
    if(m_parent.isNoId())
      return NULL;

    /* TODO: fix this
    else if(TokenId::convertable(parent))
      return new PSTokenImpl((TokenId) parent);
    else if(ObjectId::convertable(parent))
      return new PSObjectImpl((ObjectId) parent);
    else if(RuleInstanceId::convertable(parent))
      return new PSTokenImpl(((RuleInstanceId)parent)->getToken());
    else {
      checkRuntimeError(ALWAYS_FAIL,
			"Variable " << toString() << " has a parent that isn't a token, " <<
			"object, or rule: " << m_var->getParent()->toString());
    }
    */

    return (PSEntity *) m_parent;
  }

  	std::string ConstrainedVariable::toString() const {
  		std::ostringstream os;

  		if (isNull())
  			os << "NULL";
  		else if (isSingleton())
  			os << getSingletonValue().toString();
  		else if (isInterval())
  			os << "[" << getLowerBound() << "," << getUpperBound() << "]";
  		else if (isEnumerated()) {
  			os << "{";
  			PSList<PSVarValue> values = getValues();
  			for (int i=0;i<values.size();i++) {
  				if (i > 0)
  					os << ", ";
  				os << values.get(i).toString();
  			}
  			os << "}";
  		}
  		else
  			os << "ERROR!";

  		return os.str();
  	}

  	// Used to be toString, but the above is shorter and more user-friendly
  	std::string ConstrainedVariable::toLongString() const{
  		std::stringstream sstr;
  		sstr << Entity::toString() << (specifiedFlag() ? " (S) " : "") << " DERIVED=" << lastDomain().toString();
  		return sstr.str();
  	}
}
