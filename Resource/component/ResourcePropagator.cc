#include "ResourcePropagator.hh"
#include "Resource.hh"
#include "ResourceConstraint.hh"
#include "ConstraintEngineDefs.hh"
#include "ConstraintEngine.hh"
#include "ConstrainedVariable.hh"
#include "Variable.hh"
#include "Constraint.hh"


namespace Prototype {

  ResourcePropagator::ResourcePropagator(const LabelStr& name, const ConstraintEngineId& constraintEngine)
    : Propagator(name, constraintEngine){}


  void ResourcePropagator::handleNotification(const ConstrainedVariableId& variable, 
					     int argIndex, 
					     const ConstraintId& constraint, 
					     const DomainListener::ChangeType& changeType){

    check_error(variable->getParent()->getName() == LabelStr("Resource.change"));

    //handle change of variables
    switch(argIndex) {
    case ResourceConstraint::OBJECT: 
      handleObjectChange(variable, changeType);
      break;
    case ResourceConstraint::TIME: 
      handleTimeChange(variable, argIndex, constraint, changeType);
      break;    
    case ResourceConstraint::USAGE: 
      handleQuantityChange(variable, argIndex, constraint, changeType);
      break;
    }
    //buffer for propagation if necessary
    handleResourcePropagation(constraint);
  }

  void ResourcePropagator::execute(){
    check_error(m_resources.size() > 0);
    check_error(!getConstraintEngine()->provenInconsistent());

    while(!m_resources.empty()){
      std::set<ResourceId>::iterator it = m_resources.begin();
      (*it)->updateTransactionProfile();
      if ((*it)->isViolated()) {
	ResourceConstraint::getCurrentDomain(m_forempty).empty();
	m_resources.clear();
	break;
      }
      m_resources.erase(it);      
    }
  }

  bool ResourcePropagator::updateRequired() const{
    return (m_resources.size() > 0);
  }


  void ResourcePropagator::handleResourcePropagation(const ConstraintId constraint) {
    check_error(ResourceConstraintId::convertable(constraint));
    ConstrainedVariableId objectvar = constraint->getScope().front();
    //delay propagation of resource until the transaction has been assigned to a resource
    if (objectvar->specifiedDomain().isSingleton()) {
      ResourceId r = ResourceConstraint::getCurrentDomain(objectvar).getSingletonValue();
      // Buffer this resource for propagation
      if (m_resources.find(r) == m_resources.end())
	m_resources.insert(r);
      //store a variable so that its domain can be emptied
      m_forempty = objectvar;
    }
  }

  void ResourcePropagator::handleObjectChange(const ConstrainedVariableId& variable, const DomainListener::ChangeType& changeType) {
    if (variable->specifiedDomain().isSingleton()) {
      TransactionId t = variable->getParent();
      //if the spec domain is a singleton and this transaction has not yet been assigned to a resource, then assign it.
      if(t->getResource() == ResourceId::noId()) {
	ResourceId r = ResourceConstraint::getCurrentDomain(variable).getSingletonValue();	
	  r->insert(t);
	}
      }
      //else if the spec domain is no longer a singleton and this transaction is still assigned this resource, then remove it. 
      else if (!variable->specifiedDomain().isSingleton() && (changeType == DomainListener::RESET) ) {
	TransactionId t = variable->getParent();
	if ( t->getResource() != ResourceId::noId()) {
	  ResourceId r = t->getResource();
	  r->remove(t);
	}
      }
  }
  void ResourcePropagator::handleQuantityChange(const ConstrainedVariableId& variable, 
						int argIndex, 
						const ConstraintId& constraint, 
						const DomainListener::ChangeType& changeType) {
    check_error(ResourceConstraintId::convertable(constraint));
    ConstrainedVariableId objectvar = constraint->getScope().front();
    if (TransactionId::convertable(variable->getParent())) {    
      TransactionId t = variable->getParent();
      if (t->getResource() != ResourceId::noId()){
	ResourceId r = t->getResource();
	r->notifyQuantityChanged(t);
      } 
      t->notifyChanged();
    }
  }

  void ResourcePropagator::handleTimeChange(const ConstrainedVariableId& variable, 
					    int argIndex, 
					    const ConstraintId& constraint, 
					    const DomainListener::ChangeType& changeType){

    check_error(ResourceConstraintId::convertable(constraint));
    if (TransactionId::convertable(variable->getParent())) {    
      ConstrainedVariableId objectvar = constraint->getScope().front();
      TransactionId t = variable->getParent();
      if (t->getResource() != ResourceId::noId()){
	ResourceId r = t->getResource();
	if (changeType == DomainListener::RELAXED)
	  r->notifyTimeRelaxed(t);
	else
	  r->notifyTimeRestricted(t);
      }
      t->notifyChanged();
    }
  }

}
