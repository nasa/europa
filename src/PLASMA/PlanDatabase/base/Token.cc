#include "Token.hh"
#include "TokenVariable.hh"
#include "UnifyMemento.hh"
#include "StackMemento.hh"
#include "MergeMemento.hh"
#include "ObjectTokenRelation.hh"
#include "PlanDatabase.hh"
#include "Object.hh"
#include "Schema.hh"
#include "IntervalIntDomain.hh"
#include "Constraint.hh"
#include "ConstraintFactory.hh"
#include "Utils.hh"
#include "Debug.hh"
#include <map>

/**
 * @author Conor McGann
 */

#define MERGING 1

namespace EUROPA{

  StateDomain::StateDomain(const char*)
    : EnumeratedDomain(false, "TokenStates") {}

  StateDomain::StateDomain(const AbstractDomain& org)
    : EnumeratedDomain(org){
    check_error(org.getTypeName().toString() == "TokenStates",
		"Attempted to construct a StateDomain with invalid type " + org.getTypeName().toString());
  }

  void StateDomain::operator>>(ostream&os) const {
    // Now commence output
    AbstractDomain::operator>>(os);
    os << "{";

    // First construct a lexicographic ordering for the set of values.
    std::set<std::string> orderedSet;

    for (std::set<double>::const_iterator it = m_values.begin(); it != m_values.end(); ++it) {
      LabelStr value = *it;
      orderedSet.insert(value.toString());
    }

    std::string comma = "";
    for (std::set<std::string>::const_iterator it = orderedSet.begin(); it != orderedSet.end(); ++it) {
      os << comma << *it;
      comma = ",";
    }

    os << "}";
  }

  /**
   * Allocate Constants for posisble state variable values
   */
  const LabelStr Token::INCOMPLETE("INCOMPLETE");
  const LabelStr Token::INACTIVE("INACTIVE");
  const LabelStr Token::ACTIVE("ACTIVE");
  const LabelStr Token::MERGED("MERGED");
  const LabelStr Token::REJECTED("REJECTED");

  const LabelStr& Token::noObject(){
    static const LabelStr sl_noObject("NO_OBJECT_ASSIGNED");
    return sl_noObject;
  }

  Token::Token(const PlanDatabaseId& planDatabase,
	       const LabelStr& predicateName,
	       bool rejectable,
	       bool isFact,
	       const IntervalIntDomain& durationBaseDomain,
	       const LabelStr& objectName,
	       bool closed)
      :Entity(),
       m_id(this),
       m_relation("none"),
       m_predicateName(predicateName),
       m_planDatabase(planDatabase) {
    commonInit(predicateName, rejectable, isFact, durationBaseDomain, objectName, closed);
  }

  // Slave tokens cannot be rejectable.
  Token::Token(const TokenId& master,
	       const LabelStr& relation,
	       const LabelStr& predicateName,
	       const IntervalIntDomain& durationBaseDomain,
	       const LabelStr& objectName,
	       bool closed)
     :Entity(),
      m_id(this),
      m_master(master),
      m_relation(relation),
      m_predicateName(predicateName),
      m_planDatabase((*master).m_planDatabase) {

       // Master must be active to add children
       check_error(m_master->isActive());
       m_master->add(m_id);
       commonInit(predicateName, false, false, durationBaseDomain, objectName, closed);
  }

  Token::~Token(){
    discard(false);
    m_id.remove();
  }

  void Token::handleDiscard(){
    debugMsg("Token:handleDiscard", "Discarding (" << getKey() << ")");

    m_deleted = true;

    // If the token has been committed, then the refCount has been incremented so we should drop by 1
    // here to balance it out on deletion
    if(m_committed)
      decRefCount();

    if(!Entity::isPurging()){ // Exploit relationships for cascaded delete
      check_error(!isIncomplete());
      check_error(isValid());

      // If it has been merged or activated then cancel it. This allows events to flow to unlink the implications
      // to dependent entities.
      if (!isInactive())
	cancel();

      // Notify objects that the token is being deleted. Allows synchronization
      const std::set<double>& objects = getObject()->getBaseDomain().getValues();
      for(std::set<double>::const_iterator it = objects.begin(); it!= objects.end(); ++it){
	ObjectId object = *it;
	object->notifyDeleted(m_id);
      }

      // Notify master of removal if appropriate - won't be done if this has arisen out of cascaded delete.
      if(!m_master.isNoId())
	m_master->remove(m_id);

      // Now remove all the variables
      discardAll(m_allVariables);

      // Now also remove all pseudoVariabls
      discardAll(m_pseudoVariables);

      m_planDatabase->notifyRemoved(m_id);
    }

    if(!m_unifyMemento.isNoId())
      m_unifyMemento.release();

    // Delegate to super class
    Entity::handleDiscard();
  }

