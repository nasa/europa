#include "ResourcePropagator.hh"
#include "Resource.hh"
#include "ResourceConstraint.hh"
#include "ConstraintEngineDefs.hh"
#include "ConstraintEngine.hh"
#include "ConstrainedVariable.hh"
#include "Constraint.hh"


namespace Prototype {

  ResourcePropagator::ResourcePropagator(const LabelStr& name, const ConstraintEngineId& constraintEngine)
    : Propagator(name, constraintEngine){}


  void ResourcePropagator::handleNotification(const ConstrainedVariableId& variable, 
					     int argIndex, 
					     const ConstraintId& constraint, 
					     const DomainListener::ChangeType& changeType){

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

  bool ResourcePropagator::checkResourcePropagationRequired(const ConstrainedVariableId& variable) const {
    check_error(TransactionId::convertable(variable->getParent()));
    TransactionId t(variable->getParent());    
    return(ResourceConstraint::getCurrentDomain(t->getObject()).isSingleton());
  }

  void ResourcePropagator::handleResourcePropagation(const ConstraintId constraint) {
    check_error(ResourceConstraintId::convertable(constraint));
    ConstrainedVariableId variable = constraint->getScope().front();
    if (checkResourcePropagationRequired(variable)) {
      TransactionId t(variable->getParent());
      ResourceId r = ResourceConstraint::getCurrentDomain(t->getObject()).getSingletonValue();

      // Buffer this resource for propagation
      if (m_resources.find(r) == m_resources.end())
	m_resources.insert(r);

      //store a variable so that its domain can be emptied
      m_forempty = variable;
    }
  }

}
