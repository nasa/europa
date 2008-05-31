#include "DbClient.hh"
#include "PlanDatabase.hh"
#include "Entity.hh"
#include "Utils.hh"
#include "ObjectFactory.hh"
#include "Object.hh"
#include "Token.hh"
#include "TokenVariable.hh"
#include "DbClientListener.hh"
#include "ConstraintEngine.hh"
#include "ConstraintLibrary.hh"
#include "TypeFactory.hh"
#include "TokenFactory.hh"
#include "Debug.hh"

#include <string>
#include <iostream>

#define publish(message) { \
   for (std::set<DbClientListenerId>::const_iterator it = m_listeners.begin(); it != m_listeners.end(); ++it) { \
     DbClientListenerId listener = *it; \
     listener->message; \
   } \
 }

namespace EUROPA {

  const char* DELIMITER = ":"; /*!< Used for delimiting streamed output */

  DbClient::DbClient(const PlanDatabaseId& db)
    : m_id((DbClient*)this), m_deleted(false), m_transactionLoggingEnabled(false) {
    check_error(db.isValid());
    m_planDb = db;
  }

  DbClient::~DbClient(){
    m_deleted = true;
    m_id.remove();
  }

  const DbClientId& DbClient::getId() const{return m_id;}

  const CESchemaId& DbClient::getCESchema() const
  {
      return m_planDb->getConstraintEngine()->getCESchema();   
  }

  const SchemaId& DbClient::getSchema() const
  {
      return m_planDb->getSchema();   
  }

  
  ConstrainedVariableId
  DbClient::createVariable(const char* typeName, const AbstractDomain& baseDomain, const char* name, bool isTmpVar,bool canBeSpecified)
  {
    ConstrainedVariableId variable = getCESchema()->createVariable(typeName, m_planDb->getConstraintEngine(), baseDomain, canBeSpecified, name);
    if (m_planDb->getSchema()->isObjectType(typeName) && !variable->isClosed()) {
      m_planDb->makeObjectVariableFromType(typeName, variable);
    }

    if (!isTmpVar) {
       // Register as a global variable
        m_planDb->registerGlobalVariable(variable);
    }

    publish(notifyVariableCreated(variable));
    return variable;
  }

  ConstrainedVariableId
  DbClient::createVariable(const char* typeName, const char* name, bool isTmpVar)
  {
    ConstrainedVariableId variable = getCESchema()->createVariable(typeName, m_planDb->getConstraintEngine(), true, name);
    if (m_planDb->getSchema()->isObjectType(typeName)) {
      m_planDb->makeObjectVariableFromType(typeName, variable);
    }

    // TODO: register TmpVariables so that they can be cleaned up easily
    if (!isTmpVar) {
       // Register as a global variable
        m_planDb->registerGlobalVariable(variable);
    }

    publish(notifyVariableCreated(variable));

    return variable;
  }

  void DbClient::deleteVariable(const ConstrainedVariableId& var) {
    if(isGlobalVariable(var->getName()))
      m_planDb->unregisterGlobalVariable(var);
    publish(notifyVariableDeleted(var));
    delete (ConstrainedVariable*) var;
  }

  ObjectId DbClient::createObject(const char* type, const char* name){
    static const std::vector<const AbstractDomain*> noArguments;
    ObjectId object = ObjectFactory::createInstance(m_planDb, type, name, noArguments);
    debugMsg("DbClient:createObject", object->toString());
    publish(notifyObjectCreated(object));
    return object;
  }

  ObjectId DbClient::createObject(const char* type, const char* name, const std::vector<const AbstractDomain*>& arguments){
    ObjectId object = ObjectFactory::createInstance(m_planDb, type, name, arguments);
    debugMsg("DbClient:createObject", object->toString());
    publish(notifyObjectCreated(object, arguments));
    return object;
  }

  void DbClient::deleteObject(const ObjectId& obj) {
    publish(notifyObjectDeleted(obj));
    delete (Object*) obj;
  }

  void DbClient::close(){
    m_planDb->close();
    debugMsg("DbClient:close", "CLOSE ALL TYPES");
    publish(notifyClosed());
  }

  void DbClient::close(const char* objectType) {
    m_planDb->close(objectType);
    debugMsg("DbClient:close", objectType);
    publish(notifyClosed(objectType));
  }
 
  TokenId DbClient::createToken(const char* predicateName, bool rejectable, bool isFact) {
    TokenId token = allocateToken(predicateName, rejectable, isFact);
    debugMsg("DbClient:createToken", token->toString());
    publish(notifyTokenCreated(token));
    return(token);
  }

  void DbClient::deleteToken(const TokenId& token, const std::string& name) {
    check_error(token.isValid());
    checkError(token->isInactive() || token->isFact(),
	       "Attempted to delete active, non-fact token " << token->toString());
    publish(notifyTokenDeleted(token, name));

    //the keys are only recorded if logging is enabled
    //this may not be the right thing...
    if(!m_keysOfTokensCreated.empty()) {
      if(m_keysOfTokensCreated.back() == token->getKey()) {
	debugMsg("DbClient:deleteToken",
		 "Removing token key " << m_keysOfTokensCreated.back());
	m_keysOfTokensCreated.pop_back();
      }
      checkError(std::find(m_keysOfTokensCreated.begin(), m_keysOfTokensCreated.end(),
			   token->getKey()) == m_keysOfTokensCreated.end(),
		 "Attempted to delete " << token->toString() << " out of order.");
    }
    delete (Token*) token;
  }