  const TokenId& Token::getId() const {
    check_error(m_id.isValid());
    return m_id;
  }

  const TokenId& Token::master() const {
    check_error(m_master.isNoId() || m_master.isValid());
    return m_master;
  }

  const LabelStr& Token::getRelation() const {
    check_error(m_master.isNoId() || m_master.isValid());
    return m_relation; // returns "NONE" if m_master isNoId()
  }

  /**
   * This works because we have key based comparators which allow us to rely on positions
   */
  const TokenId& Token::getSlave(int slavePosition) const{
    int i = 0;
    for(TokenSet::const_iterator it = m_slaves.begin(); it != m_slaves.end(); ++it){
      if(i == slavePosition)
	return *it;
      else
	i++;
    }
    return TokenId::noId();
  }

  int Token::getSlavePosition(const TokenId& slave) const{
    int i = 0;
    for(TokenSet::const_iterator it = m_slaves.begin(); it != m_slaves.end(); ++it){
      TokenId token = *it;
      if(token == slave)
	return i;
      else
	i++;
    }
    return -1;
  }

  const LabelStr& Token::getBaseObjectType() const {return m_baseObjectType;}

  const LabelStr& Token::getPredicateName() const {return m_predicateName;}

  const LabelStr& Token::getUnqualifiedPredicateName() const {return m_unqualifiedPredicateName;}

  const PlanDatabaseId& Token::getPlanDatabase() const {
    check_error(m_planDatabase.isValid());
    return m_planDatabase;
}
  const StateVarId& Token::getState() const{
    checkError(m_state.isValid(), m_state);
    return m_state;
  }
  const ObjectVarId& Token::getObject() const{
    checkError(m_object.isValid(), m_object);
    return m_object;
  }
  const TempVarId& Token::duration() const{
    checkError(m_duration.isValid(), m_duration);
    return m_duration;
  }
  const std::vector<ConstrainedVariableId>& Token::parameters() const {return m_parameters;}
  const std::vector<ConstrainedVariableId>& Token::getVariables() const {return m_allVariables;}
  const TokenSet& Token::slaves() const {return m_slaves;}
  const TokenSet& Token::getMergedTokens() const {return m_mergedTokens;}
  const TokenId& Token::getActiveToken() const {return m_activeToken;}

  const ConstrainedVariableId Token::getVariable(const LabelStr& name) const{
    const std::vector<ConstrainedVariableId>& vars = getVariables();
    for(std::vector<ConstrainedVariableId>::const_iterator it = vars.begin();
	it != vars.end(); ++it){
      ConstrainedVariableId var = *it;
      checkError(var.isValid(), "Invalid variable id: " << var << " found in token: " << m_id);
      if(var->getName() == name)
	return var;
    }
    if(getPlanDatabase()->isGlobalVariable(name))
      return getPlanDatabase()->getGlobalVariable(name);
    else
      return ConstrainedVariableId::noId();
  }

  void Token::add(const TokenId& slave){
    check_error(!isIncomplete());
    check_error(m_slaves.find(slave) == m_slaves.end());
    check_error(slave->getPlanDatabase() == m_planDatabase);
    check_error(slave->master() == m_id);
    m_slaves.insert(slave);
  }

  void Token::remove(const TokenId& slave){
    check_error(!Entity::isPurging());
    check_error(!isIncomplete());

    m_slaves.erase(slave);
  }

  bool Token::isIncomplete() const {return m_state->baseDomain().isOpen();}

  bool Token::isInactive() const {
    checkError(m_state.isValid(), "ID:"<< m_state << " Token Key:" << getKey());
    return (!isIncomplete() && !m_state->isSpecified());
  }

  bool Token::isActive() const {
    checkError(m_state.isValid(), "ID:"<< m_state << " Token Key:" << getKey());
    return (m_state->isSpecified() && m_state->getSpecifiedValue() == ACTIVE);
  }

  bool Token::isMerged() const {
    checkError(m_state.isValid(), "ID:"<< m_state << " Token Key:" << getKey());
    return (m_state->isSpecified() && m_state->getSpecifiedValue() == MERGED);
  }

