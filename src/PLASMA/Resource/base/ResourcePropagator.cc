/**
 * @file ResourcePropagator.cc
 * @author Conor McGann
 * @date 2005
 * @brief Primarily provides the 'execute' behavior
 */

#include "ResourcePropagator.hh"
#include "Resource.hh"
#include "ResourceConstraint.hh"
#include "ConstraintEngineDefs.hh"
#include "ConstraintEngine.hh"
#include "ConstrainedVariable.hh"
#include "Variable.hh"
#include "Constraint.hh"
namespace EUROPA {

  ResourcePropagator::ResourcePropagator(const LabelStr& name,
					 const ConstraintEngineId& constraintEngine,
					 const PlanDatabaseId& planDatabase)
    : DefaultPropagator(name, constraintEngine), m_planDb(planDatabase)
  {
  }

  /**
   * This method over-rides that of the base class. The agenda is managed automatically by the
   * base class. Here we simply use it as the mechanism to trigger a constraint check on modified
   * resources.
   */
  void ResourcePropagator::execute(){
    // Invoke default propagation to apply constraints on the agenda. They will propagate to the resources
    // that require updates
    if(!m_agenda.empty())
      DefaultPropagator::execute();

    std::list<ResourceId> resources = getDirtyResources();

    // Finally, iterate over all the resources we have discovered which require propagation
    for (std::list<ResourceId>::const_iterator it = resources.begin(); it != resources.end(); ++it){
      ResourceId r = *it;
      if (r->isViolated()) { // Find a variable and empty it. Should be once connected to resource constraints.
	TokenId tx = *(r->tokens().begin());
	check_error(tx.isValid());
	ConstrainedVariableId varToEmpty = tx->getObject();
	ResourceConstraint::getCurrentDomain(varToEmpty).empty();
	break;
      }
    }
  }

  const LabelStr& ResourcePropagator::resourceString() {
    static const LabelStr sl_resourceString("Resource");
    return sl_resourceString;
  }

  // This check can't be made when the propagator is created, since the Schema may not have been populated by then
  bool ResourcePropagator::isEnabled() const
  {
	  return m_planDb->getSchema()->isObjectType(resourceString());
  }

  bool ResourcePropagator::updateRequired() const {
    if(!isEnabled())
      return false;
    else
      return (DefaultPropagator::updateRequired() || !getDirtyResources().empty());
  }

  const std::list<ResourceId> ResourcePropagator::getDirtyResources() const {
    std::list<ResourceId> allResources;
    std::list<ResourceId> dirtyResources;

    m_planDb->getObjectsByType(resourceString(), allResources);

    // Finally, iterate over all the resources we have discovered which require propagation
    for (std::list<ResourceId>::const_iterator it = allResources.begin(); it != allResources.end(); ++it){
      ResourceId r = *it;
      if(r->isDirty())
	dirtyResources.push_back(r);
    }

    return dirtyResources;
  }
}
