#include "Utils.hh"
#include "ConstraintType.hh"
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
#include "Domains.hh"
#include <sstream>

#include <boost/algorithm/string.hpp>
namespace EUROPA {

RuleInstance::RuleInstance(const RuleId rule, const TokenId token, 
                           const PlanDatabaseId planDb)
    : m_id(this), m_rule(rule), m_token(token), m_planDb(planDb), m_rulesEngine(), 
      m_parent(), m_guards(),
      m_guardDomain(0), m_guardListener(), m_isExecuted(false), m_isPositive(true),
      m_constraints(), m_childRules(), m_variables(), m_slaves(), 
      m_variablesByName(), m_slavesByName(),
      m_constraintsByName() {
  check_error(rule.isValid(), "Parent must be a valid rule id.");
  check_error(isValid());
  commonInit();
}

RuleInstance::RuleInstance(const RuleId rule, const TokenId token, 
                           const PlanDatabaseId planDb,
                           const std::vector<ConstrainedVariableId>& guards)
    : m_id(this), m_rule(rule), m_token(token), m_planDb(planDb), m_rulesEngine(),
      m_parent(), m_guards(),
      m_guardDomain(0), m_guardListener(), m_isExecuted(false), m_isPositive(true),
      m_constraints(), m_childRules(), m_variables(), m_slaves(), m_variablesByName(),
      m_slavesByName(), m_constraintsByName() {
  check_error(isValid());
  setGuard(guards);
  commonInit();
}

RuleInstance::RuleInstance(const RuleId rule, const TokenId token,
                           const PlanDatabaseId planDb,
                           const ConstrainedVariableId guard, const Domain& domain)
    : m_id(this), m_rule(rule), m_token(token), m_planDb(planDb), m_rulesEngine(),
      m_parent(), m_guards(),
      m_guardDomain(0), m_guardListener(), m_isExecuted(false), m_isPositive(true),
      m_constraints(), m_childRules(), m_variables(), m_slaves(), m_variablesByName(), 
      m_slavesByName(), m_constraintsByName() {
  check_error(isValid());
  setGuard(guard, domain);
  commonInit();
}

/**
 * @brief Constructor refers to parent for tokens, and variables that are accessible in its scope.
 */
RuleInstance::RuleInstance(const RuleInstanceId parent, 
                           const std::vector<ConstrainedVariableId>& guards)
    : m_id(this), m_rule(parent->getRule()), m_token(parent->getToken()),
      m_planDb(parent->getPlanDatabase()),m_rulesEngine() , m_parent(parent), 
      m_guards(), m_guardDomain(0), m_guardListener(), m_isExecuted(false),
      m_isPositive(true), m_constraints(), m_childRules(), m_variables(), m_slaves(), 
      m_variablesByName(), m_slavesByName(), m_constraintsByName() {
  check_error(isValid());
  setGuard(guards);
}

/**
 * @brief Constructor refers to parent for tokens, and variables that are accessible in its scope.
 */
RuleInstance::RuleInstance(const RuleInstanceId parent, 
                           const std::vector<ConstrainedVariableId>& guards, 
                           const bool positive)
    : m_id(this), m_rule(parent->getRule()), m_token(parent->getToken()),
      m_planDb(parent->getPlanDatabase()), m_rulesEngine(), m_parent(parent), 
      m_guards(), m_guardDomain(0), m_guardListener(), m_isExecuted(false),
      m_isPositive(positive), m_constraints(), m_childRules(), m_variables(),
      m_slaves(), m_variablesByName(), m_slavesByName(), m_constraintsByName() {
  check_error(isValid());
  setGuard(guards);
}

/**
 * @brief Constructor refers to parent for tokens, and variables that are accessible in its scope.
 */
RuleInstance::RuleInstance(const RuleInstanceId parent,
                           const ConstrainedVariableId guard, const Domain& domain)
    : m_id(this), m_rule(parent->getRule()), m_token(parent->getToken()),
      m_planDb(parent->getPlanDatabase()), m_rulesEngine(), m_parent(parent),
      m_guards(), m_guardDomain(0), m_guardListener(), m_isExecuted(false),
      m_isPositive(true), m_constraints(), m_childRules(), m_variables(), m_slaves(),
      m_variablesByName(), m_slavesByName(), m_constraintsByName() {
  check_error(isValid());
  setGuard(guard, domain);
}