  bool Token::isRejected() const {
    checkError(m_state.isValid(), "ID:"<< m_state << " Token Key:" << getKey());
    return (m_state->isSpecified() && m_state->getSpecifiedValue() == REJECTED);
  }

  void Token::close() {
    check_error(isIncomplete());
    m_state->close();
    m_planDatabase->notifyAdded(m_id);
  }

  void Token::cancel(){
    checkError(!isIncomplete() && !isInactive(), getState()->toString());
    check_error(!Entity::isPurging());
    LabelStr state = m_state->getSpecifiedValue();

    if(state == ACTIVE)
      deactivate();
    else if(state == MERGED)
      split();
    else if(state == REJECTED)
      reinstate();
    else
      check_error(ALWAYS_FAILS);
  }

  void Token::doMerge(const TokenId& activeToken){
    check_error(isValid());
    check_error(isInactive());
    check_error(activeToken.isValid());
    check_error(activeToken->isActive());
    checkError(m_state->lastDomain().isMember(MERGED), "Not permitted to merge." << toString());
    check_error(getPlanDatabase()->getSchema()->isA(activeToken->getPredicateName(), m_predicateName),
		"Cannot merge tokens with different predicates: " +
		m_predicateName.toString() + ", " + activeToken->getPredicateName().toString());

    m_state->setSpecified(MERGED);
    m_unifyMemento = UnifyMementoId(new UnifyMemento(m_id, activeToken));
    m_activeToken = activeToken;
    m_activeToken->addMergedToken(m_id);

    /** Send a message to all objects that it has been rejected **/
    const std::set<double>& objects = getObject()->getBaseDomain().getValues();
    for(std::set<double>::const_iterator it = objects.begin(); it!= objects.end(); ++it){
      ObjectId object = *it;
      object->notifyMerged(m_id);
    }

    m_planDatabase->notifyMerged(m_id);
  }

  void Token::split(){
    check_error(isValid());
    check_error(isMerged());
    check_error(m_unifyMemento.isValid());

    // Make changes to the state before removing from active token as there can be a circularity
    // when making non-chronological deletes.
    m_state->resetSpecified();
    TokenId activeToken = m_activeToken;
    UnifyMementoId memento = m_unifyMemento;
    m_unifyMemento = UnifyMementoId::noId();
    m_activeToken = TokenId::noId();

    // Now clean up
    bool activeTokenDeleted = activeToken->removeMergedToken(m_id);
    memento->undo(activeTokenDeleted);
    memento.release();

    m_planDatabase->notifySplit(m_id);
  }

  void Token::reject(){
    check_error(isValid());
    check_error(isInactive());
    check_error(m_master.isNoId());

    /*
     * A more complicated test requires that there are no constraints linking this token's variables to any other
     * variable in the system. This is necessary so that rejecting a token is strictly a restriction of the problem.
     * Prior to rejection, all such constraints must be removed, if present.
     * CMG: This is to strong. If the problem is posed with such constraints, we will allow them to persist. It will
     * be up to the user/modeler to handle correct logic of rejection vs activation should the constraints matter.
     */
    //check_error(noExternalConstraints(),
    //	"Attempted to reject token '" + getName().toString() + "' that has external constraints on it.");

    m_state->setSpecified(REJECTED);

    /** Send a message to all objects that it has been rejected **/
    const std::set<double>& objects = getObject()->getBaseDomain().getValues();
    for(std::set<double>::const_iterator it = objects.begin(); it!= objects.end(); ++it){
      ObjectId object = *it;
      object->notifyRejected(m_id);
    }

    m_planDatabase->notifyRejected(m_id);
  }

  void Token::reinstate(){
    check_error(isRejected());
    m_state->resetSpecified();
    m_planDatabase->notifyReinstated(m_id);

  }

  void Token::activate(){
    checkError(isInactive(),
	       "Token " << Entity::toString() <<
		" is not INACTIVE. Only inactive tokens may be activated.");
    check_error(isValid());
    m_state->setSpecified(ACTIVE);
    m_planDatabase->notifyActivated(m_id);
  }

