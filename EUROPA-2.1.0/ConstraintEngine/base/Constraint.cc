#include "Constraint.hh"
#include "ConstraintEngine.hh"
#include "ConstrainedVariable.hh"
#include "Propagator.hh"
#include "AbstractDomain.hh"
#include "Debug.hh"
#include "Error.hh"
#include "LockManager.hh"

#include <sstream>

namespace EUROPA {

  Constraint::Constraint(const LabelStr& name,
			 const LabelStr& propagatorName,
			 const ConstraintEngineId& constraintEngine, 
			 const std::vector<ConstrainedVariableId>& variables)
    : Entity(), m_name(name), m_constraintEngine(constraintEngine),
      m_variables(variables), m_id(this), m_isUnary(true), 
      m_createdBy(LockManager::instance().getCurrentUser()),
      m_deactivationRefCount(0),
      m_isRedundant(false){
    check_error(m_constraintEngine.isValid());
    check_error(!m_variables.empty());
    m_constraintEngine->add(m_id, propagatorName);

    debugMsg("Constraint:Constraint", 
	     "Creating constraint " << getKey() << ":" << name.toString() <<
	     " registered with propagator " << propagatorName.toString() << " with arity " << variables.size());

    check_error(isValid());

    int variableCount = 0;
    for (unsigned int i = 0; i < m_variables.size(); i++) {

      // To support determining if it is in fact a Unary, we test for the possible
      // volatility of a variable. Extend this when locking comes on stream!
      if(!m_variables[i]->baseDomain().isSingleton())
	variableCount++;

      if(!m_variables[i]->isActive())
	m_deactivationRefCount++;

      checkError(m_variables[i].isValid(), 
		 "The argIndex " << i << " is not a valid variable for constraint " << name.toString());

      // It is important that the call to add the constraint is only made after the deactivation reference
      // count has been adjusted since the former effects the active status of the constraint and that may be used by
      // a handler in figuring how to treat the constraint
      m_variables[i]->addConstraint(m_id, i);
    }

    if(variableCount > 1)
      m_isUnary = false;

    // Update if redundant immediately
    notifyBaseDomainRestricted(m_variables[0]);

    if(!isActive()){
      m_constraintEngine->notifyDeactivated(m_id);
      handleDeactivate();
    }
  }

  Constraint::~Constraint(){
    debugMsg("Constraint:~Constraint", Entity::toString() << " Id=" << m_id);
    discard(false);
    m_id.remove();
  }

  void Constraint::handleDiscard(){
    debugMsg("Constraint:handleDiscard", getName().toString() << "(" << getKey() << ") " << m_id);

    if(!Entity::isPurging()){
      check_error(isValid());
      for(unsigned int i=0;i<m_variables.size();i++)
	m_variables[i]->removeConstraint(m_id, i);
    }

    m_constraintEngine->remove(m_id);

    Entity::handleDiscard();
  }

  double Constraint::getViolation() const {
    // TODO: each constraint must eventually know whether it is being violated and it must know how to compute its 
    // penalty value
    if (m_constraintEngine->isViolated(getId()))
        return 1.0;
    else
        return 0.0;
  }

  std::string Constraint::getViolationExpl() const
  {
  	// TODO: this must eventually come from the model
  	std::ostringstream os;

    os << getName().toString() << "(";  	
    for(unsigned int i=0;i<m_variables.size();i++) {
    	if (i > 0)
    	  os << ",";
    	std::string name = m_variables[i]->getName().toString();
    	if (name.substr(0,4) != "$VAR")
	        os << name;
    	else
    		os << m_variables[i]->lastDomain().toString();
    }
    os << ")";
    
  	return os.str();
  }

  const ConstraintId& Constraint::getId() const {
    return m_id;
  }

  const LabelStr& Constraint::getName() const {return m_name;}

  const PropagatorId& Constraint::getPropagator() const {return m_propagator;}

  const std::vector<ConstrainedVariableId>& Constraint::getScope() const {return m_variables;}

  const LabelStr& Constraint::getCreatedBy() const {return m_createdBy;}

  void Constraint::setPropagator(const PropagatorId& propagator){
    check_error(m_propagator.isNoId());
    check_error(propagator->getConstraintEngine() == m_constraintEngine);

    m_propagator = propagator;
  }

  bool Constraint::isVariableOf(const ConstrainedVariableId& variable){
    std::vector<ConstrainedVariableId>::iterator it = m_variables.begin();
    while(it !=  m_variables.end()){
      if (*it == variable)
	return true;
      ++it;
    }
    return false;
  }

