#include "Object.hh"
#include "PlanDatabase.hh"
#include "Schema.hh"
#include "Token.hh"
#include "TokenVariable.hh"
#include "Constraint.hh"
#include "ConstraintLibrary.hh"
#include "ConstraintEngine.hh"
#include "EnumeratedDomain.hh"
#include "TypeFactory.hh"
#include "Utils.hh"
#include "Debug.hh"
#include "EntityIterator.hh"

#include <sstream>

/**
 * @todo Add type checking for objects, and compositions.
 */
namespace EUROPA {
  
  Object::Object(const PlanDatabaseId& planDatabase, const LabelStr& type, const LabelStr& name, bool open)
    : m_id(this), m_type(type), m_name(name), m_planDatabase(planDatabase), 
      m_state(INCOMPLETE), m_lastOrderingChoiceCount(0), 
      m_thisVar((new Variable< ObjectDomain>(m_planDatabase->getConstraintEngine(), 
					     ObjectDomain(m_id, type.c_str())))->getId()) {
    check_error(m_planDatabase.isValid());
    if (!open)
      close();
  }

  Object::Object(const ObjectId& parent, const LabelStr& type, const LabelStr& localName, bool open) 
    : m_id(this), m_parent(parent), 
      m_type(type), 
      m_name(std::string(parent->getName().toString() + "." + localName.toString())), 
      m_planDatabase(parent->getPlanDatabase()),
      m_state(INCOMPLETE), 
      m_thisVar((new Variable< ObjectDomain>(m_planDatabase->getConstraintEngine(), 
					     ObjectDomain(m_id, type.c_str())))->getId()) {
    check_error(m_parent.isValid());
    check_error(m_planDatabase->getSchema()->canContain(parent->getType(), type, localName),
		"Object " + parent->getName().toString() + 
		" cannot contain " + localName.toString() + " of type " + type.toString());

    parent->add(m_id);
    if (!open)
      close();
  }

  Object::~Object() {
    check_error(m_id);

    discard(false);

    m_id.remove();
  }
  
  /*
   * Hack! Code generation currently skips the factories and directly calls the constructor that specifies the parent,
   * so this is necessary for the interpreter to provide the same behavior
   * Everybody should be going through the factories
   */
  void Object::setParent(const ObjectId& parent)
  {
  	m_parent = parent;
  	m_parent->add(m_id);
  }

  void Object::handleDiscard(){
    if(!Entity::isPurging()){ // Exploit relationships to cascade delete.
      //!!! Relaxing this so we can play some transactions backwards.  I'm not sure this
      //!!! is safe, but we'll see.
//       checkError(m_planDatabase->getTokens().empty(),
// 		 "Objects cannot be deleted while there remain tokens in the database.");
      checkError(getTokens().empty(),
		 "Object cannot be deleted while there remain active tokens that could be on them.");

      check_error(isValid());

      // Remove thisVar
      check_error(m_thisVar.isValid());
      m_thisVar->discard();

      // Remove object variables
      std::vector<ConstrainedVariableId>::iterator itVCV = m_variables.begin();
      for ( ; itVCV != m_variables.end(); ++itVCV) {
	ConstrainedVariableId variable = *itVCV;
	variable->discard();
      }

      // Do cascaded delete on components
      ObjectSet::const_iterator itSO = m_components.begin();
      for ( ; itSO != m_components.end(); ++itSO)
	(*itSO)->cascadeDelete();

      if (!m_parent.isNoId())
	m_parent->remove(m_id);

      m_planDatabase->notifyRemoved(m_id);
    }

    Entity::handleDiscard();
  }

  const ObjectId& Object::getId() const {
    return(m_id);
  }

  const ObjectId& Object::getParent() const {
    return(m_parent);
  }

  const LabelStr& Object::getType() const {
    return(m_type);
  }

  const LabelStr Object::getRootType() const {
    LabelStr rootType = getType();

    while(m_planDatabase->getSchema()->hasParent(rootType))
      rootType = m_planDatabase->getSchema()->getParent(rootType);

    return rootType;
  }

  const LabelStr& Object::getName() const {
    return(m_name);
  }

  std::string Object::toString() const {
    std::stringstream sstr;
    sstr << getType().toString() << ":" << getName().toString();
    return sstr.str();
  }

  const PlanDatabaseId& Object::getPlanDatabase() const {
    return(m_planDatabase);
  }

  const ConstrainedVariableId& Object::getThis() const {
    return m_thisVar;
  }