  void Token::deactivate(){
    check_error(isActive());
    check_error(isValid());

    while(!m_mergedTokens.empty()){
      TokenId tokenToSplit = *(m_mergedTokens.begin());
      check_error(tokenToSplit.isValid());
      check_error(tokenToSplit->m_activeToken == m_id);
      check_error(tokenToSplit->isMerged());
      tokenToSplit->split();
    }


    // If it has any remaining slaves, remove them. Keep pulling from the beginning
    // since the slave may make a call back and remove itself from this buffer.
    while(!m_slaves.empty()){
      TokenId slave = *(m_slaves.begin());
      slave->removeMaster(m_id);
    }

    // Must wait till now to reset since mergedTokens would otherwise be included in the new spec domain.
    m_state->resetSpecified();

    // Notify of deactivation, which may impact the set of slaves.
    m_planDatabase->notifyDeactivated(m_id);
  }


  bool Token::isStandardConstraint(const ConstraintId& constraint) const{
    return(m_standardConstraints.find(constraint) != m_standardConstraints.end());
  }

  const std::set<ConstraintId>& Token::getStandardConstraints() const
  {
	  return m_standardConstraints;
  }


  // TODO: include constraints currently held by the RuleInstance that was triggered by this token
  double Token::getViolation() const
  {
  	  double total = 0.0;
      for (std::set<ConstraintId>::const_iterator it = m_standardConstraints.begin(); it != m_standardConstraints.end(); ++it) {
  	      ConstraintId c = *it;
  	      total += c->getViolation();
      }

      return total;
  }

  std::string Token::getViolationExpl() const
  {
  	  std::ostringstream os;

      for (std::set<ConstraintId>::const_iterator it = m_standardConstraints.begin(); it != m_standardConstraints.end(); ++it) {
  	      ConstraintId c = *it;
  	      if (c->getViolation() > 0.0)
  	          os << c->getViolationExpl() << std::endl;
      }

      return os.str();
  }

  bool Token::isValid() const {
    bool result = true;

    // State Variable should never include INACTIVE
    if(m_state->baseDomain().isMember(INACTIVE)){
      result = false;
      debugMsg("Token:isValid", "Cannot be incomplete.");
    }
    else if(m_id.isInvalid() || (!m_master.isNoId() && m_master.isInvalid())){
      result = false;
      debugMsg("Token:isValid", "Bad id or master id.");
    }
    else if(isActive()){
      result = m_unifyMemento.isNoId() && m_activeToken.isNoId();
      condDebugMsg(!result, "Token:isValid", "Cannot have merged ids if active.");
    }
    else if(isMerged()){
      result = (m_mergedTokens.empty() && m_unifyMemento.isValid() && m_activeToken.isValid() && m_slaves.empty());
      condDebugMsg(!result, "Token:isValid", "Not correctly merged");
    }
    else {// Otherwise - REJECTED or INACTIVE
      result = (m_mergedTokens.empty() && m_unifyMemento.isNoId() && m_activeToken.isNoId() && m_slaves.empty());
      condDebugMsg(!result, "Token:isValid", "Invalid rejected or inactive token.");
    }

    return result;
  }