  /**
   * @brief Constructor refers to parent for tokens, and variables that are accessible in its scope.
   */
RuleInstance::RuleInstance(const RuleInstanceId parent, 
                           const ConstrainedVariableId guard,
                           const Domain& domain, const bool positive)
    : m_id(this), m_rule(parent->getRule()), m_token(parent->getToken()),
      m_planDb(parent->getPlanDatabase()), m_rulesEngine(), m_parent(parent), 
      m_guards(), m_guardDomain(0), m_guardListener(), m_isExecuted(false),
      m_isPositive(positive), m_constraints(), m_childRules(), m_variables(), 
      m_slaves(), m_variablesByName(), m_slavesByName(), m_constraintsByName() {
  check_error(isValid());
  setGuard(guard, domain);
}

RuleInstance::RuleInstance(const RuleInstanceId parent, 
                           const ConstrainedVariableId guard,
                           const Domain& domain, const bool positive,
                           const std::vector<ConstrainedVariableId>& guardComponents)
    : m_id(this), m_rule(parent->getRule()), m_token(parent->getToken()),
      m_planDb(parent->getPlanDatabase()), m_rulesEngine(), m_parent(parent), 
      m_guards(), m_guardDomain(0), m_guardListener(), m_isExecuted(false),
      m_isPositive(positive), m_constraints(), m_childRules(), m_variables(), 
      m_slaves(), m_variablesByName(), m_slavesByName(), m_constraintsByName() {
  check_error(isValid());
  setGuard(guard, domain, guardComponents);
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
    checkError(m_token.isValid(), m_token);

    if(isExecuted())
      undo();

    // If there is a guard domain, delete it
    if(m_guardDomain != 0)
      delete m_guardDomain;

    // Delete the guard listener if we are not in purge mode. Purge mode will handle the deletion
    // of the constraint in the Constraint Engine. We can deactivate this constraint since it does not impose a restriction
    // on any variable and thus its removal is not a relaxtion. Since we are already discarding the rule, its implications
    // will have been undone appropriately.
    if(!m_guardListener.isNoId() && !Entity::isPurging()){
      checkError(m_guardListener.isValid(), m_guardListener);
      m_guardListener->removeDependent(this);
      m_guardListener->deactivate();
      m_guardListener->discard();
    }

    Entity::handleDiscard();
  }

  const RuleInstanceId RuleInstance::getId() const{return m_id;}

  const RuleId RuleInstance::getRule() const {return m_rule;}

  const PlanDatabaseId RuleInstance::getPlanDatabase() const {return m_planDb;}

  const TokenId RuleInstance::getToken() const {return m_token;}

  const std::vector<TokenId> RuleInstance::getSlaves() const
  {return std::vector<TokenId>(m_slaves);}

  bool RuleInstance::isExecuted() const {
    return m_isExecuted;
  }

  const RulesEngineId &RuleInstance::getRulesEngine() const {return m_rulesEngine;}

  void RuleInstance::setRulesEngine(const RulesEngineId &rulesEngine) {
    check_error(m_rulesEngine.isNoId());
    m_rulesEngine = rulesEngine;
    if(m_guards.empty())// && test(m_guards))
      execute();
  }

  bool RuleInstance::willNotFire() const{
    for(std::vector<ConstrainedVariableId>::const_iterator it = m_guards.begin(); it != m_guards.end(); ++it){
      ConstrainedVariableId guard = *it;
      if(!guard->lastDomain().isSingleton())
	return false;
    }

    return m_guards.empty() || !test(m_guards);
  }