  const ObjectSet& Object::getComponents() const {
    return(m_components);
  }

  const std::vector<ConstrainedVariableId>& Object::getVariables() const {
    return(m_variables);
  }

  void Object::add(const ObjectId& component) {
    check_error(m_components.find(component) == m_components.end());
    check_error(component->getPlanDatabase() == m_planDatabase);
    check_error(component->getParent() == m_id);
    m_components.insert(component);
  }

  void Object::remove(const ObjectId& component) {
    check_error(!Entity::isPurging());
    check_error(m_components.find(component) != m_components.end());
    m_components.erase(component);
  }

  void Object::add(const TokenId& token) {
    check_error(isComplete());
    debugMsg("Object:add:token", "Adding token " << token->getPredicateName().toString() << "(" << token->getKey() << ")");
    m_tokens.insert(token);
    m_planDatabase->notifyAdded(m_id, token);
  }

  void Object::remove(const TokenId& token) {
    check_error(token.isValid());

    debugMsg("Object:remove:token", "Removing token " << token->getPredicateName().toString() << "(" << token->getKey() << ")");
    check_error(isValid());
    check_error(!Entity::isPurging());

    m_tokens.erase(token);

    // A set is used to avoid duplicate deletions
    std::set<ConstraintId> constraints;

    // Gather all the constraints and remove various index entries
    std::multimap<int, ConstraintId>::iterator it = m_constraintsByTokenKey.find(token->getKey());
    while(it != m_constraintsByTokenKey.end()){

      if(it->first != token->getKey())
	break;

      // Obtain and store the constraint
      ConstraintId constraint = it->second;
      check_error(constraint.isValid());
      constraints.insert(constraint);
      m_constraintsByTokenKey.erase(it++);

      debugMsg("Object:remove:token", "Also removing " << constraint->toString());

      // If the constraint is a precedence constraint, the additional cleanup required
      std::map<int, int>::iterator pos = m_keyPairsByConstraintKey.find(constraint->getKey());
      if(pos != m_keyPairsByConstraintKey.end()) 
	removePrecedenceConstraint(constraint);
    }

    
    // If there are constraints to delete, we must go through one
    // more passof constraintsByTokenKey as there could be dangling
    // entries still not cleaned up. A better data structure should 
    // be developed to improve this as it will be too slow.
    if(!constraints.empty()){
      it = m_constraintsByTokenKey.begin();
      while(it != m_constraintsByTokenKey.end()){
	ConstraintId constraint = it->second;
	if(constraints.find(constraint) != constraints.end()){
	  debugMsg("Object:remove:token", "Also removing " << constraint->toString());
	  m_constraintsByTokenKey.erase(it++);
	}
	else
	  ++it;
      }

      Entity::discardAll(constraints);
    }

    m_planDatabase->notifyRemoved(m_id, token);
  }

  const TokenSet& Object::getTokens() const {
    return(m_tokens);
  }

  bool Object::hasToken(const TokenId& token) const {
    return m_tokens.find(token) != m_tokens.end() && token->getObject()->lastDomain().isSingleton();
  }

  void Object::getOrderingChoices(const TokenId& token, 
				  std::vector< std::pair<TokenId, TokenId> >& results,
				  unsigned int limit) {
    check_error(limit > 0, "Cannot set limit to less than 1.");
    results.push_back(std::make_pair(token, token));
  }

  unsigned int Object::countOrderingChoices(const TokenId& token, unsigned int limit){
    std::vector< std::pair<TokenId, TokenId> > results;
    getOrderingChoices(token, results, limit);
    m_lastOrderingChoiceCount = results.size();
    return m_lastOrderingChoiceCount;
  }

  unsigned int Object::lastOrderingChoiceCount(const TokenId& token) const{ return m_lastOrderingChoiceCount; }

  void Object::getTokensToOrder(std::vector<TokenId>& results) {}

  bool Object::hasTokensToOrder() const {
    return(false);
  }

  void Object::constrain(const TokenId& predecessor, const TokenId& successor){
    constrain(predecessor, successor, true);
  }