  /**
   * @todo - add typechecking here
   */
  void Token::commonInit(const LabelStr& predicateName,
			 bool rejectable,
			 bool isFact,
			 const IntervalIntDomain& durationBaseDomain,
			 const LabelStr& objectName,
			 bool closed){
    // The plan database must be valid
    check_error(m_planDatabase.isValid());

    if(predicateName.countElements(".") == 2)
      m_unqualifiedPredicateName = predicateName.getElement(1, ".");
    else
      m_unqualifiedPredicateName = predicateName;

    m_committed = false;
    m_deleted = false;
    m_terminated = false;
    m_isFact = false;

    if (isFact)
        makeFact();

    debugMsg("Token:commonInit",
	     "Initializing token (" << getKey() << ") for predicate " << predicateName.toString()
	     << " on object " << objectName.toString());

    // Allocate the state variable with initial base domain.
    StateDomain stateBaseDomain;
    stateBaseDomain.insert(ACTIVE);
    stateBaseDomain.insert(MERGED);
    if (rejectable)
      stateBaseDomain.insert(REJECTED);

    m_state = (new TokenVariable<StateDomain>(m_id,
					      m_allVariables.size(),
					      m_planDatabase->getConstraintEngine(),
					      stateBaseDomain,
					      false, // TODO: fixme
					      false,
					      LabelStr("state")))->getId();
    m_allVariables.push_back(m_state);

    // If present the schema must be valid and the predicate must be OK.
    check_error(m_planDatabase->getSchema().isNoId() ||
		m_planDatabase->getSchema()->isPredicate(predicateName),
		"Invalid predicate: " + predicateName.toString());

    // Allocate an object variable with an empty domain
    m_baseObjectType = m_planDatabase->getSchema()->getObjectType(m_predicateName);
    m_object = (new TokenVariable<ObjectDomain>(m_id,
						m_allVariables.size(),
						m_planDatabase->getConstraintEngine(),
						ObjectDomain(m_baseObjectType.c_str()),
						false, // TODO: fixme
						true,
						LabelStr("object")))->getId();

    checkError(!m_planDatabase->getObjectsByType(m_baseObjectType).empty(),
	       "Allocated a token with no object instance available of type " << m_baseObjectType.toString());

    // Call the plan database to fill it in, and maintain synchronization for dynamic objects
    m_planDatabase->makeObjectVariableFromType(m_baseObjectType, m_object);
    // If a specific object has been specified, validate that it can be assigned
    if (objectName != noObject()) {
      ObjectId object = m_planDatabase->getObject(objectName);
      check_error(object.isValid());
      check_error(m_object->baseDomain().isMember(object));
      m_object->specify(object);
    }

    m_allVariables.push_back(m_object);

    if(!m_object->isClosed())
      m_object->close();

    m_duration = (new TokenVariable<IntervalIntDomain>(m_id,
						       m_allVariables.size(),
						       m_planDatabase->getConstraintEngine(),
						       durationBaseDomain,
						       false, // TODO: fixme
						       true,
						       LabelStr("duration")))->getId();

    m_allVariables.push_back(m_duration);

    // A constraint is used to propagate changes in the object variable and thus maintain an accurate
    // relationship between the set of tokens and the set of objects. However, in order for this to work
    // we require that the object variable can be actively propagated. Thus we must close the variable even if the type is open.
    // This means that if later instances arrive in the database they will not be part of the base domain of the token.
    // Consequently, one must be very careful how on uses such objects when they are the object variables of tokens.
    if(!m_object->isClosed())
      m_object->close();

    // Allocate constraint directly. No factory used or required as this constraint
    // is not dynamically created.
    Id<ObjectTokenRelation> objectTokenRelation =
      (new ObjectTokenRelation("ObjectTokenRelation",
			       "Default",
			       m_planDatabase->getConstraintEngine(),
			       makeScope(m_state, m_object)))->getId();

    m_standardConstraints.insert(objectTokenRelation);

    if (closed) // close it if appropriate
      close();

    check_error(isValid());
  }

  void Token::handleAdditionOfInactiveConstraint(const ConstraintId& constraint){
    check_error(constraint.isValid());
    check_error(isMerged());

    // Delegate to the unify memento
    m_unifyMemento->handleAdditionOfInactiveConstraint(constraint);
  }

  void Token::handleRemovalOfInactiveConstraint(const ConstraintId& constraint){
    check_error(constraint.isValid());
    check_error(isMerged());

    // Delegate to the unify memento when not terminated
    if(!isTerminated())
      m_unifyMemento->handleRemovalOfInactiveConstraint(constraint);
  }

  bool Token::isAssigned() const {
    if (!isActive() || !getObject()->lastDomain().isSingleton())
      return false;

    ObjectId object = getObject()->lastDomain().getSingletonValue();
    return object->hasToken(m_id);
  }

  bool Token::isDeleted() const {return m_deleted;}

  void Token::makeFact() {
  	m_isFact = true;
  }

  void Token::commit() {
    static StateDomain sl_activeOnly;
    static bool sl_initialized(false);
    if(!sl_initialized){
      sl_activeOnly.insert(Token::ACTIVE);
      sl_activeOnly.close();
      sl_initialized = true;
    }

    check_error( false == m_committed );
    check_error( canBeCommitted(), "Attempt to commit a token that cannot be committed.");
    m_committed = true;
    incRefCount();

    // The base domain is restricted to active since this cannot be rolled back.
    m_state->restrictBaseDomain(sl_activeOnly);

    m_planDatabase->notifyCommitted(m_id);
  }


  bool Token::isPending() const {
    return !isIncomplete() && !isCommitted() && !isTerminated();
  }

  /**
   * @brief Review tighter specification for commitment
   */
  bool Token::canBeCommitted() const {
    return isPending() && isActive();
  }

  bool Token::isCommitted() const {
    return m_committed;
  }

  /**
   * @todo Could speed up considerably if warranted
   */
  void Token::restrictBaseDomains(){
    for(unsigned int i=1; i<m_allVariables.size(); i++){
      ConstrainedVariableId var = m_allVariables[i];
      checkError(var.isValid(), var);
      var->restrictBaseDomain(var->lastDomain());
    }

    for(ConstrainedVariableSet::const_iterator it = m_localVariables.begin(); it != m_localVariables.end(); ++it){
      ConstrainedVariableId var = *it;
      checkError(var.isValid(), var);
      var->restrictBaseDomain(var->lastDomain());
    }
  }

