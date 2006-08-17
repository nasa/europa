#include "Utils.hh"
#include "ConstraintLibrary.hh"
#include "UnifyMemento.hh"
#include "Token.hh"
#include "TokenVariable.hh"
#include "Object.hh"
#include "Schema.hh"
#include "Rule.hh"
#include "RuleVariableListener.hh"
#include "RuleInstance.hh"
#include "Debug.hh"
#include "ProxyVariableRelation.hh"
#include "BoolDomain.hh"
#include <sstream>

namespace EUROPA {

  RuleInstance::RuleInstance(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb)
    : m_id(this), m_rule(rule), m_token(token), m_planDb(planDb), m_rulesEngine(), m_guardDomain(0), m_isExecuted(false), m_isPositive(true){
    check_error(rule.isValid(), "Parent must be a valid rule id.");
    check_error(isValid());
    commonInit();
  }

  RuleInstance::RuleInstance(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb, 
			     const std::vector<ConstrainedVariableId>& guards)
    : m_id(this), m_rule(rule), m_token(token), m_planDb(planDb), m_rulesEngine(), m_guardDomain(0), m_isExecuted(false), m_isPositive(true){
    check_error(isValid());
    setGuard(guards);
    commonInit();
  }

  RuleInstance::RuleInstance(const RuleId& rule, const TokenId& token, const PlanDatabaseId& planDb, 
			     const ConstrainedVariableId& guard, const AbstractDomain& domain)
    : m_id(this), m_rule(rule), m_token(token), m_planDb(planDb), m_rulesEngine(), m_guardDomain(0), m_isExecuted(false), m_isPositive(true){
    check_error(isValid());
    setGuard(guard, domain);
    commonInit();
  }

  /**
   * @brief Constructor refers to parent for tokens, and variables that are accessible in its scope.
   */
  RuleInstance::RuleInstance(const RuleInstanceId& parent, const std::vector<ConstrainedVariableId>& guards)
    : m_id(this), m_rule(parent->getRule()), m_token(parent->getToken()), 
    m_planDb(parent->getPlanDatabase()),m_rulesEngine() , m_parent(parent), m_guardDomain(0), m_isExecuted(false), m_isPositive(true){
    setGuard(guards);
  }

  /**
   * @brief Constructor refers to parent for tokens, and variables that are accessible in its scope.
   */
  RuleInstance::RuleInstance(const RuleInstanceId& parent, const std::vector<ConstrainedVariableId>& guards, const bool positive)
    : m_id(this), m_rule(parent->getRule()), m_token(parent->getToken()), 
    m_planDb(parent->getPlanDatabase()),m_rulesEngine() , m_parent(parent), m_guardDomain(0), m_isExecuted(false), m_isPositive(positive){
    setGuard(guards);
  }

  /**
   * @brief Constructor refers to parent for tokens, and variables that are accessible in its scope.
   */
  RuleInstance::RuleInstance(const RuleInstanceId& parent, const ConstrainedVariableId& guard, const AbstractDomain& domain)
    : m_id(this), m_rule(parent->getRule()), m_token(parent->getToken()), 
    m_planDb(parent->getPlanDatabase()), m_rulesEngine(), m_parent(parent), m_guardDomain(0), m_isExecuted(false), m_isPositive(true){
    setGuard(guard, domain);
  }

  /**
   * @brief Constructor refers to parent for tokens, and variables that are accessible in its scope.
   */
  RuleInstance::RuleInstance(const RuleInstanceId& parent, const ConstrainedVariableId& guard, const AbstractDomain& domain, const bool positive)
    : m_id(this), m_rule(parent->getRule()), m_token(parent->getToken()), 
    m_planDb(parent->getPlanDatabase()), m_rulesEngine(), m_parent(parent), m_guardDomain(0), m_isExecuted(false), m_isPositive(positive){
    setGuard(guard, domain);
  }

  /**
   * @brief Clean up all the allocated elements
   */
  RuleInstance::~RuleInstance(){
    discard(false);

    // We do not delete the guard, since we NEVER allocate it. Always allocated in the parent.
    m_id.remove();
  }

  void RuleInstance::handleDiscard(){
    if(isExecuted())
      undo();

    // If there is a guard domain, delete it
    if(m_guardDomain != 0)
      delete m_guardDomain;

    // Delete the guard listener if we are not in purge mode. Purge mode will handle the deletion
    // of the constraint in the Constraint Engine
    if(!m_guardListener.isNoId() && !Entity::isPurging())
      m_guardListener->discard();

    Entity::handleDiscard();
  }