  /**
   * Cases for TEST:
   * 1. No guard specified
   * 2. No guard value specified
   * 3. Guard and guard value specifed
   */
  bool RuleInstance::test(const std::vector<ConstrainedVariableId>& guards) const {
    checkError(m_rule.isValid(), m_rule);
    debugMsg("RuleInstance:test",
             "Testing rule " << toString() << " for " << m_rule->getName() <<
             " from " << m_rule->getSource());

    if(m_guardDomain != 0) { // Case of explicit guard on a single variable
      debugMsg("RuleInstance:test", "Case of explicit guard on a single variable");
      // checkError(guards.size() == 1, "Explicit guard on one variable only");
      bool result = false;
      //+ 1 here because the first variable may be the one created implicitly for some
      //testEQ or similar constraint and therefore may not be available to the planner
      for(std::vector<ConstrainedVariableId>::const_iterator it = guards.begin() + 1; 
          it != guards.end(); ++it) {
        if(!((*it)->isSpecified() || (*it)->baseDomain().isSingleton()))
          return false;
      }
      if(guards[0]->lastDomain().isSingleton()) {
        result = (m_guardDomain->isMember(guards[0]->lastDomain().getSingletonValue()) == m_isPositive);
      }
      else {
        result = !(guards[0]->lastDomain().intersects(*m_guardDomain) || m_isPositive);
      }

      debugMsg("RuleInstance:test", "variable " << guards[0]->toLongString()
	       << " guard domain " << *m_guardDomain
	       << (result ? " passed" : " failed"));
      condDebugMsg(guards[0]->lastDomain().isSingleton() && !m_guardDomain->isMember(guards[0]->lastDomain().getSingletonValue()), "RuleInstance:test",
		   "Specified value '" << guards[0]->lastDomain().getSingletonValue() << "' of guard " << guards[0]->toString() << " not in guard domain " << *m_guardDomain);

      return result;
    }

    // Otherwise, we must be dealing with implied singleton guards
    debugMsg("RuleInstance:test", "Implied singleton or no guards case.");
    int counter = 0;
    for(std::vector<ConstrainedVariableId>::const_iterator it = guards.begin(); it != guards.end(); ++it){
      ConstrainedVariableId guard = *it;
      checkError(guard.isValid(), guard);
      check_error(m_isPositive); // negative testing isn't allowed on singleton guards.

      debugMsg("RuleInstance:test", "checking  " << counter << " argument:" << guard->toString());

      if(!(guard->isSpecified() || guard->baseDomain().isSingleton())){
        debugMsg("RuleInstance:test", "argument " << counter << " is not specified " << guard->baseDomain().toString());
	return false;
      }

      ++counter;
    }

    debugMsg("RuleInstance:test", "Rule " << toString() << " passed");
    return true; // All passed
  }

  bool RuleInstance::test() const {
    return test(m_guards);
  }

  void RuleInstance::prepare() {
    if(!isExecuted())
      m_rulesEngine->scheduleForExecution(getId());
    else
      m_rulesEngine->scheduleForUndoing(getId());
  }

  void RuleInstance::execute() {
    check_error(!isExecuted(), "Cannot execute a rule if already executed.");
    debugMsg("RuleInstance:execute", "Executing:" << m_rule->toString());
    m_isExecuted = true;
    handleExecute();
    m_rulesEngine->notifyExecuted(getId());
    debugMsg("europa:model", ruleExecutionContext());
    debugMsg("RuleInstance:execute", "Executed:" << m_rule->toString());
  }

  /**
   * Delete any allocated elements due to firing, and reset status
   */
void RuleInstance::undo() {
  check_error(isExecuted(), "Cannot undo a rule if not already executed.");

  // Clear child rules before destroying local entities. This is the reverse order of allocation
  discardAll(m_childRules);

  if(!Entity::isPurging()){
    m_rulesEngine->notifyUndone(getId());
    // Clear slave lookups
    m_slavesByName.clear();

    // Clear variable lookups - may include token variables so we have to be careful
    for(std::vector<ConstrainedVariableId>::const_iterator it = m_variables.begin(); it != m_variables.end(); ++it){
      ConstrainedVariableId var = *it;
      checkError(var.isValid(), var);
      m_variablesByName.erase(var->getName());
    }

    // Copy collection to avoid iterator changing due to call back
    std::vector<ConstraintId> constraints = m_constraints;
    for(std::vector<ConstraintId>::const_iterator it = constraints.begin(); it != constraints.end(); ++it){
      ConstraintId constr = *it;
      checkError(constr.isValid(), constr);

      // Only discard constraints that are connected to the master since the master may persist after the rule has cleaned up
      // but the constraints should not. If it is not connected to the master then it applies to local variables or slave variables.
      // if a full roll-back of the rule occurs, slaves and local variables will be deleted, causing a delete of the attendant constraints.
      // In the event that a slave persists, as cann occur when the master is removed through termination, then the constraints among
      // remaining slaves should also be retained
      if(connectedToToken(constr, m_token)){
        debugMsg("RuleInstance:undo",
                 "Removing connected constraint " << constr->toLongString());
        constr->discard();
      }
      else // If we are not removing the constraint, we must remove the dependency on it
        constr->removeDependent(this);
    }

    m_constraints.clear();

    // Clean up slaves if not already de-allocated. Copy collection to avoid call back changing the set of
    // slaves
    debugMsg("RuleInstance:undo", "Processing slaves");
    std::vector<TokenId> slaves = m_slaves;
    for(std::vector<TokenId>::const_iterator it = slaves.begin(); it != slaves.end(); ++it){
      TokenId slave = *it;
      checkError(slave.isValid(), slave);
      TokenId master = slave->master();
      checkError(master.isNoId() || master == m_token, master);

      // Remove the dependent since the slave MAY NOT GO AWAY
      slave->removeDependent(this);

      if(master.isId())
        slave->removeMaster(m_token);
    }

    m_slaves.clear();

    // Cleanup local variables
    debugMsg("RuleInstance:undo", "Cleaning up local variables");

    for(std::vector<ConstrainedVariableId>::const_iterator it = m_variables.begin();
        it != m_variables.end();
        ++it){
      ConstrainedVariableId var = *it;
      checkError(var.isValid(), var);
      debugMsg("RuleInstance:undo", "Removing " << var->toLongString());
      getToken()->removeLocalVariable(var);

      if(var->parent() == m_id)
        var->discard();

      checkError(var.isValid(), var << " should still be valid after a discard.");
    }
    m_variables.clear();
    m_isExecuted = false;
  }
}