  bool Token::isTerminated() const { return m_terminated;}

  /**
   * @brief Tests if a token can be terminated.
   * @see terminate
   */
  bool Token::canBeTerminated(unsigned int tick) const{
    if(isTerminated())
      return false;

    // Rejected tokens can be immediately terminated without any consideration of their variables or their constraints
    if(isRejected())
      return true;

    // Now if it has any merged tokens, it cannot be terminated. The merged tokens must be removed first
    if(!m_mergedTokens.empty()){
      debugMsg("Token:canBeTerminated", "Cannot terminate " << toString() << " because of remaining supported token " << ((TokenId) * (m_mergedTokens.begin()))->toString() );
      return false;
    }

    // Use this count for iteration later
    const unsigned int varCount = m_allVariables.size();

    // If merged, it is redundant if the variables in its scope are supersets of the corresponding active token variable base domain
    if(isMerged()){
      TokenId activeToken = getActiveToken();

      // No basis for termination if active token is not committed.
      if(!activeToken->isCommitted()){
	debugMsg("Token:canBeTerminated", "Cannot terminate " << toString() << " which is a slave of " << m_master->toString());
	return false;
      }

      // Definitiely can terminate if the active token is terminated
      if(activeToken->isTerminated())
	return true;

      // Finally, we have to analyze the variables to see if the merged token is imposing restrictions on the active token
      // that would be lost if it were removed. The anaylsis will check that the base domain of the active token is a subset of the
      // derived domain of the merged token.
      const std::vector<ConstrainedVariableId>& activeVariables = activeToken->getVariables();
      // All variables except state variable
      for(unsigned int i = 1; i < varCount; i++){
	const AbstractDomain& activeBaseDomain = activeVariables[i]->baseDomain();
	const AbstractDomain& inactiveDerivedDomain = m_allVariables[i]->lastDomain();
	if(!activeBaseDomain.isSubsetOf(inactiveDerivedDomain)){
	  debugMsg("Token:canBeTerminated",
		   "Cannot terminate " << this->toString() << activeBaseDomain.toString() << " can be further restricted by " << inactiveDerivedDomain.toString() << std::endl <<
		   "Active Variable: " << activeVariables[i]->toString() << " Inactive Variable: " << m_allVariables[i]->toString());

	  return false;
	}
      }

      return true;
    }

    //
    // Declare a set to pull together all variables in the scope of a token into a single easy to check collection.
    // Could manage this incrementally on the token also for greater efficiency
    std::set<int> allVars;

    // Construct the set of constraints on variables of this token. Use a constraint set to avoid memory dependent order.
    ConstraintSet constraints;
    for(unsigned int i = 0; i < varCount; i++){
      ConstrainedVariableId var = m_allVariables[i];
      var->constraints(constraints);
      allVars.insert(var->getKey());
    }

    for(ConstrainedVariableSet::const_iterator it = m_localVariables.begin(); it != m_localVariables.end(); ++it){
      ConstrainedVariableId var = *it;
      var->constraints(constraints);
      allVars.insert(var->getKey());
    }

    for(ConstraintSet::const_iterator it = constraints.begin(); it != constraints.end(); ++it){
      ConstraintId constraint = *it;
      checkError(constraint.isValid(), constraint);

      // No problem if the constraint has been deactivated already
      if(!constraint->isActive() || constraint->isRedundant())
	continue;

      // If it is active, then we should ensure it has at least one external variable
      const std::vector<ConstrainedVariableId>& scope = constraint->getScope();
      for(unsigned int i=0;i<scope.size();i++){
	ConstrainedVariableId var = scope[i];

	// If the variable has no parent, its scope is not defined temporally. This is typically only arising
	// in initialization.
	if (var->parent().isNoId())
	  continue;

	// If it is a culprit with an unbound domain, and an external variable,
	// then it is an external constraint that must be retained and so we cannot terminate
	if(allVars.find(var->getKey()) == allVars.end() && !var->baseDomain().isSingleton()){
	  debugMsg("Token:canBeTerminated",
		   "Cannot terminate " << toString() << ". "
		   << var->toString() << " has an active external constraint "
		   << constraint->toString() << ".");

	  return false;
	}
      }
    }

    return true;
  }