  const RuleInstanceId& RuleInstance::getId() const{return m_id;}

  const RuleId& RuleInstance::getRule() const {return m_rule;}

  const PlanDatabaseId& RuleInstance::getPlanDatabase() const {return m_planDb;}

  const TokenId& RuleInstance::getToken() const {return m_token;}

  const std::vector<TokenId> RuleInstance::getSlaves() const 
  {return std::vector<TokenId>(m_slaves);}

  bool RuleInstance::isExecuted() const {
    return m_isExecuted;
  }

  const RulesEngineId &RuleInstance::getRulesEngine() const {return m_rulesEngine;}

  void RuleInstance::setRulesEngine(const RulesEngineId &rulesEngine) {
    check_error(m_rulesEngine.isNoId());
    m_rulesEngine = rulesEngine;
    if(test())
      execute();
  }

  /**
   * Cases for TEST:
   * 1. No guard specified
   * 2. No guard value specified
   * 3. Guard and guard value specifed
   */
  bool RuleInstance::test() const {
    debugMsg("RuleInstance:test", "Testing rule " << m_id << " for " << m_rule->getName().toString() << " from " << m_rule->getSource().toString());
    if(m_guardDomain != 0) { // Case of explicit guard on a single variable
      debugMsg("RuleInstance:test", "Case of explicit guard on a single variable");
      checkError(m_guards.size() == 1, "Explcit guard on one variable only");
      bool result = m_guards[0]->isSpecified() &&
      (m_guardDomain->isMember(m_guards[0]->getSpecifiedValue()) ^ !m_isPositive);
      
      debugMsg("RuleInstance:test", "variable " << m_guards[0]->getId()
	       << " name " << m_guards[0]->toString()
	       << " specified " << m_guards[0]->isSpecified()
	       << " guard domain " << *m_guardDomain
	       << (result ? " passed" : " failed"));
      condDebugMsg(!m_guards[0]->isSpecified(), "RuleInstance:test", 
		   "Guard " << m_guards[0]->toString() << " not specified.");
      condDebugMsg(m_guards[0]->isSpecified() && !m_guardDomain->isMember(m_guards[0]->getSpecifiedValue()), "RuleInstance:test", 
		   "Specified value '" << m_guards[0]->getSpecifiedValue() << "' of guard " << m_guards[0]->toString() << " not in guard domain " << *m_guardDomain);
      
      return result;
    }

    // Otherwise, we must be dealing with implied singleton guards
    debugMsg("RuleInstance:test", "Implied singleton or no guards case.");
    int counter = 0;
    for(std::vector<ConstrainedVariableId>::const_iterator it = m_guards.begin(); it != m_guards.end(); ++it){
      ConstrainedVariableId guard = *it;
      check_error(guard.isValid());
			check_error(m_isPositive); // negative testing isn't allowed on singleton guards.

      debugMsg("RuleInstance:test", "checking  " << counter << " argument:" << guard->toString());

      if(!guard->isSpecified()){
        debugMsg("RuleInstance:test", "argument " << counter << " is not specified " << guard->baseDomain().toString());
	return false;
      }

      ++counter;
    }

    debugMsg("RuleInstance:test", "Rule passed");
    return true; // All passed
  }

  void RuleInstance::execute() {
    check_error(!isExecuted(), "Cannot execute a rule if already executed.");
    m_isExecuted = true;
    handleExecute();
    m_rulesEngine->notifyExecuted(getId());
  }

  /**
   * Delete any allocated elements due to firing, and reset status
   */
  void RuleInstance::undo(){
    check_error(isExecuted(), "Cannot undo a rule if not already executed.");

    if(!Entity::isPurging()){
      m_rulesEngine->notifyUndone(getId());
      // Clear slave lookups
      m_slavesByName.clear();

      // Clear variable lookups - may include token variables so we have to be careful
      for(std::vector<ConstrainedVariableId>::const_iterator it = m_variables.begin(); it != m_variables.end(); ++it){
	ConstrainedVariableId var = *it;
	checkError(var.isValid(), var);
	double key = var->getName().getKey();
	m_variablesByName.erase(key);
      }

      // Copy collection to avoid iterator changing due to call back
      std::vector<ConstraintId> constraints = m_constraints;
      for(std::vector<ConstraintId>::const_iterator it = constraints.begin(); it != constraints.end(); ++it){
	ConstraintId constraint = *it;
	checkError(constraint.isValid(), constraint);
	constraint->discard();
      }

      m_constraints.clear();

      cleanup(m_childRules);

      // Clean up slaves if not already de-allocated. Copy collection to avoid call back changing the set of
      // slaves
      std::vector<TokenId> slaves = m_slaves;
      for(std::vector<TokenId>::const_iterator it = slaves.begin(); it != slaves.end(); ++it){
	TokenId slave = *it;
	checkError(slave.isValid(), slave);
	TokenId master = slave->getMaster();
	checkError(master.isNoId() || master == m_token, master);

	// Remove the dependent since the slave MAY NOT GO AWAY
	slave->removeDependent(this);

	if(master.isId())
	  slave->removeMaster(m_token);
      }
      m_slaves.clear();

      // Cleanup local variables
      for(std::vector<ConstrainedVariableId>::const_iterator it = m_variables.begin();
	  it != m_variables.end();
	  ++it){
	ConstrainedVariableId var = *it;
	getToken()->removeLocalVariable(var);

	if(var->getParent() == m_id)
	  var->discard();
      }
      m_variables.clear();
      m_isExecuted = false;
    }
    else // Just clean up child rules
      cleanup(m_childRules);
  }