  void RuleInstance::setGuard(const std::vector<ConstrainedVariableId>& guards){
    check_error(m_guards.empty());
    m_guards = guards;
    m_guardListener = (new RuleVariableListener(m_planDb->getConstraintEngine(), m_id, m_guards))->getId();
    m_guardListener->addDependent(this);
    debugMsg("RuleInstance:setGuard", "Added guard: " << m_guardListener->toLongString());
  }

  void RuleInstance::setGuard(const ConstrainedVariableId guard, const Domain& domain){
    check_error(m_guards.empty());
    check_error(guard.isValid());
    m_guards.push_back(guard);
    check_error(Domain::canBeCompared(guard->baseDomain(), domain),
					      "Failed attempt to compare " + guard->baseDomain().getTypeName() +
					      " with " + domain.getTypeName());
    m_guardDomain = domain.copy();
    m_guardListener = (new RuleVariableListener(m_planDb->getConstraintEngine(), m_id, m_guards))->getId();
    m_guardListener->addDependent(this);
    debugMsg("RuleInstance:setGuard", "Added guard: " << m_guardListener->toLongString());
  }

void RuleInstance::setGuard(const ConstrainedVariableId guard, const Domain& domain,
                            const std::vector<ConstrainedVariableId>& guardComponents){
  check_error(m_guards.empty());
  check_error(guard.isValid());
  m_guards.push_back(guard);
  m_guards.insert(m_guards.end(), guardComponents.begin(), guardComponents.end());
  checkError(Domain::canBeCompared(guard->baseDomain(), domain),
             "Failed attempt to compare " << 
             guard->baseDomain().getTypeName()  << " with " <<
             domain.getTypeName());
  m_guardDomain = domain.copy();
  m_guardListener = (new RuleVariableListener(m_planDb->getConstraintEngine(), m_id, 
                                              m_guards))->getId();
  m_guardListener->addDependent(this);
  debugMsg("RuleInstance:setGuard", "Added guard: " << m_guardListener->toLongString());
}

TokenId RuleInstance::addSlave(Token* slave){
  m_slaves.push_back(slave->getId());
  slave->addDependent(this);
  return slave->getId();
}

ConstrainedVariableId RuleInstance::addVariable( const Domain& baseDomain,
                                                 bool canBeSpecified,
                                                 const std::string& name){
  // If there is already a name-value pair for retrieving a variable by name,
  // we erase it. Though we do not erase the actual variable stored in the list since it still
  // has to be cleaned up when the rule instance is undone. This is done reluctantly, since it
  // is based on assumptions that there will be no child rules. This is all required to support the
  // looping construct used to implement the 'foreach' semantics. Therefore, we overwrite the old
  // value with the new value.
  if(!getVariable(name).isNoId()) {
    m_variablesByName.erase(name);

    // Also erase all variables that may be derived from the variable we're removing
    std::string prefix = name + ".";
    std::map<std::string,ConstrainedVariableId>::iterator it = m_variablesByName.begin();
    while (it != m_variablesByName.end()) {
      std::string varName = it->first;
      ++it;
      if (varName.find(prefix)==0)
        m_variablesByName.erase(varName);
    }
  }

  ConstrainedVariableId localVariable = (new Variable<Domain>(m_planDb->getConstraintEngine(),
                                                              baseDomain,
                                                              false, // TODO: Maybe true?
                                                              canBeSpecified,
                                                              name,
                                                              m_id))->getId();
  // Only allowed add a variable for an executed rule instance
  check_error(isExecuted());

  m_variables.push_back(localVariable);
  addVariable(localVariable, name);
  return localVariable;
}