  void Token::terminate(){
    // Set this flag immediately so that other tokens can evaluate its state when we are dealing with cascaded effects
    m_terminated = true;

    // Deactivate all variables to ensure the termination will not cause a relaxation. It is important to do this prior to treating
    // merged tokens
    for(std::vector<ConstrainedVariableId>::const_iterator it = m_allVariables.begin();
	it != m_allVariables.end();
	++it){
      ConstrainedVariableId var = *it;
      checkError(var.isValid(), var << " is not valid for token " << getKey());
      var->deactivate();
    }

    for(std::vector<ConstrainedVariableId>::const_iterator it = m_pseudoVariables.begin();
	it != m_pseudoVariables.end();
	++it){
      ConstrainedVariableId var = *it;
      checkError(var.isValid(), var << " is not valid for token " << getKey());
      var->deactivate();
    }

    for(ConstrainedVariableSet::const_iterator it = m_localVariables.begin();
	it != m_localVariables.end();
	++it){
      ConstrainedVariableId var = *it;
      checkError(var.isValid(), var << " is not valid for token " << getKey());
      var->deactivate();
    }

    m_planDatabase->notifyTerminated(m_id);
  }

  bool Token::noExternalConstraints() const{
    for(unsigned int i=0; i<m_allVariables.size(); i++){
      std::set<ConstraintId> constraints;
      m_allVariables[i]->constraints(constraints);
      for(std::set<ConstraintId>::const_iterator it = constraints.begin(); it != constraints.end(); ++it){
	ConstraintId constraint = *it;
	check_error(constraint.isValid());
	const std::vector<ConstrainedVariableId>& scope = constraint->getScope();
	for(unsigned int j=0;j<scope.size();j++){
	  EntityId parent = scope[j]->parent();
	  // It must not be related to a variable of another token in the plan
	  if(!parent.isNoId() && parent != m_id && TokenId::convertable(parent)){
	    debugMsg("Token:noExternalConstraints", "Found external constraint " << constraint->toString());
	    return false;
	  }
	}
      }
    }

    return true;
  }

  bool Token::canBeCompared(const EntityId& entity) const{
    return TokenId::convertable(entity);
  }

  const LabelStr&  Token::getName() const{
    return m_predicateName;
  }

  void Token::activateInternal(){
    m_state->setSpecified(ACTIVE);
    m_planDatabase->notifyActivated(m_id);
  }

  void Token::addMergedToken(const TokenId& token){
    checkError(isActive(), "Must be ative to merge onto it.");
    m_mergedTokens.insert(token);
    incRefCount();
  }

  bool Token::removeMergedToken(const TokenId& token){
    checkError(isActive(), "Must be ative to merge onto it.");
    m_mergedTokens.erase(token);
    return decRefCount();
  }

  bool Token::removeMaster(const TokenId& token){
    checkError(m_master.isValid(), "Master is not present or not valid.");
    checkError(m_master == token, "Trying to remove " << token->toString() << " instead of " << m_master->toString());
    m_master = TokenId::noId();

    // Call-back to unlink the slave if necessary.
    token->remove(m_id);

    // On removal of the master, the token will be discarded if its ref count is 1.
    bool willBeDiscarded = (refCount() == 1);

    // If the master is terminated and the slave is to be discarded, then terminate the slave. Otherwise just
    // decrement the reference count. This is to prevent relaxations arising from removal of the slave when initiated
    // by a termination
    if(willBeDiscarded && token->isTerminated())
      terminate();
    else
      decRefCount();

    return willBeDiscarded;
  }

  bool Token::isStateVariable(const ConstrainedVariableId& var){
    static const LabelStr sl_stateStr("state");

    bool result = (var->getName() == sl_stateStr);

    // It is possible a StateVar could be allocated without it being a TokenStateVariable. Discourage this practice
    checkError(!result || (var->getIndex() == 0 && TokenId::convertable(var->parent())),
			   var->toString() << " is not really a token state variable.");
    return result;
  }

  void Token::addLocalVariable(const ConstrainedVariableId& var){
    m_localVariables.insert(var);
  }

  void Token::removeLocalVariable(const ConstrainedVariableId& var){
    m_localVariables.erase(var);
  }

  const ConstrainedVariableSet& Token::getLocalVariables(){
    return m_localVariables;
  }