  void RuleInstance::setGuard(const std::vector<ConstrainedVariableId>& guards){
    check_error(m_guards.empty());
    m_guards = guards;
    m_guardListener = (new RuleVariableListener(m_planDb->getConstraintEngine(), m_id, m_guards))->getId();
  }

  void RuleInstance::setGuard(const ConstrainedVariableId& guard, const AbstractDomain& domain){
    check_error(m_guards.empty());
    check_error(guard.isValid());
    m_guards.push_back(guard);
    check_error(AbstractDomain::canBeCompared(guard->baseDomain(), domain),
					      "Failed attempt to compare " + guard->baseDomain().getTypeName().toString() +
					      " with " + domain.getTypeName().toString());
    m_guardDomain = domain.copy();
    m_guardListener = (new RuleVariableListener(m_planDb->getConstraintEngine(), m_id, m_guards))->getId();
  }

  TokenId RuleInstance::addSlave(Token* slave){
    m_slaves.push_back(slave->getId());
    slave->addDependent((Entity*) this);

    return slave->getId();
  }

  void RuleInstance::addVariable(const ConstrainedVariableId& var, const LabelStr& name){
    check_error(var.isValid(), "Tried to add invalid variable " + name.toString());
    m_variablesByName.insert(std::make_pair(name, var));
    getToken()->addLocalVariable(var);
  }

  /**
   * This is going to be slow as we iterate over a load of variables and do string manipulate in them. Could optimize
   * if this seems a problem.
   */
  void RuleInstance::clearLoopVar(const LabelStr& loopVarName){
    std::map<double, ConstrainedVariableId>::iterator it = m_variablesByName.begin();
    while (it != m_variablesByName.end()){
      const LabelStr& name = it->first;
      const ConstrainedVariableId& var = it->second;
      // If we get a match straight away, remove the entry.
      if(var->getParent() == getId() && (name == loopVarName ||	(name.countElements(".") > 0 && loopVarName == name.getElement(0, "."))))
	m_variablesByName.erase(it++);
      else
	++it;
    }
  }

  /**
   * @see addVariable
   */
  TokenId RuleInstance::addSlave(Token* slave, const LabelStr& name){

    // As with adding variables, we have to handle case of re-use of name when exeuting the inner
    // loop of 'foreach'
    m_slavesByName.erase(name.getKey());

    m_slavesByName.insert(std::pair<double, TokenId>(name.getKey(), slave->getId()));
    return addSlave(slave);
  }

  void RuleInstance::addConstraint(const LabelStr& name, std::vector<ConstrainedVariableId>& scope){
    ConstraintId constraint =  ConstraintLibrary::createConstraint(name,
								   getPlanDatabase()->getConstraintEngine(),
								   scope);
    addConstraint(constraint);
  }

  void RuleInstance::addConstraint(const ConstraintId& constraint){
    m_constraints.push_back(constraint);
    const LabelStr& name = constraint->getName();
    m_constraintsByName.erase(name.getKey());
    m_constraintsByName.insert(std::pair<double, ConstraintId>(name.getKey(), constraint));
    constraint->addDependent((Entity*) this);
  }

  void RuleInstance::addChildRule(RuleInstance* instance){
    m_childRules.push_back(instance->getId());
    instance->setRulesEngine(getRulesEngine());
  }