  void Constraint::execute()
  {
   for(unsigned int i=0;i<m_variables.size();i++)
    	m_variables[i]->setCurrentPropagatingConstraint(m_id);
				 	
    handleExecute();
    
    for(unsigned int i=0;i<m_variables.size();i++)
    	m_variables[i]->setCurrentPropagatingConstraint(ConstraintId::noId());   
  }
  
  void Constraint::execute(const ConstrainedVariableId& variable, 
				 int argIndex, 
				 const DomainListener::ChangeType& changeType) {   

   for(unsigned int i=0;i<m_variables.size();i++)
    	m_variables[i]->setCurrentPropagatingConstraint(m_id);
				 	
    handleExecute(variable,argIndex,changeType);

    for(unsigned int i=0;i<m_variables.size();i++)
    	m_variables[i]->setCurrentPropagatingConstraint(ConstraintId::noId());   
  }
  
  void Constraint::handleExecute(const ConstrainedVariableId& variable, 
				 int argIndex, 
				 const DomainListener::ChangeType& changeType) {   
    handleExecute();
  }

  bool Constraint::canIgnore(const ConstrainedVariableId& variable, 
			     int argIndex, 
			     const DomainListener::ChangeType& changeType) {
    return false;
  }

  const std::vector<ConstrainedVariableId>& Constraint::getModifiedVariables(const ConstrainedVariableId& variable) const {
    return getScope();
  }

  /**
   * @todo Figure a way to propagate first and deactivate only after safe
   * propagation.
   */
  void Constraint::notifyBaseDomainRestricted(const ConstrainedVariableId& var) {
    debugMsg("Constraint:notifyBaseDomainRestricted",
	     "Base domain of " << var->toString() << " restricted in " << m_id->toString());

    // If already redundant, nothing to do
    if(m_isRedundant)
      return;

    // Update
    m_isRedundant = testIsRedundant(var);

    // In the event of a transition, deactivate yourself
    if(m_isRedundant)
      m_constraintEngine->notifyRedundant(m_id);
  }

  /**
   * @brief Passed the variable to allow a quick exit test
   */
  bool Constraint::testIsRedundant(const ConstrainedVariableId& var) const {

    if(var.isId() && (!var->baseDomain().isSingleton() || var->baseDomain().isOpen()))
      return false;

    for(std::vector<ConstrainedVariableId>::const_iterator it = m_variables.begin(); it != m_variables.end(); ++it){
      ConstrainedVariableId v = *it;
      if(!v->baseDomain().isSingleton()){
	debugMsg("Constraint:testIsRedundant", v->toString() << " has base domain of " << v->baseDomain());
	return false;
      }
    }

    // If we still think it is redundant, now we invoke execute. 
    debugMsg("Constraint:testIsRedundant",
	     " constraint " << this->toString() << " is redundant");
    return true;
  }

  AbstractDomain& Constraint::getCurrentDomain(const ConstrainedVariableId& var) {
    check_error(var.isValid());

    return var->getCurrentDomain();
  }

  bool Constraint::isValid() const {
    std::set<ConstrainedVariableId> vars;
    for (std::vector<ConstrainedVariableId>::const_iterator it = m_variables.begin(); it != m_variables.end(); ++it) {
      vars.insert(*it);
    }

    return(!m_variables.empty() &&
           m_propagator.isValid() &&
           m_constraintEngine.isValid());
  }

  void Constraint::deactivate() {
    check_error(!Entity::isPurging());
    m_deactivationRefCount++;

    debugMsg("Constraint:deactivation", 
	     "RefCount: [" << m_deactivationRefCount << "]" << toString());

    // If this is a transition, handle it
    if(m_deactivationRefCount == 1){
      m_constraintEngine->notifyDeactivated(m_id);
      handleDeactivate();
    }
  }

  void Constraint::undoDeactivation(){
    check_error(!Entity::isPurging());

    // Prevent deactivation if redundant
    if(isRedundant())
      return;

    m_deactivationRefCount--;

    debugMsg("Constraint:undoDeactivation", 
	     "RefCount: [" << m_deactivationRefCount << "]" << toString());

    // If this is a transition, handle it
    if(isActive()){
      m_constraintEngine->notifyActivated(m_id);
      handleActivate();
    }
  }

  unsigned int Constraint::deactivationCount() const {return m_deactivationRefCount;}

  void Constraint::notifyViolated()
  {
	  m_propagator->getConstraintEngine()->getViolationMgr().addViolatedConstraint(m_id);
  }
  