  void Object::constrain(const TokenId& predecessor, const TokenId& successor, bool isExplicit) { 
    check_error(predecessor.isValid());
    checkError(predecessor->isActive(), predecessor->toString() << ": " << predecessor->getState()->toString());
    check_error(successor.isValid());
    checkError(successor->isActive(), successor->toString() << ": " << successor->getState()->toString());

    check_error(!isConstrainedToPrecede(predecessor, successor),
		"Attempted to constrain previously constrained tokens.");

    // NOTE: Used to force propagation here, claiming that it was necessary for TokenRelationships
    // to be up to date. However, if all we are doing is adding constraints, that should not be required. However,
    // if there has been a relaxtion, we will require that the state is not 'provenInconsistent'.
    check_error(!m_planDatabase->getConstraintEngine()->provenInconsistent(),
		"Cannot constrain " + predecessor->getPredicateName().toString() + " when the database is already inconsistent.");

    // Post constraints on object variable, predecessor only in event they are equal
    constrainToThisObjectAsNeeded(predecessor);

    int encodedKey = makeKey(predecessor, successor);

    condDebugMsg(m_constraintsByKeyPair.find(encodedKey) != m_constraintsByKeyPair.end(), "Object:makeKey",
		 "Collision detected for " << predecessor->toString() << " and " << successor->toString() << 
		 " with " << encodedKey);

    ConstraintId constraint;

    // If successor is not noId then add the precede constraint.
    if (predecessor != successor) {
      constrainToThisObjectAsNeeded(successor); // Also constrain successor if necessary

      // Create the precedence constraint
      std::vector<ConstrainedVariableId> vars;
      vars.push_back(predecessor->getEnd());
      vars.push_back(successor->getStart());
      constraint =  ConstraintLibrary::createConstraint(LabelStr("precedes"),
								     getPlanDatabase()->getConstraintEngine(),
								     vars);

      // Store for bi-directional access by encoded key pair and constraint
      m_constraintsByKeyPair.insert(std::pair<int, ConstraintId>(encodedKey, constraint));
      m_keyPairsByConstraintKey.insert(std::make_pair(constraint->getKey(), encodedKey));

      // Store for access by token
      m_constraintsByTokenKey.insert(std::pair<int, ConstraintId>(predecessor->getKey(), constraint));
      m_constraintsByTokenKey.insert(std::pair<int, ConstraintId>(successor->getKey(), constraint));
    }

    if(isExplicit)
      m_explicitConstraints.insert(constraint.isId() ? constraint->getKey() : predecessor->getKey());

    m_planDatabase->notifyConstrained(m_id, predecessor, successor);
    check_error(isValid());
  }

  void Object::free(const TokenId& predecessor, const TokenId& successor){
    free(predecessor, successor, true);
  }

  void Object::free(const TokenId& predecessor, const TokenId& successor, bool isExplicit) {
    check_error(!Entity::isPurging());
    check_error(predecessor.isValid());
    check_error(successor.isValid());

    check_error(m_constraintsByTokenKey.find(predecessor->getKey()) != m_constraintsByTokenKey.end(),
					     "No constraint found on predecessor.");

    check_error(m_constraintsByTokenKey.find(successor->getKey()) != m_constraintsByTokenKey.end(),
					     "No constraint found on successor.");


    check_error(predecessor == successor ||  getPrecedenceConstraint(predecessor, successor).isValid(),
		"A precedence constraint is required to free.");

    if(predecessor == successor){
      if(isExplicit){
	check_error( m_explicitConstraints.find(predecessor->getKey()) != m_explicitConstraints.end(),
		     "May only explicit free and explicit constraint.");

	// Remove as an explicit constraint. No harm if it does not exist
	m_explicitConstraints.erase(predecessor->getKey());
      }
      // Clean up for token
      clean(predecessor);
    }
    else {
      // Now retrieve the constraint to be deleted
      ConstraintId constraint = getPrecedenceConstraint(predecessor, successor);

      removePrecedenceConstraint(constraint);

      // Now clean up tokens if there are no more precedence constraints left - will remove restriction
      // posted in this object
      clean(predecessor);
      clean(successor);
    }

    m_planDatabase->notifyFreed(m_id, predecessor, successor);
    check_error(isValid());
  }

  void Object::cascadeDelete() {
    check_error(m_parent.isValid());
    m_parent = ObjectId::noId();
    delete this;
  }

  const ConstrainedVariableId& Object::getVariable(const LabelStr& name) const {
    for (std::vector<ConstrainedVariableId>::const_iterator it = m_variables.begin();
         it != m_variables.end(); ++it) {
      const ConstrainedVariableId& var = *it;
      check_error(var.isValid());
      if (var->getName() == name)
        return(var);
    }
    return(ConstrainedVariableId::noId());
  }