  void DbClient::constrain(const ObjectId& object, const TokenId& predecessor, const TokenId& successor){
    object->constrain(predecessor, successor);
    debugMsg("DbClient:constrain", predecessor->toString() << " before " << successor->toString());
    publish(notifyConstrained(object, predecessor, successor));
  }

  void DbClient::free(const ObjectId& object, const TokenId& predecessor, const TokenId& successor){
    object->free(predecessor, successor);
    debugMsg("DbClient:free", predecessor->toString() << " before " << successor->toString());
    publish(notifyFreed(object, predecessor, successor));
  }

  void DbClient::activate(const TokenId& token){
    token->activate();
    debugMsg("DbClient:activate", token->toString());
    publish(notifyActivated(token));
  }

  void DbClient::merge(const TokenId& token, const TokenId& activeToken){
    static unsigned int sl_counter(0);
    sl_counter++;
    checkError(token.isValid(), token << ":" << sl_counter);
    token->doMerge(activeToken);
    debugMsg("DbClient:merge", token->toString() << " onto " << activeToken->toString());
    publish(notifyMerged(token, activeToken));
  }

  void DbClient::reject(const TokenId& token){
    check_error(token.isValid());
    token->reject();
    debugMsg("DbClient:reject", token->toString());
    publish(notifyRejected(token));
  }

  void DbClient::cancel(const TokenId& token){
    check_error(token.isValid());

    publish(notifyCancelled(token));

    TokenId activeToken;

    if(token->isMerged())
       activeToken = token->getActiveToken();

    checkError(activeToken.isNoId() || activeToken.isValid(), activeToken);

    if(activeToken.isId() && activeToken->refCount() == 1){
      // Expect that the last key created and stored will be this key
      checkError(!isTransactionLoggingEnabled() || 
		 m_keysOfTokensCreated.back() == activeToken->getKey(),
		 "If transaction logging enabled then we require chronological retraction. " << activeToken->toString());
      if(isTransactionLoggingEnabled()) {
	debugMsg("DbClient:cancel",
		 "Removing token key " << m_keysOfTokensCreated.back());
	m_keysOfTokensCreated.pop_back();
      }
    }


    token->cancel();

    debugMsg("DbClient:cancel", token->toString());
  }

  ConstraintId DbClient::createConstraint(const char* name, 
				 const std::vector<ConstrainedVariableId>& scope){

    // Use the constraint library factories to create the constraint
    ConstraintId constraint = ConstraintLibrary::createConstraint(name, 
								  m_planDb->getConstraintEngine(), 
								  scope);
    debugMsg("DbClient:createConstraint", constraint->toString());
    publish(notifyConstraintCreated(constraint));
    return constraint;
  }

  void DbClient::deleteConstraint(const ConstraintId& constr) {
    publish(notifyConstraintDeleted(constr));
    delete (Constraint*) constr;
  }

  void DbClient::restrict(const ConstrainedVariableId& variable, const AbstractDomain& domain){
    debugMsg("DbClient:restrict", variable->toString() << " to " << domain.toString());
    variable->restrictBaseDomain(domain);
    publish(notifyVariableRestricted(variable));
  }

  void DbClient::specify(const ConstrainedVariableId& variable, double value){
    debugMsg("DbClient:specify", "before:" << variable->toString() << " to " << variable->toString(value));
    variable->specify(value);
    debugMsg("DbClient:specify", "after:" << variable->toString());
    publish(notifyVariableSpecified(variable));
  }

  void DbClient::close(const ConstrainedVariableId& variable){
    variable->close();
    debugMsg("DbClient:close", variable->toString());
    publish(notifyVariableClosed(variable));
  }
  void DbClient::reset(const ConstrainedVariableId& variable){
    debugMsg("DbClient:reset","before:" << variable->toString());
    variable->reset();
    debugMsg("DbClient:reset", "after:" << variable->toString());
    publish(notifyVariableReset(variable));
  }

  bool DbClient::propagate(){
    m_planDb->getConstraintEngine()->propagate();

    return m_planDb->getConstraintEngine()->constraintConsistent();
  }


  ObjectId DbClient::getObject(const char* name) const {return m_planDb->getObject(name);}

