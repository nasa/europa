#include "ResourcePropagator.hh"
#include "Resource.hh"
#include "ResourceConstraint.hh"
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
    check_error(ResourceConstraintId::convertable(constraint));

    TransactionId t(variable->getParent());
    if (t->getObject()->getDerivedDomain().isSingleton()) {	
      ResourceId r = t->getObject()->getDerivedDomain().getSingletonValue();

      // If this transaction has not been assigned to a resource already, the assign it.
      if (t->getResource() == ResourceId::noId())
	r->insert(t);

      // Buffer this resource for propagation
      if (m_resources.find(r) == m_resources.end())
	m_resources.insert(r);
    }
  }

  void ResourcePropagator::execute(){
    check_error(m_resources.size() > 0);
    check_error(!getConstraintEngine()->provenInconsistent());

    while(!m_resources.empty()){
      std::set<ResourceId>::iterator it = m_resources.begin();
      std::cout << "AM CALLING UPDATE PROFILE!!!!\n";
      (*it)->updateTransactionProfile();
      m_resources.erase(it);
    }
  }

  bool ResourcePropagator::updateRequired() const{
    return (m_resources.size() > 0);
  }
}