  LabelStr Token::makePseudoVarName(){
    static int sl_varKey(0);
    static std::string sl_prefix("PSEUDO_VARIABLE_");
    std::stringstream ss;
    ss << sl_prefix;
    ss << sl_varKey++;
    return ss.str();
  }

// PS Methods:
const std::string& Token::getEntityType() const
{
	static const std::string TOKEN_STR("TOKEN");
	return TOKEN_STR;
}

std::string Token::getTokenType() const
{
	return getUnqualifiedPredicateName().toString();
}

PSObject* Token::getOwner() const {
  if(!isAssigned())
    return NULL;

  ObjectVarId objVar = getObject();
  ObjectId id = ObjectId(objVar->lastDomain().getSingletonValue());
  return (PSObject *) id;
  //  return new PSObject(ObjectId(objVar->lastDomain().getSingletonValue()));
}

PSToken* Token::getMaster() const {
	TokenId m = master();
	if (m.isNoId())
	    return NULL;

	return (PSToken *) m;
}

PSList<PSToken*> Token::getSlaves() const {
  const TokenSet& tokens = slaves();
  PSList<PSToken*> retval;

  for(TokenSet::const_iterator it = tokens.begin(); it != tokens.end(); ++it) {
    TokenId id = *it;
	  //PSToken* tok = new PSToken(*it);
    retval.push_back((PSToken *) id);
  }
  return retval;
}

PSToken* Token::getActive() const
{
    return (PSToken*)((Token*)getActiveToken());
}

PSList<PSToken*> Token::getMerged() const
{
    PSList<PSToken*> retval;

    TokenSet::const_iterator it=m_mergedTokens.begin();
    for (;it != m_mergedTokens.end(); ++it) {
        retval.push_back(*it);
    }

    return retval;
}


PSToken::PSTokenState Token::getTokenState() const
{
    if (isActive())
        return PSToken::ACTIVE;

    if (isInactive())
        return PSToken::INACTIVE;

    if (isMerged())
        return PSToken::MERGED;

    if (isRejected())
        return PSToken::REJECTED;

    check_error(ALWAYS_FAIL,"Unknown token state");
    return PSToken::INACTIVE;
}

PSVariable* Token::getStart() const
{
    return (PSVariable *) start();
}

PSVariable* Token::getEnd() const
{
	  return (PSVariable *) end();
}

PSVariable* Token::getDuration() const
{
	  return (PSVariable *) duration();
}

PSList<PSVariable*> Token::getParameters() const
{
  PSList<PSVariable*> retval;
  const std::vector<ConstrainedVariableId>& vars = getVariables();
  for(std::vector<ConstrainedVariableId>::const_iterator it = vars.begin(); it != vars.end();++it) {
	  ConstrainedVariableId id = *it;
	  PSVariable* psVar = (PSVariable *) id;
	  retval.push_back(psVar);
  }
  return retval;
}

PSVariable* Token::getParameter(const std::string& name) const
{
  LabelStr realName(name);
  PSVariable* retval = NULL;
  const std::vector<ConstrainedVariableId>& vars = getVariables();
  for(std::vector<ConstrainedVariableId>::const_iterator it = vars.begin(); it != vars.end();
	++it) {
  	ConstrainedVariableId id = *it;
    if((*it)->getName() == realName) {
	retval = (PSVariable *) id;
	break;
    }
  }
  return retval;
}

void Token::merge(PSToken* activeToken)
{
    check_error(activeToken != NULL, "Can't merge on NULL token");
    TokenId tok = getPlanDatabase()->getEntityByKey(activeToken->getKey());
    doMerge(tok);
}

PSList<PSToken*> Token::getCompatibleTokens(unsigned int limit, bool useExactTest)
{
    std::vector<TokenId> tokens;
    getPlanDatabase()->getCompatibleTokens(this,tokens,limit,useExactTest);
    PSList<PSToken*> retval;

    for(unsigned int i=0;i<tokens.size();i++) {
    	TokenId id = tokens[i];
    	retval.push_back((PSToken *) id);
    }

    return retval;
}

std::string Token::toLongString() const
{
	std::ostringstream os;

	os << "Token(" << toString() << ") {" << std::endl;
	os << "    isFact:" << isFact() << std::endl;

	if (isMerged())
	    os << "    mergedInto:" << getActiveToken()->getKey() << std::endl;

	PSList<PSVariable*> vars = getParameters();
	for (int i=0;i<vars.size();i++) {
	    os << "    " << vars.get(i)->getEntityName() << " : " << vars.get(i)->toString() << std::endl;
	}

	os << "}" << std::endl;

	return os.str();
}

}