  ConstrainedVariableId Object::getVariable(const std::vector<unsigned int>& path) const {
    unsigned int index = path[0];

    checkError(index >= 0 && index < m_variables.size(), 
	       "index of " << index << " out of bounds for " << getName().toString());

    ConstrainedVariableId var = m_variables[index];
    if(path.size() > 1){
      ObjectId object = var->lastDomain().getSingletonValue();
      std::vector<unsigned int>::const_iterator it = path.begin();
      std::vector<unsigned int> newPath(++it, path.end());

      var = object->getVariable(newPath);
    }

    return var;
  }

  void Object::close() {
    check_error(!isComplete());
    m_state = COMPLETE;
    m_planDatabase->notifyAdded(m_id);
  }

  bool Object::isComplete() const {
    return(m_state == COMPLETE);
  }

  /**
   * Will only delete a constraint if there is exactly one constraint for this token and it is
   * there through implication, rather explicit user action.
   */
  void Object::clean(const TokenId& token) {
    check_error(token.isValid());

    // Find the first constraint on this token
    std::multimap<int, ConstraintId>::iterator it = m_constraintsByTokenKey.find(token->getKey());

    check_error(it != m_constraintsByTokenKey.end(), "Should be at least one constraint on clean");

    // If it is the only constraint, we should remove it - singleton constraint to this object
    ConstraintId candidateForRemoval = it->second;
    ++it;
    if(it->first != token->getKey() && !hasExplicitConstraint(token)){ // Then there is only one, so clean it
      --it;
      m_constraintsByTokenKey.erase(it);
      candidateForRemoval->discard();
    }
  }

  /**
   * Implementation relies on the internal store of constraints by token key. There will
   * be at least one constraint present for this token for it to be true.
   */
  bool Object::isConstrainedToThisObject(const TokenId& token) const {
    check_error(token.isValid());

    // Must be at least one entry for this token
    return m_constraintsByTokenKey.find(token->getKey()) != m_constraintsByTokenKey.end();
  }

  ConstraintId Object::getPrecedenceConstraint(const TokenId& predecessor, const TokenId& successor) const{
    int encodedKey = makeKey(predecessor, successor);
    std::multimap<int, ConstraintId>::const_iterator it = m_constraintsByKeyPair.find(encodedKey);
    while(it != m_constraintsByKeyPair.end() && it->first == encodedKey){
      ConstraintId constraint = it->second;
      if(constraint->getScope()[0] == predecessor->getEnd() &&
	 constraint->getScope()[1] == successor->getStart())
	return constraint;
      ++it;
    }

    return ConstraintId::noId();
  }

  /**
   * Use encoded lookup
   */
  bool Object::isConstrainedToPrecede(const TokenId& predecessor, const TokenId& successor) const{
    return getPrecedenceConstraint(predecessor, successor).isId();
  }

  /**
   * Get all constraints for this token. And the accompanying encoded key pair
   */
  void Object::getPrecedenceConstraints(const TokenId& token, 
					std::vector<ConstraintId>& results) const{
    check_error(isValid());
    check_error(results.empty());

    // Find the first constraint for this token.
    std::multimap<int, ConstraintId>::const_iterator it = m_constraintsByTokenKey.find(token->getKey());

    // Record if we find the singleton constraint, as we should only get one. That constraint does not have
    // a key pair.
    bool singletonFound = false; 
    while(it != m_constraintsByTokenKey.end() && it->first == token->getKey()){
      ConstraintId constraint = it->second;
      std::map<int, int>::const_iterator pos = m_keyPairsByConstraintKey.find(constraint->getKey());
      if(pos == m_keyPairsByConstraintKey.end()){
	check_error(singletonFound == false,
		    "Can only find one singleton constraint per token.");
	singletonFound = true;
      }
      else
	results.push_back(constraint);
      ++it;
    }
  }


  bool Object::hasExplicitConstraint(const TokenId& token) const {
    // Get all the constraints for this token
    std::vector<ConstraintId> constraints;
    getPrecedenceConstraints(token, constraints);
    for(std::vector<ConstraintId>::const_iterator it = constraints.begin();
	it != constraints.end();
	++it){
      ConstraintId constraint = *it;

      checkError(constraint->getScope()[1]->getParent() == token || constraint->getScope()[0]->getParent() == token,
		 "Problem with constraint caching.");

      // If an explict constraint, and the given token is the successor
      if(m_explicitConstraints.find(constraint->getKey()) != m_explicitConstraints.end())
	return true;
    }
    

    // Finally we try for an explicit singleton constraint. Should be a rare thing!
    return m_explicitConstraints.find(token->getKey()) != m_explicitConstraints.end();
  }