  void RuleInstance::addVariable(const ConstrainedVariableId var, const std::string& name){
    check_error(var.isValid(), "Tried to add invalid variable " + name);
    m_variablesByName.insert(std::make_pair(name, var));
    getToken()->addLocalVariable(var);
  }

  /**
   * This is going to be slow as we iterate over a load of variables and do string manipulate in them. Could optimize
   * if this seems a problem.
   */
void RuleInstance::clearLoopVar(const std::string& loopVarName){
  std::map<std::string, ConstrainedVariableId>::iterator it = m_variablesByName.begin();
  while (it != m_variablesByName.end()){
    const std::string& name = it->first;
    const ConstrainedVariableId var = it->second;
    // If we get a match straight away, remove the entry.
    if(var->parent() == getId() &&
       (name == loopVarName ||
        //(name.countElements(".") > 0 && loopVarName == name.getElement(0, "."))
        (name.find('.') != std::string::npos && loopVarName == name.substr(0, name.find('.')))
        ))
      m_variablesByName.erase(it++);
    else
      ++it;
  }
}

  std::string RuleInstance::makeImplicitVariableName(){
    std::stringstream sstr;
    sstr << "PSEUDO_VARIABLE_" << m_variablesByName.size();
    return sstr.str();
  }

  /**
   * @see addVariable
   */
  TokenId RuleInstance::addSlave(Token* slave, const std::string& name){

    // As with adding variables, we have to handle case of re-use of name when executing the inner
    // loop of 'foreach'
    m_slavesByName.erase(name);

    m_slavesByName.insert(std::make_pair(name, slave->getId()));
    return addSlave(slave);
  }

void RuleInstance::addConstraint(const std::string& name, const std::vector<ConstrainedVariableId>& scope){
  ConstraintId constr =  
      getPlanDatabase()->getConstraintEngine()->createConstraint(name,
                                                                 scope);
  addConstraint(constr);
}

void RuleInstance::addConstraint(const ConstraintId constr){
  m_constraints.push_back(constr);
  const std::string& name = constr->getName();
  m_constraintsByName.erase(name);
  m_constraintsByName.insert(std::make_pair(name, constr));
  constr->addDependent(this);
  debugMsg("RuleInstance:addConstraint",
           "added constraint:" << constr->toString());
}

  void RuleInstance::addChildRule(RuleInstance* instance){
    m_childRules.push_back(instance->getId());
    instance->setRulesEngine(getRulesEngine());
  }

  bool RuleInstance::isValid() const{
    check_error(m_rule.isValid());
    check_error(m_token.isValid());
    check_error(m_token->isActive(),
		m_token->getPredicateName() + " is not active");
    check_error(m_planDb->getSchema()->isA(m_token->getPredicateName(), m_rule->getName()),
		"Cannot have rule " + m_rule->getName() +
		" on predicate " + m_token->getPredicateName());
    return true;
  }

ConstrainedVariableId RuleInstance::getVariable(const std::string& name) const {
  std::map<std::string, ConstrainedVariableId>::const_iterator it =
      m_variablesByName.find(name);
  if(it != m_variablesByName.end())
    return it->second;
  else if (!m_parent.isNoId())
    return m_parent->getVariable(name);
  else if(getPlanDatabase()->isGlobalVariable(name))
    return getPlanDatabase()->getGlobalVariable(name);
  else
    return ConstrainedVariableId::noId();
}

TokenId RuleInstance::getSlave(const std::string& name) const {
  static const std::string sl_this("this");
  // Special handling for 'this'
  if(name == sl_this)
    return m_token;

  std::map<std::string, TokenId>::const_iterator it = m_slavesByName.find(name);
  if(it != m_slavesByName.end())
    return it->second;
  else if (!m_parent.isNoId())
    return m_parent->getSlave(name);
  else
    return TokenId::noId();
}


ConstraintId RuleInstance::getConstraint(const std::string& name) const {
  std::map<std::string, ConstraintId>::const_iterator it = m_constraintsByName.find(name);
  if(it != m_constraintsByName.end())
    return it->second;
  else if (!m_parent.isNoId())
    return m_parent->getConstraint(name);
  else
    return ConstraintId::noId();
}