  /**
   * @brief Traverse the path and obtain the right token
   */
  TokenId DbClient::getTokenByPath(const std::vector<int>& relativePath) const
  {
    check_error(isTransactionLoggingEnabled());
    check_error(!relativePath.empty());
    check_error(!m_keysOfTokensCreated.empty());
    check_error(relativePath[0] >= 0); // Can never be a valid path

    // Quick check for the root of the path
    if((unsigned)relativePath[0] >= m_keysOfTokensCreated.size()) // Cannot be a path for a token with this key set
      return TokenId::noId();

    // Obtain the root token key using the first element in the path to index the tokenKeys.
    int rootTokenKey = m_keysOfTokensCreated[relativePath[0]];

    // Now source the token as an enityt lookup by key. This works because we have a shared pool
    // of entities per process
    EntityId entity = Entity::getEntity(rootTokenKey);

    if (entity.isNoId())
      return(TokenId::noId());

    TokenId rootToken = entity;
    for (unsigned int i = 1;
         !rootToken.isNoId() && i < relativePath.size();
         i++)
      rootToken = rootToken->getSlave(relativePath[i]);

    return(rootToken);
  }

  const ConstrainedVariableId DbClient::getGlobalVariable(const LabelStr& varName) const{
    return m_planDb->getGlobalVariable(varName);
  }

  bool DbClient::isGlobalVariable(const LabelStr& varName) const {
    return m_planDb->isGlobalVariable(varName);
  }

  /**
   * @brief Build the path from the bottom up, and return it from the top down.
   */
  std::vector<int> DbClient::getPathByToken(const TokenId& targetToken) const
  {
    check_error(isTransactionLoggingEnabled());
    std::list<int> path; // Used to build up the path from the bottom up.

    TokenId master = targetToken->master();
    TokenId slave = targetToken;

    while(!master.isNoId()){
      int slavePosition = master->getSlavePosition(slave);
      check_error(slavePosition >= 0);
      path.push_front(slavePosition);
      slave = slave->master();
      master = master->master();
    }

    // Loop terminates where slave is the root, so get the masters key from the slave pointer.
    int keyOfMaster = slave->getKey();

    // Now we must obtain a key value based on relative position in the sequence of created master
    // tokens. This is done so that we can use the path to replay transactions, but resulting in different
    // key values.
    int indexOfMaster = -1;
    for(unsigned int i=0; i< m_keysOfTokensCreated.size(); i++)
      if(m_keysOfTokensCreated[i] == keyOfMaster){
	indexOfMaster = i;
	break;
      }

    check_error(indexOfMaster >= 0);

    // Now push the key for the root and generate path going from top down
    path.push_front(indexOfMaster);

    std::vector<int> pathAsVector;
    for(std::list<int>::const_iterator it = path.begin(); it != path.end(); ++it)
      pathAsVector.push_back(*it);

    return pathAsVector;
  }

  std::string DbClient::getPathAsString(const TokenId& targetToken) const {
    check_error(isTransactionLoggingEnabled());
    const std::vector<int> path = getPathByToken(targetToken);
    std::stringstream s;
    std::vector<int>::const_iterator it = path.begin();
    s << *it;
    for(++it ; it != path.end() ; ++it) {
      s << "." << *it;
    }
    return s.str();
  }

  ConstrainedVariableId DbClient::getVariableByIndex(unsigned int index){
    return m_planDb->getConstraintEngine()->getVariable(index);
  }

  unsigned int DbClient::getIndexByVariable(const ConstrainedVariableId& var){
    return m_planDb->getConstraintEngine()->getIndex(var);
  }

  ConstraintId DbClient::getConstraintByIndex(unsigned int index) {
    return m_planDb->getConstraintEngine()->getConstraint(index);
  }

  unsigned int DbClient::getIndexByConstraint(const ConstraintId& constr) {
    return m_planDb->getConstraintEngine()->getIndex(constr);
  }

  void DbClient::notifyAdded(const DbClientListenerId& listener){
    check_error(m_listeners.find(listener) == m_listeners.end());
    m_listeners.insert(listener);
  }

  void DbClient::notifyRemoved(const DbClientListenerId& listener){
    check_error(m_listeners.find(listener) != m_listeners.end());
    if(!m_deleted){
      m_listeners.erase(listener);
    }
  }

  bool DbClient::constraintConsistent() const {
    return m_planDb->getConstraintEngine()->constraintConsistent();
  }

  bool DbClient::supportsAutomaticAllocation() const{
    return TokenFactory::hasFactory();
  }

  TokenId DbClient::allocateToken(const LabelStr& predicateName, bool rejectable, bool isFact) {
    checkError(supportsAutomaticAllocation(), "Cannot allocate tokens from the schema.");
    TokenId token = TokenFactory::createInstance(m_planDb, predicateName, rejectable, isFact);

    if (isTransactionLoggingEnabled()) {
      debugMsg("DbClient:allocateToken",
	       "Saving token key " << token->getKey());
      m_keysOfTokensCreated.push_back(token->getKey());
    }

    checkError(token.isValid(), "Failed to allocate token for " << predicateName.toString());
    return token;
  }

  void DbClient::enableTransactionLogging() {
    check_error(!isTransactionLoggingEnabled());
    m_transactionLoggingEnabled = true;
  }

  void DbClient::disableTransactionLogging() {
    check_error(isTransactionLoggingEnabled());
    m_transactionLoggingEnabled = false;
  }

  bool DbClient::isTransactionLoggingEnabled() const { return m_transactionLoggingEnabled; }
}