  bool Object::canBeCompared(const EntityId& entity) const {
    return(ObjectId::convertable(entity));
  }

  void Object::getAncestors(std::list<ObjectId>& results) const {
    if (!m_parent.isNoId()) {
      results.push_back(m_parent);
      m_parent->getAncestors(results);
    }
  }

  void Object::clean(const ConstraintId& constraint, int tokenKey) {
    // Remove the entry in the predecessor list if necessary
    std::multimap<int, ConstraintId>::iterator it = m_constraintsByTokenKey.find(tokenKey);
    while(it != m_constraintsByTokenKey.end() && it->first == tokenKey && it->second != constraint)
      ++it;
    if(it != m_constraintsByTokenKey.end() && it->second == constraint)
      m_constraintsByTokenKey.erase(it);
  }

  void Object::constrainToThisObjectAsNeeded(const TokenId& token) {
    check_error(token.isValid());
    check_error(token->isActive());
    check_error(token->getObject()->lastDomain().isMember(m_id),
      "Cannot assign token " + token->getName().toString() + " to  object " + getName().toString() + ", it is not part of derived domain.");

    // Place this token on the object. We use a constraint since token assignment is done by specifying
    // the object variable. This only needs to be done once, so test first if it has already been constrained
    if (!isConstrainedToThisObject(token)) {
      ConstraintId thisObject =
	ConstraintLibrary::createConstraint(LabelStr("eq"),
					    getPlanDatabase()->getConstraintEngine(),
					    makeScope(token->getObject(), m_thisVar));
      m_constraintsByTokenKey.insert(std::pair<int, ConstraintId>(token->getKey(), thisObject));
    }
  }


  void Object::freeImplicitConstraints(const TokenId& token){
    // Get all the constraints for this token
    std::vector<ConstraintId> constraints;
    getPrecedenceConstraints(token, constraints);
    for(std::vector<ConstraintId>::const_iterator it = constraints.begin();
	it != constraints.end();
	++it){

      ConstraintId constraint = *it;

      check_error(m_explicitConstraints.find(constraint->getKey()) == m_explicitConstraints.end(),
		  "Should never be freeing explicit constraints in this manner.");

      TokenId predecessor = constraint->getScope()[0]->getParent();
      TokenId successor = constraint->getScope()[1]->getParent();
      free(predecessor, successor, false);
    }
  }

  bool Object::isValid() const {
    check_error(m_constraintsByKeyPair.size() == m_keyPairsByConstraintKey.size(),
		"Lookup tables should have identical sizes. Must be out of synch.");

    // Validate tokens and constraints
    for(std::multimap<int, ConstraintId>::const_iterator it = m_constraintsByTokenKey.begin();
	it != m_constraintsByTokenKey.end();
	++it){
      checkError(it->second.isValid(), it->second);
      check_error(Entity::getEntity(it->first));
    }

    for(std::map<int, ConstraintId>::const_iterator it = m_constraintsByKeyPair.begin();
	it != m_constraintsByKeyPair.end();
	++it){
      checkError(it->second.isValid(), "Invalid constraint for key pair " << LabelStr(it->first).toString());
      checkError(m_keyPairsByConstraintKey.find(it->second->getKey())->second == it->first,
		  "Lookup should be symmetric.");
    }

    return true;
  }

  ObjectDomain::ObjectDomain(const char* typeName)
  : EnumeratedDomain(false, typeName){
    check_error(!isNumeric());
  }

  ObjectDomain::ObjectDomain(const std::list<ObjectId>& initialValues, const char* typeName)
  : EnumeratedDomain(false, typeName){
    check_error(!isNumeric());
    for(std::list<ObjectId>::const_iterator it = initialValues.begin(); it != initialValues.end(); ++it){
      ObjectId object = *it;
      check_error(object.isValid());
      insert(object);
    }
    close();
  }

  ObjectDomain::ObjectDomain(const ObjectId& initialValue, const char* typeName): 
    EnumeratedDomain(initialValue, false, typeName){check_error(!isNumeric());}

  ObjectDomain::ObjectDomain(const AbstractDomain& org)
    : EnumeratedDomain(org){
    check_error(org.isEmpty() || ObjectId(org.getLowerBound()).isValid(),
		"Attempted to construct an object domain with values of non-object type " + 
		org.getTypeName().toString());
  }

  const LabelStr&
  ObjectDomain::getDefaultTypeName()
  {
    static const LabelStr sl_typeName("Objects");
    return(sl_typeName);
  }

