#include "ResourcePropagator.hh"
#include "Resource.hh"
#include "ResourceConstraint.hh"
#include "ConstraintEngineDefs.hh"
#include "ConstraintEngine.hh"
#include "ConstrainedVariable.hh"
#include "Variable.hh"
#include "Constraint.hh"
#include "PlanDatabaseListener.hh"

namespace Prototype {

  namespace ResourceProp {
    DbResourceConnector::DbResourceConnector(const PlanDatabaseId& planDatabase, const ResourcePropagatorId& resourcePropagator)
      : PlanDatabaseListener(planDatabase), m_resourcePropagator(resourcePropagator){}

    void DbResourceConnector::notifyConstrained(const ObjectId& object, const TokenId& token, const TokenId& successor) {
      if (TransactionId::convertable(token)) {
	check_error(ResourceId::convertable(object));
	m_resourcePropagator->handleResourcePropagation(ResourceId(object), token->getObject());
      }
    }
  }

  using namespace ResourceProp;
		      

  ResourcePropagator::ResourcePropagator(const LabelStr& name, const ConstraintEngineId& constraintEngine, const PlanDatabaseId& planDatabase)
    : Propagator(name, constraintEngine), m_planDb(planDatabase){
    m_planDbListener = (new DbResourceConnector(m_planDb, getId()))->getId();
  }

  ResourcePropagator::~ResourcePropagator() {
  }

  void ResourcePropagator::handleNotification(const ConstrainedVariableId& variable, 
					     int argIndex, 
					     const ConstraintId& constraint, 
					     const DomainListener::ChangeType& changeType){

    check_error(variable->getParent()->getName() == LabelStr("Resource.change"));
    //std::cout << "RP: argindex " << argIndex << std::endl;
    //handle change of variables

    if(variable->lastDomain().isEmpty())
      return;

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

    for (std::set<ResourceId>::const_iterator it = m_resources.begin(); it != m_resources.end(); ++it){
      ResourceId r = *it;
      r->updateTransactionProfile();
      if (r->isViolated()) { // Find a variable and empty it. Should be once connected to resource constraints.
	TokenId tx = *(r->getTokens().begin());
	check_error(tx.isValid());
	ConstrainedVariableId varToEmpty = tx->getObject();
	ResourceConstraint::getCurrentDomain(varToEmpty).empty();
	break;
      }  
    }
    m_resources.clear();
  }

  bool ResourcePropagator::updateRequired() const{
    return (!m_resources.empty());
  }


  void ResourcePropagator::handleResourcePropagation(const ResourceId& r, const ConstrainedVariableId& variable) {
    // Buffer this resource for propagation
    check_error(variable.isValid());
    if (m_resources.find(r) == m_resources.end()) {
      m_resources.insert(r);
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
    // ConstrainedVariableId objectvar = constraint->getScope().front();
    if (TransactionId::convertable(variable->getParent())) {    
      TransactionId t = variable->getParent();
      if (t->getResource() != ResourceId::noId()){
	ResourceId r = t->getResource();
	r->notifyQuantityChanged(t);
	//buffer for propagation if necessary
	handleResourcePropagation(r, variable);
      }
    }
  }

  void ResourcePropagator::handleTimeChange(const ConstrainedVariableId& variable, 
					    int argIndex, 
					    const ConstraintId& constraint, 
					    const DomainListener::ChangeType& changeType){

    check_error(ResourceConstraintId::convertable(constraint));
    if (TransactionId::convertable(variable->getParent())) {    
      // ConstrainedVariableId objectvar = constraint->getScope().front();
      TransactionId t = variable->getParent();
      check_error(t.isValid());
      if (t->getResource() != ResourceId::noId()){
	ResourceId r = t->getResource();
	check_error(r.isValid());
	if (changeType == DomainListener::RELAXED)
	  r->notifyTimeRelaxed(t);
	else
	  r->notifyTimeRestricted(t);
	//buffer for propagation if necessary
	handleResourcePropagation(r, variable);
      }
    }
  }

}