  void Constraint::notifyNoLongerViolated()
  {
	  m_propagator->getConstraintEngine()->getViolationMgr().removeViolatedConstraint(m_id);
  }  
  
  std::vector<ConstrainedVariableId>& makeScope(const ConstrainedVariableId& arg1){
    static std::vector<ConstrainedVariableId> sl_scope;
    check_error(arg1.isValid());

    sl_scope.clear();
    sl_scope.push_back(arg1);
    return sl_scope;
  }

  std::vector<ConstrainedVariableId>& makeScope(const ConstrainedVariableId& arg1,
						const ConstrainedVariableId& arg2){
    check_error(arg1.isValid());
    check_error(arg2.isValid());

    std::vector<ConstrainedVariableId>& scope =  makeScope(arg1);
    scope.push_back(arg2);
    return scope;
  }

  std::vector<ConstrainedVariableId>& makeScope(const ConstrainedVariableId& arg1,
					       const ConstrainedVariableId& arg2,
					       const ConstrainedVariableId& arg3){
    check_error(arg3.isValid());
    std::vector<ConstrainedVariableId>& scope =  makeScope(arg1, arg2);
    scope.push_back(arg3);
    return scope;
  }

  std::vector<ConstrainedVariableId>& makeScope(const ConstrainedVariableId& arg1,
					       const ConstrainedVariableId& arg2,
					       const ConstrainedVariableId& arg3,
					       const ConstrainedVariableId& arg4){
    check_error(arg4.isValid());
    std::vector<ConstrainedVariableId>& scope =  makeScope(arg1, arg2, arg3);
    scope.push_back(arg4);
    return scope;
  }

  std::vector<ConstrainedVariableId>& makeScope(const ConstrainedVariableId& arg1,
					       const ConstrainedVariableId& arg2,
					       const ConstrainedVariableId& arg3,
					       const ConstrainedVariableId& arg4,
					       const ConstrainedVariableId& arg5){
    check_error(arg5.isValid());
    std::vector<ConstrainedVariableId>& scope =  makeScope(arg1, arg2, arg3, arg4);
    scope.push_back(arg5);
    return scope;
  }

  std::vector<ConstrainedVariableId>& makeScope(const ConstrainedVariableId& arg1,
					       const ConstrainedVariableId& arg2,
					       const ConstrainedVariableId& arg3,
					       const ConstrainedVariableId& arg4,
					       const ConstrainedVariableId& arg5,
					       const ConstrainedVariableId& arg6){
    check_error(arg6.isValid());
    std::vector<ConstrainedVariableId>& scope =  makeScope(arg1, arg2, arg3, arg4, arg5);
    scope.push_back(arg6);
    return scope;
  }

  std::vector<ConstrainedVariableId>& makeScope(const ConstrainedVariableId& arg1,
					       const ConstrainedVariableId& arg2,
					       const ConstrainedVariableId& arg3,
					       const ConstrainedVariableId& arg4,
					       const ConstrainedVariableId& arg5,
					       const ConstrainedVariableId& arg6,
					       const ConstrainedVariableId& arg7){
    check_error(arg7.isValid());
    std::vector<ConstrainedVariableId>& scope =  makeScope(arg1, arg2, arg3, arg4, arg5, arg6);
    scope.push_back(arg7);
    return scope;
  }

  std::vector<ConstrainedVariableId>& makeScope(const ConstrainedVariableId& arg1,
					       const ConstrainedVariableId& arg2,
					       const ConstrainedVariableId& arg3,
					       const ConstrainedVariableId& arg4,
					       const ConstrainedVariableId& arg5,
					       const ConstrainedVariableId& arg6,
					       const ConstrainedVariableId& arg7,
					       const ConstrainedVariableId& arg8){
    check_error(arg8.isValid());
    std::vector<ConstrainedVariableId>& scope =  makeScope(arg1, arg2, arg3, arg4, arg5, arg6, arg7);
    scope.push_back(arg8);
    return scope;
  }

  std::string Constraint::toString() const{
    std::stringstream sstr;
    sstr << Entity::toString() << std::endl;
    unsigned int i = 0;
    for(std::vector<ConstrainedVariableId>::const_iterator it = getScope().begin();
	it != getScope().end(); ++it){
      ConstrainedVariableId var = *it;
      check_error(var.isValid(), "Invalid argument in constraint");
      sstr << " ARG[" << i++ << "]:" << var->toString() << std::endl;
    }

    return sstr.str();
  }
}
