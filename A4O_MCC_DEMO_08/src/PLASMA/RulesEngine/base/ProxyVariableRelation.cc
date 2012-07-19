#include "ProxyVariableRelation.hh"
#include "Error.hh"

namespace EUROPA {

  ProxyVariableRelation::ProxyVariableRelation(const ConstrainedVariableId& objectVar,
					       const ConstrainedVariableId& proxyVar,
					       const std::vector<unsigned int>& path)
    : Constraint("proxyRelation", "Default", objectVar->getConstraintEngine(), makeScope(objectVar, proxyVar)),
      m_objectDomain(static_cast<ObjectDomain&>(getCurrentDomain(objectVar))),
      m_proxyDomain(static_cast<EnumeratedDomain&>(getCurrentDomain(proxyVar))),
      m_path(path),
      m_sourceConstraintKey(0){
    checkError(getScope().size() == ARG_COUNT, toString());
  }

  ProxyVariableRelation::ProxyVariableRelation(const LabelStr& name,
					       const LabelStr& propagatorName,
					       const ConstraintEngineId& constraintEngine,
					       const std::vector<ConstrainedVariableId>& variables)
    : Constraint(name, propagatorName, constraintEngine, variables),
      m_objectDomain(static_cast<ObjectDomain&>(getCurrentDomain(variables[0]))),
      m_proxyDomain(static_cast<EnumeratedDomain&>(getCurrentDomain(variables[1]))){
    checkError(getScope().size() == ARG_COUNT, toString());
  }

  void ProxyVariableRelation::handleExecute(){
    // Update path rfom source constraint if necessary. Lazy evaluation pattern
    updatePathFromSource();

    // Handle case of closure from the object
    if(m_objectDomain.isClosed() && m_proxyDomain.isOpen())
      m_proxyDomain.close();

    // Handle case of closure from the proxy
    if(m_proxyDomain.isClosed() && m_objectDomain.isOpen())
      m_objectDomain.close();

    // First prune the objects againts the proxy values
    EnumeratedDomain remainingValues(m_proxyDomain.isNumeric(), m_proxyDomain.getTypeName().c_str());

    const std::set<double>& objects = m_objectDomain.getValues();
    ObjectDomain remainingObjects(m_objectDomain.getTypeName().c_str());
    for(std::set<double>::const_iterator it = objects.begin(); it != objects.end(); ++it){
      ObjectId object = *it;
      ConstrainedVariableId var = object->getVariable(m_path);
      double value = var->lastDomain().getSingletonValue();
      if(m_proxyDomain.isMember(value)){
	remainingValues.insert(value);
	remainingObjects.insert(object);
      }
    }

    if(m_objectDomain.isClosed()){
      remainingObjects.close();
      remainingValues.close();
    }

    // Enforce restriction on object variable
    if(m_objectDomain.intersect(remainingObjects) && m_objectDomain.isEmpty())
      return;

    // Enforce restriction on proxy variable
    m_proxyDomain.intersect(remainingValues);
  }


  bool ProxyVariableRelation::canIgnore(const ConstrainedVariableId& variable, 
					int argIndex, 
					const DomainListener::ChangeType& changeType){
    // If the object variable is specified to a singleton, and the proxy variable can be specified, then
    // we will force the proxy to be set to the field value of the given object
    if(argIndex == 0 && changeType == DomainListener::SET_TO_SINGLETON && 
       getScope()[1]->canBeSpecified() &&
       !getScope()[1]->isSpecified()){
      ObjectId object = variable->getSpecifiedValue();
      ConstrainedVariableId fieldVar = object->getVariable(m_path);
      checkError(fieldVar->isSpecified(), fieldVar->toString());
      m_autoSpecified = true;
      getScope()[1]->specify(fieldVar->getSpecifiedValue());
      return true;
    }

    // If the object variable is RESET and we had automatically set it, then we should reset it.
    if(argIndex == 0 && changeType == DomainListener::RESET && m_autoSpecified){
      getScope()[1]->reset();
      m_autoSpecified = false;
      return true;
    }

    // If the proxy variable is RESET, but the object variable remains specified, re-specify the proxy
    if(argIndex == 1 && changeType == DomainListener::RESET && getScope()[0]->isSpecified()){
      ObjectId object = getScope()[0]->getSpecifiedValue();
      ConstrainedVariableId fieldVar = object->getVariable(m_path);
      checkError(fieldVar->isSpecified(), fieldVar->toString());
      m_autoSpecified = true;
      getScope()[1]->specify(fieldVar->getSpecifiedValue());
      return true;
    }

    // Otherwise we cannot ignore
    return false;
  }

  void ProxyVariableRelation::setSource(const ConstraintId& sourceConstraint){
    checkError(sourceConstraint->getName() == LabelStr("proxyRelation"), sourceConstraint->toString());
    checkError(m_path.empty(), "Should be empty when setting up the source from " << sourceConstraint->toString());
    m_sourceConstraint = sourceConstraint;
  }

  /**
   * In the lifetime of a constraint, this should only be called at most once. It is used to lazily copy a source path under conditions
   * of migration. The reason for this is that we cannot guarantee the source constraint is fully constructed when the copy is
   * invoked.
   * @see MergeMemento
   * @see Constraint::handleAddition
   */
  void ProxyVariableRelation::updatePathFromSource(){
    if(m_sourceConstraint.isId() && m_sourceConstraintKey != (signed int) m_sourceConstraint->getKey()){
      ProxyVariableRelation* proxyConstraint = (ProxyVariableRelation*) m_sourceConstraint;
      m_path = proxyConstraint->m_path;
      m_sourceConstraint = ConstraintId::noId();
    }
  }
}