  bool RuleInstance::isValid() const{
    check_error(m_rule.isValid());
    check_error(m_token.isValid());
    check_error(m_token->isActive(), 
		m_token->getPredicateName().toString() + " is not active");
    check_error(Schema::instance()->isA(m_token->getPredicateName(), m_rule->getName()),
		"Cannot have rule " + m_rule->getName().toString() + 
		" on predicate " + m_token->getPredicateName().toString());
    return true;
  }

  ConstrainedVariableId RuleInstance::getVariable(const LabelStr& name) const{
    std::map<double, ConstrainedVariableId>::const_iterator it = m_variablesByName.find(name.getKey());
    if(it != m_variablesByName.end())
      return it->second;
    else if (!m_parent.isNoId())
      return m_parent->getVariable(name);
    else if(getPlanDatabase()->isGlobalVariable(name))
      return getPlanDatabase()->getGlobalVariable(name);
    else
      return ConstrainedVariableId::noId();
  }

  TokenId RuleInstance::getSlave(const LabelStr& name) const{
    static const LabelStr sl_this("this");
    // Special handling for 'this'
    if(name == sl_this)
      return m_token;

    std::map<double, TokenId>::const_iterator it = m_slavesByName.find(name.getKey());
    if(it != m_slavesByName.end())
      return it->second;
    else if (!m_parent.isNoId())
      return m_parent->getSlave(name);
    else
      return TokenId::noId();
  }


  ConstraintId RuleInstance::getConstraint(const LabelStr& name) const{
    std::map<double, ConstraintId>::const_iterator it = m_constraintsByName.find(name.getKey());
    if(it != m_constraintsByName.end())
      return it->second;
    else if (!m_parent.isNoId())
      return m_parent->getConstraint(name);
    else
      return ConstraintId::noId();
  }

  ConstraintId RuleInstance::constraint(const std::string& name) const{
    return getConstraint(LabelStr(name));
  }


  void RuleInstance::commonInit(){
    const std::vector<ConstrainedVariableId>& vars = m_token->getVariables();
    for(std::vector<ConstrainedVariableId>::const_iterator it = vars.begin(); it != vars.end(); ++it){
      ConstrainedVariableId var = *it;
      m_variablesByName.insert(std::pair<double, ConstrainedVariableId>(var->getName().getKey(), var));
    }
  }

  std::vector<ConstrainedVariableId> RuleInstance::getVariables(const std::string& delimitedVars) const{
    std::vector<ConstrainedVariableId> scope;
    LabelStr strScope(delimitedVars);
    unsigned int size = strScope.countElements(":");
    for(unsigned int i=0; i < size; i++){
      LabelStr strVar = strScope.getElement(i, ":");
      ConstrainedVariableId var;

      // It is possible that the label is of the form "predicate.member" in which case we
      // must obtain the token from current scope and then getthe variable from there.
      if(strVar.countElements(".") == 1)
	var = getVariable(strVar);
      else {
	LabelStr slaveName = strVar.getElement(0, ".");
	LabelStr varName = strVar.getElement(1, ".");
	TokenId slave = getSlave(slaveName);
	check_error(slave.isValid());
	var = slave->getVariable(varName);
      }
      check_error(var.isValid());
      scope.push_back(var);
    }

    return scope;
  }

  ConstrainedVariableId RuleInstance::varFromObject(const std::string& objectString, 
						    const std::string& varString,
						    bool canBeSpecified){
    std::string fullName = objectString + "." + varString;
    ConstrainedVariableId retVar = getVariable(fullName);
    if(retVar.isNoId())
      retVar = varFromObject(getVariable(objectString), varString, fullName, canBeSpecified);

    return retVar;
  }