  bool ObjectDomain::convertToMemberValue(const std::string& strValue, double& dblValue) const{
    int value = atoi(strValue.c_str());
    EntityId entity = Entity::getEntity(value);

    if(entity.isId() && isMember(entity)){
      dblValue = entity;
      return true;
    }

    return false;
  }

  std::string ObjectDomain::toString() const{ 
     return "OBJECT-"+EnumeratedDomain::toString();
  }

  std::string ObjectDomain::toString(double value) const {
    check_error(isMember(value), "Caught an invalid attempt to display a value not in this domain");
    ObjectId object = value;
    std::ostringstream os;
    os << object->getName().toString()<<"("<< object->getKey() <<")";
    return  os.str();
  }

  std::list<ObjectId> ObjectDomain::makeObjectList(const std::list<double>& inputs){
    std::list<ObjectId> outputs;
    for (std::list<double>::const_iterator it = inputs.begin(); it != inputs.end(); ++it)
      outputs.push_back((ObjectId)(*it));
    return outputs;
  }


  std::list<ObjectId> ObjectDomain::makeObjectList() const {
    std::list<ObjectId> objects;
    const std::set<double>& values = getValues();
    for(std::set<double>::const_iterator it = values.begin(); it != values.end(); ++it){
      ObjectId object = *it;
      objects.push_back(object);
    }

    return objects;
  }
  
  ObjectDomain *ObjectDomain::copy() const {
    ObjectDomain *ptr = new ObjectDomain(*this);
    check_error(ptr != 0);
    return(ptr);
  }
  

  ConstrainedVariableId Object::addVariable(const AbstractDomain& baseDomain, const char* name){
      check_error(!isComplete(), 
		  "Cannot add variable " + std::string(name) +
		  " after completing object construction for " + m_name.toString());

      check_error(m_planDatabase->getSchema()->canContain(m_type, baseDomain.getTypeName(), name),
		  "Cannot add a variable " + std::string(name) + " of type " + 
		  baseDomain.getTypeName().toString() +
		  " to objects of type " + m_type.toString());

      std::string fullVariableName(m_name.toString() + "." + name);

      ConstrainedVariableId id = 
	TypeFactory::createVariable(baseDomain.getTypeName().c_str(),
				    m_planDatabase->getConstraintEngine(), 
				    baseDomain,
				    true,
				    fullVariableName.c_str(),
				    m_id,
				    m_variables.size());
      m_variables.push_back(id);

      if(id->baseDomain().isSingleton())
	id->specify(id->baseDomain().getSingletonValue());

      return(id);
    }


  void Object::notifyOrderingRequired(const TokenId& token){
    m_planDatabase->notifyOrderingRequired(m_id, token);
  }

  void Object::notifyOrderingNoLongerRequired(const TokenId& token){
    m_planDatabase->notifyOrderingNoLongerRequired(m_id, token);
  }


  /** Stubs for notification handlers **/
  void Object::notifyMerged(const TokenId& token){}

  void Object::notifyRejected(const TokenId& token){}

  void Object::notifyDeleted(const TokenId& token){
    remove(token);
  }
  
  std::string Object::toString(ObjectVarId objVar)
  {
  	std::ostringstream os;
  	if (objVar->lastDomain().isSingleton()) {
  		ObjectId obj = objVar->lastDomain().getSingletonValue(); 
  	    os << obj->getName().toString();
  	}
  	else {
  		os << objVar->toString();
  	}
  	
  	return os.str();
  }

  int Object::makeKey(const TokenId& a, const TokenId& b){
    return (a->getKey() << 16) ^ b->getKey();
  }

  void Object::removePrecedenceConstraint(const ConstraintId& constraint){
    TokenId predecessor = constraint->getScope()[0]->getParent();
    TokenId successor = constraint->getScope()[1]->getParent();

    int encodedKey = makeKey(predecessor, successor);
    std::multimap<int, ConstraintId>::iterator it = m_constraintsByKeyPair.find(encodedKey);
    while(it != m_constraintsByKeyPair.end() && it->first == encodedKey){
      ConstraintId c = it->second;
      if(c == constraint){
	m_constraintsByKeyPair.erase(it);
	break;
      }

      ++it;
    }

    m_keyPairsByConstraintKey.erase(constraint->getKey());
    m_explicitConstraints.erase(constraint->getKey());

      // Remove the entries in the m_constraintsByTokenKey list for predecessor and successor
    clean(constraint, predecessor->getKey());
    clean(constraint, successor->getKey());

      // Delete the actual constraint
    constraint->discard();
  }
}