  ConstraintId RuleInstance::constraint(const std::string& name) const{
    return getConstraint(name);
  }


void RuleInstance::commonInit() {
  const std::vector<ConstrainedVariableId>& vars = m_token->getVariables();
  for(std::vector<ConstrainedVariableId>::const_iterator it = vars.begin(); it != vars.end(); ++it){
    ConstrainedVariableId var = *it;
    m_variablesByName.insert(std::make_pair(var->getName(), var));
  }
}

std::vector<ConstrainedVariableId> RuleInstance::getVariables(const std::string& delimitedVars) const{
  std::vector<ConstrainedVariableId> scope;
  std::vector<std::string> strScope;
  boost::split(strScope, delimitedVars, boost::is_any_of(":"));
  for(std::vector<std::string>::const_iterator it = strScope.begin(); it != strScope.end();
      ++it) {
    const std::string& strVar = *it;
    ConstrainedVariableId var;
    std::string::size_type dot = strVar.find(".");
    // It is possible that the label is of the form "predicate.member" in which case we
    // must obtain the token from current scope and then getthe variable from there.
    if(dot == std::string::npos)
      var = getVariable(strVar);
    else {
      std::string slaveName = strVar.substr(0, dot);
      std::string varName = strVar.substr(dot + 1);
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

ConstrainedVariableId RuleInstance::varFromObject(const ConstrainedVariableId obj,
                                                  const std::string& varString,
                                                  const std::string& fullName,
                                                  bool canBeSpecified){
  std::vector<std::string> names;
  boost::split(names, varString, boost::is_any_of("."));

  unsigned int varindex = 0;

  // First we compute the position index, and get the type of the last variable. This will then
  // be used to populate the base domain of the proxy variable by iteration over the base domain.

  // Initialize with any object in the domain
  ObjectId iObj = Entity::getTypedEntity<Object>(obj->lastDomain().getLowerBound());
  std::vector<unsigned int> path; /*!< Push indexes as they are found */

  // Traverse the object structure using the names in each case. Store indexes as we go to build a path
  for (; varindex < names.size()-1; ++varindex) {
    ConstrainedVariableId iVar = iObj->getVariable(iObj->getName() + "." + names[varindex]);
    path.push_back(static_cast<unsigned int>(iVar->getIndex()));
    checkError(iVar->lastDomain().isSingleton(), iVar->toString() + ", " + iObj->getName() + "." + names[varindex]);
    iObj = Entity::getTypedEntity<Object>(iVar->lastDomain().getSingletonValue());
  }

  // Finally, handle the terminal point - the field variable itself
  std::string field_name = iObj->getName() + "." + names[varindex];
  ConstrainedVariableId fieldVar = iObj->getVariable(field_name);
  checkError(fieldVar.isValid(), "No variable named '" << field_name << "' in " << iObj->getName());
  path.push_back(static_cast<unsigned int>(fieldVar->getIndex()));

  // Get the field type for the resulting domain.
  const bool isOpen = fieldVar->baseDomain().isOpen();
  const bool isBool = fieldVar->baseDomain().isBool();

  // At this point, the index is complete, and we know the type. We can allocate and populate the domain
  const ObjectDomain& objectDomain = static_cast<const ObjectDomain&>(obj->baseDomain());

  // Iterate over each object. For each, obtain the variable using the path, and store its value
  const std::list<ObjectId> objects = objectDomain.makeObjectList();

  EnumeratedDomain proxyBaseDomain(fieldVar->baseDomain().getDataType());

  std::list<edouble> values;
  for(std::list<ObjectId>::const_iterator it = objects.begin(); it != objects.end(); ++it){
    ObjectId object = *it;
    ConstrainedVariableId objectFieldVar = object->getVariable(path);
    checkError(objectFieldVar->lastDomain().isSingleton(),
               objectFieldVar->toString() << " : " <<
               objectFieldVar->lastDomain().toString() << " is not a singleton.");
    edouble value = objectFieldVar->lastDomain().getSingletonValue();
    proxyBaseDomain.insert(value);
    debugMsg("RuleInstance:varFromObject", "Adding value from " << objectFieldVar->toString());
  }

  // Allocate the proxy variable
  ConstrainedVariableId proxyVariable;

  // If it is a boolean, allocate a bool domain instead of the enumerated domain
  if(isBool){
    BoolDomain boolDomain(fieldVar->baseDomain().getDataType());
    edouble lb = proxyBaseDomain.getLowerBound();
    edouble ub = proxyBaseDomain.getUpperBound();

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

  ConstrainedVariableId RuleInstance::varfromtok(const TokenId token, const std::string varstring) {
    std::string local_name = varstring.substr(0, varstring.find(Schema::getDelimiter()));
    checkError(token.isValid(), "Cannot get variable : " << varstring << " from token with id " << token);
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

  checkError(dynamic_cast<const Token*>(entity) != 0 || dynamic_cast<const Constraint*>(entity),
             "Must be a constraint or a token: " << entity->getKey());

  // Is it a slave? If so, reference to it
  if(dynamic_cast<const Constraint*>(entity) == 0){
    for(std::vector<TokenId>::iterator it = m_slaves.begin(); it != m_slaves.end(); ++it){
      TokenId token = *it;
      checkError(token.isValid(), token);
      if(token->getKey() == entity->getKey()){
        m_slaves.erase(it);
        return;
      }
    }

    return;
  }

  // Is it the guard listener
  if(m_guardListener.isId() && entity->getKey() == m_guardListener->getKey()){
    m_guardListener = ConstraintId::noId();
    m_guards.clear();
    return;
  }

  // If neither of the above, it must be a regular constraint
  for(std::vector<ConstraintId>::iterator it = m_constraints.begin(); it != m_constraints.end(); ++it){
    ConstraintId constr = *it;
    checkError(constr.isValid(), constr);
    if(constr->getKey() == entity->getKey()){
      m_constraints.erase(it);
      return;
    }
  }
}

bool RuleInstance::connectedToToken(const ConstraintId constr,
                                    const TokenId token) const {
  // If the constrant is actually a rule variable listener then it is part of the context of the rule instance
  // and thus part of the context of the token
  if(RuleVariableListenerId::convertable(constr))
    return true;

  for(std::vector<ConstrainedVariableId>::const_iterator it = constr->getScope().begin();
      it != constr->getScope().end(); ++it){
    ConstrainedVariableId var = *it;
    EntityId parent = var->parent();
    if(parent == token || parent == m_id)
      return true;
  }

  return false;
}

std::string RuleInstance::ruleExecutionContext() const {
  static const std::string TAB_DELIMITER("    ");
  std::stringstream ss;

  // What is the token
  ss << "[" << getToken()->getKey() << "]Rule fired on master token: " <<
      getToken()->toString() << ". The rule instance context is given below:" << std::endl << std::endl;

  // What rule
  ss << "Rule: " << getRule()->toString() << std::endl << std::endl;

  // What guards are involved
  if(m_guards.empty())
    ss << "No Guards" << std::endl;
  else {
    ss << "Guards:" << std::endl;

    for(std::vector<ConstrainedVariableId>::const_iterator it = m_guards.begin(); it != m_guards.end(); ++it){
      ConstrainedVariableId guard = *it;
      ss << TAB_DELIMITER << guard->getName() << " == " << guard->lastDomain().toString() << std::endl;
    }
  }

  ss << std::endl;

  // What slaves are created
  if(m_slaves.empty())
    ss << "No Slaves" << std::endl;
  else {
    ss << "Slaves: " << std::endl;
    for(std::map<std::string, TokenId>::const_iterator it = m_slavesByName.begin(); it != m_slavesByName.end(); ++it){
      std::string name(it->first);
      TokenId token = it->second;
      ss << TAB_DELIMITER << name << "==" << token->toString() << std::endl;
    }
  }

  ss << "++++++++++++++++++x+++++++";
  return ss.str();
}

  bool RuleInstance::hasEmptyGuard() const {
    if(m_guards.empty())
      return false;
    for(std::vector<ConstrainedVariableId>::const_iterator it = m_guards.begin(); 
        it != m_guards.end(); ++it)
      if((*it)->lastDomain().isEmpty())
        return true;
    return false;
  }

}