  ConstrainedVariableId RuleInstance::varFromObject(const ConstrainedVariableId& obj, 
						    const std::string& varString,
						    const std::string& fullName,
						    bool canBeSpecified){
    std::vector<std::string> names;
    tokenize(varString, names, std::string(Schema::getDelimiter()));
    unsigned int varindex = 0;

    // First we compute the position index, and get the type of the last variable. This will then
    // be used to populate the base domain of the proxy variable by iteration over the base domain.
    
    // Initialize with any object in the domain
    ObjectId iObj = obj->lastDomain().getLowerBound();
    std::vector<unsigned int> path; /*!< Push indexes as they are found */

    // Traverse the object structure using the names in each case. Store indexes as we go to build a path
    for (; varindex < names.size()-1; ++varindex) {
      ConstrainedVariableId iVar = iObj->getVariable(LabelStr(iObj->getName().toString() + "." + names[varindex]));
      path.push_back(iVar->getIndex());
      checkError(iVar->lastDomain().isSingleton(), iVar->toString());
      iObj = iVar->lastDomain().getSingletonValue();
    }

    // Finally, handle the terminal point - the field variable itself
    ConstrainedVariableId fieldVar = iObj->getVariable(LabelStr(iObj->getName().toString() + "." + names[varindex]));
    path.push_back(fieldVar->getIndex());

    // Get the field type for the resulting domain.
    const LabelStr& fieldType = fieldVar->baseDomain().getTypeName();
    const bool isNumeric = fieldVar->baseDomain().isNumeric();
    const bool isOpen = fieldVar->baseDomain().isOpen();
    const bool isBool = fieldVar->baseDomain().isBool();

    // At this point, the index is complete, and we know the type. We can allocate and populate the domain
    const ObjectDomain& objectDomain = static_cast<const ObjectDomain&>(obj->baseDomain());

    // Iterate over each object. For each, obtain the variable using the path, and store its value
    const std::list<ObjectId> objects = objectDomain.makeObjectList();

    EnumeratedDomain proxyBaseDomain(isNumeric, fieldType.c_str());

    std::list<double> values;
    for(std::list<ObjectId>::const_iterator it = objects.begin(); it != objects.end(); ++it){
      ObjectId object = *it;
      ConstrainedVariableId fieldVar = object->getVariable(path);
      checkError(fieldVar->lastDomain().isSingleton(), fieldVar->toString());
      checkError(fieldVar->baseDomain().getTypeName() == fieldType, fieldVar->toString());
      double value = fieldVar->lastDomain().getSingletonValue();
      proxyBaseDomain.insert(value);
      debugMsg("RuleInstance:varFromObject", "Adding value from " << fieldVar->toString());
    }

    // Allocate the proxy variable
    ConstrainedVariableId proxyVariable;

    // If it is a boolean, allocate a bool domain instead of the enumerated domain
    if(isBool){
      BoolDomain boolDomain(fieldType.c_str());
      double lb = proxyBaseDomain.getLowerBound();
      double ub = proxyBaseDomain.getUpperBound();

      // If a singleton, set as such
      if(lb == ub)
	boolDomain.set(ub);

      proxyVariable = addVariable(boolDomain, canBeSpecified, fullName);
    }
    else {
      // Close if necessary
      if(!isOpen)
	proxyBaseDomain.close();

      proxyVariable = addVariable(proxyBaseDomain, canBeSpecified, fullName);
    }

    // Post the new constraint
    ConstraintId proxyVariableRelation = (new ProxyVariableRelation(obj, proxyVariable, path))->getId();
    addConstraint(proxyVariableRelation);

    // Return the new variable
    return proxyVariable;
  }

  ConstrainedVariableId RuleInstance::varfromtok(const TokenId& token, const std::string varstring) {
    std::string local_name = varstring.substr(0, varstring.find(Schema::getDelimiter()));
    ConstrainedVariableId retVar;
    if (varstring.find(Schema::getDelimiter()) == std::string::npos) {
      retVar = token->getVariable(local_name);
    }
    else {
      std::stringstream fullName;
      fullName << token->getKey() << ":" << varstring;
      ConstrainedVariableId obj = token->getVariable(local_name);
      std::string suffix = varstring.substr(varstring.find(Schema::getDelimiter())+1, varstring.size());
      retVar = varFromObject(obj, suffix, fullName.str());
    }

    checkError(retVar.isValid(), "Failed to retrieve " << varstring << " from token " << token->toString());
    return retVar; //
  }

  void RuleInstance::notifyDiscarded(const Entity* entity){
    bool isToken = false;

    // If not a constraint, then it must be a token
    if(dynamic_cast<const Constraint*>(entity) == 0)
      isToken = true;

    checkError(dynamic_cast<const Token*>(entity) != 0 || dynamic_cast<const Constraint*>(entity),
	       "Must be a constraint or a token: " << entity->getKey());
 
    // Is it a slave? If so, reference to it
    if(isToken){
      for(std::vector<TokenId>::iterator it = m_slaves.begin(); it != m_slaves.end(); ++it){
	TokenId token = *it;
	checkError(token.isValid(), token);
	if(token->getKey() == entity->getKey()){
	  m_slaves.erase(it);
	  return;
	}
      }
    }
    else {
      for(std::vector<ConstraintId>::iterator it = m_constraints.begin(); it != m_constraints.end(); ++it){
	ConstraintId constraint = *it;
	checkError(constraint.isValid(), constraint);
	if(constraint->getKey() == entity->getKey()){
	  m_constraints.erase(it);
	  return;
	}
      }
    }
  }
}
