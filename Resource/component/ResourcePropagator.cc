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
    //std::cout << "RP: argindex " << argIndex << std::endl;
    //handle change of variables
    switch(argIndex) {
    case ResourceConstraint::OBJECT: 
      handleObjectChange(variable);
      break;
    case ResourceConstraint::TIME: 
      handleTimeChange(variable, argIndex, constraint, changeType);
      break;    
    case ResourceConstraint::USAGE: 
      handleQuantityChange(variable, argIndex, constraint, changeType);
      break;
    }
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


  void ResourcePropagator::handleResourcePropagation(const ResourceId& r, const ConstrainedVariableId& variable) {
    // Buffer this resource for propagation
    if (m_resources.find(r) == m_resources.end()) {
      m_resources.insert(r);
      //store a variable so that its domain can be emptied
      m_forempty = variable;
    }
  }

  void ResourcePropagator::handleObjectChange(const ConstrainedVariableId& variable) {
    if (TransactionId::convertable(variable->getParent())) {
      TransactionId t = variable->getParent();
      if ( t->getResource() != ResourceId::noId()) {
	ResourceId r = t->getResource();
	//buffer for propagation if necessary
	handleResourcePropagation(r, variable);
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
	//buffer for propagation if necessary
	handleResourcePropagation(r, variable);
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
	//buffer for propagation if necessary
	handleResourcePropagation(r, variable);
      }
      t->notifyChanged();
    }
  }

}
